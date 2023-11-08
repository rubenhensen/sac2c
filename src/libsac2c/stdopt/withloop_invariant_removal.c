/******************************************************************************
 *
 * file:   withloop_invariant_removal.c
 *
 * prefix: WLIR
 *
 * description:
 *   This module implements the actual code motion of the withloop invariant
 *   removal on code in ssa form.
 *   It requires WLIRI (withloop_invariant_removal_inference.c) to have been
 *   run before it as WLIRI tags all variables with the WL-level that their
 *   assignment should end up in.
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
 *   Assume a tagged function body of the following form (for details how the
 *   annotation is done please see withloop_invariant_removal_inference.c):
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
 *   This traversal now pulls-up all assignments to their destination places.
 *   While doing so, we need to preserve the order of assignments. However,
 *   WL assignments that are pulled up need to be placed *behind* the inner
 *   assignments of the very WL that are lifted to the same level!
 *   In the above example, this means that that the assignments to v and w
 *   precede that of z, and that q precedes p. Overall, we obtain:
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
 * Implementation notes:
 *   This traversal relies on a correct annotation of all N_avis nodes at
 *   AVIS_DEFDEPTH as established by WLIRI.
 *   The actual lifting of assignments happens in a bottom-up traversal as
 *   this simplifies the assembly of to be moved N_assign chains. During
 *   this traversal, we collect N_assign chains to be inserted directly 
 *   before the WL further up that "created" the nesting level directly
 *   "above" the destination level.
 *   Since we move multiple assignments to multiple destination levels
 *   at the same time, we need as many chains as we have WL nesting levels.
 *   We keep these in an array of node * pointers in INFO_PREASSIGNS.
 *   We preallocate an array of INFO_MAXNESTING elements (initially 16)
 *   and potentially extend this array by further 16 levels should we ever
 *   encounter nestings of higher depth.
 *   There are two key actions: the insertion of assignments into the 
 *   correct destination chain in INFO_PREASSIGNS and the unloading of
 *   destination chains before the WL where the assignment are coming from.
 *   Both these actions happen in the N_assign node during a bottom-up
 *   traversal.
 *   First, the assign peeks into the LHS to find out whether the 
 *   current assignment needs to be moved. To make that decision,
 *   we need to know the actual nesting level. As in WLIRI, we keep
 *   track of the current WL nesting level through INFO_WITHDEPTH.
 *   If the current assignment needs to be moved up
 *   (AVIS_DEFDEPTH < INFO_WITHDEPTH), we prepend it to the N-assign
 *   chain in INFO_PREASSIGNS [AVIS_DEFDEPTH]. Thereafter, we 
 *   traverse into the RHS. Iff the RHS is a WL, just before we 
 *   return, we set INFO_PREASSIGN to INFO_PREASSIGNS [INFO_WITHDEPTH].
 *   This triggers the Assign code to insert those assignments directly
 *   before the current arg_node, be it the original one or the previously
 *   ASSIGN_NEXT, if the entire WL is being lifted.
 *   
 *****************************************************************************/
#include "withloop_invariant_removal.h"

#define DBUG_PREFIX "WLIR"
#include "debug.h"

#include "tree_basic.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "tree_compound.h"
#include "globals.h"

/******************************************************************************
 *
 * @struct INFO
 *
 * @param PREASSIGNS          array of INFO_MAXNESTING length that harbours
 *                            pointers to N_assign chains
 * @param MAXNESTING          current length of allocated array
 * @param PREASSIGN           assignments to be inserted before the current N_assign
 * @param WITHDEPTH           current nesting level
 *
 ******************************************************************************/

struct INFO {
    node **preassigns;
    int maxnesting;
    node *preassign;
    int withdepth;
};

