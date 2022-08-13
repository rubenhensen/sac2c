/******************************************************************************
 *
 * file:   loop_invariant_removal.c
 *
 * prefix: DLIR
 *
 * description:
 *   this module implements loop invariant removal on code in ssa form. loop
 *   invariant assignments in do (LIR) are moved out of
 *   the loops. because the implemented algorithm supports no while loops all
 *   while loops are transformed into do loops before. this happens in
 *   the While2Do() transformation called in optimize.c
 *
 * details:
 *   do-loop-invariant removal: an assignment that depends only on constants
 *   and other loop-invariant expressions is itself loop invariant and can be
 *   moved up before the do loop.
 *
 *   If we find an assignment in the loop body that is only referenced in the
 *   else-conditional part of the loop, this assignment can be moved down
 *   behind the loop. [I think the rationale here is that, because
 *   the loop is recursive, an assignment in the ELSE part only affects
 *   the state of affairs after the loop.]
 *
 *   If an assignment can be moved up and down at the same
 *   time it is moved up, because this leads to fewer arguments/results.
 *   We have to analyse only one loop because one special fundef can contain
 *   only one loop.
 *
 *   To find the loop invariant args, SSAInferLoopInvariants() checks all
 *   arg arguments that are used unchanged in the recursive loop function call.
 *   All loop-invariant args are marked as AVIS_SSALPINV(avis) == TRUE.
 *
 *   This information is used in other optimization phases, too, e.g. constant
 *   folding in SSACF, copy propagation in SSACSE, loop-unrolling analysis
 *   in SSALUR.
 *
 *   example: do-loop invariant removal
 *
 *   ...                            lir_y = expr(a,b,5);
 *   x = f_do(a,b,c,d,e)        --> lir_x,lir_c,lir_g = f_do(a,b,c,d,e,lir_y);
 *   ...                            x = expr2(lir_c, lir_g);
 *
 *   int f_do(a,b,c,d,e)            int,... f_do(a,b,c,d,e,y)
 *   {                              {
 *     ...                            ...
 *     y = expr(a,b,5);               <removed>
 *     z = expr2(c,g);                <removed>
 *     if(cond) {                     if(cond) {
 *       ... = f_do(a,b,c,d',e');       .. = d_do(a,b,c,d',e',y);
 *     } else {                       } else {
 *       ...                            ...
 *       z2  = z;                       c2 = c;
 *       ...                            g2 = g;
 *     }                              }
 *     return(z2);                    return(z2, c2, g2);
 *   }                              }
 *
 *
 * Implementation notes:
 *
 *   Because loop invariant removal is a quite difficult task, we need two
 *   traversals to implement it. A first traversal checks expressions
 *   that are do-loop invariant and marks them; it also marks local identifiers,
 *   e.g. used in withloops.
 *
 *   the do-loop-invariant marking allows three tags:
 *
 *   move_up: an expression can be moved up in front of the do-loop, because it
 *               depends only on loop invariant expressions.
 *
 *   move_down: an expression can be moved down behind the do-loop, because it
 *               is referenced only in the else part of the loop conditional.
 *
 *   local: an expression is needed only in local usage, e.g. in withloops
 *
 *   On bottom-up traversal, we check if all results of an assignment are
 *   marked for move-down. If so, we can move down the whole expression.
 *
 * Remark: because the concept of global objects cannot handle withloops
 *   correctly, assignments that define unique identifiers are not moved
 *   at all. if this problem is fixed later you can set CREATE_UNIQUE_BY_HEAP
 *   to enable loop-independent removal for unique identifiers, too.
 *
 *   in a second traversal (lirmov) the marked do-invariant expressions are
 *   moved in the surrounding fundef and the function signature is adjusted.
 *   we used DupTree() with a special LookUpTable to do the code movement. in
 *   the LUT we store pairs of internal/external vardec/avis/name to get the
 *   correct substitution when we copy the code.
 *
 *   to have the correct replacements we need two LUTs, one for the general
 *   mapping between args and calling parameters and the moved local
 *   identifiers and a second one for the mapping between return expressions
 *   and results.
 *
 *   the LUT is created freshly for each movement, because DupTree() modifies
 *   the LUT with additional entries. for move_up only the general LUT is
 *   needed but if we move code down, we must update the entries of the general
 *   LUT with the entries of the result mapping LUT.
 *
 * Remark on DLIR fundef traversal:
 *
 *   LACFUNs are only traversed from their single point of call, i.e.,
 *   the N_ap in the calling function. The recursive call in a LOOPfun
 *   is never traversed.
 *
 *   This means that:
 *     In non-GLF mode, we ignore LACFUNs in LIRfundef.
 *     This only happens at the N_module-level invocation of LIR.
 *
 *     In GLF mode, it means we never traverse LOCALFUNS.
 *     Hence, the only way for a LACFUN to end up in LIRfundef
 *     is via the N_ap of its caller.
 *
 * Remark from jsa:
 *   I do not understand the meaning of INFO_TRAVSTART in this traversal.
 *   It looks to me that the DLIR does different things to a normal fundef when
 *   the fundef is (a) reached from a module spine (the DLIR is invoked on
 *   a module), or (b) the DLIR is directly invoked on that normal fundef.
 *   Whether this is a desirable behaviour or a bug I cannot tell.
 *   And, to complicate the matters more, the DLIR may be also invoked directly
 *   on lacfuns in the AWLFI's SimplifySymbioticExpression().
 *
 *****************************************************************************/
#include "loop_invariant_removal.h"

#define DBUG_PREFIX "DLIR"
#include "debug.h"

#include "tree_basic.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "change_signature.h"
#include "globals.h"
#include "new_types.h"
#include "phase.h"
#include "ctinfo.h"

/*
 * INFO structure
 */

typedef enum { TS_fundef, TS_module } travstart;

struct INFO {
    node *fundef;
    node *preassign;
    node *postassign;
    node *assign;
    int nonliruse;
    int condstatus;
    int withdepth;
    bool topblock;
    int flag;
    node *extpreassign;
    node *extpostassign;
    lut_t *movelut;
    node *apargchain;
    node *apreschain;
    node *extfundef;
    nodelist *resultmap;
    nodelist *inslist;
    node *fundefextassign;
    node *fundefintassign;
    travstart mtravstart;
    bool travinlac;
    node *lhs; // TODO: could be removed
};

