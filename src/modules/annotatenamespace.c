/*
 *
 * $Log$
 * Revision 1.18  2004/12/06 11:53:34  sah
 * fixed handling of fundefs
 *
 * Revision 1.17  2004/12/06 09:58:11  sah
 * fixed handling of args
 *
 * Revision 1.16  2004/12/05 21:05:20  sah
 * added namespace detection for ids aka global objects
 *
 * Revision 1.15  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.14  2004/11/25 21:48:01  sah
 * COMPILES!
 *
 *
 * Revision 1.1  2004/10/22 08:49:55  sah
 * Initial revision
 *
 */

#include "annotatenamespace.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "symboltable.h"
#include "new_types.h"
#include "stringset.h"
#include "free.h"
#include "Error.h"

/*
 * INFO structure
 */
struct INFO {
    sttable_t *symbols;
    const char *current;
    node *module;
    stringset_t *ids;
};

/*
 * INFO macros
 */
#define INFO_ANS_SYMBOLS(info) ((info)->symbols)
#define INFO_ANS_CURRENT(info) ((info)->current)
#define INFO_ANS_MODULE(info) ((info)->module)
#define INFO_ANS_IDS(info) ((info)->ids)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_ANS_SYMBOLS (result) = STinit ();
    INFO_ANS_CURRENT (result) = NULL;
    INFO_ANS_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_ANS_SYMBOLS (info) = STdestroy (INFO_ANS_SYMBOLS (info));

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * local helper functions
 */
static void
CheckUseUnique (sttable_t *table)
{
    stsymboliterator_t *iterator;

    DBUG_ENTER ("CheckUseUnique");

    iterator = STsymbolIteratorGet (table);

    while (STsymbolIteratorHasMore (iterator)) {
        stsymbol_t *symbol = STsymbolIteratorNext (iterator);
        stentryiterator_t *entries = STentryIteratorGet (STsymbolName (symbol), table);

        if (STentryIteratorHasMore (entries)) {
            stentry_t *entry = STentryIteratorNext (entries);

            if (STentryIteratorHasMore (entries)) {
                ERROR (0, ("Symbol `%s' used more than once", symbol));
                CONT_ERROR (("... from module `%s'", STentryName (entry)));

                while (STentryIteratorHasMore (entries)) {
                    entry = STentryIteratorNext (entries);
                    CONT_ERROR (("... from module `%s'", STentryName (entry)));
                }
            }
        }

        entries = STentryIteratorRelease (entries);
    }

    iterator = STsymbolIteratorRelease (iterator);

    DBUG_VOID_RETURN;
}

static void
CheckLocalNameClash (const char *symbol, sttable_t *table, int lineno)
{
    DBUG_ENTER ("CheckLocalNameClash");

    if (STcontains (symbol, table)) {
        ERROR (lineno, ("Symbol `%s' used and locally defined", symbol));
    }

    DBUG_VOID_RETURN;
}

static char *
LookupNamespaceForSymbol (const char *name, info *info)
{
    char *result;

    DBUG_ENTER ("LookupNamespaceForSymbol");

    if (STcontains (name, INFO_ANS_SYMBOLS (info))) {
        /*
         * There is a namespace annotated to this symbolname, e.g. it was used
         * -> use that namespace
         */
        stentry_t *entry = STgetFirstEntry (name, INFO_ANS_SYMBOLS (info));

        result = ILIBstringCopy (STentryName (entry));
    } else {
        /*
         * this symbol is local
         */
        result = ILIBstringCopy (MODULE_NAME (INFO_ANS_MODULE (info)));
    }

    DBUG_RETURN (result);
}

static void
AddNamespaceToDependencies (const char *ns, info *info)
{
    DBUG_ENTER ("AddNamespaceToDependencies");

    if (!ILIBstringCompare (MODULE_NAME (INFO_ANS_MODULE (info)), ns)) {
        /*
         * this symbol comes from another namespace
         *  -> add the namespace to the dependency list
         */
        MODULE_DEPENDENCIES (INFO_ANS_MODULE (info))
          = STRSadd (ns, STRS_saclib, MODULE_DEPENDENCIES (INFO_ANS_MODULE (info)));
    }

    DBUG_VOID_RETURN;
}

/*
 * Traversal functions
 */

static types *
ANStypes (types *arg_types, info *arg_info)
{
    DBUG_ENTER ("ANStypes");

    if (TYPES_MOD (arg_types) == NULL) {
        /* look up correct type */
        TYPES_MOD (arg_types)
          = LookupNamespaceForSymbol (TYPES_NAME (arg_types), arg_info);
    }

    AddNamespaceToDependencies (TYPES_MOD (arg_types), arg_info);

    DBUG_RETURN (arg_types);
}

static ntype *
ANSntype (ntype *arg_ntype, info *arg_info)
{
    ntype *scalar = NULL;

    DBUG_ENTER ("ANSntype");

    if (TYisArray (arg_ntype)) {
        scalar = TYgetScalar (arg_ntype);
    } else if (TYisScalar (arg_ntype)) {
        scalar = arg_ntype;
    }
#ifndef DBUG_OFF
    else {
        DBUG_ASSERT (0, "ask the guy who implemented the type system what has "
                        "happened...");
    }
#endif

    if (TYisSymb (scalar)) {
        if (TYgetMod (scalar) == NULL) {
            /* we have to add the correct namespace here */
            scalar = TYsetMod (scalar,
                               LookupNamespaceForSymbol (TYgetName (scalar), arg_info));

            DBUG_PRINT ("ANS", ("Updated namespace for type %s to %s", TYgetName (scalar),
                                TYgetMod (scalar)));
        }

        /* save the namespace as a dependency */
        AddNamespaceToDependencies (TYgetMod (scalar), arg_info);
    }

    DBUG_RETURN (arg_ntype);
}

