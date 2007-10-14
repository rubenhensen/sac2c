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

    DBUG_RETURN (syntax_tree);
}

node *
LMCdoLoadModuleContents (node *syntax_tree)
{
    DBUG_ENTER ("LMCdoLoadModuleContents");

    DBUG_ASSERT ((syntax_tree == NULL),
                 "SMCdoLoadModuleContents can only be called as long as no syntax tree "
                 "has been created!");

    syntax_tree
      = TBmakeModule (NSgetCWrapperNamespace (), F_modimp, NULL, NULL, NULL, NULL, NULL);

    DSinitDeserialize (syntax_tree);

    syntax_tree
      = STRSfold ((strsfoldfun_p)&LoadModule, global.exported_modules, syntax_tree);

    DSfinishDeserialize (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
