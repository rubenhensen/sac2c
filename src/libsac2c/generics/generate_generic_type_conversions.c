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
#include "type_utils.h"
#include "user_types.h"
#include "str.h"
#include "free.h"
#include "DupTree.h"
#include "tree_basic.h"
#include "shape.h"
#include "deserialize.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *providedsymbols;
    node *notprovidedsymbols;
    node *notexportedsymbols;
    node *fundefs;
    node *fundecs;
    node *append;
};

/**
 * A template entry in the template info structure
 */
#define INFO_PROVIDEDSYMBOLS(n) ((n)->providedsymbols)
#define INFO_NOTPROVIDEDSYMBOLS(n) ((n)->notprovidedsymbols)
#define INFO_NOTEXPORTEDSYMBOLS(n) ((n)->notexportedsymbols)
#define INFO_FUNDEFS(n) ((n)->fundefs)
#define INFO_FUNDECS(n) ((n)->fundecs)
#define INFO_APPEND(n) ((n)->append)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PROVIDEDSYMBOLS (result) = NULL;
    INFO_NOTPROVIDEDSYMBOLS (result) = NULL;
    INFO_NOTEXPORTEDSYMBOLS (result) = NULL;
    INFO_FUNDEFS (result) = NULL;
    INFO_FUNDECS (result) = NULL;
    INFO_APPEND (result) = NULL;

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

/** <!-- ****************************************************************** -->
 * @brief Inserts a fundec for <ns>::unwrap<name> into the funs chain and
 *        adds a symbol for that type to the symbols chain.
 *
 * @param ns          namespace of type
 * @param name        type name
 * @param *symbols    chain of provided symbols
 * @param *notexports chain of not to be exported symbols
 * @param funs        function chain
 *
 * @return modified funs chain
 ******************************************************************************/
static node *
BuildWrap (namespace_t *ns, const char *name, node **symbols, node **notexports,
           node *funs)
{
    node *result;
    char *funname;
    node *udtarg, *sourcearg;
    node *sacargret;
    usertype sacarg;

    DBUG_ENTER ("BuildWrap");

    funname = STRcat ("wrap", name);

    sacarg = UTfindUserType ("SACarg", NSgetNamespace (global.preludename));
    DBUG_ASSERT ((sacarg != UT_NOT_DEFINED), "cannot find SACarg udt in prelude!");
    sacargret = TBmakeRet (TYmakeAKS (TYmakeUserType (sacarg), SHmakeShape (0)), NULL);

    sourcearg = TBmakeArg (TBmakeAvis (TRAVtmpVar (),
                                       TYmakeAKS (TYmakeSymbType (STRcpy (name),
                                                                  NSdupNamespace (ns)),
                                                  SHmakeShape (0))),
                           NULL);
    udtarg = TBmakeArg (TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHmakeShape (0))),
                        sourcearg);

    RET_LINKSIGN (sacargret) = 1;
    RET_HASLINKSIGNINFO (sacargret) = TRUE;
    AVIS_DECLTYPE (ARG_AVIS (udtarg)) = TYcopyType (AVIS_TYPE (ARG_AVIS (udtarg)));
    ARG_LINKSIGN (udtarg) = 2;
    ARG_HASLINKSIGNINFO (udtarg) = TRUE;
    AVIS_DECLTYPE (ARG_AVIS (sourcearg)) = TYcopyType (AVIS_TYPE (ARG_AVIS (sourcearg)));
    ARG_LINKSIGN (sourcearg) = 3;
    ARG_HASLINKSIGNINFO (sourcearg) = TRUE;

    result = TBmakeFundef (STRcpy (funname), NSdupNamespace (ns), sacargret, udtarg, NULL,
                           funs);

    FUNDEF_LINKNAME (result) = STRcpy ("SACARGwrapUdt");
    FUNDEF_ISEXTERN (result) = TRUE;

    *symbols = TBmakeSymbol (STRcpy (funname), *symbols);
    *notexports = TBmakeSymbol (funname, *notexports);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Inserts a fundec for <ns>::unwrap into the funs chain and
 *        adds a symbol for that type to the symbols chain.
 *
 * @param ns          namespace of type
 * @param name        type name
 * @param *symbols    chain of provided symbols
 * @param *notexports chain of not to be exported symbols
 * @param funs        function chain
 *
 * @return
 ******************************************************************************/
