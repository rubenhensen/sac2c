#include "extend_wrapper_types.h"

#define DBUG_PREFIX "EWT"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "type_utils.h"
#include "create_wrappers.h"
#include "str.h"
#include "memory.h"

/*
 * INFO structure
 */
struct INFO {
    bool finalise;
};

/*
 * INFO macros
 */
#define INFO_FINALISE(n) ((n)->finalise)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FINALISE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

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
    DBUG_ENTER ();

    /*
     * set this instances return types to AUD[*]
     *
     * depending on whether this function has already been
     * typechecked or not, we transform its return types into
     * fixed or non-fixed alphas...
     * furthermore, for external funs the type has always to
     * be fixed, as no more precise type can be infered by the
     * typechecker.
     */
    if ((FUNDEF_TCSTAT (fundef) == NTC_checked) || (FUNDEF_ISEXTERN (fundef))) {
        FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
    } else {
        FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
    }

    /*
     * add the fundef to the wrappertype
     */
    type = TYmakeOverloadedFunType (TUcreateFuntype (fundef), type);

    DBUG_RETURN (type);
}

static ntype *
ExtendWrapperType (ntype *type)
{
    ntype *new_type;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisFun (type), "ExtendWrapperType called on non-fun type!");

    new_type
      = (ntype *)TYfoldFunctionInstances (type, (void *(*)(node *, void *))buildWrapper,
                                          NULL);

    DBUG_RETURN (new_type);
}

static ntype *
buildProductType (node *fundef, ntype *type)
{
    DBUG_ENTER ();

    /*
     * there should be only one instance
     */
    DBUG_ASSERT (type == NULL, "function with no args but multiple instances found");

    /*
     * set this instances return types to AUD[*]
     *
     * depending on whether this function has already been
     * typechecked or not, we transform its return types into
     * fixed or non-fixed alphas...
     * furthermore, for external funs the type has always to
     * be fixed, as no more precise type can be infered by the
     * typechecker.
     */
    if ((FUNDEF_TCSTAT (fundef) == NTC_checked) || (FUNDEF_ISEXTERN (fundef))) {
        FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
    } else {
        FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
    }

    /*
     * generate a product type
     */
    type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));

    DBUG_RETURN (type);
}

static ntype *
WrapperType2ProductType (ntype *type)
{
    ntype *new_type;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisFun (type), "WrapperType2ProductType called on non-fun type!");

    new_type
      = (ntype *)TYfoldFunctionInstances (type,
                                          (void *(*)(node *, void *))buildProductType,
                                          NULL);

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

    DBUG_ENTER ();

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        type = FUNDEF_WRAPPERTYPE (arg_node);

        /*
         * as we may have added/removed some args, we must check whether
         * we have some args. In that case we have to build a proper
         * funtype instead of a product type!
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            if (TYisFun (type)) {
                new_type = ExtendWrapperType (type);
            } else {
                fundef = FUNDEF_IMPL (arg_node);

                /*
                 * depending on whether this function has already been typechecked,
                 * we transform the return vars into fixed or non-fixed alphas here
                 * furthermore, if the function is extern, we have to generate a
                 * fixed type as well as none can be infered!
                 */
                if ((FUNDEF_TCSTAT (fundef) == NTC_checked)
                    || (FUNDEF_ISEXTERN (fundef))) {
                    FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
                } else {
                    FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
                }

                new_type = TUcreateFuntype (fundef);
            }
        } else {
            if (TYisFun (type)) {
                new_type = WrapperType2ProductType (type);
            } else {
                fundef = FUNDEF_IMPL (arg_node);

                /*
                 * depending on whether this function has already been typechecked,
                 * we transform the return vars into fixed or non-fixed alphas here
                 * furthermore, if the function is extern, we have to generate a
                 * fixed type as well as none can be infered!
                 */
                if ((FUNDEF_TCSTAT (fundef) == NTC_checked)
                    || (FUNDEF_ISEXTERN (fundef))) {
                    FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
                } else {
                    FUNDEF_RETS (fundef) = TUrettypes2alphaMax (FUNDEF_RETS (fundef));
                }

                new_type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
            }
        }
        FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
        FUNDEF_WRAPPERTYPE (arg_node) = new_type;

        if (INFO_FINALISE (arg_info)) {
            /*
             * fix the wrapper type
             */
            new_type = TYfixAndEliminateAlpha (FUNDEF_WRAPPERTYPE (arg_node));
            FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (FUNDEF_WRAPPERTYPE (arg_node));
            FUNDEF_WRAPPERTYPE (arg_node) = new_type;
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    if (INFO_FINALISE (arg_info)) {
        /*
         * fix the ret nodes
         */
        if (FUNDEF_RETS (arg_node) != NULL) {
            type = TUmakeProductTypeFromRets (FUNDEF_RETS (arg_node));
            new_type = TYfixAndEliminateAlpha (type);
            FUNDEF_RETS (arg_node) = TUreplaceRetTypes (FUNDEF_RETS (arg_node), new_type);
            type = TYfreeType (type);
            new_type = TYfreeType (new_type);
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
EWTdoExtendWrapperTypes (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_ewt);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

node *
EWTdoExtendWrapperTypesAfterTC (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_ewt);

    INFO_FINALISE (info) = TRUE;
    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
