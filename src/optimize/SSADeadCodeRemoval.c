/*
 *
 * $Log$
 * Revision 1.28  2005/03/04 21:21:42  cg
 * FUNDEF_USED counter etc removed.
 * Handling of FUNDEF_EXT_ASSIGNS drastically simplified.
 *
 * Revision 1.27  2005/01/11 12:58:15  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.26  2004/11/26 21:32:42  khf
 * correcte DCRid
 *
 * Revision 1.25  2004/11/26 21:26:53  khf
 * SacDevCamp04: COMPILES!!!
 *
 * Revision 1.24  2004/10/15 12:43:43  ktr
 * COND nodes with two empty branches are no longer removed as they are still
 * needed in order to annotate reference counting instructions for a
 * remaining funcond.
 * NOTE: This is not problematic as the representation of conditionals in
 * FUN form always consists of a COND and a FUNCOND.
 *
 * Revision 1.23  2004/08/08 13:48:41  ktr
 * Fixed bug #43: accu() can never be removed by DCR.
 *
 * Revision 1.22  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.21  2004/05/04 14:26:19  khf
 * NCODE_CEXPR in SSADCRNcode() replaced by NCODE_CEXPRS
 *
 * Revision 1.20  2004/03/05 19:14:27  mwe
 * representation of conditional changed
 * using N_funcond node instead of phi
 *
 * Revision 1.19  2004/02/06 14:19:33  mwe
 * remove usage of PHIASSIGN and ASSIGN2
 * implement usage of primitive phi function instead
 *
 * Revision 1.18  2001/05/31 11:33:30  nmw
 * blocks with NULL assignment chain will get an N_empty node instead
 *
 * Revision 1.17  2001/05/25 08:43:34  nmw
 * comments added
 *
 * Revision 1.16  2001/05/17 12:05:53  nmw
 * MALLOC/FREE changed to Malloc/Free (using Free() result)
 *
 * Revision 1.15  2001/05/15 07:59:59  nmw
 * macro FUNDEF_IS_LACFUN used
 *
 * Revision 1.14  2001/04/30 12:06:37  nmw
 * count eliminated arrays for ArrayElimination statistic
 *
 * Revision 1.13  2001/04/18 12:55:42  nmw
 * debug output for OPT traversal added
 *
 * Revision 1.12  2001/04/17 15:49:43  nmw
 * removal of phi-copy-targets improved (removes both assignments in one traversal)
 *
 * Revision 1.11  2001/04/03 14:21:52  nmw
 * SSADCRlet frees expression to early
 *
 * Revision 1.10  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.9  2001/03/23 09:30:33  nmw
 * SSADCRdo/while removed
 *
 * Revision 1.8  2001/03/22 21:13:33  dkr
 * include of tree.h eliminated
 *
 * Revision 1.7  2001/03/16 11:58:19  nmw
 * add missing static for local functions
 *
 * Revision 1.6  2001/03/12 09:16:05  nmw
 * Debug Messages added, no DCR for recursive function call
 *
 * Revision 1.4  2001/03/02 15:51:49  nmw
 * CSRemoveArg/CSRemoveResult moved to separate file change_signature
 *
 * Revision 1.3  2001/03/02 14:34:15  nmw
 * SSADeadCodeRemoval implemented
 *
 * Revision 1.2  2001/02/27 16:05:35  nmw
 * SSADeadCodeRemoval for intraprocedural code implemented
 *
 * Revision 1.1  2001/02/23 13:37:50  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   SSADeadCodeRemoval.c
 *
 * prefix: SSADCR
 *
 * description:
 *    this module traverses one function (and its liftet special fundefs)
 *    and removes all dead code. this transformation is NOT conservative
 *    (it might remove endless loops, if they are not necessary to compute
 *     the result)
 *
 * implementation:
 *    we start at the return statement and do a bottom-up traversal marking
 *    all needed identifier. if we find a call to a special fundef, we remove
 *    all results not needed before traversing the special fundef. when we
 *    reach the top of a special fundef, we look for the unused args and
 *    remove them, too.
 *    These signature changes are only possible for special fundefs where we
 *    know that there is always exactly on calling application that we can
 *    modifiy, too.
 *    the flags if we need an identifier are stored in the avis nodes
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSADeadCodeRemoval.h"
#include "optimize.h"
#include "change_signature.h"

/*
 * INFO structure
 */
