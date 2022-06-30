#define DBUG_PREFIX "ANS"
#include "debug.h"

#include "annotatenamespace.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "symboltable.h"
#include "new_types.h"
#include "stringset.h"
#include "free.h"
#include "ctinfo.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"

/*
 * INFO structure
 */
struct INFO {
    sttable_t *symbols;
    const char *current;
    node *module;
    stringset_t *ids;
    bool insidemop;
    bool insideobjlist;
    bool checkimport;
};

/*
 * INFO macros
 */
#define INFO_SYMBOLS(info) ((info)->symbols)
#define INFO_CURRENT(info) ((info)->current)
#define INFO_MODULE(info) ((info)->module)
#define INFO_IDS(info) ((info)->ids)
#define INFO_INSIDEMOP(info) ((info)->insidemop)
#define INFO_INSIDEOBJLIST(info) ((info)->insideobjlist)
#define INFO_CHECKIMPORT(info) ((info)->checkimport)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SYMBOLS (result) = STinit ();
    INFO_CURRENT (result) = NULL;
    INFO_MODULE (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_INSIDEMOP (result) = FALSE;
    INFO_INSIDEOBJLIST (result) = FALSE;
    INFO_CHECKIMPORT (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_SYMBOLS (info) = STdestroy (INFO_SYMBOLS (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * local helper functions
 */
static void
CheckUseUnique (sttable_t *table)
{
    stsymboliterator_t *iterator;

    DBUG_ENTER ();

    iterator = STsymbolIteratorGet (table);

    while (STsymbolIteratorHasMore (iterator)) {
        stsymbol_t *symbol = STsymbolIteratorNext (iterator);
        stentryiterator_t *entries = STentryIteratorGet (STsymbolName (symbol), table);

        if (STentryIteratorHasMore (entries)) {
            stentry_t *entry = STentryIteratorNext (entries);

            if (STentryIteratorHasMore (entries)) {
                CTIerrorBegin (EMPTY_LOC, 
                               "Symbol `%s' used more than once\n"
                               "... from module `%s'", 
                               STsymbolName (symbol), STentryName (entry));

                while (STentryIteratorHasMore (entries)) {
                    entry = STentryIteratorNext (entries);
                    CTIerrorContinued ("... from module `%s'", STentryName (entry));
                }
                CTIerrorEnd ();
            }
        }

        entries = STentryIteratorRelease (entries);
    }

    iterator = STsymbolIteratorRelease (iterator);

    DBUG_RETURN ();
}

static void
CheckImportNameClash (const char *symbol, const char *module, sttable_t *table)
{
    stentryiterator_t *iterator;

    DBUG_ENTER ();

    if (STcontains (symbol, table)) {
        iterator = STentryIteratorGet (symbol, table);

        CTIerrorBegin (EMPTY_LOC, "Symbol `%s' imported from module '%s' and", 
                       symbol, module);

        while (STentryIteratorHasMore (iterator)) {
            CTIerrorContinued ("...used from module '%s'",
                               STentryName (STentryIteratorNext (iterator)));
        }
        CTIerrorEnd ();
        iterator = STentryIteratorRelease (iterator);
    }

    DBUG_RETURN ();
}

static void
CheckLocalNameClash (const char *symbol, sttable_t *table, struct location loc)
{
    DBUG_ENTER ();

    if (STcontains (symbol, table)) {
        CTIerror (loc, "Symbol `%s' used and locally defined", symbol);
    }

    DBUG_RETURN ();
}

namespace_t *
LookupNamespaceForSymbol (const char *name, info *info)
{
    namespace_t *result;

    DBUG_ENTER ();

    if (STcontains (name, INFO_SYMBOLS (info))) {
        /*
         * There is a namespace annotated to this symbolname, e.g. it was used
         * -> use that namespace
         */
        stentry_t *entry = STgetFirstEntry (name, INFO_SYMBOLS (info));

        result = NSgetNamespace (STentryName (entry));
    } else {
        /*
         * this symbol is local
         */
        result = NSdupNamespace (MODULE_NAMESPACE (INFO_MODULE (info)));
    }

    DBUG_RETURN (result);
}

/*
 * Traversal functions
 */

static ntype *
ANSntype (ntype *arg_ntype, info *arg_info)
{
    ntype *scalar = NULL;

    DBUG_ENTER ();

    if (TYisArray (arg_ntype)) {
        scalar = TYgetScalar (arg_ntype);
    } else if (TYisScalar (arg_ntype)) {
        scalar = arg_ntype;
    }
#ifndef DBUG_OFF
    else {
        DBUG_UNREACHABLE ("ask the guy who implemented the type system what has "
                          "happened...");
    }
#endif

    if (TYisSymb (scalar)) {
        if (TYgetNamespace (scalar) == NULL) {
            /* we have to add the correct namespace here */
            scalar = TYsetNamespace (scalar, LookupNamespaceForSymbol (TYgetName (scalar),
                                                                       arg_info));

            DBUG_PRINT ("Updated namespace for type %s to %s", TYgetName (scalar),
                        NSgetName (TYgetNamespace (scalar)));
        }
    }

    DBUG_RETURN (arg_ntype);
}

node *
ANSsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CHECKIMPORT (arg_info)) {
        CheckImportNameClash (SYMBOL_ID (arg_node), INFO_CURRENT (arg_info),
                              INFO_SYMBOLS (arg_info));
    } else {
        STadd (SYMBOL_ID (arg_node), SVT_local, INFO_CURRENT (arg_info), SET_namespace,
               INFO_SYMBOLS (arg_info), 0); /* XXX may be this nees adjustment... */
    }

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSuse (node *arg_node, info *arg_info)
{
    node *result;

    DBUG_ENTER ();

    INFO_CURRENT (arg_info) = USE_MOD (arg_node);

    USE_SYMBOL (arg_node) = TRAVdo (USE_SYMBOL (arg_node), arg_info);

    INFO_CURRENT (arg_info) = NULL;

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
    DBUG_ENTER ();

    /*
     * we do not add imports to the symbol list as
     * they are not needed for namespace annotation.
     */

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    /*
     * on the way up, we check whether we have common symbols in
     * use and import statements.
     */
    INFO_CHECKIMPORT (arg_info) = TRUE;
    INFO_CURRENT (arg_info) = IMPORT_MOD (arg_node);

    if (IMPORT_SYMBOL (arg_node) != NULL) {
        IMPORT_SYMBOL (arg_node) = TRAVdo (IMPORT_SYMBOL (arg_node), arg_info);
    }

    INFO_CHECKIMPORT (arg_info) = FALSE;
    INFO_CURRENT (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
ANSexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* exports are ignored in this traversal */

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* provides are ignored in this traversal */

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_IDS (arg_info) == NULL, "found leftover ids in ans info");

    CheckLocalNameClash (FUNDEF_NAME (arg_node), INFO_SYMBOLS (arg_info),
                         NODE_LOCATION (arg_node));

    if (FUNDEF_NS (arg_node) == NULL) {
        FUNDEF_NS (arg_node) = NSdupNamespace (MODULE_NAMESPACE (INFO_MODULE (arg_info)));
    }

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_RETS (arg_node) != NULL) {
        FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
    }

    if (FUNDEF_AFFECTEDOBJECTS (arg_node) != NULL) {
        INFO_INSIDEOBJLIST (arg_info) = TRUE;
        FUNDEF_AFFECTEDOBJECTS (arg_node)
          = TRAVdo (FUNDEF_AFFECTEDOBJECTS (arg_node), arg_info);
        INFO_INSIDEOBJLIST (arg_info) = FALSE;
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_IDS (arg_info) = STRSfree (INFO_IDS (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANStypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CheckLocalNameClash (TYPEDEF_NAME (arg_node), INFO_SYMBOLS (arg_info),
                         NODE_LOCATION (arg_node));

    if (TYPEDEF_NS (arg_node) == NULL) {
        DBUG_PRINT ("Updated namespace for typedef %s to %s", CTIitemName (arg_node),
                    NSgetName (MODULE_NAMESPACE (INFO_MODULE (arg_info))));

        TYPEDEF_NS (arg_node)
          = NSdupNamespace (MODULE_NAMESPACE (INFO_MODULE (arg_info)));
    }

    if (TYPEDEF_NTYPE (arg_node) != NULL) {
        TYPEDEF_NTYPE (arg_node) = ANSntype (TYPEDEF_NTYPE (arg_node), arg_info);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CheckLocalNameClash (OBJDEF_NAME (arg_node), INFO_SYMBOLS (arg_info),
                         NODE_LOCATION (arg_node));

    if (OBJDEF_NS (arg_node) == NULL) {
        OBJDEF_NS (arg_node) = NSdupNamespace (MODULE_NAMESPACE (INFO_MODULE (arg_info)));
    }

    if (OBJDEF_TYPE (arg_node) != NULL) {
        OBJDEF_TYPE (arg_node) = ANSntype (OBJDEF_TYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (SPAP_NS (arg_node) == NULL) {
        /*
         * look up the correct namespace
         */

        SPAP_NS (arg_node) = LookupNamespaceForSymbol (SPAP_NAME (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * whe have to traverse the body first, as it may contain
     * additional spids which may be referenced in the conditional
     * expression.
     */

    WHILE_BODY (arg_node) = TRAVdo (WHILE_BODY (arg_node), arg_info);
    WHILE_COND (arg_node) = TRAVdo (WHILE_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * whe have to traverse the body first, as it may contain
     * additional spids which may be referenced in the conditional
     * expression.
     */

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_TYPE (arg_node) != NULL) {
        AVIS_TYPE (arg_node) = ANSntype (AVIS_TYPE (arg_node), arg_info);
    }

    if (AVIS_DECLTYPE (arg_node) != NULL) {
        AVIS_DECLTYPE (arg_node) = ANSntype (AVIS_DECLTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ARRAY_ELEMTYPE (arg_node) != NULL) {
        ARRAY_ELEMTYPE (arg_node) = ANSntype (ARRAY_ELEMTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANScast (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (CAST_NTYPE (arg_node) != NULL) {
        CAST_NTYPE (arg_node) = ANSntype (CAST_NTYPE (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_IDS (arg_info)
      = STRSadd (ARG_NAME (arg_node), STRS_unknown, INFO_IDS (arg_info));

    if (ARG_AVIS (arg_node) != NULL) {
        ARG_AVIS (arg_node) = TRAVdo (ARG_AVIS (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (RET_TYPE (arg_node) != NULL) {
        RET_TYPE (arg_node) = ANSntype (RET_TYPE (arg_node), arg_info);
    }

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSspids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * mark this id as locally defined
     */
    INFO_IDS (arg_info)
      = STRSadd (SPIDS_NAME (arg_node), STRS_unknown, INFO_IDS (arg_info));

    DBUG_PRINT ("found local id '%s'", SPIDS_NAME (arg_node));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSspmop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_INSIDEMOP (arg_info) == FALSE, "found illegal mop stacking!");
    /*
     * process ops in special mop-mode
     */
    if (SPMOP_OPS (arg_node) != NULL) {
        INFO_INSIDEMOP (arg_info) = TRUE;

        SPMOP_OPS (arg_node) = TRAVdo (SPMOP_OPS (arg_node), arg_info);

        INFO_INSIDEMOP (arg_info) = FALSE;
    }

    if (SPMOP_EXPRS (arg_node) != NULL) {
        SPMOP_EXPRS (arg_node) = TRAVdo (SPMOP_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSspid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INSIDEMOP (arg_info)) {
        /*
         * in special mop-mode, all SPID represent function calls,
         * so no need for a locality check
         */
        if (SPID_NS (arg_node) == NULL) {
            SPID_NS (arg_node)
              = LookupNamespaceForSymbol (SPID_NAME (arg_node), arg_info);
        }
    } else if (INFO_INSIDEOBJLIST (arg_info)) {
        /*
         * in special objlist mode, all SPID represent objdefs,
         * so we may need to add a namespace.
         */
        if (SPID_NS (arg_node) == NULL) {
            SPID_NS (arg_node)
              = LookupNamespaceForSymbol (SPID_NAME (arg_node), arg_info);
        }
    } else {
        if (SPID_NS (arg_node) == NULL) {
            /*
             * check whether this id is local
             */
            if (!STRScontains (SPID_NAME (arg_node), INFO_IDS (arg_info))) {
                /*
                 * look up the correct namespace
                 */

                SPID_NS (arg_node)
                  = LookupNamespaceForSymbol (SPID_NAME (arg_node), arg_info);

                DBUG_PRINT ("found ns '%s' for id '%s'", NSgetName (SPID_NS (arg_node)),
                            SPID_NAME (arg_node));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
ANSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * first traverse the RHS; this ensures that global
     * objects on the RHS are being found even if the
     * LHS re-difines them!
     */
    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    /*
     * then traverse all defining ids
     */
    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * make sure, the withid is traversed prior to the code!
     */
    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSspfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((SPFOLD_NS (arg_node) == NULL) && (SPFOLD_FUN (arg_node) != NULL)) {
        /*
         * look up the correct namespace
         */

        SPFOLD_NS (arg_node) = LookupNamespaceForSymbol (SPFOLD_FUN (arg_node), arg_info);
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MODULE (arg_info) = arg_node;

    /*
     * traverse use/import statements
     */

    if (MODULE_INTERFACE (arg_node) != NULL) {
        MODULE_INTERFACE (arg_node) = TRAVdo (MODULE_INTERFACE (arg_node), arg_info);
    }

    /*
     * check uniqueness property of symbols refernced by use statements
     */

    CheckUseUnique (INFO_SYMBOLS (arg_info));

    /*
     * traverse fundecs
     */

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    /*
     * traverse fundefs
     */

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * traverse funspecs
     */

    if (MODULE_FUNSPECS (arg_node) != NULL) {
        MODULE_FUNSPECS (arg_node) = TRAVdo (MODULE_FUNSPECS (arg_node), arg_info);
    }
    /*
     * traverse typedefs
     */

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    /*
     * traverse objdefs
     */

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSdoAnnotateNamespace (node *module)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_ans);

    module = TRAVdo (module, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (module);
}

#undef DBUG_PREFIX