static node *
BuildUnWrap (namespace_t *ns, const char *name, node **symbols, node **notexports,
             node *funs)
{
    node *result;
    char *funname;
    node *udtarg;
    node *destret;
    usertype sacarg;

    DBUG_ENTER ("BuildUnWrap");

    funname = STRcpy ("unwrap");

    destret = TBmakeRet (TYmakeAKS (TYmakeSymbType (STRcpy (name), NSdupNamespace (ns)),
                                    SHmakeShape (0)),
                         NULL);

    sacarg = UTfindUserType ("SACarg", NSgetNamespace (global.preludename));
    DBUG_ASSERT ((sacarg != UT_NOT_DEFINED), "cannot find SACarg udt in prelude!");
    udtarg = TBmakeArg (TBmakeAvis (TRAVtmpVar (),
                                    TYmakeAKS (TYmakeUserType (sacarg), SHmakeShape (0))),
                        NULL);

    RET_LINKSIGN (destret) = 1;
    RET_HASLINKSIGNINFO (destret) = TRUE;
    AVIS_DECLTYPE (ARG_AVIS (udtarg)) = TYcopyType (AVIS_TYPE (ARG_AVIS (udtarg)));
    ARG_LINKSIGN (udtarg) = 2;
    ARG_HASLINKSIGNINFO (udtarg) = TRUE;

    result
      = TBmakeFundef (STRcpy (funname), NSdupNamespace (ns), destret, udtarg, NULL, funs);

    FUNDEF_LINKNAME (result) = STRcpy ("SACARGunwrapUdt");
    FUNDEF_ISEXTERN (result) = TRUE;

    *symbols = TBmakeSymbol (STRcpy (funname), *symbols);
    *notexports = TBmakeSymbol (funname, *notexports);

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

    /*
     * load SACarg type
     */
    DSinitDeserialize (arg_node);
    DSaddSymbolByName ("SACarg", SET_typedef, global.preludename);
    DSfinishDeserialize (arg_node);

    INFO_FUNDEFS (arg_info) = MODULE_FUNS (arg_node);
    INFO_FUNDECS (arg_info) = MODULE_FUNDECS (arg_node);

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    /*
     * propagate notexported/notprovided information
     */
    if (MODULE_INTERFACE (arg_node) != NULL) {
        MODULE_INTERFACE (arg_node) = TRAVdo (MODULE_INTERFACE (arg_node), arg_info);
    }

    if (INFO_NOTEXPORTEDSYMBOLS (arg_info) != NULL) {
        INFO_NOTEXPORTEDSYMBOLS (arg_info)
          = FREEdoFreeTree (INFO_NOTEXPORTEDSYMBOLS (arg_info));
    }
    if (INFO_NOTPROVIDEDSYMBOLS (arg_info) != NULL) {
        INFO_NOTPROVIDEDSYMBOLS (arg_info)
          = FREEdoFreeTree (INFO_NOTPROVIDEDSYMBOLS (arg_info));
    }

    if (INFO_PROVIDEDSYMBOLS (arg_info) != NULL) {
        MODULE_INTERFACE (arg_node)
          = TBmakeProvide (MODULE_INTERFACE (arg_node), INFO_PROVIDEDSYMBOLS (arg_info));
        INFO_PROVIDEDSYMBOLS (arg_info) = NULL;
    }

    MODULE_FUNDECS (arg_node) = INFO_FUNDECS (arg_info);
    INFO_FUNDECS (arg_info) = NULL;
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

        INFO_NOTPROVIDEDSYMBOLS (arg_info)
          = TBmakeSymbol (STRcpy (to_name), INFO_NOTPROVIDEDSYMBOLS (arg_info));
        INFO_NOTEXPORTEDSYMBOLS (arg_info)
          = TBmakeSymbol (to_name, INFO_NOTEXPORTEDSYMBOLS (arg_info));
        INFO_NOTPROVIDEDSYMBOLS (arg_info)
          = TBmakeSymbol (STRcpy (from_name), INFO_NOTPROVIDEDSYMBOLS (arg_info));
        INFO_NOTEXPORTEDSYMBOLS (arg_info)
          = TBmakeSymbol (from_name, INFO_NOTEXPORTEDSYMBOLS (arg_info));

        tdef_type = TYfreeType (tdef_type);
    }

    if (TYPEDEF_ISLOCAL (arg_node) && !TYPEDEF_ISUNIQUE (arg_node)) {
        INFO_FUNDEFS (arg_info)
          = BuildWrap (TYPEDEF_NS (arg_node), TYPEDEF_NAME (arg_node),
                       &INFO_PROVIDEDSYMBOLS (arg_info),
                       &INFO_NOTEXPORTEDSYMBOLS (arg_info), INFO_FUNDEFS (arg_info));
        INFO_FUNDEFS (arg_info)
          = BuildUnWrap (TYPEDEF_NS (arg_node), TYPEDEF_NAME (arg_node),
                         &INFO_PROVIDEDSYMBOLS (arg_info),
                         &INFO_NOTEXPORTEDSYMBOLS (arg_info), INFO_FUNDEFS (arg_info));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGTCexport( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCexport");

    if (EXPORT_ALL (arg_node) && (INFO_NOTEXPORTEDSYMBOLS (arg_info) != NULL)) {
        if (EXPORT_SYMBOL (arg_node) == NULL) {
            EXPORT_SYMBOL (arg_node) = DUPdoDupTree (INFO_NOTEXPORTEDSYMBOLS (arg_info));
        } else {
            INFO_APPEND (arg_info) = DUPdoDupTree (INFO_NOTEXPORTEDSYMBOLS (arg_info));
            EXPORT_SYMBOL (arg_node) = TRAVdo (EXPORT_SYMBOL (arg_node), arg_info);
        }
    }

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGTCprovide( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCprovide");

    if (PROVIDE_ALL (arg_node) && (INFO_NOTPROVIDEDSYMBOLS (arg_info) != NULL)) {
        if (PROVIDE_SYMBOL (arg_node) == NULL) {
            PROVIDE_SYMBOL (arg_node) = DUPdoDupTree (INFO_NOTPROVIDEDSYMBOLS (arg_info));
        } else {
            INFO_APPEND (arg_info) = DUPdoDupTree (INFO_NOTPROVIDEDSYMBOLS (arg_info));
            PROVIDE_SYMBOL (arg_node) = TRAVdo (PROVIDE_SYMBOL (arg_node), arg_info);
        }
    }

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGTCsymbol( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCsymbol");

    if (INFO_APPEND (arg_info) != NULL) {
        if (SYMBOL_NEXT (arg_node) == NULL) {
            SYMBOL_NEXT (arg_node) = INFO_APPEND (arg_info);
            INFO_APPEND (arg_info) = NULL;
        } else {
            SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal -->
 *****************************************************************************/