struct INFO {
    int depth;
    bool remassign;
    node *fundef;
    bool remresults;
    node *apfundef;
    int rescount;
    int resneeded;
    node *let;
    node *assign;
    bool travwithid;
    node *module;
};

/*
 * INFO macros
 */
#define INFO_DCR_DEPTH(n) (n->depth)
#define INFO_DCR_REMASSIGN(n) (n->remassign)
#define INFO_DCR_FUNDEF(n) (n->fundef)
#define INFO_DCR_REMRESULTS(n) (n->remresults)
#define INFO_DCR_APFUNDEF(n) (n->apfundef)
#define INFO_DCR_RESCOUNT(n) (n->rescount)
#define INFO_DCR_RESNEEDED(n) (n->resneeded)
#define INFO_DCR_LET(n) (n->let)
#define INFO_DCR_ASSIGN(n) (n->assign)
#define INFO_DCR_TRAVWITHID(n) (n->travwithid)
#define INFO_DCR_MODULE(n) (n->module)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_DCR_DEPTH (result) = 0;
    INFO_DCR_REMASSIGN (result) = FALSE;
    INFO_DCR_FUNDEF (result) = NULL;
    INFO_DCR_REMRESULTS (result) = FALSE;
    INFO_DCR_APFUNDEF (result) = NULL;
    INFO_DCR_RESCOUNT (result) = 0;
    INFO_DCR_RESNEEDED (result) = 0;
    INFO_DCR_LET (result) = NULL;
    INFO_DCR_ASSIGN (result) = NULL;
    INFO_DCR_TRAVWITHID (result) = FALSE;
    INFO_DCR_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/* local constants */
#define DCR_TOPLEVEL 0
#define DCR_NOTNEEDED 0

/* helper functions for local use */
static void InitAvisFlags (node *fundef);

/******************************************************************************
 *
 * function:
 *   void InitAvisFlags(node *fundef)
 *
 * description:
 *   inits flags needed for this module in all vardec and args.
 *
 *****************************************************************************/
static void
InitAvisFlags (node *fundef)
{
    node *tmp;

    DBUG_ENTER ("InitAvisFlags");

    /* process args */
    tmp = FUNDEF_ARGS (fundef);
    while (tmp != NULL) {
        AVIS_NEEDCOUNT (ARG_AVIS (tmp)) = DCR_NOTNEEDED;
        tmp = ARG_NEXT (tmp);
    }

    /* process vardecs */
    if (FUNDEF_BODY (fundef) != NULL) {
        tmp = BLOCK_VARDEC (FUNDEF_BODY (fundef));
        while (tmp != NULL) {
            AVIS_NEEDCOUNT (VARDEC_AVIS (tmp)) = DCR_NOTNEEDED;
            tmp = VARDEC_NEXT (tmp);
        }
    }

    DBUG_VOID_RETURN;
}

/* traversal fundefs for DeadCodeRemoval */
/******************************************************************************
 *
 * function:
 *   node *DCRfundef(node *arg_node , info *arg_info)
 *
 * description:
 *   Starts the traversal of a given fundef. Does NOT traverse to
 *   next fundef in chain! The traversal mode (on toplevel, in special
 *   function) is annotated in the stacked INFO_DCR_DEPTH attribute.
 *   If not on toplevel, the unused arguments are cleared. For the toplevel
 *   functions changes of the signature are not possible.
 *
 *****************************************************************************/
