/** <!--********************************************************************-->
 *
 * @defgroup gntc Generate Nested Type Conversions
 *
 * Generates nested conversion functions:
 *   - nest_XXX / denest_XXX
 *
 * @ingroup gntc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file generate_nested_type_conversions.c
 *
 * Prefix: GNTC
 *
 *****************************************************************************/
#include "generate_nested_type_conversions.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "user_types.h"
#include "str.h"
#include "free.h"
#include "DupTree.h"
#include "tree_basic.h"
#include "shape.h"
#include "deserialize.h"
#include "globals.h"

struct INFO {
    node *fundefs;
    node *fundecs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_FUNDEFS(n) ((n)->fundefs)
#define INFO_FUNDECS(n) ((n)->fundecs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEFS (result) = NULL;
    INFO_FUNDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *GNTCdoGenerateGenericTypeConversions( node *syntax_tree)
 *
 *****************************************************************************/
node *
GNTCdoGenerateNestedTypeConversions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_gntc);
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

/** <!-- ****************************************************************** -->
 * @brief builds nest/denest function for nesting or denesting from type from to
 * type to
 *
 * @param name    name of function to be built
 * @param ns      ns of function to be built
 * @param from    type to convert from
 * @param to      type to convert to
 * @param prf     primitive function to stick in
 *
 * @return N_fundef node representing the type conversion
 ******************************************************************************/
static node *
BuildTypeConversion (const char *name, namespace_t *ns, ntype *from, ntype *to, prf prf)
{
    node *result;
    node *avisarg;
    node *assign;
    node *block;

    DBUG_ENTER ();

    avisarg = TBmakeAvis (STRcpy ("from"), TYcopyType (from));
    AVIS_DECLTYPE (avisarg) = TYcopyType (AVIS_TYPE (avisarg));

    /*
     * return( res);
     */
    assign = TBmakeAssign (TBmakeReturn (
                             TBmakeExprs (TBmakeSpid (NULL, STRcpy ("result")), NULL)),
                           NULL);
    /*
     * res = _prf_( from, to, aviarg );
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("result"), NULL),
                                      TCmakePrf3 (prf, TBmakeType (TYcopyType (from)),
                                                  TBmakeType (TYcopyType (to)),
                                                  TBmakeId (avisarg))),
                           assign);

    /*
     * create the fundef body block
     */
    block = TBmakeBlock (assign, NULL);

    /*
     * create the fundef node
     */
    result = TBmakeFundef (STRcpy (name), NSdupNamespace (ns),
                           TBmakeRet (TYcopyType (to), NULL), TBmakeArg (avisarg, NULL),
                           block, NULL);

    FUNDEF_ISINLINE (result) = TRUE;

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

node *
GNTCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEFS (arg_info) = MODULE_FUNS (arg_node);
    INFO_FUNDECS (arg_info) = MODULE_FUNDECS (arg_node);

    MODULE_TYPES (arg_node) = TRAVopt(MODULE_TYPES (arg_node), arg_info);

    MODULE_FUNDECS (arg_node) = INFO_FUNDECS (arg_info);
    INFO_FUNDECS (arg_info) = NULL;
    MODULE_FUNS (arg_node) = INFO_FUNDEFS (arg_info);
    INFO_FUNDEFS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GNTCtypedef( node *arg_node, info *arg_info)
 *
 * @brief If the typedef is a nested type create the nest and denest function
 *
 *****************************************************************************/
node *
GNTCtypedef (node *arg_node, info *arg_info)
{
    node *to_fun, *from_fun;
    char *to_name, *from_name;
    ntype *tdef_type;
    usertype udt;

    DBUG_ENTER ();
    if (TYPEDEF_ISLOCAL (arg_node)) {
        udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

        DBUG_ASSERT (udt != UT_NOT_DEFINED, "Cannot find user type!");

        if (TYPEDEF_ISNESTED (arg_node)) {
            /*
             * Hide the types!
             */
            ntype *nested = TYmakeHiddenSimpleType (udt);
            nested = TYmakeAKS (nested, SHmakeShape (0));
            UTsetBaseType (udt, nested);

            tdef_type = TYmakeAKS (TYmakeUserType (udt), SHmakeShape (0));

            to_name = STRcat ("enclose_", TYPEDEF_NAME (arg_node));
            from_name = STRcat ("disclose_", TYPEDEF_NAME (arg_node));

            to_fun = BuildTypeConversion (to_name, TYPEDEF_NS (arg_node),
                                          TYPEDEF_NTYPE (arg_node), tdef_type, F_enclose);

            from_fun = BuildTypeConversion (from_name, TYPEDEF_NS (arg_node), tdef_type,
                                            TYPEDEF_NTYPE (arg_node), F_disclose);
            FUNDEF_NEXT (to_fun) = INFO_FUNDEFS (arg_info);
            FUNDEF_NEXT (from_fun) = to_fun;
            INFO_FUNDEFS (arg_info) = from_fun;

            tdef_type = TYfreeType (tdef_type);
        }
    }

    TYPEDEF_NEXT (arg_node) = TRAVopt(TYPEDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal -->
 *****************************************************************************/

#undef DBUG_PREFIX
