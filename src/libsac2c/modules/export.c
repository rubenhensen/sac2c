#define DBUG_PREFIX "EXP"
#include "debug.h"

#include "export.h"
#include "serialize.h"
#include "traverse.h"
#include "types.h"
#include "free.h"
#include "globals.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "ctinfo.h"
#include "namespaces.h"

/*
 * INFO structure
 */
enum symbmode_t { SYM_filter, SYM_check };

struct INFO {
    char *symbol;
    bool exported;
    bool provided;
    bool result;
    node *interface;
    int filetype;
    namespace_t *mnamespace;
    enum symbmode_t symbmode;
};

/*
 * INFO macros
 */
#define INFO_SYMBOL(n) ((n)->symbol)
#define INFO_EXPORTED(n) ((n)->exported)
#define INFO_PROVIDED(n) ((n)->provided)
#define INFO_RESULT(n) ((n)->result)
#define INFO_INTERFACE(n) ((n)->interface)
#define INFO_FILETYPE(n) ((n)->filetype)
#define INFO_NAMESPACE(n) ((n)->mnamespace)
#define INFO_SYMBMODE(n) ((n)->symbmode)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SYMBOL (result) = NULL;
    INFO_EXPORTED (result) = FALSE;
    INFO_PROVIDED (result) = FALSE;
    INFO_RESULT (result) = FALSE;
    INFO_INTERFACE (result) = NULL;
    INFO_FILETYPE (result) = 0;
    INFO_NAMESPACE (result) = NULL;
    INFO_SYMBMODE (result) = SYM_filter;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_SYMBOL (info) = NULL;
    INFO_INTERFACE (info) = NULL;

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Helper Functions
 */

static bool
CheckExport (bool all, node *symbol, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RESULT (arg_info) = FALSE;

    if (symbol != NULL) {
        symbol = TRAVdo (symbol, arg_info);
    }

    if (all) {
        /* in case of an all flag, the symbollist was
         * an except, thus the result has to be inversed
         */
        INFO_RESULT (arg_info) = !INFO_RESULT (arg_info);
    }

    DBUG_RETURN (INFO_RESULT (arg_info));
}

static node *
CheckSymbolsUsed (node *symbols, info *info)
{
    DBUG_ENTER ();

    if (symbols != NULL) {
        symbols = TRAVdo (symbols, info);
    }

    DBUG_RETURN (symbols);
}

/*
 * Traversal Functions
 */

