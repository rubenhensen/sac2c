/** <!--********************************************************************-->
 *
 * @defgroup racc Remove Alias Results from Conformity Checks
 *
 * Various conformity check operations yield aliases their arguments in order
 * to prevent instruction sequences from being shuffled around.
 * At the same time, they return a proper boolean result.
 * To use the memory management's fill operation, we strip the alias results
 * from the following operations:
 *   - type_constraint
 *   - same_shape_AxA
 *   - shape_matches_dim_VxA
 *   - non_neg_val_V
 *   - non_neg_val_S
 *   - val_lt_shape_VxA
 *   - val_le_val_VxV
 *   - val_le_val_SxS
 *   - val_lt_val_SxS
 *   - prod_matches_prod_shape_VxA
 *   - guard
 * However, as the LHS-alias might have a more precise type, we introduce
 * aliasing assignments to trigger a conversion of the data representation
 * in the backend.
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file rm_alias_results_cc.c
 *
 * Prefix: RACC
 *
 *****************************************************************************/
#include "rm_alias_results_cc.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "RACC"
#include "debug.h"

#include "traverse.h"
#include "new_types.h"
#include "type_utils.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "memory.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *let;
    node *postassign;
};

/**
 * A pointer to the last LET node
 */
#define INFO_LET(n) ((n)->let)
#define INFO_POSTASSIGN(n) ((n)->postassign)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LET (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;

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
 * @fn node *EMRACCdoRemoveAliasResultsFromConformityChecks( node *syntax_tree)
 *
 *****************************************************************************/
node *
EMRACCdoRemoveAliasResultsFromConformityChecks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting to remove Alias Results from Conformity Checks.");

    TRAVpush (TR_emracc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("Removal of Alias Results from Conformity Checks complete.");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Local helper functions
 * @{
 *
 *****************************************************************************/

info *
Substitute (node **ids, node *rhs, info *arg_info)
{
    DBUG_ENTER ();

    if ((NODE_TYPE (rhs) == N_id)
        && TYeqTypes (AVIS_TYPE (IDS_AVIS (*ids)), ID_NTYPE (rhs))) {
        /*
         * substitute
         */
        AVIS_SUBST (IDS_AVIS (*ids)) = ID_AVIS (rhs);
    } else {
        /*
         * emit assignment
         */
        if (NODE_TYPE (rhs) != N_id) {
            INFO_POSTASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (*ids), NULL),
                                         DUPdoDupTree (rhs)),
                              INFO_POSTASSIGN (arg_info));
        } else if ((!TUisScalar (AVIS_TYPE (IDS_AVIS (*ids))))
                   && (!TUisScalar (ID_NTYPE (rhs)))) {
            INFO_POSTASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (*ids), NULL),
                                         TBmakeId (ID_AVIS (rhs))),
                              INFO_POSTASSIGN (arg_info));
        } else {
            INFO_POSTASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (*ids), NULL),
                                         TCmakePrf1 (F_copy, TBmakeId (ID_AVIS (rhs)))),
                              INFO_POSTASSIGN (arg_info));
        }
        AVIS_SSAASSIGN (IDS_AVIS (*ids)) = INFO_POSTASSIGN (arg_info);
    }

    *ids = FREEdoFreeNode (*ids);

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 * @}  <!-- Local helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain entering the body
 *
 *****************************************************************************/
node *
EMRACCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCarg( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_SUBST (ARG_AVIS (arg_node)) = NULL;

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (arg_node) = TRAVopt(BLOCK_VARDECS (arg_node), arg_info);

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_SUBST (VARDEC_AVIS (arg_node)) = NULL;

    VARDEC_NEXT (arg_node) = TRAVopt(VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACClet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCprf (node *arg_node, info *arg_info)
{
    size_t i, n;

    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    switch (PRF_PRF (arg_node)) {
    /**
     * v, p = type_constraint (t, a); R
     *   => p = type_constraint (t, a); R[v\a]
     */
    case F_type_constraint:
        arg_info = Substitute (&LET_IDS (INFO_LET (arg_info)),
                               PRF_ARG2 (arg_node),
                               arg_info);
        break;

    /**
     * va, vb, p = same_shape_AxA (a, b); R
     *   => p = same_shape_AxA (a, b); R[va\a][vb\b]
     */
    case F_same_shape_AxA:
        arg_info = Substitute (&LET_IDS (INFO_LET (arg_info)),
                               PRF_ARG1 (arg_node),
                               arg_info);
        arg_info = Substitute (&LET_IDS (INFO_LET (arg_info)),
                               PRF_ARG2 (arg_node),
                               arg_info);
        break;

    /**
     * v, p = constraint (a, b); R
     *   => p = constraint (a, b); R[v\a]
     */
    case F_shape_matches_dim_VxA:
    case F_non_neg_val_V:
    case F_non_neg_val_S:
    case F_val_lt_shape_VxA:
    case F_val_le_val_VxV:
    case F_val_le_val_SxS:
    case F_val_lt_val_SxS:
    case F_prod_matches_prod_shape_VxA:
        arg_info = Substitute (&LET_IDS (INFO_LET (arg_info)),
                               PRF_ARG1 (arg_node),
                               arg_info);
        break;

    /**
     * acc' = conditional_error (acc, pred, "msg"); R
     *   => constraint (pred, "msg"); R[acc'\acc]
     */
    case F_conditional_error:
        // Substitute acc
        arg_info = Substitute (&LET_IDS (INFO_LET (arg_info)),
                               PRF_ARG1 (arg_node),
                               arg_info);
        // Remove acc from args chain
        //PRF_ARGS (arg_node) = FREEdoFreeNode (PRF_ARGS (arg_node));
        break;

    /**
     * v1, .., vn = guard (x1, .., xn, p1, .., pm); R
     *   => constraint (p1, .., pm); R[vi\xi]
     */
    case F_guard:
        n = PRF_NUMVARIABLERETS (arg_node);
        for (i = 0; i < n; i++) {
            // Substitute xi
            arg_info = Substitute (&LET_IDS (INFO_LET (arg_info)),
                                   PRF_ARG1 (arg_node),
                                   arg_info);
            // Remove xi from args chain
            PRF_ARGS (arg_node) = FREEdoFreeNode (PRF_ARGS (arg_node));
        }

        PRF_NUMVARIABLERETS (arg_node) = 0;
        break;

    /**
     * Nothing to do
     */
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRACCid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMRACCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
