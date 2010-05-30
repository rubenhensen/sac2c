/******************************************************************************
 *
 * $Id$
 *
 * file:   withloop_invariant_removal.c
 *
 * prefix: WLIR
 *
 * description:
 *   this module implements withloop invariant removal on code in ssa form.
 *   loop invariant assignments in with-loops are moved out of the loops.
 *
 * details:
 *   assignments that depends only on expressions
 *   defined in outer level withloops can be moved to the level
 *   with the minimal possible level:
 *
 *   c = 5;                    -->  c = 5;
 *   x = with(..iv1..) {            y = expr(5,c);
 *       y = expr(5,c);             w = expr(c);
 *       z = with(..iv2..) {        x = with(..iv1..) {
 *           u = expr(y,iv2);           v = expr(c,iv1);
 *           v = expr(c,iv1);           z = with(..iv2..) {
 *           w = expr(c);                   u = expr(y,iv2);
 *           } op(...);                     } op(...);
 *       } op(...);                     } op(...);
 *
 *   In the next optimization cycle, the z = withloop will be moved out of the
 *   surrounding withloop:
 *                                  c = 5;
 *                                  y = expr(5,c);
 *                                  w = expr(c);
 *                                  z = with(..iv2..) {
 *                                      u = expr(y,iv2);
 *                                      } op(...);
 *                                  x = with(..iv2..) {
 *                                      v = expr(c, iv1)
 *                                      } op(...);
 *
 * Implementation notes:
 *
 *   Every assignment is tagged with its definition-depth in stacked withloops.
 *   if we get an expression that depends only on expressions with a smaller
 *   definition depth, we can move up the assignment in the context of the
 *   surrounding withloop with this depth. to do so, we add this assignments
 *   to a stack of assignment chains (one for each definition level) in the
 *   movement target level. When the withloop of one level has been processed,
 *   all moved assignment are inserted in front of the withloop assignment.
 *   that is why we cannot move a withloop in the same step as other
 *   assignments because it can lead to wrong code, when we move code together
 *   with the moved withloop do another level.
 *
 *
 *****************************************************************************/
#include "withloop_invariant_removal.h"

#include "dbug.h"
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
    bool remassign;
    node *preassign;
    node *postassign;
    node *assign;
    node *lhsavis;
    int withdepth;
    bool topblock;
    int maxdepth;
    int setdepth;
    nodelist *inslist;
    travstart travstart;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_ASSIGN(n) (n->assign)
