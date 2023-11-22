#include "free_dispatch_information.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        if (FUNDEF_WRAPPERTYPE (arg_node) != NULL) {
            FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
        } else {
            FUNDEF_IMPL (arg_node) = NULL;
        }
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
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = TRAVopt(MODULE_FUNDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
FDIap (node *arg_node, info *arg_info)
{
    ntype *bottom;

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    TRAVpush (TR_fdi);

    module = TRAVdo (module, NULL);

    TRAVpop ();

    DBUG_RETURN (module);
}

#undef DBUG_PREFIX
