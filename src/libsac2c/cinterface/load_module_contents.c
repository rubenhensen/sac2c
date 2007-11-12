/* $Id$ */

#include "load_module_contents.h"

#include "dbug.h"
#include "globals.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "stringset.h"
#include "deserialize.h"
#include "modulemanager.h"
#include "symboltable.h"
#include "stringset.h"
#include "traverse.h"
#include "ctinfo.h"

static node *
CheckForUniqueTypes (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CheckForUniqueTypes");

    if (TYPEDEF_ISUNIQUE (arg_node)) {
        CTIerror ("Unique type `%s' is not supported.", CTIitemName (arg_node));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
LoadModule (const char *name, strstype_t kind, node *syntax_tree)
{
    module_t *module;
    const sttable_t *table;
    stsymboliterator_t *iterator;

    DBUG_ENTER ("LoadModule");

    DBUG_PRINT ("LMC", ("Loading module '%s'.", name));

    module = MODMloadModule (name);
    table = MODMgetSymbolTable (module);
    iterator = STsymbolIteratorGet (table);

    while (STsymbolIteratorHasMore (iterator)) {
        stsymbol_t *symbol = STsymbolIteratorNext (iterator);

        if ((STsymbolVisibility (symbol) == SVT_provided)
            || (STsymbolVisibility (symbol) == SVT_exported)) {
            DBUG_PRINT ("LMC", ("Adding Symbol '%s'.", STsymbolName (symbol)));

            DSaddSymbolByName (STsymbolName (symbol), SET_wrapperhead, name);
        }
    }

    iterator = STsymbolIteratorRelease (iterator);
    module = MODMunLoadModule (module);

    global.dependencies = STRSadd (name, STRS_saclib, global.dependencies);

    DBUG_RETURN (syntax_tree);
}

node *
LMCdoLoadModuleContents (node *syntax_tree)
{
    anontrav_t checktypes[2] = {{N_typedef, &CheckForUniqueTypes}, {0, NULL}};

    DBUG_ENTER ("LMCdoLoadModuleContents");

    DBUG_ASSERT ((syntax_tree == NULL),
                 "SMCdoLoadModuleContents can only be called as long as no syntax tree "
                 "has been created!");

    syntax_tree
      = TBmakeModule (NSgetCWrapperNamespace (), F_cmod, NULL, NULL, NULL, NULL, NULL);

    DSinitDeserialize (syntax_tree);

    syntax_tree
      = STRSfold ((strsfoldfun_p)&LoadModule, global.exported_modules, syntax_tree);

    DSfinishDeserialize (syntax_tree);

    if (MODULE_TYPES (syntax_tree) != NULL) {
        TRAVpushAnonymous (checktypes, &TRAVsons);
        MODULE_TYPES (syntax_tree) = TRAVdo (MODULE_TYPES (syntax_tree), NULL);
        TRAVpop ();
    }

    if (MODULE_OBJS (syntax_tree) != NULL) {
        CTIerror ("Global objects are not supported.");
    }

    CTIabortOnError ();

    DBUG_RETURN (syntax_tree);
}
