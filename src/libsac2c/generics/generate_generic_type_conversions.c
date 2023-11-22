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

#define SACARG_NAME "SACarg"
/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
#include "globals.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 * @brief Returns the name of the innermost simpletype of a user
 *        defined type.
 *
 * @param ns    namespace of type
 * @param name  name of type
 *
 * @return      appropriate string for that type
 ******************************************************************************/
static const char *
GetInnerTypeName (namespace_t *ns, const char *name)
{
    const char *result;
    ntype *base;
    usertype udt;

    DBUG_ENTER ();

    udt = UTfindUserType (name, ns);

    DBUG_ASSERT (udt != UT_NOT_DEFINED, "cannot find usertype for typedef!");

    udt = UTgetUnAliasedType (udt);

    do {
        base = UTgetBaseType (udt);
    } while (TUisArrayOfUser (base));

    switch (TYgetSimpleType (TYgetScalar (base))) {
    case T_byte:
        result = "Byte";
        break;
    case T_short:
        result = "Short";
        break;
    case T_int:
        result = "Int";
        break;
    case T_long:
        result = "Long";
        break;
    case T_longlong:
        result = "Longlong";
        break;
    case T_ubyte:
        result = "Ubyte";
        break;
    case T_ushort:
        result = "Ushort";
        break;
    case T_uint:
        result = "Uint";
        break;
    case T_ulong:
        result = "Ulong";
        break;
    case T_ulonglong:
        result = "Ulonglong";
        break;
    case T_bool:
        result = "Bool";
        break;
    case T_float:
        result = "Float";
        break;
    case T_double:
        result = "Double";
        break;
    case T_char:
        result = "Char";
        break;
    case T_hidden:
        result = "";
        break;
    default:
        DBUG_UNREACHABLE ("unhandled simple type");
        result = "Unknown";
        break;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Constructs the apropriate linkname for a wrap function.
 *
 * @param ns   namespace of type
 * @param name name of type
 *
 * @return     a matching linksign name
 ******************************************************************************/
static char *
GetWrapUdtLinkName (namespace_t *ns, const char *name)
{
    char *result;

    DBUG_ENTER ();

    result = STRcat ("SACARGwrapUdt", GetInnerTypeName (ns, name));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Constructs the apropriate linkname for an unwrap function.
 *
 * @param ns   namespace of type
 * @param name name of type
 *
 * @return     a matching linksign name
 ******************************************************************************/
static char *
GetUnwrapUdtLinkName (namespace_t *ns, const char *name)
{
    char *result;

    DBUG_ENTER ();

    result = STRcat ("SACARGunwrapUdt", GetInnerTypeName (ns, name));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Inserts a fundec for <ns>::wrap<name> into the funs chain and
 *        adds a symbol for that type to the symbols chain.
 *
 * @param type        the actual type
 * @param ns          namespace of type
 * @param name        type name
 * @param *symbols    chain of provided symbols
 * @param *notexports chain of not to be exported symbols
 * @param funs        function chain
 *
 * @return modified funs chain
 ******************************************************************************/
static node *
BuildWrap (ntype *type, namespace_t *ns, const char *name, node **symbols,
           node **notexports, node *funs)
{
    node *result;
    char *funname;
    node *udtarg, *sourcearg;
    node *sacargret;
    usertype sacargudt;
    ntype *argtype;

    DBUG_ENTER ();

    funname = STRcat ("wrap", name);

    sacargudt = UTfindUserType (SACARG_NAME, NSgetNamespace (global.preludename));

    DBUG_ASSERT (sacargudt != UT_NOT_DEFINED, "Cannot find sacarg udt!");

    sacargret = TBmakeRet (TYmakeAKS (TYmakeUserType (sacargudt), SHmakeShape (0)), NULL);

    /*
     * External user defined types only occur as scalars and that's
     * how the backend likes it. So we better build a scalar version here.
     * The underlying problem is that non-scalar versions would
     * trigger a copy to create the AUD which is not neccessary.
     * One fine day, somebody might want to extend the backend, though.
     */
    if (TUisHidden (type)) {
        argtype = TYmakeAKS (TYcopyType (TYgetScalar (type)), SHmakeShape (0));
    } else {
        argtype = TYmakeAUD (TYcopyType (TYgetScalar (type)));
    }

    sourcearg = TBmakeArg (TBmakeAvis (TRAVtmpVar (), argtype), NULL);

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

    FUNDEF_LINKNAME (result) = GetWrapUdtLinkName (ns, name);
    FUNDEF_ISEXTERN (result) = TRUE;
    FUNDEF_ISSACARGCONVERSION (result) = TRUE;

    *symbols = TBmakeSymbol (STRcpy (funname), *symbols);
    *notexports = TBmakeSymbol (funname, *notexports);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Inserts a fundec for <ns>::unwrap<name> into the funs chain and
 *        adds a symbol for that type to the symbols chain.
 *
 * @param type        the actual type
 * @param ns          namespace of type
 * @param name        type name
 * @param *symbols    chain of provided symbols
 * @param *notexports chain of not to be exported symbols
 * @param funs        function chain
 *
 * @return
 ******************************************************************************/
static node *
BuildUnWrap (ntype *type, namespace_t *ns, const char *name, node **symbols,
             node **notexports, node *funs)
{
    node *result;
    char *funname;
    node *udtarg;
    node *destret;
    usertype sacargudt;
    ntype *rettype;

    DBUG_ENTER ();

    funname = STRcat ("unwrap", name);

    sacargudt = UTfindUserType (SACARG_NAME, NSgetNamespace (global.preludename));

    DBUG_ASSERT (sacargudt != UT_NOT_DEFINED, "Cannot find sacarg udt!");

    /*
     * we have to ensure that the return is a scalar type for
     * external udts as array types of externals are not
     * supported properly by the backend.
     */
    if (TUisHidden (type)) {
        rettype = TYmakeAKS (TYcopyType (TYgetScalar (type)), SHmakeShape (0));
    } else {
        rettype = TYmakeAUD (TYcopyType (TYgetScalar (type)));
    }

    destret = TBmakeRet (rettype, NULL);

    udtarg = TBmakeArg (TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeUserType (sacargudt),
                                                              SHmakeShape (0))),
                        NULL);

    RET_LINKSIGN (destret) = 1;
    RET_HASLINKSIGNINFO (destret) = TRUE;
    AVIS_DECLTYPE (ARG_AVIS (udtarg)) = TYcopyType (AVIS_TYPE (ARG_AVIS (udtarg)));
    ARG_LINKSIGN (udtarg) = 2;
    ARG_HASLINKSIGNINFO (udtarg) = TRUE;

    result
      = TBmakeFundef (STRcpy (funname), NSdupNamespace (ns), destret, udtarg, NULL, funs);

    FUNDEF_LINKNAME (result) = GetUnwrapUdtLinkName (ns, name);
    FUNDEF_ISEXTERN (result) = TRUE;
    FUNDEF_ISSACARGCONVERSION (result) = TRUE;

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
    DBUG_ENTER ();

    /*
     * load SACarg type if not compiling prelude
     */
    if (global.loadprelude) {
        DSinitDeserialize (arg_node);
        DSaddSymbolByName ("SACarg", SET_typedef, global.preludename);
        DSfinishDeserialize (arg_node);
    }

    INFO_FUNDEFS (arg_info) = MODULE_FUNS (arg_node);
    INFO_FUNDECS (arg_info) = MODULE_FUNDECS (arg_node);

    MODULE_TYPES (arg_node) = TRAVopt(MODULE_TYPES (arg_node), arg_info);

    /*
     * propagate notexported/notprovided information
     */
    MODULE_INTERFACE (arg_node) = TRAVopt(MODULE_INTERFACE (arg_node), arg_info);

    INFO_NOTEXPORTEDSYMBOLS (arg_info) = FREEoptFreeTree(INFO_NOTEXPORTEDSYMBOLS (arg_info));
    INFO_NOTPROVIDEDSYMBOLS (arg_info) = FREEoptFreeTree(INFO_NOTPROVIDEDSYMBOLS (arg_info));

    if (INFO_PROVIDEDSYMBOLS (arg_info) != NULL) {
        /*
         * add provides only if we are not in the top
         * namespace, i.e., compiling a program.
         */
        if (global.filetype != FT_prog) {
            MODULE_INTERFACE (arg_node) = TBmakeProvide (MODULE_INTERFACE (arg_node),
                                                         INFO_PROVIDEDSYMBOLS (arg_info));
        }
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
    node *to_fun, *from_fun;
    char *to_name, *from_name;
    ntype *tdef_type;
    usertype udt;

    DBUG_ENTER ();

    if (TYPEDEF_ISLOCAL (arg_node)) {
        udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

        DBUG_ASSERT (udt != UT_NOT_DEFINED, "Cannot find user type!");

        tdef_type = TYmakeAKS (TYmakeUserType (udt), SHmakeShape (0));

        if (TYPEDEF_ISUNIQUE (arg_node)) {
            to_name = STRcat ("to_", TYPEDEF_NAME (arg_node));
            from_name = STRcat ("from_", TYPEDEF_NAME (arg_node));

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
        }

        if (!TYPEDEF_ISNESTED (arg_node)) {
            INFO_FUNDEFS (arg_info)
              = BuildWrap (tdef_type, TYPEDEF_NS (arg_node), TYPEDEF_NAME (arg_node),
                           &INFO_PROVIDEDSYMBOLS (arg_info),
                           &INFO_NOTEXPORTEDSYMBOLS (arg_info), INFO_FUNDEFS (arg_info));
            INFO_FUNDEFS (arg_info)
              = BuildUnWrap (tdef_type, TYPEDEF_NS (arg_node), TYPEDEF_NAME (arg_node),
                             &INFO_PROVIDEDSYMBOLS (arg_info),
                             &INFO_NOTEXPORTEDSYMBOLS (arg_info),
                             INFO_FUNDEFS (arg_info));
        }

        tdef_type = TYfreeType (tdef_type);
    }

    TYPEDEF_NEXT (arg_node) = TRAVopt(TYPEDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    if (EXPORT_ALL (arg_node) && (INFO_NOTEXPORTEDSYMBOLS (arg_info) != NULL)) {
        if (EXPORT_SYMBOL (arg_node) == NULL) {
            EXPORT_SYMBOL (arg_node) = DUPdoDupTree (INFO_NOTEXPORTEDSYMBOLS (arg_info));
        } else {
            INFO_APPEND (arg_info) = DUPdoDupTree (INFO_NOTEXPORTEDSYMBOLS (arg_info));
            EXPORT_SYMBOL (arg_node) = TRAVdo (EXPORT_SYMBOL (arg_node), arg_info);
        }
    }

    EXPORT_NEXT (arg_node) = TRAVopt(EXPORT_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    if (PROVIDE_ALL (arg_node) && (INFO_NOTPROVIDEDSYMBOLS (arg_info) != NULL)) {
        if (PROVIDE_SYMBOL (arg_node) == NULL) {
            PROVIDE_SYMBOL (arg_node) = DUPdoDupTree (INFO_NOTPROVIDEDSYMBOLS (arg_info));
        } else {
            INFO_APPEND (arg_info) = DUPdoDupTree (INFO_NOTPROVIDEDSYMBOLS (arg_info));
            PROVIDE_SYMBOL (arg_node) = TRAVdo (PROVIDE_SYMBOL (arg_node), arg_info);
        }
    }

    PROVIDE_NEXT (arg_node) = TRAVopt(PROVIDE_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

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

#undef DBUG_PREFIX
