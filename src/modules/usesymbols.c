/*
 *
 * $Log$
 * Revision 1.12  2005/03/17 14:02:26  sah
 * corrected handling of mops
 *
 * Revision 1.11  2005/01/07 19:40:45  cg
 * Converted compile time output from Error.h to ctinfo.c
 *
 * Revision 1.10  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.9  2004/11/25 11:35:11  sah
 * COMPILES
 *
 * Revision 1.8  2004/11/17 19:48:50  sah
 * interface changes
 *
 * Revision 1.7  2004/11/14 15:23:45  sah
 * some cleanup
 *
 * Revision 1.6  2004/11/11 14:29:40  sah
 * added some traversal functions for USS traversal
 *
 * Revision 1.5  2004/11/07 18:06:05  sah
 * fixed a minor bug
 *
 * Revision 1.4  2004/10/28 17:20:09  sah
 * now deserialisation has an internal state
 * ,
 *
 * Revision 1.3  2004/10/26 09:33:56  sah
 * ongoing implementation
 *
 * Revision 1.2  2004/10/22 14:48:16  sah
 * fixed some typeos
 *
 * Revision 1.1  2004/10/22 13:50:44  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "usesymbols.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "modulemanager.h"
#include "deserialize.h"
#include "new_types.h"
#include "symboltable.h"
#include "ctinfo.h"

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

    result = ILIBmalloc (sizeof (info));

    INFO_USS_MODULE (result) = NULL;
    INFO_USS_INSIDEMOP (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * helper functions
 */

static void
CheckSymbolVisibility (const char *mod, const char *symb)
{
    module_t *module;
    sttable_t *table;
    stsymbol_t *symbol;

    DBUG_ENTER ("CheckSymbolVisibility");

    module = MODMloadModule (mod);
    table = MODMgetSymbolTable (module);
    symbol = STget (symb, table);

    if ((symbol == NULL)
        || ((!(STsymbolVisibility (symbol) == SVT_exported))
            && (!(STsymbolVisibility (symbol) == SVT_provided)))) {
        CTIerrorLine (global.linenum, "Symbol `%s:%s' not defined", mod, symb);
    }

    table = STdestroy (table);
    module = MODMunLoadModule (module);

    DBUG_VOID_RETURN;
}

static void
MakeSymbolAvailable (const char *mod, const char *symb, stentrytype_t type, info *info)
{
    DBUG_ENTER ("MakeSymbolAvailable");

    if (!ILIBstringCompare (mod, MODULE_NAME (INFO_USS_MODULE (info)))) {
        CheckSymbolVisibility (mod, symb);

        DSaddSymbolByName (symb, type, mod);
    }

    DBUG_VOID_RETURN;
}

/*
 * Traversal functions
 */

ntype *
USSNtype (ntype *arg_ntype, info *arg_info)
{
    ntype *scalar;

    DBUG_ENTER ("USSNtype");

    /* get scalar base type of type */
    if (TYisArray (arg_ntype)) {
        scalar = TYgetScalar (arg_ntype);
    } else if (TYisScalar (arg_ntype)) {
        scalar = arg_ntype;
    } else {
        DBUG_ASSERT (0, "don't know what to do here");
    }

    /* if it is external, get the typedef */
    if (TYisSymb (scalar)) {
        MakeSymbolAvailable (TYgetMod (scalar), TYgetName (scalar), SET_typedef,
                             arg_info);
    }

    DBUG_RETURN (arg_ntype);
}

node *
USStypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USStypedef");

    if (TYPEDEF_NTYPE (arg_node) != NULL) {
        TYPEDEF_NTYPE (arg_node) = USSNtype (TYPEDEF_NTYPE (arg_node), arg_info);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
USSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSarg");

    DBUG_RETURN (arg_node);
}

node *
USSvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSvardec");

    DBUG_RETURN (arg_node);
}

node *
USSfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSfold");

    DBUG_RETURN (arg_node);
}

node *
USSspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("USSspap");

    MakeSymbolAvailable (SPAP_MOD (arg_node), SPAP_NAME (arg_node), SET_wrapperhead,
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

        MakeSymbolAvailable (SPID_MOD (arg_node), SPID_NAME (arg_node), SET_wrapperhead,
                             arg_info);
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
