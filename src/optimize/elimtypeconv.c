/*
 * $Log$
 * Revision 1.2  2005/09/06 14:10:14  ktr
 * the global optimization counter is now maintained
 *
 * Revision 1.1  2005/08/20 12:11:28  ktr
 * Initial revision
 *
 */
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "free.h"
#include "traverse.h"
#include "dbug.h"
#include "globals.h"

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

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ETCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ETCprf");

    if (PRF_PRF (arg_node) == F_type_conv) {
        if (TYleTypes (ID_NTYPE (PRF_ARG2 (arg_node)), TYPE_TYPE (PRF_ARG1 (arg_node)))) {
            node *res = PRF_ARG2 (arg_node);
            PRF_ARG2 (arg_node) = NULL;
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = res;

            global.optcounters.etc_expr += 1;
        }
    }

    DBUG_RETURN (arg_node);
}
