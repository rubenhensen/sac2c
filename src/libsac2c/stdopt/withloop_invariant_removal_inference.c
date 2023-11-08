/******************************************************************************
 *
 * file:   withloop_invariant_removal_inference.c
 *
 * prefix: WLIRI
 *
 * description:
 *   This module implements the analysis phase of withloop invariant removal
 *   on code in ssa form.
 *   The overall idea is that this traversal tags all variables with the 
 *   WL-level that their assignment should end up in. The actual movement
 *   of assignments happens in WLIR (withloop_invariant_removal.c) itself.
 *
 *   We use two phases instead of one because the tagging needs to happen
 *   top-down, whereas the actual movement benefits from a bottom-up 
 *   traversal. Due to the tree-like nesting of WLs, these two cannot 
 *   easily be combined: if we were to combine them, the traversal
 *   within one WL would need to perform all movement within that body
 *   already while performing the overall top down traversal!
 *   While we could maintain the required movement info locally, this adds
 *   quite some overhead and complexity which can be avoided by separating
 *   it into two independent traversals.
 *
 * overall effect:
 *   
 *   We annotate all variables (in their AVIS) with the WL-depth that we
 *   foresee for the lifted program!
 *   In case of multiple levels of a nested WL, we annotate the minimal level
 *   possible. For example, we annotate
 *
 *   c = 5;
 *   x = with(..iv1..) {
 *       y = expr(5,c);
 *       z = with(..iv2..) {
 *           u = expr(y,iv2);
 *           v = expr(c);
 *           w = expr(c,v);
 *           p = with(..iv3..) {
 *               q = expr(c,u);
 *               } op(..);
 *           } op(...);
 *       } op(...);
 *
 *   as follows:
 *
 *   c<0> = 5;
 *   x<0> = with(..iv1<1>..) {
 *          y<0> = expr(5,c);
 *          z<0> = with(..iv2<2>..) {
 *                 u<2> = expr(y,iv2);
 *                 v<0> = expr(c);
 *                 w<0> = expr(c,v);
 *                 p<2> = with(..iv3<3>..) {
 *                        q<2> = expr(c,u);
 *                        } op(..);
 *                 } op(...);
 *             } op(...);
 *
 *   In general, the idea is that we annotate the LHS of an assignment 
 *   with the maximum level of the variables being referred to in the RHS.
 *   For entire with-loops this is a bit more involved. In essence, we
 *   identify the maximum of all referenced variables in the body, but we
 *   exclude the WL-local ones. For details see the implementation
 *   description below.
 *
 *   In the WLIR phase, we then move all tagged assignments to their
 *   destination while preserving the order within any given tag, yet
 *   lifting inner assignments *before* their WL on the corresponding
 *   level:
 *
 *   c<0> = 5;
 *   y<0> = expr(5,c);
 *   v<0> = expr(c);
 *   w<0> = expr(c,v);
 *   z<0> = with(..iv2<2>..) {
 *          u<2> = expr(y,iv2);
 *          q<2> = expr(c,u);
 *          p<2> = with(..iv3<3>..) {
 *                 } op(...);
 *   x<0> = with(..iv2<1>..) {
 *          } op(...);
 *
 *   For details on this actual lifting see withloop_invariant_removal.c.
 *
 * Implementation notes:
 *
 *    All this traversal does is to annotate all N_avis nodes at AVIS_DEFDEPTH.
 *    This is done in a top-down traversal. While doing so, we keep the 
 *    current WL nesting level in INFO_WITHDEPTH. On every assignment, we 
 *    start with INFO_SETDEPTH = 0. While traversing the RHS, we infer the 
 *    level where this very assignment should go. Once the RHS is done,
 *    we store the found INFO_SETDEPTH in the AVIS_DEFDEPTH of all LHS vars.
 *
 *    The RHS traversal works as follows:
 *    For constants INFO_SETDEPTH stays unmodified.
 *    For variables we take the maximum of INFO_SETDEPTH and the AVIS_DEFDEPTH.
 *    For WLs, this is more involved:
 *      - we increment INFO_WITHDEPTH and tag the idx-var's AVIS_DEFDEPTH's 
 *        accordingly.
 *      - we create / stack an INFO_WLSETDEPTH counter which we initialise 0.
 *      - during the traversal of the code, we now update INFO_WLSETDEPTH 
 *        alongside with INFO_SETDEPTH. However, in contrast to INFO_SETDEPTH,
 *        INFO_WLSETDEPTH ignores tags that are identical to INFO_WITHDEPTH.
 *      - once we come back to the WL, we use INFO_WLSETDEPTH to set
 *        INFO_SETDEPTH for the entire WL, and we pop INFO_WLSETDEPTH.
 *
 *    Unfortunately, WLs are even more involved. It does not suffice to
 *    update the current INFO_WLSETDEPTH, but we need to also update
 *    surrounding ones. An example for this case is:
 *
 *   c<0> = expr(5);
 *   x<0> = with { (. <= iv1<1> <= .) {
 *          y<?> = with { (. <= iv2<2> <= .) {    <= this annotation is the
 *                                                   challenge! We need to end
 *                                                   up with a 1, not a 0, here!
 *                   z<2> = with { (. <= iv3<3> <= .) {
 *                            q<2> = expr(iv1, iv2);    <= Here, we infer <2>
 *                                                         Unfortunately, this
 *                                                         shadows the <1> of iv1
 *                                                         which has to impact
 *                                                         INFO_WLSETDEPTH for y!
 *                                 If we do not observe this, we would end
 *                                 up lifting y to level 0 while being forced
 *                                 to keep this assignment within the lifted
 *                                 WL. This means we would reference iv1 from
 *                                 outside of the WL defining x!
 *                       } : q;
 *                    } : genarray([10]);
 *                 } : expr (5, z);
 *              } :genarray([10]);
 *           } : expr (5, y);
 *        } :genarray([10]);
 *
 *   To deal with such shadowing effects properly, we make INFO_WLSETDEPTH an
 *   array of counters, allowing us to affect all INFO_WLSETDEPTH's that are
 *   potentially effected.
 *
 *****************************************************************************/