#define INFO_LHSAVIS(n) (n->lhsavis)
#define INFO_WITHDEPTH(n) (n->withdepth)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_RESULTMAP(n) (n->resultmap)
#define INFO_MAXDEPTH(n) (n->maxdepth)
#define INFO_SETDEPTH(n) (n->setdepth)
#define INFO_INSLIST(n) (n->inslist)
#define INFO_TRAVSTART(n) (n->travstart)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_REMASSIGN (result) = FALSE;
    INFO_PREASSIGN (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_LHSAVIS (result) = NULL;
    INFO_WITHDEPTH (result) = 0;
    INFO_TOPBLOCK (result) = FALSE;
    INFO_MAXDEPTH (result) = 0;
    INFO_SETDEPTH (result) = 0;
    INFO_INSLIST (result) = NULL;
    INFO_TRAVSTART (result) = TS_fundef;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/* AVIS_DEFDEPTH */
#define DD_UNDEFINED -1

/* functions for local usage only */
#ifndef CREATE_UNIQUE_BY_HEAP
static bool ForbiddenMovement (node *chain);
#endif
static nodelist *InsListPushFrame (nodelist *il);
static nodelist *InsListPopFrame (nodelist *il);
static nodelist *InsListAppendAssigns (nodelist *il, node *assign, int depth);
static nodelist *InsListSetAssigns (nodelist *il, node *assign, int depth);
static node *InsListGetAssigns (nodelist *il, int depth);
static nodelist *InsListGetFrame (nodelist *il, int depth);

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
static bool
ForbiddenMovement (node *chain)
{
    bool res;

    DBUG_ENTER ("ForbiddenMovement");
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

/******************************************************************************
 *
 * function:
 *   nodelist *InsListPushFrame(nodelist *il)
 *
 * description:
 *   Creates a new frame in the InsertList il. This datastructure is used like
 *   a stack (push/pop) when entering a new contained with-loop and leaving
 *   it. but it also allows direct access to lower frames via their depth
 *   number to add moved assignments. So this data structure is based on a
 *   chained list nodelist.
 *
 *****************************************************************************/
static nodelist *
InsListPushFrame (nodelist *il)
{
    DBUG_ENTER ("InsListPushFrame");

    if (il == NULL) {
        /* new insert list, create level 0 */
        il = TBmakeNodelistNode (NULL, NULL);
        NODELIST_INT (il) = 0;
    } else {
        /* insert new frame in front of list, increment level */
        il = TBmakeNodelistNode (NULL, il);
        NODELIST_INT (il) = NODELIST_INT (NODELIST_NEXT (il)) + 1;
    }

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListPopFrame(nodelist *il)
 *
 * description:
 *   removed the latest frame from the InserList il
 *
 *****************************************************************************/
static nodelist *
InsListPopFrame (nodelist *il)
{
    DBUG_ENTER ("InsListPopFrame");

    DBUG_ASSERT ((il != NULL), "tried to pop of empty insert list");

    il = FREEfreeNodelistNode (il);

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListAppendAssigns(nodelist *il, node* assign, int depth)
 *
 * description:
 *   appends given assignment(s) to the chain in frame depth.
 *
 *****************************************************************************/
static nodelist *
InsListAppendAssigns (nodelist *il, node *assign, int depth)
{
    nodelist *tmp;

    DBUG_ENTER ("InsListAppendAssigns");

    tmp = InsListGetFrame (il, depth);
    NODELIST_NODE (tmp) = TCappendAssign (NODELIST_NODE (tmp), assign);

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListAppendAssigns(nodelist *il, node* assign, int depth)
 *
 * description:
 *   set the given assignment in in frame depth.
 *
 *****************************************************************************/
static nodelist *
InsListSetAssigns (nodelist *il, node *assign, int depth)
{
    nodelist *tmp;

    DBUG_ENTER ("InsListSetAssigns");

    tmp = InsListGetFrame (il, depth);

    NODELIST_NODE (tmp) = assign;

    DBUG_RETURN (il);
}

/******************************************************************************
 *
 * function:
 *   node *InsListGetAssigns(nodelist *il, int depth)
 *
 * description:
 *   returns the assigns chain for given depth.
 *
 *****************************************************************************/
static node *
InsListGetAssigns (nodelist *il, int depth)
{
    DBUG_ENTER ("InsListGetAssigns");

    DBUG_RETURN (NODELIST_NODE (InsListGetFrame (il, depth)));
}

/******************************************************************************
 *
 * function:
 *   nodelist *InsListGetFrame(nodelist *il, int depth)
 *
 * description:
 *   gets the insert list frame for "depth".
 *   this function is for internal use only
 *
 *****************************************************************************/
static nodelist *
InsListGetFrame (nodelist *il, int depth)
{
    int pos;
    nodelist *tmp;

    DBUG_ENTER ("InsListGetFrame");

    DBUG_ASSERT ((il != NULL), "try to access empty insert list");

    DBUG_ASSERT (((depth >= 0) && (depth <= NODELIST_INT (il))),
                 "parameter depth out of range of given insert list");

    /* search for nodelist element of given depth */
    tmp = il;
    for (pos = NODELIST_INT (il); pos > depth; pos--) {
        DBUG_ASSERT ((tmp != NULL), "unexpected end of insert list");
        tmp = NODELIST_NEXT (tmp);
    }

    DBUG_ASSERT ((NODELIST_INT (tmp) == depth),
                 "select wrong frame - maybe corrupted insert list");

    DBUG_RETURN (tmp);
}

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* WLIRfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
WLIRfundef (node *arg_node, info *arg_info)
{
    info *info;

    DBUG_ENTER ("WLIRfundef");

    DBUG_PRINT ("WLIR", ("loop-invariant removal in fundef %s", FUNDEF_NAME (arg_node)));

    /**
     * only traverse fundef node if fundef is not lacfun, or if traversal
     * was initialized in ap-node (travinlac == TRUE)
     */
    info = MakeInfo ();

    INFO_FUNDEF (info) = arg_node;

    /* traverse args */
    DBUG_PRINT ("WLIR", ("Traversing FUNDEF_ARGS for %s", FUNDEF_NAME (arg_node)));
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

    /* top level (not [directly] contained in any withloop) */
    INFO_WITHDEPTH (info) = 0;

    /* init InsertList for with-loop independed removal */
    INFO_INSLIST (info) = InsListPushFrame (NULL);

    /* traverse function body */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), info);

    /* clean up insert list */
    INFO_INSLIST (info) = InsListPopFrame (INFO_INSLIST (info));

    info = FreeInfo (info);

    /**
     * traverse only in next fundef if traversal started in module node
     */
    if (INFO_TRAVSTART (arg_info) == TS_module) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRarg(node *arg_node, info *arg_info)
 *
 * description:
 *   Set definition depth of each argument to 0
 *
 *****************************************************************************/
node *
WLIRarg (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("WLIRarg");

    avis = ARG_AVIS (arg_node);

    /* init avis data for argument */
    AVIS_DEFDEPTH (avis) = 0;

    /* traverse to next arg */
    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRvardec(node *arg_node, info *arg_info)
 *
 * description:
 *   init avis data of vardecs
 *
 *****************************************************************************/
node *
WLIRvardec (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("WLIRvardec");

    avis = VARDEC_AVIS (arg_node);

    AVIS_DEFDEPTH (avis) = DD_UNDEFINED;

    /* traverse to next vardec */
    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRblock(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse vardecs and block in this order
 *   set INFO_TOPBLOCK according to block type
 *
 *****************************************************************************/
node *
WLIRblock (node *arg_node, info *arg_info)
{
    int old_flag;

    DBUG_ENTER ("WLIRblock");

    /* save block mode */
    old_flag = INFO_TOPBLOCK (arg_info);

    if (FUNDEF_BODY (INFO_FUNDEF (arg_info)) == arg_node) {
        /* top block */
        INFO_TOPBLOCK (arg_info) = TRUE;
    } else {
        /* any other block */
        INFO_TOPBLOCK (arg_info) = FALSE;
    }

    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);
    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    /* in case of an empty block, insert at least the empty node */
    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    /* restore block mode */
    INFO_TOPBLOCK (arg_info) = old_flag;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse assign instructions in top-down order to infer do LI-assignments,
 *   mark move up expressions and do the WLIR movement on the bottom up
 *   return traversal.
 *
 *****************************************************************************/
node *
WLIRassign (node *arg_node, info *arg_info)
{
    bool remove_assign;
    node *pre_assign;
    node *tmp;
    int old_maxdepth;
    node *old_assign;
    int wlir_move_up;

    DBUG_ENTER ("WLIRassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node)), "missing instruction in assignment");

    /* init traversal flags */
    INFO_REMASSIGN (arg_info) = FALSE;
    old_assign = INFO_ASSIGN (arg_info);
    INFO_ASSIGN (arg_info) = arg_node;
    INFO_PREASSIGN (arg_info) = NULL;
    old_maxdepth = INFO_MAXDEPTH (arg_info);
    INFO_MAXDEPTH (arg_info) = 0;

    /* start traversal of instruction */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* analyse and store results of instruction traversal */
    INFO_ASSIGN (arg_info) = old_assign;
    remove_assign = INFO_REMASSIGN (arg_info);
    INFO_REMASSIGN (arg_info) = FALSE;

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
     * next opt cycle as standalone expression without dependent preassigns.
     */
    if ((INFO_MAXDEPTH (arg_info) < INFO_WITHDEPTH (arg_info))
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
#ifndef CREATE_UNIQUE_BY_HEAP
        && (!ForbiddenMovement (LET_IDS (ASSIGN_INSTR (arg_node))))
#endif
        && (!((NODE_TYPE (ASSIGN_RHS (arg_node)) == N_with) && (pre_assign != NULL)))) {
        wlir_move_up = INFO_MAXDEPTH (arg_info);

        /*
         * now we add this assignment to the respective insert level chain
         * and add a new assignment node in the current chain that will be removed
         * later on bottom up traversal (this gives us a correct chain).
         */
        tmp = arg_node;
        arg_node = TBmakeAssign (NULL, ASSIGN_NEXT (arg_node));

        DBUG_ASSERT ((remove_assign == FALSE), "wlur expression must not be removed");

        remove_assign = TRUE;
        ASSIGN_NEXT (tmp) = NULL;

        /*
         * append assignment and surrounding code
         * to InsertList on movement target level
         */
        /* first the preassign code */
        if (pre_assign != NULL) {
            INFO_INSLIST (arg_info)
              = InsListAppendAssigns (INFO_INSLIST (arg_info), pre_assign, wlir_move_up);
            pre_assign = NULL;
        }

        /* append this assignment */
        INFO_INSLIST (arg_info)
          = InsListAppendAssigns (INFO_INSLIST (arg_info), tmp, wlir_move_up);

        /* increment optimize statistics counter */
        global.optcounters.wlir_expr++;
    }

    /* traverse next assignment */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /* in bottom-up traversal: */

    /* remove this assignment */
    if (remove_assign) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FREEdoFreeNode (tmp);
    }

    /*
     * insert pre-assign code
     * remark: the pre-assigned code will not be traversed during this cycle
     */
    if (pre_assign != NULL) {
        arg_node = TCappendAssign (pre_assign, arg_node);
    }

    /* restore old maxdepth counter */
    INFO_MAXDEPTH (arg_info) = old_maxdepth;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRlet(node *arg_node, info *arg_info)
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
WLIRlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("WLIRlet");

    INFO_LHSAVIS (arg_info) = IDS_AVIS (LET_IDS (arg_node));
    DBUG_PRINT ("WLIR", ("looking at %s", AVIS_NAME (INFO_LHSAVIS (arg_info))));
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /* detect withloop-independent expression, will be moved up */
    if ((INFO_MAXDEPTH (arg_info) < INFO_WITHDEPTH (arg_info))
#ifndef CREATE_UNIQUE_BY_HEAP
        && (!ForbiddenMovement (LET_IDS (arg_node)))
#endif
        && (!((NODE_TYPE (LET_EXPR (arg_node)) == N_with)
              && (INFO_PREASSIGN (arg_info) != NULL)))) {
        /* set new target definition depth */
        INFO_SETDEPTH (arg_info) = INFO_MAXDEPTH (arg_info);
        DBUG_PRINT ("WLIR", ("moving assignment from depth %d to depth %d",
                             INFO_WITHDEPTH (arg_info), INFO_MAXDEPTH (arg_info)));

    } else {
        /* set current depth */
        INFO_SETDEPTH (arg_info) = INFO_WITHDEPTH (arg_info);
    }

    /* traverse ids to mark them as loop-invariant/local or normal */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    INFO_LHSAVIS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRid(node *arg_node, info *arg_info)
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
WLIRid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIRid");

    /*
     * calc the maximum definition depth of all identifiers in the
     * current assignment
     */
    DBUG_ASSERT ((AVIS_DEFDEPTH (ID_AVIS (arg_node)) != DD_UNDEFINED),
                 "usage of undefined identifier");
    if (INFO_MAXDEPTH (arg_info) < AVIS_DEFDEPTH (ID_AVIS (arg_node))) {
        INFO_MAXDEPTH (arg_info) = AVIS_DEFDEPTH (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRwith(node *arg_node, info *arg_info)
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
WLIRwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIRwith");

    DBUG_PRINT ("WLIR", ("Looking at %s=with...", AVIS_NAME (INFO_LHSAVIS (arg_info))));
    /* clear current InsertListFrame */
    INFO_INSLIST (arg_info)
      = InsListSetAssigns (INFO_INSLIST (arg_info), NULL, INFO_WITHDEPTH (arg_info));

    /* increment withdepth counter */
    INFO_WITHDEPTH (arg_info) = INFO_WITHDEPTH (arg_info) + 1;

    /* create new InsertListFrame */
    INFO_INSLIST (arg_info) = InsListPushFrame (INFO_INSLIST (arg_info));

    /* traverse partitions */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /* traverse code blocks */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /* traverse withop */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /* remove top InsertListFrame */
    INFO_INSLIST (arg_info) = InsListPopFrame (INFO_INSLIST (arg_info));

    /* decrement withdepth counter */
    INFO_WITHDEPTH (arg_info) = INFO_WITHDEPTH (arg_info) - 1;

    /* move the assigns of this depth into PREASSIGN */
    INFO_PREASSIGN (arg_info)
      = TCappendAssign (INFO_PREASSIGN (arg_info),
                        InsListGetAssigns (INFO_INSLIST (arg_info),
                                           INFO_WITHDEPTH (arg_info)));
    /* clear this frame */
    INFO_INSLIST (arg_info)
      = InsListSetAssigns (INFO_INSLIST (arg_info), NULL, INFO_WITHDEPTH (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   mark index vectors as local variables to allow code moving
 *
 *****************************************************************************/
node *
WLIRwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLIRwithid");

    /* traverse all local definitions to mark their depth in withloops */
    INFO_SETDEPTH (arg_info) = INFO_WITHDEPTH (arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *WLIRids (node *arg_ids, info *arg_info)
 *
 * description:
 *   set current withloop depth as ids definition depth
 *   set current movement flag as ids LIRMOV flag
 *
 *****************************************************************************/
node *
WLIRids (node *arg_ids, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("WLIRids");

    avis = IDS_AVIS (arg_ids);

    /* set current withloop depth as definition depth */
    AVIS_DEFDEPTH (avis) = INFO_SETDEPTH (arg_info);

    /* traverse to next expression */
    IDS_NEXT (arg_ids) = TRAVopt (IDS_NEXT (arg_ids), arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRdoLoopInvariantRemovalOneFundef(node* fundef)
 *
 * description:
 *   starts the do-loop/with-loop invariant removal for fundef nodes.
 *
 *****************************************************************************/
node *
WLIRdoLoopInvariantRemovalOneFundef (node *fundef)
{
    info *info;
    DBUG_ENTER ("LIRdoLoopInvariantRemovalOneFundef");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "LIRdoLoopInvariantRemovalOneFundef called for non-fundef node");

    info = MakeInfo ();

    INFO_TRAVSTART (info) = TS_fundef;

    TRAVpush (TR_wlir);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRdoLoopInvariantRemoval(node* module)
 *
 * description:
 *   starts the loop invariant removal for module nodes.
 *
 *****************************************************************************/
node *
WLIRdoLoopInvariantRemoval (node *module)
{
    int movedsofar;

    info *info;

    DBUG_ENTER ("WLIRdoLoopInvariantRemoval");

    DBUG_ASSERT ((NODE_TYPE (module) == N_module),
                 "WLIRdoLoopInvariantRemoval called for non-module node");

    movedsofar = global.optcounters.lir_expr;

    info = MakeInfo ();

    INFO_TRAVSTART (info) = TS_module;

    TRAVpush (TR_wlir);
    module = TRAVdo (module, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (module);
}
