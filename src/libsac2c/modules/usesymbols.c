/*
 * $Id$
 */

#include "usesymbols.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "modulemanager.h"
#include "deserialize.h"
#include "new_types.h"
#include "symboltable.h"
#include "ctinfo.h"
#include "namespaces.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO {
    node *module;
    bool insidemop;
};

/*
 * INFO macros
 */
#define INFO_USS_MODULE(info) ((info)->module)
#define INFO_USS_INSIDEMOP(info) ((info)->insidemop)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_USS_MODULE (result) = NULL;
    INFO_USS_INSIDEMOP (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */

static bool
CheckSymbolVisibility (const namespace_t *ns, const char *symb)
{
    module_t *module;
    const sttable_t *table;
    stsymbol_t *symbol;
    bool result = TRUE;

    DBUG_ENTER ("CheckSymbolVisibility");

    module = MODMloadModule (NSgetModule (ns));
    table = MODMgetSymbolTable (module);
    symbol = STget (symb, table);

    if ((symbol == NULL)
        || ((!(STsymbolVisibility (symbol) == SVT_exported))
            && (!(STsymbolVisibility (symbol) == SVT_provided)))) {
        CTIerrorLine (global.linenum, "Symbol `%s::%s' not defined", NSgetName (ns),
                      symb);

        result = FALSE;
    }

    module = MODMunLoadModule (module);

    DBUG_RETURN (result);
}

static void
MakeSymbolAvailable (const namespace_t *ns, const char *symb, stentrytype_t type,
                     info *info)
{
    DBUG_ENTER ("MakeSymbolAvailable");

    if (!NSequals (ns, MODULE_NAMESPACE (INFO_USS_MODULE (info)))) {
        if (CheckSymbolVisibility (ns, symb)) {
            DSaddSymbolByName (symb, type, NSgetModule (ns));
        }
    }

    DBUG_VOID_RETURN;
}

/*
 * Traversal functions
 */

static ntype *
USSntype (ntype *arg_ntype, info *arg_info)
{
    ntype *scalar;

    DBUG_ENTER ("USSntype");

    /* get scalar base type of type */
    if (TYisArray (arg_ntype)) {
        scalar = TYgetScalar (arg_ntype);
    } else if (TYisScalar (arg_ntype)) {
        scalar = arg_ntype;
    } else {
        DBUG_ASSERT (0, "don't know what to do here");
        scalar = NULL;
    }

    /* if it is external, get the typedef */
    if (TYisSymb (scalar)) {
        MakeSymbolAvailable (TYgetNamespace (scalar), TYgetName (scalar), SET_typedef,
                             arg_info);
    }

    DBUG_RETURN (arg_ntype);
}

node *
USStypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USStypedef");

    if (TYPEDEF_NTYPE (arg_node) != NULL) {
        TYPEDEF_NTYPE (arg_node) = USSntype (TYPEDEF_NTYPE (arg_node), arg_info);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
USSobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSobjdef");

    if (OBJDEF_TYPE (arg_node) != NULL) {
        OBJDEF_TYPE (arg_node) = USSntype (OBJDEF_TYPE (arg_node), arg_info);
    }

    if (OBJDEF_EXPR (arg_node) != NULL) {
        OBJDEF_EXPR (arg_node) = TRAVdo (OBJDEF_EXPR (arg_node), arg_info);
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
USSavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSavis");

    if (AVIS_TYPE (arg_node) != NULL) {
        AVIS_TYPE (arg_node) = USSntype (AVIS_TYPE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
USScast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USScast");

    if (CAST_NTYPE (arg_node) != NULL) {
        CAST_NTYPE (arg_node) = USSntype (CAST_NTYPE (arg_node), arg_info);
    }

    CAST_EXPR (arg_node) = TRAVdo (CAST_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
USSarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSarray");

    if (ARRAY_ELEMTYPE (arg_node) != NULL) {
        ARRAY_ELEMTYPE (arg_node) = USSntype (ARRAY_ELEMTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
USSret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSret");

    if (RET_TYPE (arg_node) != NULL) {
        RET_TYPE (arg_node) = USSntype (RET_TYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
USSspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSspfold");

    MakeSymbolAvailable (SPFOLD_NS (arg_node), SPFOLD_FUN (arg_node), SET_wrapperhead,
                         arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
USSspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSspap");

    MakeSymbolAvailable (SPAP_NS (arg_node), SPAP_NAME (arg_node), SET_wrapperhead,
                         arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
USSspmop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSspmop");

    DBUG_ASSERT ((INFO_USS_INSIDEMOP (arg_info) == FALSE),
                 "illegal stacking of spmop found");

    if (SPMOP_OPS (arg_node) != NULL) {
        INFO_USS_INSIDEMOP (arg_info) = TRUE;

        SPMOP_OPS (arg_node) = TRAVdo (SPMOP_OPS (arg_node), arg_info);

        INFO_USS_INSIDEMOP (arg_info) = FALSE;
    }

    if (SPMOP_EXPRS (arg_node) != NULL) {
        SPMOP_EXPRS (arg_node) = TRAVdo (SPMOP_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
USSspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSspid");

    if (INFO_USS_INSIDEMOP (arg_info) == TRUE) {

        MakeSymbolAvailable (SPID_NS (arg_node), SPID_NAME (arg_node), SET_wrapperhead,
                             arg_info);
    } else {
        if (SPID_NS (arg_node) != NULL) {
            MakeSymbolAvailable (SPID_NS (arg_node), SPID_NAME (arg_node), SET_objdef,
                                 arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
USSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSmodule");

    INFO_USS_MODULE (arg_info) = arg_node;

    DSinitDeserialize (arg_node);

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

node *
USSdoUseSymbols (node *modul)
{
    info *info;

    DBUG_ENTER ("USSdoUseSymbols");

    info = MakeInfo ();

    TRAVpush (TR_uss);

    modul = TRAVdo (modul, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (modul);
}
