/*
 *
 * $Log$
 * Revision 1.15  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 1.14  2005/02/15 21:07:40  sah
 * module system fixes
 *
 * Revision 1.13  2005/01/11 12:32:52  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.12  2004/12/19 17:54:26  sah
 * bugfix
 *
 * Revision 1.11  2004/11/29 13:34:36  sah
 * dfr is now switchable
 *
 * Revision 1.10  2004/11/26 23:41:58  jhb
 * cchanged type from void to node of EXPdoExport
 *
 * Revision 1.9  2004/11/26 23:29:39  jhb
 * DoExport changed to EXPdoExport
 *
 * Revision 1.8  2004/11/25 21:14:38  sah
 * COMPILES
 *
 *
 * Revision 1.1  2004/10/17 14:51:07  sah
 * Initial revision
 *
 *
 *
 */

#include "dbug.h"

#include "export.h"
#include "DeadFunctionRemoval.h"
#include "serialize.h"
#include "traverse.h"
#include "types.h"
#include "free.h"
#include "globals.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "ctinfo.h"
#include "namespaces.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    char *symbol;
    bool exported;
    bool provided;
    bool result;
    node *interface;
    char *modname;
    int filetype;
};

/*
 * INFO macros
 */
#define INFO_EXP_SYMBOL(n) (n->symbol)
#define INFO_EXP_EXPORTED(n) (n->exported)
#define INFO_EXP_PROVIDED(n) (n->provided)
#define INFO_EXP_RESULT(n) (n->result)
#define INFO_EXP_INTERFACE(n) (n->interface)
#define INFO_EXP_FILETYPE(n) (n->filetype)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_EXP_SYMBOL (result) = NULL;
    INFO_EXP_EXPORTED (result) = FALSE;
    INFO_EXP_PROVIDED (result) = FALSE;
    INFO_EXP_RESULT (result) = FALSE;
    INFO_EXP_INTERFACE (result) = NULL;
    INFO_EXP_FILETYPE (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_EXP_SYMBOL (info) = NULL;
    INFO_EXP_INTERFACE (info) = NULL;

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * Helper Functions
 */

static bool
CheckExport (bool all, node *symbol, info *arg_info)
{
    DBUG_ENTER ("CheckExport");

    INFO_EXP_RESULT (arg_info) = FALSE;

    if (symbol != NULL) {
        symbol = TRAVdo (symbol, arg_info);
    }

    if (all) {
        /* in case of a all flag, the symbollist was
         * an except, thus the result has to be inversed
         */
        INFO_EXP_RESULT (arg_info) = !INFO_EXP_RESULT (arg_info);
    }

    DBUG_RETURN (INFO_EXP_RESULT (arg_info));
}

/*
 * Traversal Functions
 */

node *
EXPuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPuse");

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPimport");

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPprovide");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    if (INFO_EXP_FILETYPE (arg_info) != F_prog) {
        if (CheckExport (PROVIDE_ALL (arg_node), PROVIDE_SYMBOL (arg_node), arg_info)) {
            INFO_EXP_PROVIDED (arg_info) = TRUE;
        }
    } else {
        CTIwarnLine (NODE_LINE (arg_node), "Provide is only allowed in modules");

        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPexport");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    if (INFO_EXP_FILETYPE (arg_info) != F_prog) {
        if (CheckExport (EXPORT_ALL (arg_node), EXPORT_SYMBOL (arg_node), arg_info)) {
            INFO_EXP_EXPORTED (arg_info) = TRUE;
        }
    } else {
        CTIwarnLine (NODE_LINE (arg_node), "Export is only allowed in modules");

        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPsymbol");

    if (!strcmp (INFO_EXP_SYMBOL (arg_info), SYMBOL_ID (arg_node))) {
        INFO_EXP_RESULT (arg_info) = TRUE;
    }

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPfundef");

    DBUG_PRINT ("EXP", ("Processing Fundef %s:%s...", NSgetName (FUNDEF_NS (arg_node)),
                        FUNDEF_NAME (arg_node)));

    if (FUNDEF_ISLOCAL (arg_node)) {
        DBUG_PRINT ("EXP", ("...local fundef"));

        INFO_EXP_SYMBOL (arg_info) = FUNDEF_NAME (arg_node);
        INFO_EXP_EXPORTED (arg_info) = FALSE;
        INFO_EXP_PROVIDED (arg_info) = FALSE;

        if (INFO_EXP_INTERFACE (arg_info) != NULL) {
            INFO_EXP_INTERFACE (arg_info)
              = TRAVdo (INFO_EXP_INTERFACE (arg_info), arg_info);
        }

        if (INFO_EXP_EXPORTED (arg_info)) {
            DBUG_PRINT ("EXP", ("...is exported"));

            FUNDEF_ISEXPORTED (arg_node) = TRUE;
            FUNDEF_ISPROVIDED (arg_node) = TRUE;
        } else if (INFO_EXP_PROVIDED (arg_info)) {
            DBUG_PRINT ("EXP", ("...is provided"));

            FUNDEF_ISEXPORTED (arg_node) = FALSE;
            FUNDEF_ISPROVIDED (arg_node) = TRUE;
        } else {
            DBUG_PRINT ("EXP", ("...is not visible"));

            FUNDEF_ISEXPORTED (arg_node) = FALSE;
            FUNDEF_ISPROVIDED (arg_node) = FALSE;
        }

        /* override exports/provide for function main in a
         * program! Main is always provided.
         */
        if (INFO_EXP_FILETYPE (arg_info) == F_prog) {
            if (ILIBstringCompare (FUNDEF_NAME (arg_node), "main")) {
                DBUG_PRINT ("EXP", ("...override for main"));

                FUNDEF_ISEXPORTED (arg_node) = FALSE;
                FUNDEF_ISPROVIDED (arg_node) = TRUE;
            }
        }
        /* TODO: adapt to new object handling */
#if 0
  } else if ( FUNDEF_STATUS( arg_node) == ST_objinitfun ) {
    /* init functions of objects are always local, as there is
     * no need for them to be used outside the module
     */
    SET_FLAG( FUNDEF, arg_node, IS_EXPORTED, FALSE);
    SET_FLAG( FUNDEF, arg_node, IS_PROVIDED, FALSE);
  } else if ( FUNDEF_STATUS( arg_node) == ST_classfun ) {
    /* classfuns are not exported as well, as that would
     * break the encapsulation
     */
    SET_FLAG( FUNDEF, arg_node, IS_EXPORTED, FALSE);
    SET_FLAG( FUNDEF, arg_node, IS_PROVIDED, FALSE);
#endif
    }

    DBUG_PRINT ("EXP", ("Fundef %s:%s has final status %d/%d [PROVIDE/EXPORT].",
                        NSgetName (FUNDEF_NS (arg_node)), FUNDEF_NAME (arg_node),
                        FUNDEF_ISPROVIDED (arg_node), FUNDEF_ISEXPORTED (arg_node)));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPtypedef");

    INFO_EXP_SYMBOL (arg_info) = TYPEDEF_NAME (arg_node);
    INFO_EXP_EXPORTED (arg_info) = FALSE;
    INFO_EXP_PROVIDED (arg_info) = FALSE;

    if (INFO_EXP_INTERFACE (arg_info) != NULL) {
        INFO_EXP_INTERFACE (arg_info) = TRAVdo (INFO_EXP_INTERFACE (arg_info), arg_info);
    }

    if (INFO_EXP_EXPORTED (arg_info)) {
        TYPEDEF_ISEXPORTED (arg_node) = TRUE;
        TYPEDEF_ISPROVIDED (arg_node) = TRUE;
    } else if (INFO_EXP_PROVIDED (arg_info)) {
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
    DBUG_ENTER ("EXPobjdef");

    INFO_EXP_SYMBOL (arg_info) = OBJDEF_NAME (arg_node);
    INFO_EXP_EXPORTED (arg_info) = FALSE;
    INFO_EXP_PROVIDED (arg_info) = FALSE;

    if (INFO_EXP_INTERFACE (arg_info) != NULL) {
        INFO_EXP_INTERFACE (arg_info) = TRAVdo (INFO_EXP_INTERFACE (arg_info), arg_info);
    }

    if (INFO_EXP_EXPORTED (arg_info)) {
        OBJDEF_ISEXPORTED (arg_node) = TRUE;
        OBJDEF_ISPROVIDED (arg_node) = TRUE;
    } else if (INFO_EXP_PROVIDED (arg_info)) {
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
    DBUG_ENTER ("EXPmodule");

    INFO_EXP_INTERFACE (arg_info) = MODULE_IMPORTS (arg_node);
    INFO_EXP_FILETYPE (arg_info) = MODULE_FILETYPE (arg_node);

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

    MODULE_IMPORTS (arg_node) = INFO_EXP_INTERFACE (arg_info);

    DBUG_RETURN (arg_node);
}

/*
 * Start of Traversal
 */

static node *
StartExpTraversal (node *modul)
{
    info *info;

    DBUG_ENTER ("StartExpTraversal");

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
    DBUG_ENTER ("EXPdoExport");

    syntax_tree = StartExpTraversal (syntax_tree);

    if (MODULE_FILETYPE (syntax_tree) != F_prog) {
        if (global.optimize.dodfr) {
            syntax_tree = DFRdoDeadFunctionRemoval (syntax_tree);
        } else {
            CTIwarn ("Dead Function Removal is disabled. This will lead to "
                     "bigger modules.");
        }

        SERdoSerialize (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}