node *
ANSsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSsymbol");

    STadd (SYMBOL_ID (arg_node), SVT_local, INFO_ANS_CURRENT (arg_info), SET_namespace,
           INFO_ANS_SYMBOLS (arg_info));

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSuse (node *arg_node, info *arg_info)
{
    node *result;

    DBUG_ENTER ("ANSuse");

    INFO_ANS_CURRENT (arg_info) = USE_MOD (arg_node);

    USE_SYMBOL (arg_node) = TRAVdo (USE_SYMBOL (arg_node), arg_info);

    INFO_ANS_CURRENT (arg_info) = NULL;

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
    }

    /* the use information is no more needed from
     * this point on, so we can free this use
     */

    result = USE_NEXT (arg_node);
    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (result);
}

node *
ANSimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSimport");

    /* imports are ignored in this traversal as imported
       functions are treated like local functions         */

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSexport");

    /* exports are ignored in this traversal */

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSProvide");

    /* provides are ignored in this traversal */

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSfundef");

    CheckLocalNameClash (FUNDEF_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info),
                         NODE_LINE (arg_node));

    if (FUNDEF_MOD (arg_node) == NULL) {
        FUNDEF_MOD (arg_node) = ILIBstringCopy (MODULE_NAME (INFO_ANS_MODULE (arg_info)));
    }

    if (FUNDEF_TYPES (arg_node) != NULL) {
        FUNDEF_TYPES (arg_node) = ANStypes (FUNDEF_TYPES (arg_node), arg_info);
    }

    INFO_ANS_IDS (arg_info) = NULL;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_ANS_IDS (arg_info) = STRSfree (INFO_ANS_IDS (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANStypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANStypedef");

    CheckLocalNameClash (TYPEDEF_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info),
                         NODE_LINE (arg_node));

    if (TYPEDEF_MOD (arg_node) == NULL) {
        TYPEDEF_MOD (arg_node)
          = ILIBstringCopy (MODULE_NAME (INFO_ANS_MODULE (arg_info)));
    }

    if (TYPEDEF_NTYPE (arg_node) != NULL) {
        TYPEDEF_NTYPE (arg_node) = ANSntype (TYPEDEF_NTYPE (arg_node), arg_info);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVcont (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSobjdef");

    CheckLocalNameClash (OBJDEF_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info),
                         NODE_LINE (arg_node));

    if (OBJDEF_MOD (arg_node) == NULL) {
        OBJDEF_MOD (arg_node) = ILIBstringCopy (MODULE_NAME (INFO_ANS_MODULE (arg_info)));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSspap");

    if (SPAP_MOD (arg_node) == NULL) {
        /*
         * look up the correct namespace
         */

        SPAP_MOD (arg_node) = LookupNamespaceForSymbol (SPAP_NAME (arg_node), arg_info);
    }

    AddNamespaceToDependencies (SPAP_MOD (arg_node), arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSarg");

    INFO_ANS_IDS (arg_info)
      = STRSadd (ARG_NAME (arg_node), STRS_unknown, INFO_ANS_IDS (arg_info));

    if (ARG_TYPE (arg_node) != NULL) {
        ARG_TYPE (arg_node) = ANStypes (ARG_TYPE (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSvardec");

    if (VARDEC_TYPE (arg_node) != NULL) {
        VARDEC_TYPE (arg_node) = ANStypes (VARDEC_TYPE (arg_node), arg_info);
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSspids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSspids");

    /*
     * mark this id as locally defined
     */
    INFO_ANS_IDS (arg_info)
      = STRSadd (SPIDS_NAME (arg_node), STRS_unknown, INFO_ANS_IDS (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSspid");

    if (SPID_MOD (arg_node) == NULL) {
        /*
         * check whether this id is local
         */
        if (!STRScontains (SPID_NAME (arg_node), INFO_ANS_IDS (arg_info))) {
            /*
             * look up the correct namespace
             */

            SPID_MOD (arg_node)
              = LookupNamespaceForSymbol (SPID_NAME (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ANSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSlet");

    /*
     * first traverse all defining ids
     */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    /*
     * then the RHS
     */
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSwith");

    /*
     * make sure, the withid is traversed prior to the code!
     */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSmodule");

    INFO_ANS_MODULE (arg_info) = arg_node;

    /* traverse use/import statements */

    if (MODULE_IMPORTS (arg_node) != NULL) {
        MODULE_IMPORTS (arg_node) = TRAVdo (MODULE_IMPORTS (arg_node), arg_info);
    }

    /* check uniqueness property of symbols in use */

    CheckUseUnique (INFO_ANS_SYMBOLS (arg_info));

    /* traverse fundecs */

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    /* traverse fundefs */

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /* traverse typedefs */

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    /* traverse objdefs */

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSdoAnnotateNamespace (node *module)
{
    info *info;

    DBUG_ENTER ("ANSdoAnnotateNamespace");

    info = MakeInfo ();

    TRAVpush (TR_ans);

    module = TRAVdo (module, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (module);
}
