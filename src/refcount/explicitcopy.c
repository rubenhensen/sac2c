/**
 *
 * $Id$
 *
 * @defgroup ec Explicit Copy
 * @ingroup rc
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file explicitcopy.c
 *
 *
 */
#include "explicitcopy.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "new_types.h"
#include "internal_lib.h"

/**
 * INFO structure
 */
struct INFO {
    node *preassign;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_PREASSIGN (result) = NULL;
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

/** <!--********************************************************************-->
 *
 * @fn node *EMECdoExplicitCopy( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMECdoExplicitCopy (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMECdoExplicitCopy");

    DBUG_PRINT ("EMEC", ("Starting explicit copy traversal."));

    info = MakeInfo ();

    TRAVpush (TR_emec);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("EMEC", ("Explicit copy traversal complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Static helper funcions
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CreateCopyId(node *arg_node, info *arg_info)
 *
 * @brief CreateCopyId( a, arg_info) will append INFO_PREASSIGN with
 *        a' = copy( a); and returns a'.
 *
 *****************************************************************************/
static node *
CreateCopyId (node *oldid, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("CreateCopyId");

    /*
     * Create a new variable for b'
     */
    avis = TBmakeAvis (ILIBtmpVarName (ID_NAME (oldid)),
                       TYcopyType (AVIS_TYPE (ID_AVIS (oldid))));

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    /*
     * Create copy operation
     */
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), TCmakePrf1 (F_copy, oldid)),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

    /*
     * Replace oldid with newid
     */
    oldid = TBmakeId (avis);

    DBUG_RETURN (oldid);
}

/******************************************************************************
 *
 * Explicit copy traversal (emec_tab)
 *
 * prefix: EMEC
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMECassign(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMECassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMECassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECfundef(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMECfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMECfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMECap (node *arg_node, info *arg_info)
{
    node *args, *exprs;

    DBUG_ENTER ("EMECap");

    exprs = AP_ARGS (arg_node);
    args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    while (args != NULL) {
        if (ARG_HASLINKSIGNINFO (args)) {
            node *rets = FUNDEF_RETS (AP_FUNDEF (arg_node));
            while (rets != NULL) {
                if ((RET_HASLINKSIGNINFO (rets))
                    && (RET_LINKSIGN (rets) == ARG_LINKSIGN (args))) {
                    EXPRS_EXPR (exprs) = CreateCopyId (EXPRS_EXPR (exprs), arg_info);
                }
                rets = RET_NEXT (rets);
            }
        }

        args = ARG_NEXT (args);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECprf(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
EMECprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMECprf");

    switch (PRF_PRF (arg_node)) {
    case F_modarray:
    case F_idx_modarray:
        /*
         * Example:
         *
         * a = modarray( b, iv, val);
         *
         * is transformed info
         *
         * b' = copy( b);
         * a  = modarray( b', iv, val);
         */
        PRF_ARG1 (arg_node) = CreateCopyId (PRF_ARG1 (arg_node), arg_info);
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/* @} */
