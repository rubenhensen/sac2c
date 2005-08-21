/*
 *
 * $Log$
 * Revision 1.4  2005/08/21 09:34:29  ktr
 * all arguments and the return value of sel and modarray must be AKS to allow conversion
 * into idx_sel and idx_modarray,respectively
 *
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
    node *fundef;
    node *intap;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_INTAP(n) ((n)->intap)
#define INFO_LHS(n) ((n)->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;

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
/**
 *
 * @name Some utility functions:
 *
 * @{
 */
static bool
eqShapes (ntype *a, ntype *b)
{
    bool res;

    DBUG_ENTER ("eqTypeShapes");

    res = SHcompareShapes (TYgetShape (a), TYgetShape (b));

    DBUG_RETURN (res);
}

static void
AddTypeToIdxTypes (node *avis, ntype *type)
{
    node *exprs;

    DBUG_ENTER ("AddTypeToIdxTypes");

    exprs = AVIS_IDXTYPES (avis);
    while ((exprs != NULL) && (!eqShapes (type, TYPE_TYPE (EXPRS_EXPR (exprs))))) {
        exprs = EXPRS_NEXT (exprs);
    }

    if (exprs == NULL) {
        AVIS_IDXTYPES (avis)
          = TBmakeExprs (TBmakeType (TYeliminateAKV (type)), AVIS_IDXTYPES (avis));
    }

    DBUG_VOID_RETURN;
}

static void
TranscribeIdxTypes (node *fromavis, node *toavis)
{
    node *exprs;

    DBUG_ENTER ("TranscribeIdxTypes");

    exprs = AVIS_IDXTYPES (fromavis);
    while (exprs != NULL) {
        AddTypeToIdxTypes (toavis, TYPE_TYPE (EXPRS_EXPR (exprs)));
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_VOID_RETURN;
}

/*@}*/

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for IVEI:
 *
 * @{
 ****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIap");

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        node *args, *exprs;

        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        } else {
            INFO_INTAP (arg_info) = arg_node;
        }

        /*
         * Add all types annotated at the formal parameters to the corresponding
         * concrete parameters
         */
        args = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        exprs = AP_ARGS (arg_node);

        while (args != NULL) {
            TranscribeIdxTypes (ARG_AVIS (args), ID_AVIS (EXPRS_EXPR (exprs)));
            args = ARG_NEXT (args);
            exprs = EXPRS_NEXT (exprs);
        }
    }

    DBUG_RETURN (arg_node);
}

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

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IVEIfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ("IVEIfundef");

    if ((!FUNDEF_ISLACFUN (arg_node)) || (INFO_FUNDEF (arg_info) != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            oldfundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            if (FUNDEF_ISDOFUN (arg_node)) {
                INFO_INTAP (arg_info) = TRAVdo (INFO_INTAP (arg_info), arg_info);
            }
            INFO_FUNDEF (arg_info) = oldfundef;
        }
    }

    if ((INFO_FUNDEF (arg_info) == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IVEIlet( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
IVEIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEIlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

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
    node *lhs, *arg1, *arg2, *arg3;
    ntype *ltype, *type1, *type2, *type3;

    DBUG_ENTER ("IVEIprf");

    switch (PRF_PRF (arg_node)) {
    case F_sel:
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);

        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)),
                     "wrong arg in F_sel application");

        ltype = IDS_NTYPE (lhs);
        type1 = ID_NTYPE (arg1);
        type2 = ID_NTYPE (arg2);

        if (TUshapeKnown (ltype) && TUshapeKnown (type2) && TUisIntVect (type1)
            && TUshapeKnown (type1)) {
            AddTypeToIdxTypes (ID_AVIS (arg1), type2);
        }
        break;

    case F_modarray:
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        arg2 = PRF_ARG2 (arg_node);
        arg3 = PRF_ARG3 (arg_node);

        DBUG_ASSERT (((NODE_TYPE (arg1) == N_id) && (NODE_TYPE (arg2) == N_id)),
                     "wrong arg in F_modarray application");

        ltype = IDS_NTYPE (lhs);
        type1 = ID_NTYPE (arg1);
        type2 = ID_NTYPE (arg2);
        type3 = ID_NTYPE (arg3);

        if (TUshapeKnown (ltype) && TUshapeKnown (type1) && TUisIntVect (type2)
            && TUshapeKnown (type2) && TUshapeKnown (type3)) {
            AddTypeToIdxTypes (ID_AVIS (arg2), type1);
        }
        break;

    default:
        break;
    }

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
 *****************************************************************************/
node *
IVEIdoIndexVectorEliminationInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IVEIdoIndexVectorEliminationInference");

    DBUG_PRINT ("OPT", ("Starting index vector inference..."));

    TRAVpush (TR_ivei);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("Index vector inference complete!"));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn  node *IVEIprintPreFun( node *arg_node, info *arg_info)
 *
 *   @brief
 *   @param
 *   @return
 *
 *****************************************************************************/
node *
IVEIprintPreFun (node *arg_node, info *arg_info)
{
    node *exprs;

    DBUG_ENTER ("IVEIprintPreFun");

    switch (NODE_TYPE (arg_node)) {
    case N_avis:
        exprs = AVIS_IDXTYPES (arg_node);
        if (exprs != NULL) {
            printf ("/*");
            while (exprs != NULL) {
                printf (":IDX(%s)",
                        SHshape2String (0, TYgetShape (TYPE_TYPE (EXPRS_EXPR (exprs)))));
                exprs = EXPRS_NEXT (exprs);
            }
            printf ("*/");
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
/*@}*/ /* defgroup ive */
