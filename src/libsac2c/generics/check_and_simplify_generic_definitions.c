/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup csgd Check and Simplify Generic Definitions
 *
 * This module checks that the variable bindings in polymorphic types
 * within a function do comply to the following restrictions:
 *
 * - Only argument positions bind new identifiers. Both, the return
 *   types and the function body may only contain previously bound
 *   identifiers.
 *
 * - Identifiers can only be used for one kind of type, i.e. identifiers
 *   can either be used in
 *
 *   * a polymorphic type/as outer type of a polymorphic user type
 *   * as inner type of a polymorphic user type
 *
 * - Identifiers used for referencing the shape of a polymorphic user
 *   type may not be used in polymorphic types
 *
 * - Identifiers used for referencing the shape of a polymorphic user
 *   type may not be shared between different types
 *
 * - Implicit denesting is allowed only in argument position
 *
 * - Implicit renesting is allowed only in return position
 *
 * Furthermore, the de- and renesting operations are made explicit by
 * introducing appropriate casts.
 *
 * @ingroup csgd
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file check_and_simplify_generic_definitions.c
 *
 * Prefix: CSGD
 *
 *****************************************************************************/
#include "check_and_simplify_generic_definitions.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "type_utils.h"
#include "str.h"
#include "ctinfo.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *args;
    node *rets;
    node *fundef;
    node *current;
    enum { CSGD_normal, CSGD_checkarg, CSGD_checkret, CSGD_strip } mode;
    bool isgeneric;
};

/**
 * A template entry in the template info structure
 */
#define INFO_ARGS(n) ((n)->args)
#define INFO_RETS(n) ((n)->rets)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_CURRENT(n) ((n)->current)
#define INFO_MODE(n) ((n)->mode)
#define INFO_ISGENERIC(n) ((n)->isgeneric)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ARGS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_CURRENT (result) = NULL;
    INFO_MODE (result) = CSGD_normal;
    INFO_ISGENERIC (result) = FALSE;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CSGDdoCheckAndSimplifyGenericDefinitions( node *syntax_tree)
 *
 *****************************************************************************/
node *
CSGDdoCheckAndSimplifyGenericDefinitions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CSGDdoCheckAndSimplifyGenericDefinitions");

    info = MakeInfo ();

    TRAVpush (TR_csgd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool PolymorphicTypeComplies(ntype *a, ntype *b)
 *
 * @brief Checks whether the type a complies with type b.
 *
 * @param a a polymorphic type
 * @param b a second type
 *
 * @return true if both types comply
 ******************************************************************************/
static bool
PolymorphicTypeComplies (ntype *a, ntype *b)
{
    bool result = TRUE;

    DBUG_ENTER ("PolymorphicTypeComplies");

    if (TYisArray (a)) {
        a = TYgetScalar (a);
    }

    if (TYisArray (b)) {
        b = TYgetScalar (b);
    }

    if (TYisPoly (a)) {
        if (TYisPoly (b)) {
            /*
             * these always comply
             */
            result = TRUE;
        } else if (TYisPolyUser (b)) {
            /*
             * here a is not allowed to be used as inner or shape
             */
            result = !STReq (TYgetPolyName (a), TYgetPolyUserInner (b))
                     && !STReq (TYgetPolyName (a), TYgetPolyUserShape (b));
        }
    } else if (TYisPolyUser (a)) {
        if (TYisPoly (b)) {
            /*
             * here a_inner and a_shape are not allowed to be b
             */
            result = !STReq (TYgetPolyUserInner (a), TYgetPolyName (b))
                     && !STReq (TYgetPolyUserShape (a), TYgetPolyName (b));
        } else if (TYisPolyUser (b)) {
            /*
             * here each identifier in a is not allowed to occur anywhere
             * in b apart from its use in a
             */
            result = !STReq (TYgetPolyUserOuter (a), TYgetPolyUserInner (b))
                     && !STReq (TYgetPolyUserOuter (a), TYgetPolyUserShape (b))
                     && !STReq (TYgetPolyUserInner (a), TYgetPolyUserOuter (b))
                     && !STReq (TYgetPolyUserInner (a), TYgetPolyUserShape (b))
                     && !STReq (TYgetPolyUserShape (a), TYgetPolyUserOuter (b))
                     && !STReq (TYgetPolyUserShape (a), TYgetPolyUserInner (b));
        }
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CSGDmodule(node *arg_node, info *arg_info)
 *
 * @brief traverses only the fundef chain
 *
 *****************************************************************************/
node *
CSGDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDfundef");

    /*
     * 0) store context in info node
     */
    INFO_FUNDEF (arg_info) = arg_node;
    INFO_ARGS (arg_info) = FUNDEF_ARGS (arg_node);
    INFO_RETS (arg_info) = FUNDEF_RETS (arg_node);
    INFO_ISGENERIC (arg_info) = FALSE;

    /*
     * 1) check compliance of args and figure out whether this is a
     *    generic function after all.
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    FUNDEF_ISGENERIC (arg_node) = INFO_ISGENERIC (arg_info);

    /*
     * 2) check compliance of rets
     */
    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    /*
     * 3) process de-/renesting and check compliance of body
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * 4) next funef
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDarg(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDarg");

    if (INFO_MODE (arg_info) == CSGD_normal) {
        if (TUisPolymorphic (ARG_NTYPE (arg_node))) {
            INFO_ISGENERIC (arg_info) = TRUE;
            INFO_CURRENT (arg_info) = arg_node;
            INFO_MODE (arg_info) = CSGD_checkarg;

            INFO_ARGS (arg_info) = TRAVdo (INFO_ARGS (arg_info), arg_info);

            INFO_MODE (arg_info) = CSGD_normal;
            INFO_CURRENT (arg_info) = NULL;
        }
    } else if (INFO_MODE (arg_info) == CSGD_checkarg) {
        /*
         * check whether the argument stored in the info structure complies
         * with the current one
         */
        if (!PolymorphicTypeComplies (ARG_NTYPE (INFO_CURRENT (arg_info)),
                                      ARG_NTYPE (arg_node))) {
            CTIerror ("In definition of %s: The type of polymorphic argument "
                      "%s does not comply with the type of argument %s.",
                      CTIitemName (INFO_FUNDEF (arg_info)),
                      ARG_NAME (INFO_CURRENT (arg_info)), ARG_NAME (arg_node));
        }
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDret(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDret");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDcast(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDcast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDcast");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDavis(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDavis");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDreturn(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDreturn");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSGDassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CSGDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CSGDassign");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
