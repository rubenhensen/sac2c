/*****************************************************************************
 *
 *
 * file:   annotate_cuda_withloop.c
 *
 * prefix: CNSTASS
 *
 * description:
 *
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
#include "LookUpTable.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "str.h"
#include "ConstVarPropagation.h"

struct INFO {
    node *constassigns;
    node *fundef;
    bool collect;
};

/**
 * INFO macros
 */

#define INFO_CONSTASSIGNS(n) (n->constassigns)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_COLLECT(n) (n->collect)

/**
 * INFO functions
 */

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
 *
 * @fn
 *
 * @brief node *CNSTASSdoCUDAconstantAssignment( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CNSTASSdoCUDAconstantAssignment (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CNSTASSdoCUDAconstantAssignment");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    syntax_tree = CVPdoConstVarPropagation (syntax_tree);

    info = MakeInfo ();

    TRAVpush (TR_cnstass);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CNSTASSfundef( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CNSTASSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_FUNDEF (arg_info) = NULL;

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CNSTASSassign( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CNSTASSassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_CONSTASSIGNS (arg_info) != NULL && !INFO_COLLECT (arg_info)) {
        arg_node = TCappendAssign (INFO_CONSTASSIGNS (arg_info), arg_node);
        INFO_CONSTASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CNSTASSwith( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CNSTASSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSwith");

    /* Only look at Withloops that are cudarizable */
    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_COLLECT (arg_info) = TRUE;
        // WITH_WITHOP( arg_node) = TRAVdo(  WITH_WITHOP( arg_node), arg_info);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_COLLECT (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CNSTASSgenerator( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CNSTASSgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CNSTASSgenerator");

    // constant *lower_bound_cnst, *upper_bound_cnst;
    node *lower_bound, *upper_bound;
    node *step, *width;
    node *lower_bound_elements, *upper_bound_elements;
    node *step_elements = NULL, *width_elements = NULL;
    node *avis;
    node *ids, *vardec;

    /* Handle Lowerbound and Upperbound */

    lower_bound = GENERATOR_BOUND1 (arg_node);
    upper_bound = GENERATOR_BOUND2 (arg_node);

    if (NODE_TYPE (lower_bound) == N_id) {
        node *ssaassign = AVIS_SSAASSIGN (ID_AVIS (lower_bound));
        DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array),
                     "Lower bound should be an N_array node!");
        GENERATOR_BOUND1 (arg_node) = FREEdoFreeNode (GENERATOR_BOUND1 (arg_node));
        GENERATOR_BOUND1 (arg_node) = DUPdoDupNode (ASSIGN_RHS (ssaassign));
    }

    if (NODE_TYPE (upper_bound) == N_id) {
        node *ssaassign = AVIS_SSAASSIGN (ID_AVIS (upper_bound));
        DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array),
                     "Upper bound should be an N_array node!");
        GENERATOR_BOUND2 (arg_node) = FREEdoFreeNode (GENERATOR_BOUND2 (arg_node));
        GENERATOR_BOUND2 (arg_node) = DUPdoDupNode (ASSIGN_RHS (ssaassign));
    }

    lower_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
    upper_bound_elements = ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node));

    while (lower_bound_elements != NULL && upper_bound_elements != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (upper_bound_elements)) == N_num) {
            avis = TBmakeAvis (TRAVtmpVarName ("ub"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
            vardec = TBmakeVardec (avis, NULL);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardec);

            ids = TBmakeIds (avis, NULL);

            INFO_CONSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, EXPRS_EXPR (upper_bound_elements)),
                              INFO_CONSTASSIGNS (arg_info));
            EXPRS_EXPR (upper_bound_elements) = TBmakeId (avis);
        }

        if (NODE_TYPE (EXPRS_EXPR (lower_bound_elements)) == N_num) {
            avis = TBmakeAvis (TRAVtmpVarName ("lb"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
            vardec = TBmakeVardec (avis, NULL);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardec);

            ids = TBmakeIds (avis, NULL);

            INFO_CONSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, EXPRS_EXPR (lower_bound_elements)),
                              INFO_CONSTASSIGNS (arg_info));
            EXPRS_EXPR (lower_bound_elements) = TBmakeId (avis);
        }
        lower_bound_elements = EXPRS_NEXT (lower_bound_elements);
        upper_bound_elements = EXPRS_NEXT (upper_bound_elements);
    }

    /* Handle Step and Width */

    step = GENERATOR_STEP (arg_node);
    width = GENERATOR_WIDTH (arg_node);

    if (step != NULL) {
        if (NODE_TYPE (step) == N_id) {
            node *ssaassign = AVIS_SSAASSIGN (ID_AVIS (step));
            DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array),
                         "Step should be an N_array node!");
            GENERATOR_STEP (arg_node) = FREEdoFreeNode (GENERATOR_STEP (arg_node));
            GENERATOR_STEP (arg_node) = DUPdoDupNode (ASSIGN_RHS (ssaassign));
        }
        step_elements = ARRAY_AELEMS (GENERATOR_STEP (arg_node));
    }

    if (width != NULL) {
        if (NODE_TYPE (width) == N_id) {
            node *ssaassign = AVIS_SSAASSIGN (ID_AVIS (width));
            DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (ssaassign)) == N_array),
                         "Width should be an N_array node!");
            GENERATOR_WIDTH (arg_node) = FREEdoFreeNode (GENERATOR_WIDTH (arg_node));
            GENERATOR_WIDTH (arg_node) = DUPdoDupNode (ASSIGN_RHS (ssaassign));
        }
        width_elements = ARRAY_AELEMS (GENERATOR_WIDTH (arg_node));
    }

    while (step_elements != NULL && width_elements != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (step_elements)) == N_num) {
            avis = TBmakeAvis (TRAVtmpVarName ("step"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
            vardec = TBmakeVardec (avis, NULL);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardec);

            ids = TBmakeIds (avis, NULL);

            INFO_CONSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, EXPRS_EXPR (step_elements)),
                              INFO_CONSTASSIGNS (arg_info));
            EXPRS_EXPR (step_elements) = TBmakeId (avis);
        }

        if (NODE_TYPE (EXPRS_EXPR (width_elements)) == N_num) {
            avis = TBmakeAvis (TRAVtmpVarName ("width"),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
            vardec = TBmakeVardec (avis, NULL);

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardec);

            ids = TBmakeIds (avis, NULL);

            INFO_CONSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, EXPRS_EXPR (width_elements)),
                              INFO_CONSTASSIGNS (arg_info));
            EXPRS_EXPR (width_elements) = TBmakeId (avis);
        }
        step_elements = EXPRS_NEXT (step_elements);
        width_elements = EXPRS_NEXT (width_elements);
    }

    DBUG_RETURN (arg_node);
}

