/** <!--********************************************************************-->
 *
 * @defgroup
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_dist_wl_cond.c
 *
 * Prefix: DISTCOND
 *
 *****************************************************************************/
#include "create_dist_wl_cond.h"

/*
 * Other includes go here
 */
#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "shape.h"

#define DBUG_PREFIX "DISTCOND"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *assigns;
    node *letids;
    node *predassigns;
    node *predavis;
    node *thenblock;
};

/*
 * INFO_FUNDEF        N_fundef node of the enclosing function
 *
 * INFO_ASSIGNS       N_assign chain to replace the distributed wl assignment
 *
 * INFO_LETIDS        N_ids of a N_let
 *
 * INFO_PREDASSIGNS   N_assign chain for the predicate of each branch of the
 *                    conditional
 *
 * INFO_PREDAVIS      N_avis for the predicate variable
 *
 * INFO_THENBLOCK     N_assign chain for one of the branches of the N_cond
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ASSIGNS(n) (n->assigns)
#define INFO_LETIDS(n) (n->letids)
#define INFO_PREDASSIGNS(n) (n->predassigns)
#define INFO_PREDAVIS(n) (n->predavis)
#define INFO_THENBLOCK(n) (n->thenblock)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ASSIGNS (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_PREDASSIGNS (result) = NULL;
    INFO_PREDAVIS (result) = NULL;
    INFO_THENBLOCK (result) = NULL;

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
 * @fn node *DISTCONDdoCreateDistWlCond( node *syntax_tree)
 *
 *****************************************************************************/
node *
DISTCONDdoCreateDistWlCond (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_distcond);
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

static void CreatePredicateAssignments (node *expr, info *arg_info);

/** <!--********************************************************************-->
 *
 * @fn void CreatePredicateAssignments( node *expr, info *arg_info)
 *
 * @brief Create variables and memory assignments for the predicate given by
 *        expr.
 *
 *****************************************************************************/
static void
CreatePredicateAssignments (node *expr, info *arg_info)
{
    node *alloc_avis, *pred_avis, *fill, *alloc_prf;

    DBUG_ENTER ();

    /* create predicate avises */
    alloc_avis = TBmakeAvis (TRAVtmpVarName ("pred_alloc"),
                             TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
    pred_avis = TBmakeAvis (TRAVtmpVarName ("pred"),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
    INFO_PREDAVIS (arg_info) = pred_avis;

    /* add new avises to variable declarations */
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (alloc_avis,
                      TBmakeVardec (pred_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info))));

    /* create assignment of new predicate variable */
    fill = TBmakeAssign (TBmakeLet (TBmakeIds (pred_avis, NULL),
                                    TCmakePrf2 (F_fill, expr, TBmakeId (alloc_avis))),
                         NULL);

    /* create assignment for allocation of predicate variable */
    alloc_prf = TCmakePrf2 (F_alloc, TBmakeNum (0), TCcreateZeroVector (0, T_bool));
    INFO_PREDASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (alloc_avis, NULL), alloc_prf), fill);

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
 * @fn node *DISTCONDfundef( node *arg_node, info *arg_info)
 *
 * @brief Check only SPMD functions.
 *
 *****************************************************************************/
node *
DISTCONDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISSPMDFUN (arg_node)) {
        DBUG_ASSERT (FUNDEF_BODY (arg_node) != NULL, "Found SPMD function with no body!");

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDassign( node *arg_node, info *arg_info)
 *
 * @brief Insert Predicate and Conditional if required.
 *
 *****************************************************************************/
node *
DISTCONDassign (node *arg_node, info *arg_info)
{
    node *res;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* if during the traversal we generated new assignments, we replace this node
     with those assignments */
    if (INFO_ASSIGNS (arg_info) != NULL) {
        res = TCappendAssign (INFO_ASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_ASSIGNS (arg_info) = NULL;
        FREEdoFreeNode (arg_node);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        res = arg_node;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDlet( node *arg_node, info *arg_info)
 *
 * @brief Traverse RHS first, then LHS.
 *
 *****************************************************************************/
node *
DISTCONDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDwiths( node *arg_node, info *arg_info)
 *
 * @brief Create nested conditional branch for each with-loop
 *
 *****************************************************************************/
node *
DISTCONDwiths (node *arg_node, info *arg_info)
{
    node *cond;

    DBUG_ENTER ();

    /*
     * traverse with-loop to get predicate avis and assignments for this withloop.
     */
    WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);

    /* create conditional with the previous withloop's assignments in the else
     * branch*/
    cond = TBmakeCond (TBmakeId (INFO_PREDAVIS (arg_info)),
                       TBmakeBlock (INFO_THENBLOCK (arg_info), NULL),
                       TBmakeBlock (INFO_ASSIGNS (arg_info), NULL));
    /* the assignments for this with-loop are the predicate assignments and the
     conditional*/
    INFO_ASSIGNS (arg_info)
      = TCappendAssign (INFO_PREDASSIGNS (arg_info), TBmakeAssign (cond, NULL));

    WITHS_NEXT (arg_node) = TRAVopt (WITHS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDwith( node *arg_node, info *arg_info)
 *
 * @brief Create CUDA predicate and concrete to distributed function calls
 *
 *****************************************************************************/
node *
DISTCONDwith (node *arg_node, info *arg_info)
{
    static int counter = 0;
    node *new_rhs;

    DBUG_ENTER ();

    /* create predicate value */
    new_rhs = TBmakePrf (F_is_cuda_thread, TBmakeExprs (TBmakeNum (counter++), NULL));

    CreatePredicateAssignments (new_rhs, arg_info);

    INFO_THENBLOCK (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                                 DUPdoDupTree (arg_node)),
                      NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DISTCONDwith2( node *arg_node, info *arg_info)
 *
 * @brief Create Host predicate and concrete to distributed function calls
 *
 *****************************************************************************/
node *
DISTCONDwith2 (node *arg_node, info *arg_info)
{
    node *new_rhs;

    DBUG_ENTER ();

    /* create predicate value */
    new_rhs = TBmakeBool (TRUE);

    CreatePredicateAssignments (new_rhs, arg_info);

    INFO_THENBLOCK (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                                 DUPdoDupTree (arg_node)),
                      NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
