/******************************************************************************
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
 * Remark on With-Loop independence detection:
 *   Nested with-loops (WL) form scope levels. When considering a WL for lifting,
 *   those variables defined within it (i.e. on WITHDEPTH+1) and all that
 *   are even deeper, have no impact on the WL's independence.
 *   These local variables are moved along with the WL, should it be lifted.
 *   However, other variables on the same or higher levels (0 <= lvl <= WITHDEPTH)
 *   that are referenced by expressions in WL (e.g. in definitions of those local
 *   variables) constitute true data dependencies. These dependencies
 *   must be honoured: a WL can be moved up precisely to the maximal (deepest) level
 *   on which variables referenced _anywhere_ within it currently reside.
 *
 *   Dependencies are tracked on a nest-level granularity in a bool array called DEPTHMASK
 *   in an info structure. When an N_id is encountered within a WL a flag
 *   corresponding to a level on which the N_id resides is set.
 *   The length of the bool array is always WITHDEPTH+1 elements because
 *   an N_id on the level WITHDEPTH can be seen at the worst.
 *   When backtracking from N_with node the array is shortened by 1 and the superfluous
 *   elements is simply discarded. The discarded flag belongs to the level that is
 *   sub-local to the parent and thus unimportant to it. In N_assign a clean array is created
 *   for sons traversal. When backtracking, the mask obtained from sons is merged using a
 *   logical-OR into a mask returned to parent. This is because the mask in fact represents
 *   data dependencies of the whole subtree, potentially consisting of several assignments.
 *
 *****************************************************************************/
#include <inttypes.h>
#include "withloop_invariant_removal.h"

#define DBUG_PREFIX "WLIR"
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

struct INFO {
    node *fundef;
    node *preassign;
    node *lhsavis; // was lhs:N_ids in SSALIR, here it's N_avis
    int withdepth;
    bool topblock;
    bool *depthmask;
    int setdepth;
    nodelist *inslist;
};

/*
 * INFO macros
 *
 * INFO_FUNDEF              : node* = set in LIRfundef()
 * INFO_PREASSIGN           : node* = assignments that should be moved just before the
 current one
 * INFO_LHSAVIS             : node* = N_avis on the left-hand side, used mainly for debug
 prints
 * INFO_WITHDEPTH           : int = with-loop depth level, reset in fundef,
 inc/decremented in LIRwith
 * INFO_TOPBLOCK            : bool = flag indicates we're in the function's top-level
 block
 * INFO_DEPTHMASK           : bool* = array of [WITHDEPTH+1] bools. Indicates there's a
 dependecy on a variable defined on a given level of the with-loop nest tree.
 * INFO_SETDEPTH            : int = the depth to which the avis definition depths
 (AVIS_DEFDEPTH) shall be set in LIRids. It may be different from INFO_WITHDEPTH in the
 case the expression is being moved.
 * INFO_INSLIST             : nodelist* = list of frames (a stack but with arbitrary
 access to any frame). Frames are pushed/poped when traversing with-loop levels.
 *
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_LHSAVIS(n) (n->lhsavis)
#define INFO_WITHDEPTH(n) (n->withdepth)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_SETDEPTH(n) (n->setdepth)
#define INFO_DEPTHMASK(n) (n->depthmask)
#define INFO_INSLIST(n) (n->inslist)

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
    INFO_LHSAVIS (result) = NULL;
    INFO_WITHDEPTH (result) = 0;
    INFO_TOPBLOCK (result) = FALSE;
    INFO_DEPTHMASK (result) = (bool *)MEMmalloc (sizeof (bool) * 1);
    INFO_DEPTHMASK (result)[0] = FALSE;
    INFO_SETDEPTH (result) = 0;
    INFO_INSLIST (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    MEMfree (INFO_DEPTHMASK (info));
    INFO_DEPTHMASK (info) = NULL;

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

/******************************************************************************
 *
 * function:
 *  static inline void depthmask_mark_level(info *inf, int level)
 *
 * Mark the given WL level as providing dependency for the current
 * expression tree. 'Level' can be from 0 (top block) up to and including
 * INFO_WITHDEPTH, indicating a dependency within the current level.
 * In the later case the expression is effectively non-WL-invariant
 * and thus cannot be moved.
 *
 *****************************************************************************/
static inline void
depthmask_mark_level (info *inf, int level)
{
    DBUG_ENTER ();
    DBUG_ASSERT ((level >= 0) && (level <= INFO_WITHDEPTH (inf)),
                 "cannot have/set dependency on a variable deeper than the current "
                 "nesting level");

    INFO_DEPTHMASK (inf)[level] = TRUE;

    DBUG_RETURN ();
}

