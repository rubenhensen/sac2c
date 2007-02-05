/* $Id$ */

/**
 * @file importsymbols.c
 * @brief traversal handling import of functions and types
 * @author Stephan Herhut
 * @date 2005-04-29
 */

#include "importsymbols.h"
#include "traverse.h"
#include "tree_basic.h"
#include "str.h"
#include "memory.h"
#include "deserialize.h"
#include "stringset.h"
#include "free.h"
#include "dbug.h"

struct INFO {
    node *module;
    const char *current;
};

#define INFO_IMP_CURRENT(n) ((n)->current)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IMP_CURRENT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
IMPimport (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("IMPimport");

    INFO_IMP_CURRENT (arg_info) = IMPORT_MOD (arg_node);

    if (IMPORT_SYMBOL (arg_node) != NULL) {
        IMPORT_SYMBOL (arg_node) = TRAVdo (IMPORT_SYMBOL (arg_node), arg_info);
    }

    INFO_IMP_CURRENT (arg_info) = NULL;

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    tmp = IMPORT_NEXT (arg_node);
    arg_node = FREEdoFreeNode (arg_node);

    DBUG_RETURN (tmp);
}

node *
IMPuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IMPuse");

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IMPexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IMPexport");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IMPprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IMPprovide");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IMPsymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IMPsymbol");

    DSimportInstancesByName (SYMBOL_ID (arg_node), INFO_IMP_CURRENT (arg_info));
    DSimportTypedefByName (SYMBOL_ID (arg_node), INFO_IMP_CURRENT (arg_info));
    DSimportObjdefByName (SYMBOL_ID (arg_node), INFO_IMP_CURRENT (arg_info));

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = TRAVdo (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IMPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IMPmodule");

    if (MODULE_INTERFACE (arg_node) != NULL) {
        DSinitDeserialize (arg_node);

        MODULE_INTERFACE (arg_node) = TRAVdo (MODULE_INTERFACE (arg_node), arg_info);

        DSfinishDeserialize (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
IMPdoImportSymbols (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IMPdoImportSymbols");

    info = MakeInfo ();

    TRAVpush (TR_imp);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