#include "withloop_invariant_removal_inference.h"

#define DBUG_PREFIX "WLIRI"
#include "debug.h"

#include "tree_basic.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "tree_compound.h"

/******************************************************************************
 *
 * @struct INFO
 *
 * @param WITHDEPTH           current WL nesting level
 * @param SETDEPTH            where the current assignment should be placed
 * @param WLSETDEPTH          where the surrounding WLs should be placed
 * @param MAXNESTING          length of the INFO_WLSETDEPTH array
 *
 ******************************************************************************/

struct INFO {
    int withdepth;
    int setdepth;
    int *wlsetdepth;
    int maxnesting;
};

/*
 * INFO macros
 */
#define INFO_WITHDEPTH(n) (n->withdepth)
#define INFO_SETDEPTH(n) (n->setdepth)
#define INFO_WLSETDEPTH(n) (n->wlsetdepth)
#define INFO_MAXNESTING(n) (n-> maxnesting)

#define NST_CHUNK_SZ 16

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_WITHDEPTH (result) = 0;
    INFO_SETDEPTH (result) = 0;
    INFO_MAXNESTING (result) = NST_CHUNK_SZ;
    INFO_WLSETDEPTH (result) = (int *)MEMmalloc (sizeof (int) 
                                       * (size_t)INFO_MAXNESTING (result));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    MEMfree (INFO_WLSETDEPTH (info));
    info = MEMfree (info);

    DBUG_RETURN (info);
}


/* traversal functions */

/******************************************************************************
 *
 * function:
 *   node* WLIRImodule(node *arg_node, info *arg_info)
 *
 * description:
 *   Traverse only funs in the module.
 *
 *****************************************************************************/
node *
WLIRImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRIfundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
WLIRIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("processing fundef %s", FUNDEF_NAME (arg_node));

    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    /* top level (not [directly] contained in any withloop) */
    INFO_WITHDEPTH (arg_info) = 0;

    /* traverse function body */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /* traverse local funs and next */
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRIarg(node *arg_node, info *arg_info)
 *
 * description:
 *   Set definition depth of each argument to 0
 *
 *****************************************************************************/
node *
WLIRIarg (node *arg_node, info *arg_info)
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
 *   node* WLIRIassign(node *arg_node, info *arg_info)
 *
 * description:
 *   top-down!
 *
 *****************************************************************************/
node *
WLIRIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRIlet(node *arg_node, info *arg_info)
 *
 * description:
 *   steer the INFO_SETDEPTH inference.
 *
 *****************************************************************************/
node *
WLIRIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SETDEPTH (arg_info) = 0;

    // compute INFO_SETDEPTH during RHS traversal
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    // use INFO_SETDEPTH to set AVIS_DEFDEPTH
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRIid(node *arg_node, info *arg_info)
 *
 * description:
 *   potentially update INFO_SETDEPTH and INFO_WLSETDEPTH.
 *
 *****************************************************************************/
