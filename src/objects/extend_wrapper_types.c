/* $Id$ */

#include "extend_wrapper_types.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "type_utils.h"
#include "create_wrappers.h"

/*
 * helper functions
 */

/** <!--********************************************************************-->
 *
 * @fn ntype *ExtendWrapperType( ntype *)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

static ntype *
buildWrapper (node *fundef, ntype *type)
{
    DBUG_ENTER ("buildWrapper");

    /*
     * set this instances return types to AUD[*]
     *
     * depending on whether this function has already been
     * typechecked or not, we transform its return types into
     * fixed or non-fixed alphas...
     */
    if (FUNDEF_TCSTAT (fundef) == NTC_checked) {
        FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
    } else {
        FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
    }

    /*
     * add the fundef to the wrappertype
     */
    type = TYmakeOverloadedFunType (CRTWRPcreateFuntype (fundef), type);

    DBUG_RETURN (type);
}

ntype *
ExtendWrapperType (ntype *type)
{
    ntype *new_type;

    DBUG_ENTER ("ExtendWrapperType");

    DBUG_ASSERT (TYisFun (type), "ExtendWrapperType called on non-fun type!");

    new_type
      = TYfoldFunctionInstances (type, (void *(*)(node *, void *))buildWrapper, NULL);

    DBUG_RETURN (new_type);
}

/**
 * traversal functions
 */

node *
EWTfundef (node *arg_node, info *arg_info)
{
    ntype *new_type, *type;
    node *fundef;

    DBUG_ENTER ("EWTfundef");

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        type = FUNDEF_WRAPPERTYPE (arg_node);
        if (TYisFun (type)) {
            new_type = ExtendWrapperType (type);
        } else {
            fundef = FUNDEF_IMPL (arg_node);

            /*
             * depending on whether this function has already been typechecked,
             * we transform the return vars into fixed or non-fixed alphas here
             */
            if (FUNDEF_TCSTAT (fundef) == NTC_checked) {
                FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
            } else {
                FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
            }

            /*
             * as we may have added some new args, we must check whether
             * we have some args. In that case we have to build a proper
             * funtype instead of a product type!
             */
            if (FUNDEF_ARGS (fundef) != NULL) {
                new_type = CRTWRPcreateFuntype (fundef);
            } else {
                new_type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
            }
        }
        FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (type);
        FUNDEF_WRAPPERTYPE (arg_node) = new_type;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
EWTdoExtendWrapperTypes (node *arg_node)
{
    DBUG_ENTER ("EWTdoExtendWrapperTypes");

    TRAVpush (TR_ewt);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}