node *
DCRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRfundef");

    DBUG_PRINT ("DCR",
                ("\nstarting dead code removal in fundef %s.", FUNDEF_NAME (arg_node)));

    /* store current vardec for later access */
    INFO_DCR_FUNDEF (arg_info) = arg_node;

    /* init of needed-flag for all vardecs/args of this fundef */
    InitAvisFlags (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if ((INFO_DCR_DEPTH (arg_info) != DCR_TOPLEVEL) && (FUNDEF_ARGS (arg_node) != NULL)) {
        /*
         * traverse args to remove unused ones from signature (not on toplevel!)
         */
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRarg(node *arg_node , info *arg_info)
 *
 * description:
 *   removes all args from function signature that have not been used in the
 *   function.
 *
 *
 *****************************************************************************/
node *
DCRarg (node *arg_node, info *arg_info)
{
    node *tmp;
    nodelist *letlist;

    DBUG_ENTER ("DCRarg");

    /* traverse to next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * process arg and remove it, if not needed anymore in code
     * or the arg is tagged as loop-invariant with one use, that means it
     * occurs at least once in the recursive call. If the argument
     * ONLY appears there, it is dead code, too.
     */
    if ((AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) == DCR_NOTNEEDED)
        || ((AVIS_NEEDCOUNT (ARG_AVIS (arg_node)) == 1)
            && (AVIS_SSALPINV (ARG_AVIS (arg_node))))) {
        DBUG_PRINT ("DCR", ("remove arg %sa", ARG_NAME (arg_node)));
        /* remove this argument from all applications */
        letlist = NULL;
        if (FUNDEF_EXT_ASSIGN (INFO_DCR_FUNDEF (arg_info)) != NULL) {
            letlist = TCnodeListAppend (letlist,
                                        ASSIGN_INSTR (
                                          FUNDEF_EXT_ASSIGN (INFO_DCR_FUNDEF (arg_info))),
                                        NULL);
        }
        if (FUNDEF_INT_ASSIGN (INFO_DCR_FUNDEF (arg_info)) != NULL) {
            letlist = TCnodeListAppend (letlist,
                                        ASSIGN_INSTR (
                                          FUNDEF_INT_ASSIGN (INFO_DCR_FUNDEF (arg_info))),
                                        NULL);
        }

        INFO_DCR_FUNDEF (arg_info)
          = CSremoveArg (INFO_DCR_FUNDEF (arg_info), arg_node, letlist, FALSE);

        FREEfreeNodelist (letlist);

        /* remove arg from function signature */
        tmp = arg_node;
        arg_node = ARG_NEXT (arg_node);
        FREEdoFreeNode (tmp);
        dead_var++;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRblock(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses instructions and vardecs in this order.
 *
 *****************************************************************************/
node *
DCRblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignmentchain in block */
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) == NULL) {
        /* the complete block is empty -> create N_empty node */
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /*
         * traverse all vardecs in block (concerns only toplevel block in
         * each function) to remove useless ones
         */
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRvardec(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses vardecs and removes unused ones.
 *
 *****************************************************************************/
node *
DCRvardec (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("DCRvardec");

    /* traverse to next vardec */
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    /* process vardec and remove it, if dead code */
    if (AVIS_NEEDCOUNT (VARDEC_AVIS (arg_node)) == DCR_NOTNEEDED) {

        /* count eliminated arrays from ArrayElimination */
        if (VARDEC_HASBEENELIMINATED (arg_node)) {
            elim_arrays++;
        }
        dead_var++;

        DBUG_PRINT ("DCR", ("remove unused vardec %s", VARDEC_NAME (arg_node)));
        tmp = arg_node;
        arg_node = VARDEC_NEXT (arg_node);
        FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRassign(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses assignment chain bottom-up and removes all assignments not
 *  needed (marked by INFO_DCR_REMASSIGN)
 *
 *****************************************************************************/
node *
DCRassign (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("DCRassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* traverse instruction */
    INFO_DCR_REMASSIGN (arg_info) = FALSE;
    INFO_DCR_ASSIGN (arg_info) = arg_node;
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "missing instruction in assign");

    /* traverse instruction */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* free this assignment if unused anymore */
    if (INFO_DCR_REMASSIGN (arg_info)) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FREEdoFreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRlet(node *arg_node , info *arg_info)
 *
 * description:
 *  checks, if at least one of the left ids vardecs are needed. then this
 *  let is needed.
 *  a functions application of a special function requieres a special
 *  handling because you can remove parts of the results an modify the
 *  functions signature
 *  a RHS containing a primitive function application of type F_accu
 *  must never become dead code as we would lose the handle to the
 *  intermediate fold result (FIXES BUG #43)
 *
 *****************************************************************************/
node *
DCRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    /* default: do not modify return values of a function application */
    INFO_DCR_REMRESULTS (arg_info) = FALSE;

    /* check for special function as N_ap expression */
    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap) {
        if ((FUNDEF_ISLACFUN (AP_FUNDEF (LET_EXPR (arg_node))))
            && (AP_FUNDEF (LET_EXPR (arg_node)) != INFO_DCR_FUNDEF (arg_info))) {

            /*
             * here we can modify the return values, because
             * there is only one call to this special function
             */
            INFO_DCR_REMRESULTS (arg_info) = TRUE;
            INFO_DCR_APFUNDEF (arg_info) = AP_FUNDEF (LET_EXPR (arg_node));

            DBUG_ASSERT ((FUNDEF_EXT_ASSIGN (AP_FUNDEF (LET_EXPR (arg_node))) != NULL),
                         "missing external application of special function");
        }
    }

    /* init ids counter for results */
    INFO_DCR_RESCOUNT (arg_info) = 0;
    INFO_DCR_RESNEEDED (arg_info) = 0;
    INFO_DCR_LET (arg_info) = arg_node;

    /* traverse left side identifier */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    /*
     * FIX TO BUG #43: accu() must never become dead code!
     */
    if ((INFO_DCR_RESNEEDED (arg_info) == 0)
        && (!((NODE_TYPE (LET_EXPR (arg_node)) == N_prf)
              && (PRF_PRF (LET_EXPR (arg_node)) == F_accu)))) {
        /* let is useless -> can be removed! */

        DBUG_PRINT ("DCR", ("removing assignment"));
        dead_expr++;

        /* corresponding assign node can be removed */
        INFO_DCR_REMASSIGN (arg_info) = TRUE;
    } else {
        /* traverse right side of let (mark needed variables) */
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

        /* do not remove assign node */
        DBUG_PRINT ("DCR", ("preserving assignment"));
        INFO_DCR_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRid(node *arg_node , info *arg_info)
 *
 * description:
 *
 *
 *****************************************************************************/
node *
DCRid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRid");

    /* increments the counter for each identifier usage */
    DBUG_PRINT ("DCR", ("mark id %s as needed", ID_NAME (arg_node)));

    AVIS_NEEDCOUNT (ID_AVIS (arg_node)) = AVIS_NEEDCOUNT (ID_AVIS (arg_node)) + 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRcond(node *arg_node , info *arg_info)
 *
 * description:
 *  traverses both conditional blocks.
 *
 *****************************************************************************/
node *
DCRcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRcond");

    /* traverse else part */
    if (COND_ELSE (arg_node) != NULL) {
        DBUG_PRINT ("DCR", ("processing else part of conditional"));
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    /* traverse then part */
    if (COND_THEN (arg_node) != NULL) {
        DBUG_PRINT ("DCR", ("processing then part of conditional"));
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    /* traverse condition */
    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    INFO_DCR_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRreturn(node *arg_node , info *arg_info)
 *
 * description:
 *   starts traversal of return expressions to mark them as needed.
 *
 *****************************************************************************/
node *
DCRreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    /* do not remove return instruction */
    INFO_DCR_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRap(node *arg_node , info *arg_info)
 *
 * description:
 *   if application of special function (cond, do, while) traverse into this
 *   function except for recursive calls of the current function.
 *   traverse all arguments to marks them as needed
 *
 *****************************************************************************/
node *
DCRap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("DCRap");

    /* traverse special fundef without recursion */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_DCR_FUNDEF (arg_info))) {
        DBUG_PRINT ("DCR", ("traverse in special fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_DCR_DEPTH (new_arg_info) = INFO_DCR_DEPTH (arg_info) + 1;
        INFO_DCR_MODULE (new_arg_info) = INFO_DCR_MODULE (arg_info);

        /* start traversal of special fundef (and maybe reduce parameters!) */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("DCR", ("traversal of special fundef %s finished"
                            "continue in fundef %s\n",
                            FUNDEF_NAME (AP_FUNDEF (arg_node)),
                            FUNDEF_NAME (INFO_DCR_FUNDEF (arg_info))));

        new_arg_info = FreeInfo (new_arg_info);
    } else {
        DBUG_PRINT ("DCR", ("do not traverse in normal fundef %s",
                            FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    /* mark all args as needed */
    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRwith(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses withop, code and partitions of withloop
 *
 *****************************************************************************/
node *
DCRwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRwith");

    /* traverse withop */
    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    /* traverse code */
    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    /* traverse withop */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRpart(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses generator, withid and next part
 *
 *****************************************************************************/
node *
DCRpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRpart");

    /* traverse generator */
    if (PART_GENERATOR (arg_node) != NULL) {
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    }

    /* traverse withid */
    if (PART_WITHID (arg_node) != NULL) {
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    }
    /* traverse next part */
    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRcode(node *arg_node , info *arg_info)
 *
 * description:
 *   traverses exprs, block and next in this order
 *
 *****************************************************************************/
node *
DCRcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRcode");

    /* traverse expression */
    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
    }

    /* traverse code block */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DCRwithid(node *arg_node , info *arg_info)
 *
 * description:
 *   marks index vector and identifier as needed to preserve them
 *   if they are not explicit used in Withloop.
 *   to do so these identifier are handeld like ids on the RIGHT side of
 *   an assignment.
 *
 *****************************************************************************/
node *
DCRwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DCRwithid");

    INFO_DCR_TRAVWITHID (arg_info) = TRUE;

    /* traverse ids */
    if (WITHID_IDS (arg_node) != NULL) {
        WITHID_IDS (arg_node) = TRAVdo (WITHID_IDS (arg_node), arg_info);
    }

    /* traverse vec */
    if (WITHID_VEC (arg_node) != NULL) {
        WITHID_VEC (arg_node) = TRAVdo (WITHID_VEC (arg_node), arg_info);
    }

    INFO_DCR_TRAVWITHID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *DCRids(node *arg_ids, info *arg_info)
 *
 * description:
 *   checks if ids is marked as needed and increments counter.
 *   if results of a special function application, unused results are removed.
 *   traverses to next ids.
 *
 *****************************************************************************/

node *
DCRids (node *arg_ids, info *arg_info)
{
    node *next_ids;
    nodelist *letlist;

    DBUG_ENTER ("DCRleftids");

    if (INFO_DCR_TRAVWITHID (arg_info)) {
        /* increments the counter for each identifier usage */
        DBUG_PRINT ("DCR", ("mark ids %s as needed", IDS_NAME (arg_ids)));

        AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) = AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) + 1;

        /* traverse next ids in ids chain */
        if (IDS_NEXT (arg_ids) != NULL) {
            IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
        }

    } else {

        /* position of result in result-list */
        INFO_DCR_RESCOUNT (arg_info) = INFO_DCR_RESCOUNT (arg_info) + 1;

        /* check result for usage */
        if (AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) == DCR_NOTNEEDED) {
            /*
             * result is not needed
             * if result of special function application - remove this result
             * but first: save next ids in chain
             */

            next_ids = IDS_NEXT (arg_ids);

            if (INFO_DCR_REMRESULTS (arg_info)) {
                DBUG_PRINT ("DCR",
                            ("check left side %s  - not needed -> remove from resultlist",
                             IDS_NAME (arg_ids)));

                /* remove result from all function applications - except this one */
                letlist = NULL;
                if (FUNDEF_EXT_ASSIGN (INFO_DCR_APFUNDEF (arg_info)) != NULL) {
                    if (ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (INFO_DCR_APFUNDEF (arg_info)))
                        != INFO_DCR_LET (arg_info)) {
                        letlist = TCnodeListAppend (letlist,
                                                    ASSIGN_INSTR (FUNDEF_EXT_ASSIGN (
                                                      INFO_DCR_FUNDEF (arg_info))),
                                                    NULL);
                    }
                }
                if (FUNDEF_INT_ASSIGN (INFO_DCR_APFUNDEF (arg_info)) != NULL) {
                    if (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_DCR_APFUNDEF (arg_info)))
                        != INFO_DCR_LET (arg_info)) {
                        letlist = TCnodeListAppend (letlist,
                                                    ASSIGN_INSTR (FUNDEF_INT_ASSIGN (
                                                      INFO_DCR_APFUNDEF (arg_info))),
                                                    NULL);
                    }
                }

                INFO_DCR_APFUNDEF (arg_info)
                  = CSremoveResult (INFO_DCR_APFUNDEF (arg_info),
                                    INFO_DCR_RESCOUNT (arg_info), letlist);

                FREEfreeNodelist (letlist);
                FREEdoFreeNode (arg_ids);

                /* decrement position in resultlist after deleting the result */
                INFO_DCR_RESCOUNT (arg_info) = INFO_DCR_RESCOUNT (arg_info) - 1;

                /* traverse rest of result chain */
                if (next_ids != NULL) {
                    next_ids = TRAVdo (next_ids, arg_info);
                }

                /* set correct return value */
                arg_ids = next_ids;

            } else {
                DBUG_PRINT ("DCR",
                            ("check left side %s  - not needed", IDS_NAME (arg_ids)));

                /*
                 * variable is not needed, but mark variable as needed to preserve
                 * vardec to avoid problems with multiple results in a function
                 * application that is not removed but where are some unused results.
                 * in this case the vardecs of the unused identifiers must not removed,
                 * because the hole application will stay in the programm.
                 */

                AVIS_NEEDCOUNT (IDS_AVIS (arg_ids))
                  = AVIS_NEEDCOUNT (IDS_AVIS (arg_ids)) + 1;

                /* traverse next ids as usual */
                if (IDS_NEXT (arg_ids) != NULL) {
                    IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
                }
            }
        } else {
            DBUG_PRINT ("DCR", ("check left side %s  - needed", IDS_NAME (arg_ids)));

            /* result is needed, increment counter */
            INFO_DCR_RESNEEDED (arg_info) = INFO_DCR_RESNEEDED (arg_info) + 1;

            /* traverse next ids */
            if (IDS_NEXT (arg_ids) != NULL) {
                IDS_NEXT (arg_ids) = TRAVdo (IDS_NEXT (arg_ids), arg_info);
            }
        }
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *DCRdoDeadCodeRemoval(node *fundef, node *modul)
 *
 * description:
 *   starting point of DeadCodeRemoval for SSA form.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in
 *   their order of usage. The traversal mode (on toplevel, in special
 *   function) is annotated in the stacked INFO_DCR_DEPTH attribute.
 *
 *****************************************************************************/
node *
DCRdoDeadCodeRemoval (node *fundef, node *module)
{
    info *arg_info;

    DBUG_ENTER ("DCRdoDeadCodeRemoval");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "DCRdoDeadCodeRemoval called for non-fundef node");

    DBUG_PRINT ("OPT", ("starting dead code removal (ssa) in function %s",
                        FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if (!(FUNDEF_ISLACFUN (fundef))) {
        arg_info = MakeInfo ();

        INFO_DCR_DEPTH (arg_info) = DCR_TOPLEVEL; /* start on toplevel */
        INFO_DCR_MODULE (arg_info) = module;

        TRAVpush (TR_dcr);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}