/*
node *CNSTASSgenarray( node *arg_node, info *arg_info)
{
  node *shp, *shp_elements;
  node *avis, *vardec, *ids;

  DBUG_ENTER("CNSTASSgenarray");

  shp = GENARRAY_SHAPE( arg_node);

  shp_elements = ARRAY_AELEMS( shp);

  while(  shp_elements != NULL)
  {
    if( NODE_TYPE( EXPRS_EXPR(  shp_elements)) == N_num)
    {
      avis = TBmakeAvis( TRAVtmpVarName( "shp"), TYmakeAKS( TYmakeSimpleType( T_int),
SHmakeShape(0))); vardec = TBmakeVardec( avis, NULL);

      FUNDEF_VARDEC( INFO_FUNDEF( arg_info)) = TCappendVardec( FUNDEF_VARDEC( INFO_FUNDEF(
arg_info)), vardec);

      ids = TBmakeIds(avis, NULL);

      INFO_CONSTASSIGNS(arg_info) = TBmakeAssign( TBmakeLet(ids, EXPRS_EXPR(
shp_elements)), INFO_CONSTASSIGNS(arg_info)); EXPRS_EXPR(  shp_elements) = TBmakeId(
avis);
    }
    shp_elements = EXPRS_NEXT(  shp_elements);
  }

  DBUG_RETURN(arg_node);
}
*/