/*
 * INFO macros
 *
 * INFO_FUNDEF              : node* = set in LIRfundef()
 * INFO_PREASSIGN           : node* = assignments that should be moved just before the
 current one
 * INFO_POSTASSIGN          : node* = assignments that should be moved just after the
 current one
 * INFO_ASSIGN              : node* = current N_assign node. Transfered to
 INFO_FUNDEFEXTASSIGN upon entering fundef.
 * INFO_NONLIRUSE           : int = number of non-LIR N_id. If in top-block and there is
 none non-LIR in N_let, then the N_let can be moved up.
 * INFO_CONDSTATUS          : int = {CONDSTATUS_NOCOND, CONDSTATUS_THENPART,
 CONDSTATUS_ELSEPART}. The segment of the conditional expression in ssa form in LAC fun.
 that we're traversing.
 * INFO_WITHDEPTH           : int = with-loop depth level, reset in fundef,
 inc/decremented in LIRwith
 * INFO_TOPBLOCK            : bool = flag indicates we're in the function's top-level
 block
 * INFO_FLAG                : int {LIR_NORMAL, LIR_MOVEUP, LIR_INRETURN, LIR_MOVELOCAL,
 LIR_MOVEDOWN} = traverse sub-mode
 * INFO_EXTPREASSIGN        : node* = N_assign's that are being moved out of lac-fun. into
 parent
 * INFO_EXTPOSTASSIGN       : node* = N_assign's that are being moved down across lac-fun
 * INFO_MOVELUT             : lut_t* = LUT between an internal arg and the external vardec
 pair
 * INFO_APARGCHAIN          : node* = in LOOP-fun: fun. arg chain of the parent ap. call;
                              Consumed in LIRarg() along its passage over fun. args. when
 they're paired with internal args.
 * INFO_APRESCHAIN          : node* = results chain of the child func.
 * INFO_EXTFUNDEF           : node* = in LAC-fun: the external calling fundef
 * INFO_RESULTMAP           : nodelist* = local id -> result ids
 * INFO_DEPTHMASK           : bool* = array of [WITHDEPTH+1] bools. Indicates there's a
 dependecy on a variable defined on a given level of the with-loop nest tree.
 * INFO_SETDEPTH            : int = the depth to which the avis definition depths
 (AVIS_DEFDEPTH) shall be set in LIRids. It may be different from INFO_WITHDEPTH in the
 case the expression is being moved.
 * INFO_INSLIST             : nodelist* = list of frames (a stack but with arbitrary
 access to any frame). Frames are pushed/poped when traversing with-loop levels.
 * INFO_FUNDEFEXTASSIGN     : node* = in LAC-fun: the external assign in the calling
 fundef
 * INFO_FUNDEFINTASSIGN     : node* = in LOOP-fun: the internal recursive assignment of
 the fun. (only for TS_module)
 * INFO_TRAVSTART           : {TS_fundef, TS_module} = mode
 * INFO_TRAVINLAC           : bool = set in LIRap() upon entering LAC fundef
 * INFO_LHS                 : node* = left-hand side, i.e. LET_IDS. Used for debug print
 purposes.
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_POSTASSIGN(n) (n->postassign)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_NONLIRUSE(n) (n->nonliruse)
#define INFO_CONDSTATUS(n) (n->condstatus)
#define INFO_WITHDEPTH(n) (n->withdepth)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_FLAG(n) (n->flag)
#define INFO_EXTPREASSIGN(n) (n->extpreassign)
#define INFO_EXTPOSTASSIGN(n) (n->extpostassign)
#define INFO_MOVELUT(n) (n->movelut)
#define INFO_APARGCHAIN(n) (n->apargchain)
#define INFO_APRESCHAIN(n) (n->apreschain)
#define INFO_EXTFUNDEF(n) (n->extfundef)
#define INFO_RESULTMAP(n) (n->resultmap)
#define INFO_INSLIST(n) (n->inslist)
#define INFO_FUNDEFEXTASSIGN(n) (n->fundefextassign)
#define INFO_FUNDEFINTASSIGN(n) (n->fundefintassign)
#define INFO_TRAVSTART(n) (n->mtravstart)
#define INFO_TRAVINLAC(n) (n->travinlac)
#define INFO_LHS(n) (n->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_NONLIRUSE (result) = 0;
    INFO_CONDSTATUS (result) = 0;
    INFO_WITHDEPTH (result) = 0;
    INFO_TOPBLOCK (result) = FALSE;
    INFO_FLAG (result) = 0;
    INFO_EXTPREASSIGN (result) = NULL;
    INFO_EXTPOSTASSIGN (result) = NULL;
    INFO_MOVELUT (result) = NULL;
    INFO_APARGCHAIN (result) = NULL;
    INFO_APRESCHAIN (result) = NULL;
    INFO_EXTFUNDEF (result) = NULL;
    INFO_RESULTMAP (result) = NULL;
    INFO_INSLIST (result) = NULL;
    INFO_FUNDEFEXTASSIGN (result) = NULL;
    INFO_FUNDEFINTASSIGN (result) = NULL;
    INFO_TRAVSTART (result) = TS_fundef;
    INFO_TRAVINLAC (result) = FALSE;
    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/* INFO_CONDSTATUS */
#define CONDSTATUS_NOCOND 0
#define CONDSTATUS_THENPART 1
#define CONDSTATUS_ELSEPART 2

/* INFO_FLAG (mode of traversal) */
#define LIR_NORMAL 0
#define LIR_MOVEUP 1
#define LIR_INRETURN 2
#define LIR_MOVELOCAL 3
#define LIR_MOVEDOWN 4

/* AVIS_LIRMOVE / LET_LIRFLAG */
#define LIRMOVE_NONE 0x0
#define LIRMOVE_UP 0x1
#define LIRMOVE_DOWN 0x2
#define LIRMOVE_LOCAL 0x4
#define LIRMOVE_STAY 0x8

/* functions for local usage only */
#ifndef CREATE_UNIQUE_BY_HEAP
#if 0
static bool ForbiddenMovement(node *chain);
#endif
#endif
static node *CheckMoveDownFlag (node *instr, info *arg_info);
static void CreateNewResult (node *avis, info *arg_info);
static lut_t *InsertMappingsIntoLUT (lut_t *move_table, nodelist *mappings);
static node *AdjustExternalResult (node *new_assigns, node *ext_assign, node *ext_fundef);

#ifndef CREATE_UNIQUE_BY_HEAP
/******************************************************************************
 *
 * function:
 *   bool ForbiddenMovement(node *chain)
 *
 * description:
 *   Checks if there is any ids in chain with vardec marked as ST_unique so
 *   we should better not move it out of withloops to avoid problems with
 *   global objects.
 *
 *   This functionality is disabled right now as ids are not yet marked
 *   as unique
 *
 *****************************************************************************/
#if 0
static bool ForbiddenMovement(node *chain)
{
  bool res;

  DBUG_ENTER ();
  res = FALSE;

#if 0
  while ((chain != NULL) && (res == FALSE)) {
    res |= AVIS_ISUNIQUE(IDS_AVIS(chain));
    chain = IDS_NEXT(chain);
  }
#endif

  DBUG_RETURN (res);
}
#endif
#endif

/******************************************************************************
 *
 * function:
 *   node *CheckMoveDownFlag(node *instr)
 *
 * description:
 *   checks the given assign-instruction (should be a let) to have all results
 *   marked as move_down to mark the whole let for move_down. a partial move
 *   down of expressions is not possible.
 *
 *****************************************************************************/
static node *
CheckMoveDownFlag (node *instr, info *arg_info)
{
    node *result;
    int non_move_down;
    int move_down;

    DBUG_ENTER ();

    if (NODE_TYPE (instr) == N_let) {
        /* traverse result-chain and check for move_down flags */
        result = LET_IDS (instr);
        non_move_down = 0;
        move_down = 0;

        while (result != NULL) {
            if (AVIS_LIRMOVE (IDS_AVIS (result)) & LIRMOVE_DOWN) {
                move_down++;
            } else {
                non_move_down++;
            }
            result = IDS_NEXT (result);
        }

        if ((move_down > 0) && (non_move_down == 0)) {
            /*
             * all results are marked for move_down, so expression can
             * be moved down
             */
            LET_LIRFLAG (instr) = LET_LIRFLAG (instr) | LIRMOVE_DOWN;
            DBUG_PRINT ("whole expression %s marked for move-down",
                        AVIS_NAME (IDS_AVIS (LET_IDS (instr))));
        } else {
            DBUG_PRINT ("whole expression %s can not be moved down",
                        AVIS_NAME (IDS_AVIS (LET_IDS (instr))));
        }
    }

    DBUG_RETURN (instr);
}

/******************************************************************************
 *
 * function:
 *   node *CreateNewResult(node *avis, info *arg_info)
 *
 * description:
 *   creates a new result in this fundef (returning given avis/vardec):
 *     1. create new vardec in external (calling) fundef
 *     2. add [ avis -> new_ext_avis] to RESULTMAP nodelist
 *     3. create new vardec in local fundef (as result in recursive call)
 *     4. create new vardec in local fundef (as phi copy target)
 *     5. modify functions signature (AddResult)
 *     6. insert phi-copy-assignments in then and else part of conditional
 *
 *****************************************************************************/