/*
 * INFO macros
 */
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_MAXNESTING(n) (n->maxnesting)
#define INFO_WITHDEPTH(n) (n->withdepth)

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

    INFO_MAXNESTING (result) = NST_CHUNK_SZ;
    INFO_PREASSIGNS (result) = (node **)MEMmalloc (sizeof (node *)
                                            * (size_t)INFO_MAXNESTING (result));
    INFO_PREASSIGN (result) = NULL;
    INFO_WITHDEPTH (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    MEMfree (INFO_PREASSIGNS (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
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
    int i;

    DBUG_ENTER ();

    DBUG_PRINT ("loop-invariant removal in fundef %s", FUNDEF_NAME (arg_node));

    INFO_WITHDEPTH (arg_info) = 0;

    for (i=0; i< INFO_MAXNESTING (arg_info); i++)
        INFO_PREASSIGNS (arg_info)[i] = NULL;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* WLIRassign(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse assign instructions in bottom-up order, move N_assign into
 *   INFO_PREASSIGNS if needed and potentially insert chain if traversal
 *   of ASSIGN_STMT sets INFO_PREASSIGN.
 *
 *****************************************************************************/
node *
WLIRassign (node *arg_node, info *arg_info)
{
    node *let, *assign;
    int destlevel;

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /*
     * Here, we peek into the LHS (avoiding a micro-traversal here!)
     * in order to see whether this particular N_assign needs to be lifted.
     * If so, we prepend the current assign to the corresponding chain in
     * INFO_PREASSIGNS.
     */
    assign = arg_node;
    let = ASSIGN_STMT (arg_node);
    if ((NODE_TYPE (let) == N_let) && (LET_IDS (let) != NULL)) {
        destlevel = AVIS_DEFDEPTH (IDS_AVIS (LET_IDS (let)));
        if (destlevel < INFO_WITHDEPTH (arg_info)) {
            /*
             * we need to take arg_node out of the N_assign chain
             * and prepend to the correct INFO_PREASSIGNS:
             */
            DBUG_ASSERT (destlevel >= 0, "It seems you try to run WLIR without"
                                         " running WLIRI first!");
            arg_node = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (assign) = INFO_PREASSIGNS (arg_info)[destlevel];
            INFO_PREASSIGNS (arg_info)[destlevel] = assign;
            global.optcounters.wlir_expr++;
        }
        /* set the defdepth to the default value to enable
         * the DBUG_ASSERTION above to work even if WLIR
         * is called multiple times!
         */
        AVIS_DEFDEPTH (IDS_AVIS (LET_IDS (let))) = -1;
    }

    /* Now, we potentially traverse into the WL */
    ASSIGN_STMT (assign) = TRAVdo (ASSIGN_STMT (assign), arg_info);

    /* if the RHS is a WL, it may trigger an insertion here! */
    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
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
 *   move all to-be-lifted-in-front-of-this-WL into INFO_PREASSIGN
 *
 *****************************************************************************/
node *
WLIRwith (node *arg_node, info *arg_info)
{
    node **assigns;
    int i;

    DBUG_ENTER ();

    INFO_WITHDEPTH (arg_info) += 1;
    if (INFO_WITHDEPTH (arg_info) >= INFO_MAXNESTING (arg_info)) {
        INFO_MAXNESTING (arg_info) += NST_CHUNK_SZ;
        assigns = (node **)MEMmalloc (sizeof (node *)
                                     * (size_t)INFO_MAXNESTING (arg_info));
        for (i=0; i< INFO_MAXNESTING (arg_info) - NST_CHUNK_SZ; i++)
            assigns[i] = INFO_PREASSIGNS (arg_info)[i];
        for (; i< INFO_MAXNESTING (arg_info); i++)
            assigns[i] = NULL;
        INFO_PREASSIGNS (arg_info) = MEMfree (INFO_PREASSIGNS (arg_info));
        INFO_PREASSIGNS (arg_info) = assigns;
    }

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_WITHDEPTH (arg_info) -= 1;

    INFO_PREASSIGN (arg_info) = INFO_PREASSIGNS (arg_info) [INFO_WITHDEPTH (arg_info)];

    INFO_PREASSIGNS (arg_info) [INFO_WITHDEPTH (arg_info)] = NULL;

    DBUG_RETURN (arg_node);
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