#ifndef DBUG_OFF
/******************************************************************************
 *
 * function:
 *  uint64_t dmask2ui64(info *inf)
 *
 * Extract INFO_WITHDEPTH as a 64-bit uint.
 * Use for debug prints only!
 *
 *****************************************************************************/
static uint64_t
dmask2ui64 (info *inf)
{
    DBUG_ENTER ();

    uint64_t v = 0;
    for (int i = 0; i <= INFO_WITHDEPTH (inf); ++i) {
        if (INFO_DEPTHMASK (inf)[i])
            v |= (1ULL << i);
    }

    DBUG_RETURN (v);
}
#endif

/******************************************************************************
 *
 * function:
 *  int depthmask_deepest_level(const info *inf)
 *
 * Find out the deepest marked WL level. The deepest level is the level onto
 * which and expression can be moved while still having all dependencies
 * satisfied.
 *
 * Examples:
 *  0000 -> 0
 *  0001 -> 0
 *  001X -> 1
 *  01XX -> 2, and so on
 *
 *****************************************************************************/
static int
depthmask_deepest_level (const info *inf)
{
    int result = 0; /* default: depends on top-level */

    DBUG_ENTER ();

    for (int i = INFO_WITHDEPTH (inf); i >= 0; --i) {
        if (INFO_DEPTHMASK (inf)[i]) {
            result = i;
            break;
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * void clear_dmask(bool *dmask, int wl_depth)
 *    Clear the whole depth-mask.
 *
 * void copy_dmask(bool *dst, bool *src, int wl_depth)
 *    Copy depth-masks.
 *
 * void merge_dmask(bool *dst, bool *src, int wl_depth)
 *    Merge depth-mask src into dst using logical OR.
 *
 *****************************************************************************/
static void
clear_dmask (bool *dmask, int wl_depth)
{
    DBUG_ENTER ();
    for (int i = 0; i <= wl_depth; ++i) {
        dmask[i] = FALSE;
    }
    DBUG_RETURN ();
}

static void
copy_dmask (bool *dst, bool *src, int wl_depth)
{
    DBUG_ENTER ();
    for (int i = 0; i <= wl_depth; ++i) {
        dst[i] = src[i];
    }
    DBUG_RETURN ();
}

static void
merge_dmask (bool *dst, bool *src, int wl_depth)
{
    DBUG_ENTER ();
    for (int i = 0; i <= wl_depth; ++i) {
        dst[i] = dst[i] || src[i];
    }
    DBUG_RETURN ();
}

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    DBUG_ASSERT (il != NULL, "tried to pop off empty insert list");

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_ASSERT (il != NULL, "try to access empty insert list");

    DBUG_ASSERT (((depth >= 0) && (depth <= NODELIST_INT (il))),
                 "parameter depth out of range of given insert list");

    /* search for nodelist element of given depth */
    tmp = il;
    for (pos = NODELIST_INT (il); pos > depth; pos--) {
        DBUG_ASSERT (tmp != NULL, "unexpected end of insert list");
        tmp = NODELIST_NEXT (tmp);
    }

    DBUG_ASSERT (NODELIST_INT (tmp) == depth,
                 "select wrong frame - maybe corrupted insert list");

    DBUG_RETURN (tmp);
}

/* traversal functions */

/******************************************************************************
 *
 * function:
 *   node* WLIRmodule(node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse only funs in the module.
 *
 *****************************************************************************/
node *
WLIRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

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

    DBUG_ENTER ();

    DBUG_PRINT ("loop-invariant removal in fundef %s", FUNDEF_NAME (arg_node));

    info = MakeInfo ();

    INFO_FUNDEF (info) = arg_node;

    /* traverse args */
    DBUG_PRINT ("Traversing FUNDEF_ARGS for %s", FUNDEF_NAME (arg_node));
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), info);

    /* top level (not [directly] contained in any withloop) */
    INFO_WITHDEPTH (info) = 0;

    /* init InsertList for with-loop-independent removal */
    INFO_INSLIST (info) = InsListPushFrame (NULL);

    /* traverse function body */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), info);

    /* clean up insert list */
    INFO_INSLIST (info) = InsListPopFrame (INFO_INSLIST (info));

    info = FreeInfo (info);

    /* traverse local funs and next */
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
    bool remove_assign = FALSE;
    node *pre_assign;
    node *tmp;
    bool *old_dmask;
    bool *new_dmask_buf;
    int deepest_lvl;

    DBUG_ENTER ();

    DBUG_ASSERT (ASSIGN_STMT (arg_node), "missing instruction in assignment");
    
    DBUG_ASSERT (INFO_WITHDEPTH (arg_info) >= 0, "With Loop Depth is invalid.");

    new_dmask_buf = (bool *)MEMmalloc ((size_t)(INFO_WITHDEPTH (arg_info) + 1) * sizeof (bool));

    /* init traversal flags */
    INFO_PREASSIGN (arg_info) = NULL;
    /* save incomming depth-mask, reset and activate a new one */
    old_dmask = INFO_DEPTHMASK (arg_info);
    clear_dmask (new_dmask_buf, INFO_WITHDEPTH (arg_info));
    INFO_DEPTHMASK (arg_info) = new_dmask_buf;

    /* start traversal of instruction */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* pick up from sons traversal */
    deepest_lvl = depthmask_deepest_level (arg_info);

    /* analyse and store results of instruction traversal */
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
    if ((deepest_lvl < INFO_WITHDEPTH (arg_info))
        && (NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)
#ifndef CREATE_UNIQUE_BY_HEAP
        && (!ForbiddenMovement (LET_IDS (ASSIGN_STMT (arg_node))))
#endif
        && (!((NODE_TYPE (ASSIGN_RHS (arg_node)) == N_with) && (pre_assign != NULL)))) {
        DBUG_PRINT ("assignment is moving to level %d", deepest_lvl);

        /*
         * now we add this assignment to the respective insert level chain
         * and add a new assignment node in the current chain that will be removed
         * later on bottom up traversal (this gives us a correct chain).
         */
        tmp = arg_node;
        arg_node = TBmakeAssign (NULL, ASSIGN_NEXT (arg_node));

        DBUG_ASSERT (remove_assign == FALSE, "wlur expression must not be removed");

        remove_assign = TRUE;
        ASSIGN_NEXT (tmp) = NULL;

        /*
         * append assignment and surrounding code
         * to InsertList on movement target level
         */
        /* first the preassign code */
        if (pre_assign != NULL) {
            INFO_INSLIST (arg_info)
              = InsListAppendAssigns (INFO_INSLIST (arg_info), pre_assign, deepest_lvl);
            pre_assign = NULL;
        }

        /* append this assignment */
        INFO_INSLIST (arg_info)
          = InsListAppendAssigns (INFO_INSLIST (arg_info), tmp, deepest_lvl);

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

    /* Update the parent depth-mask with ours so as to collect the
     * dependency info. of all the nested expressions.
     * If we're lifting this assignment up then the current mask still needs to be
     * collected because it tracks the real dependency in the tree. */
    merge_dmask (old_dmask, INFO_DEPTHMASK (arg_info), INFO_WITHDEPTH (arg_info));
    INFO_DEPTHMASK (arg_info) = old_dmask;

    MEMfree (new_dmask_buf);

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
    node *old_lhsavis;
    int deepest_lvl;

    DBUG_ENTER ();

    old_lhsavis = INFO_LHSAVIS (arg_info);
    INFO_LHSAVIS (arg_info) = IDS_AVIS (LET_IDS (arg_node));
    DBUG_PRINT ("looking at %s", AVIS_NAME (INFO_LHSAVIS (arg_info)));
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /* pick up from sons traversal */
    deepest_lvl = depthmask_deepest_level (arg_info);

    /* depedencies on nested variables should have been masked out in N_with */
    DBUG_ASSERT (deepest_lvl <= INFO_WITHDEPTH (arg_info),
                 "expression reported to depend on a nested variable");

    DBUG_PRINT ("expression %s on level %d has max. dep. on level %d (dmask=0x%" PRIu64 "X)",
                AVIS_NAME (INFO_LHSAVIS (arg_info)), INFO_WITHDEPTH (arg_info),
                deepest_lvl, dmask2ui64 (arg_info));

    /* detect withloop-independent expression, will be moved up */
    if ((deepest_lvl < INFO_WITHDEPTH (arg_info))
#ifndef CREATE_UNIQUE_BY_HEAP
        && (!ForbiddenMovement (LET_IDS (arg_node)))
#endif
        && (!((NODE_TYPE (LET_EXPR (arg_node)) == N_with)
              && (INFO_PREASSIGN (arg_info) != NULL)))) {
        /* set new target definition depth */
        INFO_SETDEPTH (arg_info) = deepest_lvl;
        DBUG_PRINT ("moving assignment from depth %d to depth %d",
                    INFO_WITHDEPTH (arg_info), deepest_lvl);

    } else {
        /* set current depth */
        DBUG_PRINT ("changing SETDEPTH: %d -> %d", INFO_SETDEPTH (arg_info),
                    INFO_WITHDEPTH (arg_info));
        INFO_SETDEPTH (arg_info) = INFO_WITHDEPTH (arg_info);
    }

    /* traverse ids to mark them as loop-invariant/local or normal */
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    INFO_LHSAVIS (arg_info) = old_lhsavis;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRid(node *arg_node, info *arg_info)
 *
 * description:
 *   note the with-loop definition depth of the id
 *
 *****************************************************************************/
node *
WLIRid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Traverse AVIS_DIM and friends */
    ID_AVIS (arg_node) = TRAVcont (ID_AVIS (arg_node), arg_info);

    /*
     * calc the maximum definition depth of all identifiers in the
     * current assignment
     */
    DBUG_ASSERT (AVIS_DEFDEPTH (ID_AVIS (arg_node)) != DD_UNDEFINED,
                 "reference to undefined identifier %s", AVIS_NAME (ID_AVIS (arg_node)));

    /* mark the definition level of this N_id, i.e. AVIS_DEFDEPTH(ID_AVIS(arg_node)),
     * in the global bitmask that helps us to establish the maximal level used in an
     * expression */
    depthmask_mark_level (arg_info, AVIS_DEFDEPTH (ID_AVIS (arg_node)));

    DBUG_PRINT ("id %s: DEFDEPTH=%d; SETDEPTH=%d; dmask=0x%" PRIu64 "X",
                AVIS_NAME (ID_AVIS (arg_node)), AVIS_DEFDEPTH (ID_AVIS (arg_node)),
                INFO_SETDEPTH (arg_info), dmask2ui64 (arg_info));

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
    bool *larger_dmask_buf;
    bool *old_dmask;

    DBUG_ENTER ();
    DBUG_ASSERT ((INFO_WITHDEPTH (arg_info) + 2) > 0, "With Loop Depth is invalid.");
    larger_dmask_buf
      = (bool *)MEMmalloc ((size_t)(INFO_WITHDEPTH (arg_info) + 2) * sizeof (bool));
    DBUG_PRINT ("Looking at %s=with...", AVIS_NAME (INFO_LHSAVIS (arg_info)));
    /* clear current InsertListFrame */
    INFO_INSLIST (arg_info)
      = InsListSetAssigns (INFO_INSLIST (arg_info), NULL, INFO_WITHDEPTH (arg_info));

    /* As WITHDEPTH is being incremented below, we must enlarge the DEPTHMASK
     * buffer so that its size stays WITHDEPTH+1 even after the increment.
     * We use a new buffer (larger_dmask_buf) and afterwards we copy its
     * content back. */
    old_dmask = INFO_DEPTHMASK (arg_info);
    copy_dmask (larger_dmask_buf, old_dmask, INFO_WITHDEPTH (arg_info));
    larger_dmask_buf[INFO_WITHDEPTH (arg_info) + 1] = FALSE;
    INFO_DEPTHMASK (arg_info) = larger_dmask_buf;

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

    /* Clear all the nested levels from the depth mask (to detect
     * with-loop dependencies). Nested levels would be moved along,
     * therefore they do not constitute a true dependency that prevents a move.
     * We simply copy back the required number of elements. */
    copy_dmask (old_dmask, INFO_DEPTHMASK (arg_info), INFO_WITHDEPTH (arg_info));
    INFO_DEPTHMASK (arg_info) = old_dmask;

    /* move the assigns of this depth into PREASSIGN */
    INFO_PREASSIGN (arg_info)
      = TCappendAssign (INFO_PREASSIGN (arg_info),
                        InsListGetAssigns (INFO_INSLIST (arg_info),
                                           INFO_WITHDEPTH (arg_info)));
    /* clear this frame */
    INFO_INSLIST (arg_info)
      = InsListSetAssigns (INFO_INSLIST (arg_info), NULL, INFO_WITHDEPTH (arg_info));

    MEMfree (larger_dmask_buf);

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
    DBUG_ENTER ();
    DBUG_PRINT ("setting SETDEPTH: %d -> %d", INFO_SETDEPTH (arg_info),
                INFO_WITHDEPTH (arg_info));

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

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_ids);
    /* set current withloop depth as definition depth */
    AVIS_DEFDEPTH (avis) = INFO_SETDEPTH (arg_info);
    DBUG_PRINT ("Looking at N_ids for %s, with depth %d", AVIS_NAME (avis),
                AVIS_DEFDEPTH (avis));

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
#if 0
node* WLIRdoLoopInvariantRemovalOneFundef(node* fundef)
{
  info *info;
  DBUG_ENTER ();

  DBUG_ASSERT (NODE_TYPE(fundef) == N_fundef,
               "LIRdoLoopInvariantRemovalOneFundef called for non-fundef node");

  info = MakeInfo();

  INFO_TRAVSTART( info) = TS_fundef;

  TRAVpush(TR_wlir);
  fundef = TRAVdo( fundef, info);
  TRAVpop();

  info = FreeInfo( info);

  DBUG_RETURN (fundef);
}
#endif

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
WLIRdoLoopInvariantRemoval (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "WLIRdoLoopInvariantRemoval called with non-module/non-fundef node");

    info = MakeInfo ();

    TRAVpush (TR_wlir);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
