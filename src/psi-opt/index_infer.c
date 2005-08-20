/*
 *
 * $Log$
 * Revision 1.2  2005/08/20 19:08:02  ktr
 * starting brushing
 *
 * Revision 1.1  2005/07/16 19:00:18  sbs
 * Initial revision
 *
 *
 */

#include "tree_basic.h"
#include "dbug.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "index_infer.h"

/*
 * INFO structure
 */
struct INFO {
};

/*
 * INFO macros
 */

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/**
 *
 * @defgroup ive IVE
 * @ingroup opt
 *
 * @{
 */

/**
 *
 * @file index_infer.c
 *
 *  This file contains the implementation of the inference of uses attributes
 *  for IVE (index vector elimination).
 *
 */

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for IVEI:
 *
 * @{
 ****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVEIassign( node *arg_node, info *arg_info )
 *
 *****************************************************************************/
node *
IVEIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIassign");

    /* Bottom up traversal!! */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEIprf( node *arg_node, info *arg_info )
 *
 *****************************************************************************/

node *
IVEIprf (node *arg_node, info *arg_info)
{
    node *arg1, *arg2, *arg3;
    ntype *type1, *type2;

    DBUG_ENTER ("IVEIprf");
#if 0
  switch( PRF_PRF( arg_node)) {
    case F_sel: 
      arg1 = PRF_ARG1( arg_node);
      arg2 = PRF_ARG2( arg_node);

      DBUG_ASSERT( ((NODE_TYPE( arg1) == N_id) &&
                    (NODE_TYPE( arg2) == N_id)),
                   "wrong arg in F_sel application");

      type1 = ID_NTYPE( arg1);
      type2 = ID_NTYPE( arg2);

      if( TUshapeKnown( type2) && TUisIntVect( type1)) {
        INFO_SHP( arg_info) = TYgetShape( type2);
        PRF_ARG1( arg_node) = TRAVdo( arg1, arg_info);
      } else {
        INFO_SHP( arg_info) = NULL;
        PRF_ARG1( arg_node) = TRAVdo(arg1, arg_info);
      }
      INFO_SHP( arg_info) = NULL;
      PRF_ARG2( arg_node) = TRAVdo(arg2, arg_info);
      break;

    case F_modarray: 
      arg1 = PRF_ARG1( arg_node);
      arg2 = PRF_ARG2( arg_node);
      arg3 = PRF_ARG3( arg_node);
      DBUG_ASSERT( ((NODE_TYPE( arg1) == N_id) &&
                    (NODE_TYPE( arg2) == N_id)),
                   "wrong arg in F_modarray application");

      type1 = ID_NTYPE( arg1);
      type2 = ID_NTYPE( arg2);

      if( TUshapeKnown( type1) && TUisIntVect( type2)) {
        INFO_SHP( arg_info) = TYgetShape( type1);
        PRF_ARG2( arg_node) = TRAVdo( arg2, arg_info);
      } else {
        INFO_SHP( arg_info) = NULL;
        PRF_ARG2( arg_node) = TRAVdo(arg2, arg_info);
      }
      INFO_SHP( arg_info) = NULL;
      PRF_ARG1( arg_node) = TRAVdo(arg1, arg_info);
      PRF_ARG3( arg_node) = TRAVdo(PRF_ARG3( arg_node), arg_info);
      break;

    default:
      PRF_ARGS( arg_node) = TRAVdo(PRF_ARGS( arg_node), arg_info);
      break;
  }
#endif
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *IVEIdoIndexVectorEliminationInference( node *syntax_tree)
 *
 *   @brief call this function for inferring all array uses.
 *   @param part of the AST (usually the entire tree) IVE is to be applied on.
 *   @return modified AST.
 *
 ******************************************************************************/

node *
IVEIdoIndexVectorEliminationInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoIndexVectorEliminationInference");

    TRAVpush (TR_ivei);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*@}*/
/*@}*/ /* defgroup ive */
