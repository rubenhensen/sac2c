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

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
#include "deadcoderemoval.h"
#include "constants.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *constassigns;
    node *fundef;
    bool in_cudawl;
    bool add_assigns;
};

#define INFO_CONSTASSIGNS(n) (n->constassigns)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDAWL(n) (n->in_cudawl)
#define INFO_ADD_ASSIGNS(n) (n->add_assigns)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CONSTASSIGNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_ADD_ASSIGNS (result) = FALSE;

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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    /* This traversal makes sure that all lower bounds
     * and upper bounds of a generator are now N_ids.
     */
    // syntax_tree = FLATGdoFlatten( syntax_tree);

    info = MakeInfo ();
    TRAVpush (TR_cnstass);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

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

    DBUG_ENTER ();

    DBUG_ASSERT (TYisAKS (AVIS_TYPE (ID_AVIS (id))),
                 "Non-AKS N_with generator component found!");
    if (NODE_TYPE (AVIS_DECL (ID_AVIS (id))) == N_arg) {
        DBUG_ASSERT (FALSE,"N_id bound through argument not supported yet");
        res = COconstant2AST (TYgetValue (AVIS_TYPE (ID_AVIS (id))));
        id = FREEdoFreeNode (id);
    } else {
        ssaassign = AVIS_SSAASSIGN (ID_AVIS (id));

        DBUG_ASSERT (NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array,
                     "Unflattened generator component must be an N_array node!");

        id = FREEdoFreeNode (id);
        res = DUPdoDupNode (ASSIGN_RHS (ssaassign));
    }

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

    DBUG_ENTER ();

    while (exprs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_num) {
            avis = TBmakeAvis (TRAVtmpVarName (suffix),
                               TUint2akv (NUM_VAL (EXPRS_EXPR (exprs))));
            vardec = TBmakeVardec (avis, NULL);
            AVIS_DECL (avis) = vardec;

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)), vardec);

            ids = TBmakeIds (avis, NULL);

            INFO_CONSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, EXPRS_EXPR (exprs)),
                              INFO_CONSTASSIGNS (arg_info));
            EXPRS_EXPR (exprs) = TBmakeId (avis);
            AVIS_SSAASSIGN (avis) = INFO_CONSTASSIGNS (arg_info);
        }
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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
    bool old_add_assigns;

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    old_add_assigns = INFO_ADD_ASSIGNS (arg_info);
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_CONSTASSIGNS (arg_info) != NULL && INFO_ADD_ASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_CONSTASSIGNS (arg_info), arg_node);
        INFO_CONSTASSIGNS (arg_info) = NULL;
    }

    INFO_ADD_ASSIGNS (arg_info) = old_add_assigns;

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
    node *old_assigns;

    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        old_assigns = INFO_CONSTASSIGNS (arg_info);

        INFO_CONSTASSIGNS (arg_info) = NULL;
        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INCUDAWL (arg_info) = FALSE;
        INFO_CONSTASSIGNS (arg_info) = old_assigns;

        INFO_ADD_ASSIGNS (arg_info) = TRUE;
    } else if (INFO_INCUDAWL (arg_info)) {
        /* For N_with in cudarizable N_with, we only traverse its N_part */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_ADD_ASSIGNS (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CNSTASSwiths( node *arg_node, info *arg_info)
 *
 * @brief We have to traverse all the distributed with-loops before adding the
 * assignments. We save the decision and reset before traversing the next
 * with-loop.
 *
 *
 *****************************************************************************/
node *
CNSTASSwiths (node *arg_node, info *arg_info)
{
    bool add_assigns;

    DBUG_ENTER ();

    WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);

    add_assigns = INFO_ADD_ASSIGNS (arg_info);
    INFO_ADD_ASSIGNS (arg_info) = FALSE;

    WITHS_NEXT (arg_node) = TRAVopt (WITHS_NEXT (arg_node), arg_info);

    INFO_ADD_ASSIGNS (arg_info) = add_assigns;

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

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_id
                  || NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_array),
                 "Lower bound is neither N_id nor N_array!");
    DBUG_ASSERT ((NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_id
                  || NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_array),
                 "Upper bound is neither N_id nor N_array!");

    if (NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_id) {
        GENERATOR_BOUND1 (arg_node)
          = UnflattenGeneratorComponent (GENERATOR_BOUND1 (arg_node));
    }

    if (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_id) {
        GENERATOR_BOUND2 (arg_node)
          = UnflattenGeneratorComponent (GENERATOR_BOUND2 (arg_node));
    }

    lower_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
    upper_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node));

    DBUG_ASSERT (TCcountExprs (lower_bound_elements)
                   == TCcountExprs (upper_bound_elements),
                 "Lower and upper bound must have same number of elements");

    FlattenBoundStepWidthElements (upper_bound_elements, "ub", arg_info);
    FlattenBoundStepWidthElements (lower_bound_elements, "lb", arg_info);

    /* Handle Step and Width */

    if (GENERATOR_STEP (arg_node) != NULL) {
        if (NODE_TYPE (GENERATOR_STEP (arg_node)) == N_id) {
            GENERATOR_STEP (arg_node)
              = UnflattenGeneratorComponent (GENERATOR_STEP (arg_node));
        }
        step_elements = ARRAY_AELEMS (GENERATOR_STEP (arg_node));
    }

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        if (NODE_TYPE (GENERATOR_WIDTH (arg_node)) == N_id) {
            GENERATOR_WIDTH (arg_node)
              = UnflattenGeneratorComponent (GENERATOR_WIDTH (arg_node));
        }
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

#undef DBUG_PREFIX
