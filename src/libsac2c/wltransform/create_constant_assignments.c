/*****************************************************************************
 *
 * @defgroup
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_constant_assignment.c
 *
 * Prefix: CNSTASS
 *
 *****************************************************************************/
#include "create_constant_assignments.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "str.h"
#include "flattengenerators.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *constassigns;
    node *fundef;
    bool collect;
};

#define INFO_CONSTASSIGNS(n) (n->constassigns)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_COLLECT(n) (n->collect)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_CONSTASSIGNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_COLLECT (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CNSTASSdoCUDAconstantAssignment( node *syntax_tree)
 *
 *****************************************************************************/
node *
CNSTASSdoCUDAconstantAssignment (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CNSTASSdoCUDAconstantAssignment");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    /* This traversal makes sure that all lower bounds
     * and upper bounds of a generator are now N_ids.
     */
    syntax_tree = FLATGdoFlatten (syntax_tree);

    info = MakeInfo ();
    TRAVpush (TR_cnstass);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node* UnflattenGeneratorComponent( node *host_avis)
 *
 * @brief
 *
 *****************************************************************************/
static node *
UnflattenGeneratorComponent (node *id)
{
    node *ssaassign;
    node *res;

    DBUG_ENTER ("UnflattenGeneratorComponent");

    ssaassign = AVIS_SSAASSIGN (ID_AVIS (id));

    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array),
                 "Unflattened generator component must be an N_array node!");

    id = FREEdoFreeNode (id);
    res = DUPdoDupNode (ASSIGN_RHS (ssaassign));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* FlattenBoundStepWidthElements( node *host_avis)
 *
 * @brief
 *
 *****************************************************************************/
static void
FlattenBoundStepWidthElements (node *exprs, char *suffix, info *arg_info)
{
    node *avis;
    node *ids, *vardec;

    DBUG_ENTER ("UnflattenBoundStepWidthElements");

    while (exprs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_num) {
            avis = TBmakeAvis (TRAVtmpVarName (suffix),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
            vardec = TBmakeVardec (avis, NULL);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardec);

            ids = TBmakeIds (avis, NULL);

            INFO_CONSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, EXPRS_EXPR (exprs)),
                              INFO_CONSTASSIGNS (arg_info));
            EXPRS_EXPR (exprs) = TBmakeId (avis);
        }
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CNSTASSfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CNSTASSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CNSTASSassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
CNSTASSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_CONSTASSIGNS (arg_info) != NULL && !INFO_COLLECT (arg_info)) {
        arg_node = TCappendAssign (INFO_CONSTASSIGNS (arg_info), arg_node);
        INFO_CONSTASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CNSTASSwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
CNSTASSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSwith");

    /* Only traverse cudarizable WL */
    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_COLLECT (arg_info) = TRUE;
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_COLLECT (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CNSTASSgenerator( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
CNSTASSgenerator (node *arg_node, info *arg_info)
{
    node *lower_bound_elements = NULL, *upper_bound_elements = NULL;
    node *step_elements = NULL, *width_elements = NULL;

    DBUG_ENTER ("CNSTASSgenerator");

    DBUG_ASSERT ((NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_id),
                 "Lower bound should be an N_id node!");
    DBUG_ASSERT ((NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_id),
                 "Upper bound should be an N_id node!");

    GENERATOR_BOUND1 (arg_node)
      = UnflattenGeneratorComponent (GENERATOR_BOUND1 (arg_node));
    GENERATOR_BOUND2 (arg_node)
      = UnflattenGeneratorComponent (GENERATOR_BOUND2 (arg_node));

    lower_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
    upper_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node));

    DBUG_ASSERT (TCcountExprs (lower_bound_elements)
                   == TCcountExprs (upper_bound_elements),
                 "Lower and upper bound must have same number of elements");

    FlattenBoundStepWidthElements (upper_bound_elements, "ub", arg_info);
    FlattenBoundStepWidthElements (lower_bound_elements, "lb", arg_info);

    /* Handle Step and Width */

    if (GENERATOR_STEP (arg_node) != NULL) {
        GENERATOR_STEP (arg_node)
          = UnflattenGeneratorComponent (GENERATOR_STEP (arg_node));
        step_elements = ARRAY_AELEMS (GENERATOR_STEP (arg_node));
    }

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        GENERATOR_WIDTH (arg_node)
          = UnflattenGeneratorComponent (GENERATOR_WIDTH (arg_node));
        width_elements = ARRAY_AELEMS (GENERATOR_WIDTH (arg_node));
    }

    FlattenBoundStepWidthElements (step_elements, "step", arg_info);
    FlattenBoundStepWidthElements (width_elements, "width", arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
