/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup ggtc Generate Generic Type Conversions
 *
 * Generates generic conversion functions:
 *   - to_XXX / from_XXX for classtypes
 *   - wrapXXX / unwrapXXX for user defined types
 *
 * @ingroup ggtc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file generate_generic_type_conversions.c
 *
 * Prefix: GGTC
 *
 *****************************************************************************/
#include "generate_generic_type_conversions.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "namespaces.h"
#include "new_types.h"
#include "str.h"
#include "tree_basic.h"
#include "shape.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *providedsymbols;
    node *fundefs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_PROVIDEDSYMBOLS(n) ((n)->providedsymbols)
#define INFO_FUNDEFS(n) ((n)->fundefs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PROVIDEDSYMBOLS (result) = NULL;
    INFO_FUNDEFS (result) = NULL;

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
 * @fn node *GGTCdoGenerateGenericTypeConversions( node *syntax_tree)
 *
 *****************************************************************************/
node *
GGTCdoGenerateGenericTypeConversions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("GGTCdoGenerateGenericTypeConversions");

    info = MakeInfo ();

    TRAVpush (TR_ggtc);
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
 * @brief builds a type conversion fun converting from type from to type to.
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
BuildTypeConversion (const char *name, const namespace_t *ns, ntype *from, ntype *to,
                     prf prf)
{
    node *result;
    node *avisarg;
    node *assign;
    node *block;

    DBUG_ENTER ("BuildTypeConversion");

    avisarg = TBmakeAvis (STRcpy ("from"), TYcopyType (from));
    AVIS_DECLTYPE (avisarg) = TYcopyType (AVIS_TYPE (avisarg));

    /*
     * return( res);
     */
    assign = TBmakeAssign (TBmakeReturn (
                             TBmakeExprs (TBmakeSpid (NULL, STRcpy ("result")), NULL)),
                           NULL);
    /*
     * res = prf( (:restype) arg);
     */
    assign = TBmakeAssign (TBmakeLet (TBmakeSpids (STRcpy ("result"), NULL),
                                      TBmakeCast (TYcopyType (to), TBmakeId (avisarg))),
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
 * @fn node *GGTCmodule( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCmodule");

    INFO_FUNDEFS (arg_info) = MODULE_FUNS (arg_node);

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    MODULE_FUNS (arg_node) = INFO_FUNDEFS (arg_info);
    INFO_FUNDEFS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGTCtypedef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCtypedef");

    if (TYPEDEF_ISUNIQUE (arg_node) && TYPEDEF_ISLOCAL (arg_node)) {
        node *to_fun, *from_fun;
        char *to_name, *from_name;
        ntype *tdef_type;

        to_name = STRcat ("to_", TYPEDEF_NAME (arg_node));
        from_name = STRcat ("from_", TYPEDEF_NAME (arg_node));

        tdef_type = TYmakeAKS (TYmakeSymbType (STRcpy (TYPEDEF_NAME (arg_node)),
                                               NSdupNamespace (TYPEDEF_NS (arg_node))),
                               SHmakeShape (0));

        to_fun = BuildTypeConversion (to_name, TYPEDEF_NS (arg_node),
                                      TYPEDEF_NTYPE (arg_node), tdef_type, F_to_unq);

        from_fun = BuildTypeConversion (from_name, TYPEDEF_NS (arg_node), tdef_type,
                                        TYPEDEF_NTYPE (arg_node), F_from_unq);

        FUNDEF_NEXT (to_fun) = INFO_FUNDEFS (arg_info);
        FUNDEF_NEXT (from_fun) = to_fun;
        INFO_FUNDEFS (arg_info) = from_fun;

        tdef_type = TYfreeType (tdef_type);
        to_name = MEMfree (to_name);
        from_name = MEMfree (from_name);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal -->
 *****************************************************************************/