void
CreateNewResult (node *avis, info *arg_info)
{
    node *new_ext_vardec;
    node *new_int_vardec;
    node *new_pct_vardec;
    char *new_name;
    nodelist *letlist;
    node *tmp;
    node *cond;
    node *funcond;

    DBUG_ENTER ();

    /* 1. create new vardec in external (calling) fundef */
    new_name = TRAVtmpVarName (AVIS_NAME (avis));
    new_ext_vardec = TBmakeVardec (TBmakeAvis (new_name, TYcopyType (AVIS_TYPE (avis))),
                                   FUNDEF_VARDECS (INFO_EXTFUNDEF (arg_info)));

    /* add vardec to chain of vardecs (ext. fundef) */
    FUNDEF_VARDECS (INFO_EXTFUNDEF (arg_info)) = new_ext_vardec;

    /* 2. add [avis -> new_ext_avis] to RESULTMAP nodelist */
    INFO_RESULTMAP (arg_info)
      = TCnodeListAppend (INFO_RESULTMAP (arg_info), avis, VARDEC_AVIS (new_ext_vardec));

    /* mark variable as being a result of this function */
    AVIS_EXPRESULT (avis) = TRUE;

    /* 3. create new vardec in local fundef (as result in recursive call) */
    new_int_vardec = TBmakeVardec (TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                                               TYcopyType (AVIS_TYPE (avis))),
                                   FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info)) = new_int_vardec;

    /* 4. create new vardec in local fundef (as PhiCopyTarget) */
    new_pct_vardec = TBmakeVardec (TBmakeAvis (TRAVtmpVarName (AVIS_NAME (avis)),
                                               TYcopyType (AVIS_TYPE (avis))),
                                   FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info)) = new_pct_vardec;

    DBUG_PRINT ("create external vardec %s for %s, local vardec %s, and pct %s", new_name,
                AVIS_NAME (avis), AVIS_NAME (VARDEC_AVIS (new_int_vardec)),
                AVIS_NAME (VARDEC_AVIS (new_pct_vardec)));

    /* 5. modify functions signature (AddResult) */
    /* recursive call */
    letlist = TCnodeListAppend (NULL, ASSIGN_STMT (INFO_FUNDEFINTASSIGN (arg_info)),
                                new_int_vardec);

    /* external call */
    letlist = TCnodeListAppend (letlist, ASSIGN_STMT (INFO_FUNDEFEXTASSIGN (arg_info)),
                                new_ext_vardec);

    INFO_FUNDEF (arg_info)
      = CSaddResult (INFO_FUNDEF (arg_info), new_pct_vardec, letlist);

    /* set correct assign nodes */
    AVIS_SSAASSIGN (VARDEC_AVIS (new_int_vardec)) = INFO_FUNDEFINTASSIGN (arg_info);

    AVIS_SSAASSIGN (VARDEC_AVIS (new_ext_vardec)) = INFO_FUNDEFEXTASSIGN (arg_info);

    /* 6. insert phi-copy-assignments in then and else part of conditional */
    /* search for conditional */
    tmp = BLOCK_ASSIGNS (FUNDEF_BODY (INFO_FUNDEF (arg_info)));
    while ((NODE_TYPE (ASSIGN_STMT (tmp)) != N_cond) && (tmp != NULL)) {
        tmp = ASSIGN_NEXT (tmp);
    }

    DBUG_ASSERT (tmp != NULL, "missing conditional in do-loop special function");
    cond = ASSIGN_STMT (tmp);

    /* create one let assign for then part */
    funcond = TBmakeFuncond (DUPdoDupNode (COND_COND (cond)),
                             TBmakeId (VARDEC_AVIS (new_int_vardec)), TBmakeId (avis));

    /* append new phi function behind cond block */
    ASSIGN_NEXT (tmp)
      = TBmakeAssign (TBmakeLet (TBmakeIds (VARDEC_AVIS (new_pct_vardec), NULL), funcond),
                      ASSIGN_NEXT (tmp));

    AVIS_SSAASSIGN (VARDEC_AVIS (new_pct_vardec)) = ASSIGN_NEXT (tmp);

    // #ifdef LETISAADOIT           not is SSALIR
    if (PHisSAAMode ()) {
        /* FIXME should set AVIS_DIM/SHAPE here */
        CTIwarn (EMPTY_LOC, "CreateNewResult could not set AVIS_SHAPE/AVIS_DIM");
    }
    // #endif // LETISAADOIT

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   lut_t* InsertMappingsIntoLUT(lut_t* move_table, nodelist *mappings)
 *
 * description:
 *   update all mappings from nodelist for vardec/avis/name into LUT.
 *   the nodelist contains pairs of [int. avis -> ext. avis].
 *
 *
 *****************************************************************************/
static lut_t *
InsertMappingsIntoLUT (lut_t *move_table, nodelist *mappings)
{
    DBUG_ENTER ();

    /* add all internal->external connections to LUT: */
    while (mappings != NULL) {
        /* vardec */
        move_table
          = LUTupdateLutP (move_table, AVIS_DECL (NODELIST_NODE (mappings)),
                           AVIS_DECL (((node *)NODELIST_ATTRIB2 (mappings))), NULL);

        /* avis */
        move_table = LUTupdateLutP (move_table, NODELIST_NODE (mappings),
                                    ((node *)NODELIST_ATTRIB2 (mappings)), NULL);

        /* id name */
        move_table
          = LUTupdateLutS (move_table,
                           VARDEC_OR_ARG_NAME (AVIS_DECL (NODELIST_NODE (mappings))),
                           VARDEC_OR_ARG_NAME (
                             AVIS_DECL (((node *)NODELIST_ATTRIB2 (mappings)))),
                           NULL);

        DBUG_PRINT ("update %s(" F_PTR ", " F_PTR ")"
                    " -> %s(" F_PTR ", " F_PTR ") into LUT for mapping",
                    VARDEC_OR_ARG_NAME (AVIS_DECL (NODELIST_NODE (mappings))),
                    (void *)NODELIST_NODE (mappings),
                    (void *)AVIS_DECL (NODELIST_NODE (mappings)),
                    AVIS_NAME (((node *)NODELIST_ATTRIB2 (mappings))),
                    (void *)((node *)NODELIST_ATTRIB2 (mappings)),
                    (void *)AVIS_DECL (((node *)NODELIST_ATTRIB2 (mappings))));

        mappings = NODELIST_NEXT (mappings);
    }

    DBUG_RETURN (move_table);
}

/******************************************************************************
 *
 * function:
 *   node *AdjustExternalResult(node *new_assigns,
 *                              node *ext_assign,
 *                              node *ext_fundef)
 *
 * description:
 *   remove duplicate definitions after inserting move down assignments by
 *   inserting new dummy variables in the result list of the external call.
 *   new_assigns is the list of moved down assignments,
 *   ext_assign/ext_fundef are links to the external assignment and fundef
 *
 *****************************************************************************/
static node *
AdjustExternalResult (node *new_assigns, node *ext_assign, node *ext_fundef)
{
    node *result_chain;
    node *new_vardec;
    node *new_avis;
    node *new_ids;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (ext_assign) == N_assign, "no external assignment node");

    /* search for corresponding result ids in external function call */
    do {
        /* get ids result chain of moved assignment */
        DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (new_assigns)) == N_let,
                     "moved assignments must be let nodes");

        new_ids = LET_IDS (ASSIGN_STMT (new_assigns));

        while (new_ids != NULL) {
            result_chain = LET_IDS (ASSIGN_STMT (ext_assign));

            while (result_chain != NULL) {
                if (IDS_AVIS (new_ids) == IDS_AVIS (result_chain)) {
                    /* matching ids found - create new vardec and rename result_ids */
                    new_avis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (result_chain)),
                                           TYcopyType (IDS_NTYPE (result_chain)));
                    new_vardec
                      = TBmakeVardec (new_avis, BLOCK_VARDECS (FUNDEF_BODY (ext_fundef)));
                    DBUG_PRINT (
                      "AdjustExternalResult created dummy external fn result vardec %s",
                      AVIS_NAME (VARDEC_AVIS (new_vardec)));

                    BLOCK_VARDECS (FUNDEF_BODY (ext_fundef)) = new_vardec;

                    /* rename ids */
                    IDS_AVIS (result_chain) = new_avis;
                    AVIS_SSAASSIGN (new_avis) = ext_assign;

                    /* stop searching */
                    result_chain = NULL;

                } else {
                    /* contiune search */
                    result_chain = IDS_NEXT (result_chain);
                }
            }

            /* traverse to next ids in chain */
            new_ids = IDS_NEXT (new_ids);
        }

        /* traverse to next move-down assignment */
        new_assigns = ASSIGN_NEXT (new_assigns);
    } while (new_assigns != NULL);

    DBUG_RETURN (ext_fundef);
}

