/*
 * $Id$
 */

#include "free_dispatch_information.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "traverse.h"
#include "dbug.h"

/** <!-- ****************************************************************** -->
 * @brief frees the wrapper type from each wrapper in the given fundef
 *        chain. furthermore, all type error functions are freed.
 *
 * @param arg_node a N_fundef node
 * @param arg_info info node
 *
 * @return cleaned up N_fundef chain
 ******************************************************************************/
node *
FDIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FDIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
        FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
    }

    if (FUNDEF_ISTYPEERROR (arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief directs traversal to fundef nodes.
 *
 * @param arg_node a N_fundef node
 * @param arg_info info node
 *
 * @return cleaned up N_fundef chain
 ******************************************************************************/
node *
FDImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FDImodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
FDIap (node *arg_node, info *arg_info)
{
    ntype *bottom;

    DBUG_ENTER ("FDIap");

    if (FUNDEF_ISTYPEERROR (AP_FUNDEF (arg_node))) {
        bottom = TUcombineBottomsFromRets (FUNDEF_RETS (AP_FUNDEF (arg_node)));

        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TCmakePrf1 (F_type_error, TBmakeType (bottom));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *FDIdoFreeDispatchInformation( node *module)
 *
 * @brief Free the wrapper type attached to each wrapper fun and removes
 *        all type errror functions.
 *
 * @param module the N_module node
 *
 * @return cleaned up N_module node
 ******************************************************************************/
node *
FDIdoFreeDispatchInformation (node *module)
{
    DBUG_ENTER ("FDIdoFreeDispatchInformation");

    TRAVpush (TR_fdi);

    module = TRAVdo (module, NULL);

    TRAVpop ();

    DBUG_RETURN (module);
}