node *
EXPuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    if (INFO_SYMBMODE (arg_info) == SYM_filter) {
        if (INFO_FILETYPE (arg_info) != FT_prog) {
            if (CheckExport (PROVIDE_ALL (arg_node), PROVIDE_SYMBOL (arg_node),
                             arg_info)) {
                INFO_PROVIDED (arg_info) = TRUE;
            }
        } else {
            CTIwarnLoc (NODE_LOCATION (arg_node),
                        "The provide directive is only allowed in modules and "
                        "classes. Ignoring...");

            arg_node = FREEdoFreeNode (arg_node);
        }
    } else if (INFO_SYMBMODE (arg_info) == SYM_check) {
        CheckSymbolsUsed (PROVIDE_SYMBOL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    if (INFO_SYMBMODE (arg_info) == SYM_filter) {
        if (INFO_FILETYPE (arg_info) != FT_prog) {
            if (CheckExport (EXPORT_ALL (arg_node), EXPORT_SYMBOL (arg_node), arg_info)) {
                INFO_EXPORTED (arg_info) = TRUE;
            }
        } else {
            CTIwarnLoc (NODE_LOCATION (arg_node),
                        "The export directive is only allowed in modules and classes. "
                        "Ignoring...");

            arg_node = FREEdoFreeNode (arg_node);
        }
    } else if (INFO_SYMBMODE (arg_info) == SYM_check) {
        CheckSymbolsUsed (EXPORT_SYMBOL (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_SYMBMODE (arg_info) == SYM_filter) {
        if (STReq (INFO_SYMBOL (arg_info), SYMBOL_ID (arg_node))) {
            INFO_RESULT (arg_info) = TRUE;

            /*
             * mark this symbol as used
             */
            SYMBOL_USED (arg_node) = TRUE;
        }
    } else if (INFO_SYMBMODE (arg_info) == SYM_check) {
        if (!SYMBOL_USED (arg_node)) {
            CTIerror (NODE_LOCATION (arg_node),
                      "Symbol '%s' used in export or provide is not defined.",
                      SYMBOL_ID (arg_node));
        }
    }

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Processing %s %s...",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "function"),
                CTIitemName (arg_node));

    if (FUNDEF_ISLOCAL (arg_node)) {
        DBUG_PRINT ("...local fundef");

        if (FUNDEF_ISLACFUN (arg_node)) {
            DBUG_PRINT ("...is not visible (LaC function)");

            /*
             * LaC functions are not provided, as nobody could
             * call them anyways
             */

            FUNDEF_ISEXPORTED (arg_node) = FALSE;
            FUNDEF_ISPROVIDED (arg_node) = FALSE;
        } else if (!NSequals (FUNDEF_NS (arg_node), INFO_NAMESPACE (arg_info))) {
            DBUG_PRINT ("...is not visible (in view)");

            /*
             * views are generally not visible as they
             * contain compiler generated functions of
             * another namespace
             */

            FUNDEF_ISEXPORTED (arg_node) = FALSE;
            FUNDEF_ISPROVIDED (arg_node) = FALSE;
        } else if ((INFO_FILETYPE (arg_info) == FT_prog) && (FUNDEF_ISLOCAL (arg_node))
                   && (STReq (FUNDEF_NAME (arg_node), "main"))) {

            /* override exports/provide for function main in a
             * program! Main is always provided.
             */

            DBUG_PRINT ("...override for main");

            FUNDEF_ISEXPORTED (arg_node) = FALSE;
            FUNDEF_ISPROVIDED (arg_node) = TRUE;
        } else {
            INFO_SYMBOL (arg_info) = FUNDEF_NAME (arg_node);
            INFO_EXPORTED (arg_info) = FALSE;
            INFO_PROVIDED (arg_info) = FALSE;

            if (INFO_INTERFACE (arg_info) != NULL) {
                INFO_INTERFACE (arg_info) = TRAVdo (INFO_INTERFACE (arg_info), arg_info);
            }

            if (INFO_EXPORTED (arg_info)) {
                DBUG_PRINT ("...is exported");

                FUNDEF_ISEXPORTED (arg_node) = TRUE;
                FUNDEF_ISPROVIDED (arg_node) = TRUE;
            } else if (INFO_PROVIDED (arg_info)) {
                DBUG_PRINT ("...is provided");

                FUNDEF_ISEXPORTED (arg_node) = FALSE;
                FUNDEF_ISPROVIDED (arg_node) = TRUE;
            } else {
                DBUG_PRINT ("...is not visible");

                FUNDEF_ISEXPORTED (arg_node) = FALSE;
                FUNDEF_ISPROVIDED (arg_node) = FALSE;
            }
        }
    } else {
        DBUG_PRINT ("...is not visible (non local)");

        FUNDEF_ISEXPORTED (arg_node) = FALSE;
        FUNDEF_ISPROVIDED (arg_node) = FALSE;
    }

    DBUG_PRINT ("%s %s has final status %d/%d [PROVIDE/EXPORT].",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "Wrapper" : "Function"),
                CTIitemName (arg_node), FUNDEF_ISPROVIDED (arg_node),
                FUNDEF_ISEXPORTED (arg_node));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SYMBOL (arg_info) = TYPEDEF_NAME (arg_node);
    INFO_EXPORTED (arg_info) = FALSE;
    INFO_PROVIDED (arg_info) = FALSE;

    if (INFO_INTERFACE (arg_info) != NULL) {
        INFO_INTERFACE (arg_info) = TRAVdo (INFO_INTERFACE (arg_info), arg_info);
    }

    if (INFO_EXPORTED (arg_info)) {
        TYPEDEF_ISEXPORTED (arg_node) = TRUE;
        TYPEDEF_ISPROVIDED (arg_node) = TRUE;
    } else if (INFO_PROVIDED (arg_info)) {
        TYPEDEF_ISEXPORTED (arg_node) = FALSE;
        TYPEDEF_ISPROVIDED (arg_node) = TRUE;
    } else {
        TYPEDEF_ISEXPORTED (arg_node) = FALSE;
        TYPEDEF_ISPROVIDED (arg_node) = FALSE;
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPobjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SYMBOL (arg_info) = OBJDEF_NAME (arg_node);
    INFO_EXPORTED (arg_info) = FALSE;
    INFO_PROVIDED (arg_info) = FALSE;

    if (INFO_INTERFACE (arg_info) != NULL) {
        INFO_INTERFACE (arg_info) = TRAVdo (INFO_INTERFACE (arg_info), arg_info);
    }

    if (INFO_EXPORTED (arg_info)) {
        OBJDEF_ISEXPORTED (arg_node) = TRUE;
        OBJDEF_ISPROVIDED (arg_node) = TRUE;
    } else if (INFO_PROVIDED (arg_info)) {
        OBJDEF_ISEXPORTED (arg_node) = FALSE;
        OBJDEF_ISPROVIDED (arg_node) = TRUE;
    } else {
        OBJDEF_ISEXPORTED (arg_node) = FALSE;
        OBJDEF_ISPROVIDED (arg_node) = FALSE;
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_INTERFACE (arg_info) = MODULE_INTERFACE (arg_node);
    INFO_FILETYPE (arg_info) = MODULE_FILETYPE (arg_node);
    INFO_NAMESPACE (arg_info) = MODULE_NAMESPACE (arg_node);
    INFO_SYMBMODE (arg_info) = SYM_filter;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    if (MODULE_FUNDECS (arg_node) != NULL) {
        MODULE_FUNDECS (arg_node) = TRAVdo (MODULE_FUNDECS (arg_node), arg_info);
    }

    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    if (MODULE_OBJS (arg_node) != NULL) {
        MODULE_OBJS (arg_node) = TRAVdo (MODULE_OBJS (arg_node), arg_info);
    }

    MODULE_INTERFACE (arg_node) = INFO_INTERFACE (arg_info);
    INFO_NAMESPACE (arg_info) = NULL;

    /*
     * now check whether all symbols specified in export/provide have
     * in fact been used.
     */
    INFO_SYMBMODE (arg_info) = SYM_check;

    if (MODULE_INTERFACE (arg_node) != NULL) {
        MODULE_INTERFACE (arg_node) = TRAVdo (MODULE_INTERFACE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Start of Traversal
 */

static node *
StartExpTraversal (node *modul)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_exp);

    modul = TRAVdo (modul, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (modul);
}

node *
EXPdoExport (node *syntax_tree)
{
    DBUG_ENTER ();

    if (MODULE_FILETYPE (syntax_tree) != FT_prog) {
        if (!global.optimize.dodfr) {
            CTIwarn ("Dead Function Removal is disabled. This will lead to "
                     "bigger modules.");
        }
    }

    syntax_tree = StartExpTraversal (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