node *
WLIRIid (node *arg_node, info *arg_info)
{
    int i, depth;

    DBUG_ENTER ();

    /* Traverse AVIS_DIM and friends */
    ID_AVIS (arg_node) = TRAVcont (ID_AVIS (arg_node), arg_info);

    depth = AVIS_DEFDEPTH(ID_AVIS(arg_node));
    DBUG_PRINT ("    found %s on %d", ID_NAME (arg_node), depth);

    /* update the current assignment level if needed */
    if (depth > INFO_SETDEPTH (arg_info)) {
        DBUG_PRINT ("    updating SETDEPTH from %d to %d",
                     INFO_SETDEPTH (arg_info), depth);
        INFO_SETDEPTH (arg_info) = depth;
    }

    /*
     * Now, we update all surrounding WL levels that are potentially 
     * affected! Since the id is (or at least will be) defined on level
     * "depth", all deeper levels between that level and the current 
     * level need to know that this reference exists. The level "depth"
     * itself does not need to know as it anyways will consider this
     * as "local". 
     */
    for (i = INFO_WITHDEPTH (arg_info); i > depth; i--) {
        if (depth > INFO_WLSETDEPTH (arg_info)[i]) {
            DBUG_PRINT ("    updating WLSETDEPTH[%d] from %d to %d", i,
                        INFO_WLSETDEPTH (arg_info)[i],  depth);
            INFO_WLSETDEPTH (arg_info)[i] = depth;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRIwith(node *arg_node, info *arg_info)
 *
 * description:
 *   traverses with-loop, handles INFO_WLSETDEPTH.
 *
 *****************************************************************************/
node *
WLIRIwith (node *arg_node, info *arg_info)
{
    int *wlsetdepths;
    int i;

    DBUG_ENTER ();

    INFO_WITHDEPTH (arg_info) += 1;

    if (INFO_WITHDEPTH (arg_info) >= INFO_MAXNESTING (arg_info)) {
        INFO_MAXNESTING (arg_info) += NST_CHUNK_SZ;
        wlsetdepths = (int *)MEMmalloc (sizeof (int)
                                     * (size_t)INFO_MAXNESTING (arg_info));
        for (i=0; i< INFO_MAXNESTING (arg_info) - NST_CHUNK_SZ; i++)
            wlsetdepths[i] = INFO_WLSETDEPTH (arg_info)[i];
        for (; i< INFO_MAXNESTING (arg_info); i++)
            wlsetdepths[i] = 0;
        INFO_WLSETDEPTH (arg_info) = MEMfree (INFO_WLSETDEPTH (arg_info));
        INFO_WLSETDEPTH (arg_info) = wlsetdepths;
    }

    INFO_WLSETDEPTH (arg_info)[INFO_WITHDEPTH (arg_info)] = 0;
    DBUG_PRINT ("  entering WL level %d", INFO_WITHDEPTH (arg_info));

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_PRINT ("  leaving WL level %d", INFO_WITHDEPTH (arg_info));
    INFO_SETDEPTH (arg_info) = INFO_WLSETDEPTH (arg_info)[INFO_WITHDEPTH (arg_info)];
    INFO_WITHDEPTH (arg_info) -= 1;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRIwithid(node *arg_node, info *arg_info)
 *
 * description:
 *   mark index vectors as local variables
 *
 *****************************************************************************/
node *
WLIRIwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SETDEPTH (arg_info) = INFO_WITHDEPTH (arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *WLIRIids (node *arg_ids, info *arg_info)
 *
 * description:
 *   set current withloop depth as ids definition depth
 *   set current movement flag as ids LIRMOV flag
 *
 *****************************************************************************/
node *
WLIRIids (node *arg_ids, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_ids);
    AVIS_DEFDEPTH (avis) = INFO_SETDEPTH (arg_info);
    DBUG_PRINT ("  setting %s to %d", AVIS_NAME (avis), AVIS_DEFDEPTH (avis));

    IDS_NEXT (arg_ids) = TRAVopt (IDS_NEXT (arg_ids), arg_info);

    DBUG_RETURN (arg_ids);
}


/******************************************************************************
 *
 * function:
 *   node* WLIRIdoLoopInvariantRemovalInference(node* module)
 *
 * description:
 *   starts the loop invariant removal inference.
 *
 *****************************************************************************/
node *
WLIRIdoLoopInvariantRemovalInference (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module) || (NODE_TYPE (arg_node) == N_fundef),
                 "WLIRIdoLoopInvariantRemovalInference called with non-module/non-fundef node");

    info = MakeInfo ();

    TRAVpush (TR_wliri);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