/******************************************************************************
 *
 * function:
 *   node *GetRecursiveCallAssignment( node *dofun)
 *
 * description:
 *   returns the assignment of the recursive application of a do function
 *
 *****************************************************************************/
static node *
GetRecursiveCallAssignment (node *dofun)
{
    node *ass;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (dofun) == N_fundef, "Illegal argument!");
    DBUG_ASSERT (FUNDEF_ISLOOPFUN (dofun), "Illegal argument!");
    DBUG_ASSERT (FUNDEF_BODY (dofun) != NULL, "Loop function without body!");

    ass = FUNDEF_ASSIGNS (dofun);
    while ((ass != NULL) && (NODE_TYPE (ASSIGN_STMT (ass)) != N_cond)) {
        ass = ASSIGN_NEXT (ass);
    }

    DBUG_ASSERT (ass != NULL, "Loop function without conditional!");

    ass = COND_THENASSIGNS (ASSIGN_STMT (ass));

    while ((ass != NULL) && (NODE_TYPE (ASSIGN_STMT (ass)) == N_annotate)) {
        ass = ASSIGN_NEXT (ass);
    }

    DBUG_ASSERT ((ass != NULL) && (NODE_TYPE (ass) == N_assign)
                   && (NODE_TYPE (ASSIGN_STMT (ass)) == N_let)
                   && (NODE_TYPE (ASSIGN_RHS (ass)) == N_ap)
                   && (AP_FUNDEF (ASSIGN_RHS (ass)) == dofun),
                 "No recursive application found in the expected position!");

    DBUG_RETURN (ass);
}

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* LIRfundef(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses fundef two times:
 *      first: infer expressions to move (and do WLIR in the local fundef)
 *     second: do the external code movement and fundef adjustment (lirmov_tab)
 *
 *****************************************************************************/
node *
DLIRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Starting do-loop-invariant removal in fundef %s",
                FUNDEF_NAME (arg_node));

    if ((FUNDEF_ISLACFUN (arg_node)
         && (INFO_TRAVINLAC (arg_info) || (INFO_TRAVSTART (arg_info) == TS_fundef)))
        || (!FUNDEF_ISLACFUN (arg_node))) {
        /**
         * only traverse fundef node if fundef is not lacfun, or if traversal
         * was initialized in ap-node (travinlac == TRUE);
         * (lacfun = loop or cond fun.)
         */
        info *info = MakeInfo ();

        INFO_TRAVSTART (info) = INFO_TRAVSTART (arg_info);
        INFO_TRAVINLAC (info) = INFO_TRAVINLAC (arg_info);
        INFO_FUNDEF (info) = arg_node; // this fundef -> info.fundef
        if (INFO_TRAVINLAC (arg_info)) {
            INFO_EXTFUNDEF (info)
              = INFO_FUNDEF (arg_info); // parent fundef -> info.extfundef
            INFO_FUNDEFEXTASSIGN (info) = INFO_ASSIGN (arg_info);
        }

        /* build up LUT for vardec move/rename operartions */
        /* also obtain assignment of recursive call */
        if (FUNDEF_ISLOOPFUN (arg_node)) {
            if (INFO_TRAVSTART (info) == TS_module) {
                INFO_MOVELUT (info) = LUTgenerateLut ();
                INFO_FUNDEFINTASSIGN (info) = GetRecursiveCallAssignment (arg_node);
            }
        }

        /* init empty result map */
        INFO_RESULTMAP (info) = NULL;

        /* save pointer to arg-chain of external function application */
        if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_ISLOOPFUN (arg_node))
            && (INFO_TRAVSTART (info) == TS_module)) {
            INFO_APARGCHAIN (info) = AP_ARGS (ASSIGN_RHS (INFO_FUNDEFEXTASSIGN (info)));
        }

        /* traverse args */
        DBUG_PRINT ("Traversing FUNDEF_ARGS for %s", FUNDEF_NAME (arg_node));
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

        /* top level (not [directly] contained in any withloop) */
        INFO_WITHDEPTH (info) = 0;

        /* traverse function body */
        if (FUNDEF_BODY (arg_node) != NULL) {
            DBUG_PRINT ("Traversing FUNDEF_BODY for %s", FUNDEF_NAME (arg_node));
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (INFO_TRAVSTART (arg_info) == TS_module) {
                /* start LIRMOV traversal of BODY to move out marked assignments */
                TRAVpush (TR_dlirmov);
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);
                TRAVpop ();
            }
        }

        /* clean up LUT */
        if (INFO_MOVELUT (info) != NULL) {
            INFO_MOVELUT (info) = LUTremoveLut (INFO_MOVELUT (info));
        }

        /* clean up result map nodelist */
        if (INFO_RESULTMAP (info) != NULL) {
            INFO_RESULTMAP (info) = TCnodeListFree (INFO_RESULTMAP (info), 0);
        }

        if (INFO_TRAVINLAC (arg_info)) {
            INFO_PREASSIGN (arg_info) = INFO_EXTPREASSIGN (info);
            INFO_POSTASSIGN (arg_info) = INFO_EXTPOSTASSIGN (info);
        }

        info = FreeInfo (info);
    }

    DBUG_PRINT ("Ended loop-invariant removal in fundef %s", FUNDEF_NAME (arg_node));

    /**
     * traverse only in next fundef if we've came here from a non-function subtree,
     * i.e. from the module chain.
     */
    if (INFO_FUNDEF (arg_info) == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRarg(node *arg_node, info *arg_info)
 *
 * description:
 *   insert mappings between args and external vardecs for all loop
 *   invariant arguemnts to move LUT.
 *
 *****************************************************************************/
node *
DLIRarg (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = ARG_AVIS (arg_node);

    /*
     * build up LUT between args and their corresponding calling vardecs
     * for all loop invariant arguments
     */
    if ((INFO_MOVELUT (arg_info) != NULL) && (INFO_APARGCHAIN (arg_info) != NULL)
        && (AVIS_SSALPINV (avis) == TRUE)) {
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (INFO_APARGCHAIN (arg_info))) == N_id,
                     "non N_id node in function application");

        /* add internal->external pairs to LUT: */
        /* avis */
        INFO_MOVELUT (arg_info)
          = LUTinsertIntoLutP (INFO_MOVELUT (arg_info), avis,
                               ID_AVIS (EXPRS_EXPR (INFO_APARGCHAIN (arg_info))));
    }

    /* init avis data for argument */
    AVIS_NEEDCOUNT (avis) = 0;
    AVIS_LIRMOVE (avis) = LIRMOVE_NONE;
    AVIS_EXPRESULT (avis) = FALSE;

    if (ARG_NEXT (arg_node) != NULL) {
        /* when building LUT traverse to next arg pf external call */
        if (INFO_APARGCHAIN (arg_info) != NULL) {
            INFO_APARGCHAIN (arg_info) = EXPRS_NEXT (INFO_APARGCHAIN (arg_info));
        }

        /* traverse to next arg */
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRvardec(node *arg_node, info *arg_info)
 *
 * description:
 *   init avis data of vardecs for LIR traversal
 *
 *****************************************************************************/
node *
DLIRvardec (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = VARDEC_AVIS (arg_node);

    DBUG_PRINT ("Initializing vardec for %s", AVIS_NAME (avis));
    AVIS_NEEDCOUNT (avis) = 0;
    AVIS_SSALPINV (avis) = FALSE;
    AVIS_LIRMOVE (avis) = LIRMOVE_NONE;
    AVIS_EXPRESULT (avis) = FALSE;

    /* traverse to next vardec */
    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse vardecs and block in this order
 *   set INFO_TOPBLOCK according to block type
 *
 *****************************************************************************/
node *
DLIRblock (node *arg_node, info *arg_info)
{
    int old_flag;

    DBUG_ENTER ();

    /* save block mode */
    old_flag = INFO_TOPBLOCK (arg_info);

    if (FUNDEF_BODY (INFO_FUNDEF (arg_info)) == arg_node) {
        /* top block */
        INFO_TOPBLOCK (arg_info) = TRUE;
    } else {
        /* any other block */
        INFO_TOPBLOCK (arg_info) = FALSE;
    }

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    /* restore block mode */
    INFO_TOPBLOCK (arg_info) = old_flag;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRgenarray(node *arg_node, info *arg_info)
 *
 * description:
 *  Traverse the sons
 *
 *****************************************************************************/
node *
DLIRgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVsons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse assign instructions in top-down order to infer do LI-assignments,
 *   mark move up expressions and do the WLIR movement on the bottom up
 *   return traversal.
 *
 *****************************************************************************/
node *
DLIRassign (node *arg_node, info *arg_info)
{
    node *pre_assign;
    node *old_assign;

    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_STMT (arg_node), "missing instruction in assignment");

    /* init traversal flags */
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;
    INFO_PREASSIGN (arg_info) = NULL;
    INFO_POSTASSIGN (arg_info) = NULL;

    /* start traversal of instruction */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* analyse and store results of instruction traversal */
    INFO_ASSIGN (arg_info) = old_assign;

    pre_assign = INFO_PREASSIGN (arg_info);
    INFO_PREASSIGN (arg_info) = NULL;

    /*
     * check for with-loop-independent expressions:
     * if all used identifiers are defined on a outer level, we can move
     * up the whole assignment to this level. when moving a complete withloop
     * this can let to wrong programms if the withloop carries some preassign
     * statements that should be inserted in front of this withloop, but on
     * the current level. if now the withloop is moved on another level we
     * must move the preassignments, too, but this might result in wrong code.
     * so we let this withloop at its current level and try to move it in the
     * next opt cycle as standalone expression without dependend preassigns.
     */

    /* Commented out: because after WLIR is lifted out, it can never be
       the case the that there are any preassigns after traversing a N_with(?)
       So essentially, the body of this conditional will never be executed.

      if ((INFO_TOPBLOCK(arg_info) == TRUE)
          && (NODE_TYPE(ASSIGN_STMT(arg_node)) == N_let)
          && (NODE_TYPE(ASSIGN_RHS(arg_node)) == N_with)
          && (pre_assign != NULL)) {
        AVIS_LIRMOVE(IDS_AVIS(LET_IDS(ASSIGN_STMT(arg_node)))) = LIRMOVE_STAY;
        LET_LIRFLAG(ASSIGN_STMT(arg_node)) = LIRMOVE_STAY;
      }
    */
    DBUG_ASSERT (!((INFO_TOPBLOCK (arg_info) == TRUE)
                   && (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)
                   && (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_with)
                   && (pre_assign != NULL)),
                 "Should never happen; see comment above");

    /* insert post-assign code */
    if (INFO_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    /* traverse next assignment */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* in bottom-up traversal: */
    /*
     * check for complete marked move-down result lists (only in topblock)
     */
    if (INFO_TOPBLOCK (arg_info)) {
        ASSIGN_STMT (arg_node) = CheckMoveDownFlag (ASSIGN_STMT (arg_node), arg_info);
    }

    /*
     * insert pre-assign code
     * remark: the pre-assigned code will not be traversed during this cycle
     */
    if (pre_assign != NULL) {
        arg_node = TCappendAssign (pre_assign, arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse let expression and result identifiers
 *   checks, if the dependent used identifiers are loop invariant and marks the
 *   expression for move_up (only in topblock). expressions in with loops
 *   that only depends on local identifiers are marked as local, too.
 *   all other expressions are marked as normal, which means nothing special.
 *
 *****************************************************************************/
node *
DLIRlet (node *arg_node, info *arg_info)
{
    node *oldlhs;
    node *ids;

    DBUG_ENTER ();

    if (INFO_TOPBLOCK (arg_info)) {
        /* on toplevel: start counting non-lir args in expression */
        INFO_NONLIRUSE (arg_info) = 0;
    }

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);
    DBUG_PRINT ("DLIRlet looking at: %s", AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /*
     * Analyse dependencies carried in the DTUL
     */
    ids = LET_IDS (arg_node);
    while (ids != NULL) {
#ifdef FIXME
        node *avis = IDS_AVIS (ids);

    The following breaks unit test SCSprf_sub.sac (and many others)
    if AVIS_DIM/SHAPE are actually present. Disable for now, which
    will likely cause other sorts of problems...

    AVIS_DIM( avis) = TRAVopt( AVIS_DIM( avis), arg_info);
    AVIS_SHAPE (avis) = TRAVopt (AVIS_SHAPE (avis), arg_info);
    AVIS_MIN (avis) = TRAVopt (AVIS_MIN (avis), arg_info);
    AVIS_MAX (avis) = TRAVopt (AVIS_MAX (avis), arg_info);
    AVIS_SCALARS (avis) = TRAVopt (AVIS_SCALARS (avis), arg_info);
#endif // FIXME
#undef FIXME
    ids = IDS_NEXT (ids);
    }

    if (INFO_TOPBLOCK (arg_info)) {
        /* in topblock mark let statement according to the inferred data */
        if ((INFO_NONLIRUSE (arg_info) == 0)
            && (INFO_CONDSTATUS (arg_info) == CONDSTATUS_NOCOND)
            && (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)))
            && (!((NODE_TYPE (LET_EXPR (arg_node)) == N_with)
                  && (INFO_PREASSIGN (arg_info) != NULL)))) {

            DBUG_PRINT ("DLIR expression %s detected - mark it for moving up",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            /*
             * expression is  not in a condition and uses only LI
             * arguments -> mark expression for move up in front of loop
             */

            LET_LIRFLAG (arg_node) = LIRMOVE_UP;
            INFO_FLAG (arg_info) = LIR_MOVEUP;
        } else {
            DBUG_PRINT ("non-LIR expression %s detected - marked for no moving",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
            LET_LIRFLAG (arg_node) = LIRMOVE_NONE;
            INFO_FLAG (arg_info) = LIR_NORMAL;
        }
    } else if (INFO_WITHDEPTH (arg_info) > 0) {
        /* in other blocks (with-loops), marks all definitions as local */
        if (INFO_CONDSTATUS (arg_info) == CONDSTATUS_NOCOND) {
            DBUG_PRINT ("local expression %s detected - mark it",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

            INFO_FLAG (arg_info) = LIR_MOVELOCAL;
        } else {
            INFO_FLAG (arg_info) = LIR_NORMAL;
        }
        LET_LIRFLAG (arg_node) = LIRMOVE_NONE;
    } else {
        /* in all other cases */
        INFO_FLAG (arg_info) = LIR_NORMAL;
        LET_LIRFLAG (arg_node) = LIRMOVE_NONE;
    }

    /* traverse ids to mark them as loop-invariant/local or normal */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    /* step back to normal mode */
    INFO_FLAG (arg_info) = LIR_NORMAL;
    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRid(node *arg_node, info *arg_info)
 *
 * description:
 *   normal mode:
 *     checks identifier for being loop invariant or increments nonliruse
 *     counter always increments the needed counter.
 *
 *   inreturn mode:
 *     checks for move down assignments and set flag in avis node
 *     creates a mapping between local vardec/avis/name and external result
 *     vardec/avis/name for later code movement with LUT. because we do not
 *     want this modification when we move up expressions, we cannot store
 *     these mapping in LUT directly. therefore we save the inferred information
 *     in a nodelist RESULTMAP for later access on move_down operations.
 *
 *****************************************************************************/
node *
DLIRid (node *arg_node, info *arg_info)
{
    node *id;

    DBUG_ENTER ();

    /* Traverse AVIS_DIM and friends */
    ID_AVIS (arg_node) = TRAVcont (ID_AVIS (arg_node), arg_info);

    switch (INFO_FLAG (arg_info)) {
    case LIR_NORMAL:
        /* increment need/uses counter */
        AVIS_NEEDCOUNT (ID_AVIS (arg_node)) = AVIS_NEEDCOUNT (ID_AVIS (arg_node)) + 1;

        /*
         * if id is NOT loop invariant or local defined
         * increment nonliruse counter
         */
        if (!((AVIS_SSALPINV (ID_AVIS (arg_node)))
              || (AVIS_LIRMOVE (ID_AVIS (arg_node)) & LIRMOVE_LOCAL))) {
            INFO_NONLIRUSE (arg_info) = INFO_NONLIRUSE (arg_info) + 1;

            DBUG_PRINT ("non-loop-invariant or non-local id %s",
                        (AVIS_NAME (ID_AVIS (arg_node))));
        } else {
            DBUG_PRINT ("loop-invariant or local id %s",
                        (AVIS_NAME (ID_AVIS (arg_node))));
        }
        break;

    case LIR_INRETURN:
        if (TCisPhiFun (arg_node)) {
            DBUG_ASSERT (FUNCOND_ELSE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node))))
                           != NULL,
                         "missing definition assignment in else part");

            /*
             * this is the identifier in the loop, that is returned via the
             * ssa-phicopy assignment
             */
            id = FUNCOND_ELSE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node))));

            /*
             * look for the corresponding result ids in the let_node of
             * the external function application and add the mapping
             * [local id -> result ids] to the RESULTMAP nodelist.
             */
            DBUG_ASSERT (INFO_APRESCHAIN (arg_info) != NULL,
                         "missing external result ids");

            INFO_RESULTMAP (arg_info)
              = TCnodeListAppend (INFO_RESULTMAP (arg_info), ID_AVIS (id),
                                  IDS_AVIS (INFO_APRESCHAIN (arg_info)));

            /* mark variable as being a result of this function */
            AVIS_EXPRESULT (ID_AVIS (id)) = TRUE;

            /*
             * if return identifier is used only once (in phi copy assignment):
             * this identifier can be moved down behind the loop, because it is
             * not needed in the loop.
             * the marked variables will be checked in the bottom up traversal
             * to be defined on an left side where all identifiers are marked for
             * move down (see CheckMoveDownFlag()).
             */

            if ((AVIS_NEEDCOUNT (ID_AVIS (id)) == 1)
                && (AVIS_LIRMOVE (ID_AVIS (id)) != LIRMOVE_STAY)) {

                DBUG_PRINT ("loop-invariant assignment (marked for move down) [%s, %s]",
                            VARDEC_OR_ARG_NAME (AVIS_DECL (ID_AVIS (id))),
                            VARDEC_OR_ARG_NAME (AVIS_DECL (ID_AVIS (arg_node))));

                (AVIS_LIRMOVE (ID_AVIS (id))) |= LIRMOVE_DOWN;
            }
        }
        break;

    default:
        DBUG_UNREACHABLE ("unable to handle LIR_FLAG in LIRid");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRap(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses in dependent special function and integrates pre/post-assignment
 *   code.
 *
 *   We never traverse from any function into a normal function,
 *   because we do not know the number of call points, and things
 *   would generally get nasty.
 *
 *****************************************************************************/
node *
DLIRap (node *arg_node, info *arg_info)
{
    bool old_trav;
    DBUG_ENTER ();

    DBUG_ASSERT (AP_FUNDEF (arg_node) != NULL, "missing fundef in ap-node");

    /*
     * Always traverse LACFUNs, but avoid the recursive loopfun call
     *
     */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info))) {
        DBUG_PRINT ("traverse of lacfun fundef %s", FUNDEF_NAME (AP_FUNDEF (arg_node)));

        old_trav = INFO_TRAVINLAC (arg_info);
        INFO_TRAVINLAC (arg_info) = TRUE;

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

        INFO_TRAVINLAC (arg_info) = old_trav;

        DBUG_PRINT ("traversal of lacfun fundef %s finished\n",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));

    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("do not traverse normal fundef %s",
                    FUNDEF_NAME (AP_FUNDEF (arg_node)));
    }

    /* traverse args of function application */
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRcond(node *arg_node, info *arg_info)
 *
 * description:
 *   set the correct conditional status flag and traverse the condition and
 *   the then and else blocks.
 *
 * remark:
 *   in ssaform there can be only one conditional per special functions, so
 *   there is no need to stack the information here.
 *
 *****************************************************************************/
