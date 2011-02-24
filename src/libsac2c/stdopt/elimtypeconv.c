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
#include "memory.h"

#include "elimtypeconv.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) (n->onefundef)

/*
 INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;

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
 * @fn node *ETCdoEliminateTypeConversionsModule( node *arg_node)
 *
 * @brief starting point of F_type_conv elimination
 *        This is non-GLF traversal.
 *
 * @param arg_node - N_module
 *
 * @return
 *
 *****************************************************************************/
node *
ETCdoEliminateTypeConversionsModule (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("ETCdoEliminateTypeConversionsModule");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module, "ETC expected N_module");

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_etc);
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

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
    info *arg_info;

    DBUG_ENTER ("ETCdoEliminateTypeConversions");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "ETC expected N_fundef");

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_etc);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

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
    bool one_fundef;

    DBUG_ENTER ("ETCfundef");

    DBUG_PRINT ("ETC", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    one_fundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = one_fundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 *
 * prefix: ETC
 *
 *****************************************************************************/
node *
ETCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ETCprf");

    DBUG_PRINT ("ETC", ("Found N_prf"));
    switch (PRF_PRF (arg_node)) {
    case F_type_conv:
        DBUG_PRINT ("ETC", ("Found F_type_conv"));
        if (TYleTypes (ID_NTYPE (PRF_ARG2 (arg_node)), TYPE_TYPE (PRF_ARG1 (arg_node)))) {
            DBUG_PRINT ("ETC", ("Eliminated F_type_conv"));
            node *res = PRF_ARG2 (arg_node);
            PRF_ARG2 (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = res;

            global.optcounters.etc_expr += 1;
        }
        break;

    case F_saabind:
        DBUG_PRINT ("ETC", ("Found F_saabind"));
        if (CompareDTypes (ID_AVIS (PRF_ARG3 (arg_node)), PRF_ARG1 (arg_node),
                           PRF_ARG2 (arg_node))) {
            DBUG_PRINT ("ETC", ("Eliminated F_saabind"));
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
