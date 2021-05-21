/** <!--********************************************************************-->
 *
 * @defgroup wls With-Loop Scalarization
 *
 * @brief With-Loop Scalarization is a high-level optimization which composes
 *        a single withloop from nested ones, in order to minimize memory
 *        transactions and thereby improving program efficiency.
 *
 * A bottom-up traversal of the SAC-program is performed.
 * When a with-loop is met, its codes are traversed first in order to
 * scalarize nested with-loops inside of these codes.
 *
 * Scalarization is then performed by means of three separate traversals:
 *
 *  - WLSCheck checks whether all criteria are met to scalarize the current WL.
 *    These criteria are described in wlscheck.c
 *
 *  - WLSWdoWithloopify modifies a with-loop's codes in order to create
 *    the pattern of perfectly nested with-loops, as seen in the example
 *    below.
 *
 *  - WLSBuild then finally replaces the with-loop nesting with a new,
 *    scalarized version of that with-loop
 *
 *
 * An example for with-loop scalarization is given below:
 *
 * <pre>
 *   A = with ( lb_1 <= iv < ub_1 ) {
 *         B = with ( lb_2 <= jv < ub_2 ) {
 *               val = expr( iv, jv);
 *             } : val
 *             genarray( shp_2);
 *       } : B
 *       genarray( shp_1);
 * </pre>
 *
 *   is transformed into
 *
 * <pre>
 *   A = with ( (lb_1++lb_2) <= kv < (ub_1++ub_2)) {
 *         iv = take( shape( lb_1), kv);
 *         jv = drop( shape( lb_1), kv);
 *         val = expr( iv, jv);
 *       } : val
 *       genarray( shp_1++shp_2);
 * </pre>
 *
 *
 *
 * An example for with-loop scalarization when -ssaiv is
 * active is given below. NB the behavior of the
 * multiple generators.
 *
 * <pre>
 *   A = with
 *       ( ivl <= iv < ivu ) {
 *         B = with
 *             ( jvl <= jv < jvu ) {
 *               val = expr( iv, jv);
 *             } : val;
 *
 *             ( kvl <= kv < kvu) {
 *               val' = expr(iv, kv);
 *             } : val';
 *             genarray( shp_2);
 *       } : B
 *       genarray( shp_1);
 * </pre>
 *
 *   is transformed into
 *
 * <pre>
 *   A = with
 *     ( (ivl++jvl) <= mv < (ivu++jvu)) {
 *         iv' = take( shape( ivl), mv);
 *         jv' = drop( shape( ivl), mv);
 *         val = expr( iv', jv');
 *       } : val;
 *     ( (ivl++kvl) <= nv < (ivu++kvu)) {
 *         iv'' = take( shape( ivl), nv);
 *         kv'' = drop( shape( ivl), nv);
 *         val = expr( iv'', kv'');
 *       } : val;
 *       genarray( shp_1++shp_2);
 * </pre>
 *
 *
 * NB. Some of the functions of WLS are now handled by IPC,
 *     but WLS still enables WLF/AWLF to do more work, so
 *     IPC is not a complete replacement for WLS.
 *     See Bug #1086 for an example of this, or look at
 *     ~/sac/testsuite/optimizations/ipc/bug1086.sac.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file wls.c
 *
 * Prefix: WLS
 *
 *****************************************************************************/
#include "wls.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "WLS"
#include "debug.h"