node *
DLIRcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse condition */
    INFO_CONDSTATUS (arg_info) = CONDSTATUS_NOCOND;
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    /* traverse then part */
    INFO_CONDSTATUS (arg_info) = CONDSTATUS_THENPART;
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    /* traverse else part */
    INFO_CONDSTATUS (arg_info) = CONDSTATUS_ELSEPART;
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    /* leaving conditional */
    INFO_CONDSTATUS (arg_info) = CONDSTATUS_NOCOND;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRreturn(node *arg_node, info *arg_info)
 *
 * description:
 *   in loops: look for possibled move-down assignments and mark them
 *
 *****************************************************************************/
node *
DLIRreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info))
        && (INFO_TRAVSTART (arg_info) == TS_module)) {
        /* init INFO_APRESCHAIN with external result chain */
        DBUG_ASSERT (INFO_FUNDEFEXTASSIGN (arg_info) != NULL,
                     "missing link to external calling fundef");

        INFO_APRESCHAIN (arg_info)
          = LET_IDS (ASSIGN_STMT (INFO_FUNDEFEXTASSIGN (arg_info)));

        INFO_FLAG (arg_info) = LIR_INRETURN;
    } else {
        /* no special loop function */
        INFO_APRESCHAIN (arg_info) = NULL;
        INFO_FLAG (arg_info) = LIR_NORMAL;
    }

    /* traverse results */
    RETURN_EXPRS (arg_node) = TRAVopt (RETURN_EXPRS (arg_node), arg_info);

    INFO_FLAG (arg_info) = LIR_NORMAL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRwith(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses with-loop, increments withdepth counter during traversal,
 *   adds a new frame to the InsertList stack for later code movement.
 *   after traversing the withloop, put the assignments moved to the current
 *   depth into INFO_PREASSIGN() to be inserted in front of this
 *   assignment.
 *
 *****************************************************************************/
node *
DLIRwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* increment withdepth counter */
    INFO_WITHDEPTH (arg_info) = INFO_WITHDEPTH (arg_info) + 1;

    /* traverse partitions */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /* traverse code blocks */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* traverse withop */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* decrement withdepth counter */
    INFO_WITHDEPTH (arg_info) = INFO_WITHDEPTH (arg_info) - 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   mark index vectors as local variables to allow code moving
 *
 *****************************************************************************/
node *
DLIRwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse all local definitions to mark their depth in withloops */
    INFO_FLAG (arg_info) = LIR_MOVELOCAL;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_FLAG (arg_info) = LIR_NORMAL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRexprs(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses all exprs nodes.
 *   when used in return of a do-loop (with given INFO_APRECHAIN
 *   do a parallel result traversal
 *
 *****************************************************************************/
node *
DLIRexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse expression */
    EXPRS_EXPR (arg_node) = TRAVopt (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        if ((INFO_APRESCHAIN (arg_info) != NULL)
            && (INFO_FLAG (arg_info) == LIR_INRETURN)) {
            /* traverse the result ids chain of the external function application */
            INFO_APRESCHAIN (arg_info) = IDS_NEXT (INFO_APRESCHAIN (arg_info));
        }

        /* traverse next exprs node */
        EXPRS_NEXT (arg_node) = TRAVdo (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *LIRids (node *arg_ids, info *arg_info)
 *
 * description:
 *   set current withloop depth as ids definition depth
 *   set current movement flag as ids LIRMOV flag
 *
 *****************************************************************************/
node *
DLIRids (node *arg_ids, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_ids);

    /* propagate the currect FLAG to the ids */
    switch (INFO_FLAG (arg_info)) {
    case LIR_MOVEUP:
        DBUG_PRINT ("mark: moving up vardec %s", IDS_NAME (arg_ids));
        AVIS_SSALPINV (avis) = TRUE;
        (AVIS_LIRMOVE (avis)) |= LIRMOVE_UP;
        break;

    case LIR_MOVELOCAL:
        DBUG_PRINT ("mark: local vardec %s", IDS_NAME (arg_ids));
        AVIS_LIRMOVE (avis) = LIRMOVE_LOCAL;
        break;

    case LIR_NORMAL:
        AVIS_LIRMOVE (avis) = LIRMOVE_NONE;
        break;

    default:
        DBUG_UNREACHABLE ("unable to handle case");
    }

    /* traverse to next expression */
    IDS_NEXT (arg_ids) = TRAVopt (IDS_NEXT (arg_ids), arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* DLIRmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse only funs in the module.
 *
 *****************************************************************************/
node *
DLIRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/* traversal functions for lirmov_tab */
/******************************************************************************
 *
 * function:
 *   node* LIRMOVblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses only body (not the vardecs)
 *
 *****************************************************************************/

node *
DLIRMOVblock (node *arg_node, info *arg_info)
{
    int old_flag;

    DBUG_ENTER ();

    /* save block mode */
    old_flag = INFO_TOPBLOCK (arg_info);

    if (FUNDEF_BODY (INFO_FUNDEF (arg_info)) == arg_node) {
        /* top block */
        INFO_TOPBLOCK (arg_info) = TRUE;
    } else {
        /* any other block */
        INFO_TOPBLOCK (arg_info) = FALSE;
    }

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    /* restore block mode */
    INFO_TOPBLOCK (arg_info) = old_flag;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses the top-level assignment chain and moves marked assignments
 *   out of the loop. In all other assignment chains traverse the instruction.
 *   if an assignment is marked for move up AND move down we will move it up
 *   because it results in fewer additional arguments.
 *
 *****************************************************************************/
node *
DLIRMOVassign (node *arg_node, info *arg_info)
{
    bool remove_assignment;
    node *tmp;
    lut_t *move_table;
    node *moved_assignments;

    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_STMT (arg_node) != NULL, "missing instruction in assignment");

    if (INFO_TOPBLOCK (arg_info)) {
        if ((NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)
            && (LET_LIRFLAG (ASSIGN_STMT (arg_node)) & LIRMOVE_UP)) {
            /* adjust identifier AND create new local variables in external fundef */
            INFO_FLAG (arg_info) = LIR_MOVEUP;
        } else if ((NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)
                   && (LET_LIRFLAG (ASSIGN_STMT (arg_node)) == LIRMOVE_DOWN)) {
            /*
             * adjust identifier, create new local variables in external fundef
             * add movedown results to RESULTMAP nodelist
             */
            INFO_FLAG (arg_info) = LIR_MOVEDOWN;
        } else {
            /* only adjust identifiers */
            INFO_FLAG (arg_info) = LIR_NORMAL;
        }
    } else {
        /* we are in some subblock - do not change LIR_FLAG set in topblock !*/
    }

    /* traverse expression */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_TOPBLOCK (arg_info)) {
        /*
         * to do the code movement via DupTree and LookUpTable
         * the look-up table contains all pairs of internal/external
         * vardecs, avis and strings of id's.
         * because duptree adds several nodes to the look-up table we have
         * to duplicate our move_table before we use it, to have
         * no wrong pointers to freed data in it in next DupTree operation.
         */
        switch (INFO_FLAG (arg_info)) {
        case LIR_MOVEUP:
            move_table = LUTduplicateLut (INFO_MOVELUT (arg_info));

            /* move up to external preassign chain */
            INFO_EXTPREASSIGN (arg_info)
              = TCappendAssign (INFO_EXTPREASSIGN (arg_info),
                                DUPdoDupNodeLut (arg_node, move_table));

            DBUG_ASSERT (AVIS_SSAASSIGN (IDS_AVIS (
                           LET_IDS (ASSIGN_STMT (INFO_EXTPREASSIGN (arg_info)))))
                           != NULL,
                         "duptree returned an assign with missing SSAASSIGN in avis");

            /* free temp. LUT */
            move_table = LUTremoveLut (move_table);

            /* one loop invarinant expression removed */
            global.optcounters.dlir_expr++;

            /* move up expression can be removed - no further references */
            remove_assignment = TRUE;
            break;

        case LIR_MOVEDOWN:
            move_table = LUTduplicateLut (INFO_MOVELUT (arg_info));

            /* init LUT result mappings */
            move_table = InsertMappingsIntoLUT (move_table, INFO_RESULTMAP (arg_info));

            /* duplicate move-down expressions with LUT */
            moved_assignments = DUPdoDupNodeLut (arg_node, move_table);

            /* adjust external result ids (resolve duplicate definitions) */
            INFO_EXTFUNDEF (arg_info)
              = AdjustExternalResult (moved_assignments, INFO_FUNDEFEXTASSIGN (arg_info),
                                      INFO_EXTFUNDEF (arg_info));

            /* move down to external postassign chain */
            INFO_EXTPOSTASSIGN (arg_info)
              = TCappendAssign (INFO_EXTPOSTASSIGN (arg_info), moved_assignments);

            /* free temp. LUT */
            move_table = LUTremoveLut (move_table);

            /* one loop invarinant expression moved */
            global.optcounters.dlir_expr++;

            /*
             * move down expressions cannot be removed - due to further references
             * all these expressions will be removed by the next SSADeadCodeRemoval
             * traversal
             */
            remove_assignment = FALSE;
            break;

        default:
            /* by default do not remove anything */
            remove_assignment = FALSE;
        }
    } else {
        remove_assignment = FALSE;
    }

    /* traverse to next assignment */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* on bottom up traversal remove marked assignments */
    if (remove_assignment) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVlet(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses right side and left side in this order
 *
 *****************************************************************************/
node *
DLIRMOVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVid(node *arg_node, info *arg_info)
 *
 * description:
 *   does the renaming according to the AVIS_SUBST setting
 *
 *****************************************************************************/
node *
DLIRMOVid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * do the necessary substitution, but only if this identifier stays
     * in this fundef. if it is moved up, we do not substitute it, because
     * the moving duptree will adjust the references to the according external
     * vardec and avis nodes and this code here is removed later
     */
    if ((AVIS_SUBST (ID_AVIS (arg_node)) != NULL)
        && (INFO_FLAG (arg_info) != LIR_MOVEUP)) {
        DBUG_PRINT ("substitution: %s -> %s", (AVIS_NAME (ID_AVIS (arg_node))),
                    (AVIS_NAME (AVIS_SUBST (ID_AVIS (arg_node)))));

        /* do renaming to new ssa vardec */
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    /*
     * when moving down an expression:
     * for each variable that is not moved out and that is not already a
     * result of this function create an additional result parameter that
     * will be added to the RESULTMAP for adjusting the used identifiers
     */
    if ((LUTsearchInLutPp (INFO_MOVELUT (arg_info), ID_AVIS (arg_node))
         == ID_AVIS (arg_node))) {
        DBUG_PRINT ("not in lut for %s", ID_NAME (arg_node));
    }
    if ((INFO_FLAG (arg_info) == LIR_MOVEDOWN)
        && (LUTsearchInLutPp (INFO_MOVELUT (arg_info), ID_AVIS (arg_node))
            == ID_AVIS (arg_node))
        && (AVIS_EXPRESULT (ID_AVIS (arg_node)) != TRUE)) {

        DBUG_PRINT ("create new result in %s for %s",
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)),
                    (AVIS_NAME (ID_AVIS (arg_node))));
        /* this call modifies avis and arg_info */
        CreateNewResult (ID_AVIS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   the withid identifiers are local variables. if we move a with-loop
 *   out of a do-loop, we have to move this local variables, too. These
 *   variables are loop invariant, because the whole with-loop must be moved.
 *   so for each local variable we create a new one in the target context and
 *   add the necessary information to the LUT to have a correct transformation
 *   information for a later moving duptree call.
 *
 *****************************************************************************/
node *
DLIRMOVwithid (node *arg_node, info *arg_info)
{
    int old_flag;

    DBUG_ENTER ();

    if ((INFO_FLAG (arg_info) == LIR_MOVEUP) || (INFO_FLAG (arg_info) == LIR_MOVEDOWN)) {
        if (WITHID_VEC (arg_node) != NULL) {
            /* traverse identifier in move_local variable mode */
            old_flag = INFO_FLAG (arg_info);

            INFO_FLAG (arg_info) = LIR_MOVELOCAL;
            WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);
            WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
            WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);

            /* switch back to previous mode */
            INFO_FLAG (arg_info) = old_flag;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* LIRMOVids(node *arg_ids, info *arg_info)
 *
 * description:
 *   creates external references for moved identifiers and modifies the
 *   function signature according to the moved identifier (for non local id).
 *
 *****************************************************************************/
node *
DLIRMOVids (node *arg_ids, info *arg_info)
{
    node *new_vardec;
    node *new_vardec_id;
    node *new_avis;
    node *new_arg;
    node *new_arg_id;
    nodelist *letlist;

    DBUG_ENTER ();

    if ((INFO_FLAG (arg_info) == LIR_MOVEUP) || (INFO_FLAG (arg_info) == LIR_MOVEDOWN)
        || (INFO_FLAG (arg_info) == LIR_MOVELOCAL)) {
        /*
         * create new vardec in ext fundef
         * set LUT information for later code movement
         */

        /* create new vardec */
        new_avis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (arg_ids)),
                               TYcopyType (IDS_NTYPE (arg_ids)));
        new_vardec
          = TBmakeVardec (new_avis,
                          BLOCK_VARDECS (FUNDEF_BODY (INFO_EXTFUNDEF (arg_info))));

        DBUG_PRINT ("create external vardec %s for %s", (AVIS_NAME (new_avis)),
                    (IDS_NAME (arg_ids)));

        /* add vardec to chain of vardecs (ext. fundef) */
        BLOCK_VARDECS (FUNDEF_BODY (INFO_EXTFUNDEF (arg_info))) = new_vardec;

        /*
         * setup LUT for later DupTree:
         *   subst local avis with external avis
         *
         * NOTE: For DupTree to set the AVIS_SSAASSIGN correctly,
         *       we have to annotate the current SSAASSIGN here.
         *       When later duplicating this assignment, the SSAASSIGN
         *       will then be set to the corresponding new N_assign node.
         *       Details can be found in DUPids and LIRMOVassign.
         */
        AVIS_SSAASSIGN (new_avis) = AVIS_SSAASSIGN (IDS_AVIS (arg_ids));

        INFO_MOVELUT (arg_info)
          = LUTinsertIntoLutP (INFO_MOVELUT (arg_info), IDS_AVIS (arg_ids), new_avis);

        /*
         *  modify functions signature:
         *    add an additional arg to fundef, ext. funap, int funap
         *    set renaming attributes for further processing
         */
        if ((INFO_FLAG (arg_info) == LIR_MOVEUP) && (INFO_TOPBLOCK (arg_info))) {
            /* make identifier for external function application */
            new_vardec_id = TBmakeId (new_avis);

            /* make new arg for this functions (instead of vardec) */
            new_arg = TBmakeArg (DUPdoDupNode (IDS_AVIS (arg_ids)), NULL);

            AVIS_SSAASSIGN (ARG_AVIS (new_arg)) = NULL;
            AVIS_SSALPINV (ARG_AVIS (new_arg)) = FALSE;

            /* make identifier for recursive function call */
            new_arg_id = TBmakeId (ARG_AVIS (new_arg));

            /* change functions signature, internal and external application */
            DBUG_ASSERT (INFO_FUNDEFINTASSIGN (arg_info) != NULL,
                         "missing recursive call");
            DBUG_ASSERT (INFO_FUNDEFEXTASSIGN (arg_info) != NULL,
                         "missing external call");

            /* recursive call */
            letlist
              = TCnodeListAppend (NULL, ASSIGN_STMT (INFO_FUNDEFINTASSIGN (arg_info)),
                                  new_arg_id);
            letlist
              = TCnodeListAppend (letlist, ASSIGN_STMT (INFO_FUNDEFEXTASSIGN (arg_info)),
                                  new_vardec_id);

            INFO_FUNDEF (arg_info) = CSaddArg (INFO_FUNDEF (arg_info), new_arg, letlist);

            /*
             * set renaming information: all old uses will be renamed to the new arg
             */
            AVIS_SUBST (IDS_AVIS (arg_ids)) = ARG_AVIS (new_arg);
        }
    }

    /* traverse next ids */
    IDS_NEXT (arg_ids) = TRAVopt (IDS_NEXT (arg_ids), arg_info);

    DBUG_RETURN (arg_ids);
}

/** <!-- ****************************************************************** -->
 * @fn node *FreeLIRSubstInfo( node *arg_node, info *arg_info)
 *
 * @brief Frees the AVIS_SUBST attribute of an N_AVIS node after the LIR phase
 *
 * @param arg_node N_avis node
 * @param arg_info unused
 *
 * @return modified N_avis node
 ******************************************************************************/
static node *
FreeLIRSubstInfo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (arg_node) != NULL) {
        AVIS_SUBST (arg_node) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FreeLIRInformation( node *arg_node)
 *
 * @brief Clears the AVIS_SUBST information from all N_avis nodes within a
 *        function/module.
 *
 * @param arg_node N_fundef node of function to be cleared or
 *               N_module node of module to be cleared.
 *
 * @return modified N_fundef node or N_module node.
 ******************************************************************************/
static node *
FreeLIRInformation (node *arg_node)
{
    anontrav_t freetrav[2] = {{N_avis, &FreeLIRSubstInfo}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "FreeLIRInformation called with non-module/non-fundef node");

    TRAVpushAnonymous (freetrav, &TRAVsons);
    if (NODE_TYPE (arg_node) == N_module) {
        MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), NULL);
    } else {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), NULL);
    }
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* DLIRdoLoopInvariantRemoval(node *arg_node)
 *
 * description:
 *   starts the loop invariant removal for module/fundef nodes.
 *
 *****************************************************************************/

node *
DLIRdoLoopInvariantRemoval (node *arg_node)
{
    DBUG_ENTER ();
    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "DLIRdoLoopInvariantRemoval called with non-module/non-fundef node");
    DBUG_PRINT ("DLIR invoked on %s %s",
                ((NODE_TYPE (arg_node) == N_module)
                   ? "module"
                   : (FUNDEF_ISLACFUN (arg_node) ? "lacfun" : "normal fun")),
                ((NODE_TYPE (arg_node) == N_module) ? "" : FUNDEF_NAME (arg_node)));

    info *arg_info = MakeInfo ();

    INFO_TRAVSTART (arg_info)
      = (N_module == NODE_TYPE (arg_node)) ? TS_module : TS_fundef;

    TRAVpush (TR_dlir);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    /*
     * we free the information gathered by LIR here as it is no longer
     * used after this transformation
     */
    arg_node = FreeLIRInformation (arg_node);

    DBUG_PRINT ("DLIR finished");
    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
