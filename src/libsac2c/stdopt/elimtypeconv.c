/*
 * $Id$
 */
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "free.h"
#include "traverse.h"
#include "dbug.h"
#include "globals.h"
#include "compare_tree.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "constants.h"

#include "elimtypeconv.h"

/** <!--********************************************************************-->
 *
 * @fn node *ETCdoEliminateTypeConversions( node *arg_node)
 *
 * @brief starting point of F_type_conv elimination
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
ETCdoEliminateTypeConversions (node *arg_node)
{
    DBUG_ENTER ("ETCdoEliminateTypeConversions");

    TRAVpush (TR_etc);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

static bool
CompareDTypes (node *avis, node *dim, node *shape)
{
    bool res = FALSE;
    ;

    DBUG_ENTER ("CompareDTypes");

#if 0
  /*
   * Commented this out as this removes some of the symbolic array attributes
   */
  if ( TUshapeKnown( AVIS_TYPE( avis))) {
    ntype *st = NTCnewTypeCheck_Expr( shape);
    constant *sco = COmakeConstantFromShape( TYgetShape( AVIS_TYPE( avis)));

    if ( TYisAKV( st)) {
      res = COcompareConstants( sco, TYgetValue( st));
    }

    st = TYfreeType( st);
    sco = COfreeConstant( sco);
  }
#endif

    if ((!res) && (AVIS_DIM (avis) != NULL) && (AVIS_SHAPE (avis) != NULL)) {
        node *ashape = AVIS_SHAPE (avis);
        node *shapeavis = ID_AVIS (shape);

        res = ((CMPTdoCompareTree (dim, AVIS_DIM (avis)) == CMPT_EQ)
               && (((NODE_TYPE (ashape) == N_id)
                    && (CMPTdoCompareTree (shape, ashape) == CMPT_EQ))
                   || ((NODE_TYPE (ashape) == N_array)
                       && (AVIS_SSAASSIGN (shapeavis) != NULL)
                       && (CMPTdoCompareTree (ASSIGN_RHS (AVIS_SSAASSIGN (shapeavis)),
                                              ashape)
                           == CMPT_EQ))));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Typeconv Elimination traversal (etc_tab)
 *
 * prefix: ETC
 *
 *****************************************************************************/
node *
ETCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ETCfundef");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

node *
ETCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ETCprf");

    switch (PRF_PRF (arg_node)) {
    case F_type_conv:
        if (TYleTypes (ID_NTYPE (PRF_ARG2 (arg_node)), TYPE_TYPE (PRF_ARG1 (arg_node)))) {
            node *res = PRF_ARG2 (arg_node);
            PRF_ARG2 (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = res;

            global.optcounters.etc_expr += 1;
        }
        break;

    case F_saabind:
        if (CompareDTypes (ID_AVIS (PRF_ARG3 (arg_node)), PRF_ARG1 (arg_node),
                           PRF_ARG2 (arg_node))) {
            node *res = PRF_ARG3 (arg_node);
            PRF_ARG3 (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = res;

            global.optcounters.etc_expr += 1;
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}