#include "new_types.h"
#include "print.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "shape.h"
#include "phase.h"
#include "flattengenerators.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *preassigns;
    node *nassign; /* for DBUG_PRINT clarification only */
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_NASSIGN(n) (n->nassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_NASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 *  NB. THIS IS ALMOST A CLONE OF CODE FROM ../flatten/WLPartitionGeneration.c.
 *      Perhaps we need a single function to do this stuff?
 *
 * @fn node *WLSflattenBound( node *arg_node, node **preassigns, node **vardecs)
 *
 *   @brief  Flattens the WL bound at arg_node.
 *           I.e., if the generator looks like this on entry:
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            z = with {
 *             (. <= iv < [s0, s1]) ...
 *            }
 *
 *          it will look like this on the way out:
 *            int[2] TMP;
 *            ...
 *            s0 = _idx_shape_sel(0,x);
 *            s1 = _idx_shape_sel(1,x);
 *            TMP = [s0, s1];
 *            z = with {
 *             (. <= iv < TMP) ...
 *            }
 *
 *          The only rationale for this change is to ensure that
 *          WL bounds are named. This allows us to associate an
 *          N_avis node with each bound, which will be used to
 *          store AVIS_MIN and AVIS_MAX for the bound.
 *          These fields, in turn, will be used by the constant
 *          folder to remove guards and do other swell optimizations.
 *
 *   @param  node *arg_node: a WL PART BOUND to be flattened.
 *           node **vardecs: The address of the vardecs chain.
 *           node **preassigns: The address of a preassigns chain.
 *
 *   @return node *node: The N_avis of the flattened arg_node.
 *
 ******************************************************************************/
node *
WLSflattenBound (node *arg_node, node **vardecs, node **preassigns)
{
    node *avis;

    DBUG_ENTER ();

    if (N_array == NODE_TYPE (arg_node)) {
        avis = FLATGexpression2Avis (DUPdoDupTree (arg_node), vardecs, preassigns, NULL);
    } else {
        DBUG_ASSERT (N_id == NODE_TYPE (arg_node), "Expected N_id or N_array");
        avis = ID_AVIS (arg_node);
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLSdoWithloopScalarization( node *fundef)
 *
 * @brief starting point of WithloopScalarization.
 *
 * @param fundef Fundef-Node to start WLS in.
 *
 * @return modified fundef.
 *
 *****************************************************************************/
node *
WLSdoWithloopScalarization (node *fundef)
{
    info *arg_info;
    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef) || (NODE_TYPE (fundef) == N_module),
                 "WLSdoWithloopScalarization called for non-fundef/module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_wls);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLSassign(node *arg_node, info *arg_info)
 *
 * @brief performs a bottom-up traversal.
 *
 *****************************************************************************/
node *
WLSassign (node *arg_node, info *arg_info)
{
    node *oldnassign;

    DBUG_ENTER ();

    /*
     * Bottom-up traversal
     */

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_ASSERT (INFO_PREASSIGNS (arg_info) == NULL, "left-over pre-assigns found!");
    /*
     * Traverse RHS
     */
    oldnassign = INFO_NASSIGN (arg_info);
    INFO_NASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_NASSIGN (arg_info) = oldnassign;

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSfundef(node *arg_node, info *arg_info)
 *
 * @brief applies WLS to a given fundef.
 *        An INFO-structure containing a pointer to the current fundef is
 *        created before traversal of the function body.
 *
 ******************************************************************************/
node *
WLSfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * traverse block of fundef
         */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    old_fundef = INFO_FUNDEF (arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    INFO_FUNDEF (arg_info) = old_fundef;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    INFO_FUNDEF (arg_info) = old_fundef;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSwith(node *arg_node, info *arg_info)
 *
 * @brief applies WLS to a with-loop in a bottom-up manner.
 *        This means, inner with-loops are scalarized first by traversing the
 *        with-loop's codes.
 *        After that it is checked whether WLS is applicable using the
 *        WLSCheck traversal.
 *        If it is, the base case of scalarization (perfectly nested
 *        with-loops) is created using WLSWdoWithloopify.
 *        Thereafter, a new with-loop is built using the WLSBuild traversal.
 *
 *****************************************************************************/
node *
WLSwith (node *arg_node, info *arg_info)
{
    size_t innerdims;

    DBUG_ENTER ();

    /*
     * First, traverse all the codes in order to apply WLS
     * to inner with-loops
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /*
     * Afterwards, try to scalarize the current with-loop
     */
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));

    /*
     * Scalarization is possible iff WLSCheck does not return 0
     */
    innerdims = WLSCdoCheck (arg_node, INFO_NASSIGN (arg_info));

    if (innerdims > 0) {

        /*
         * Apply withloopification
         */
        arg_node = WLSWdoWithloopify (arg_node, INFO_FUNDEF (arg_info), innerdims);

        /*
         * Build the new with-loop
         */
        arg_node
          = WLSBdoBuild (arg_node, INFO_FUNDEF (arg_info), &INFO_PREASSIGNS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverse only funs in the module.
 *
 *****************************************************************************/
node *
WLSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- WLS -->
 *****************************************************************************/

#undef DBUG_PREFIX
