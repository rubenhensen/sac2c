#include "libstat.h"

#define DBUG_PREFIX "LIBSTAT"
#include "debug.h"

#include "modulemanager.h"
#include "serialize.h"
#include "deserialize.h"
#include "add_function_body.h"
#include "tree_basic.h"
#include "print.h"
#include "stringset.h"
#include "symboltable.h"
#include "namespaces.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"

static void
PrintLibStatHeader (module_t *module)
{
    DBUG_ENTER ();

    printf ("\nLibrary Information for Module `%s':\n\n", MODMgetModuleName (module));

    DBUG_RETURN ();
}

static void
PrintLibStatDependencies (module_t *module)
{
    DBUG_ENTER ();

    printf ("\n\nModule Dependencies:\n\n");

    STRSprint (MODMgetDependencyTable (module));

    DBUG_RETURN ();
}

static void
PrintLibStatTable (const sttable_t *table)
{
    DBUG_ENTER ();
#ifndef DBUG_OFF
    printf ("\n\nModule Symbols:\n\n");

    STprint (table);
#endif
    DBUG_RETURN ();
}

static void
PrintLibStatCodeAddBodies (module_t *module, node *modnode, node *fundef)
{
    DBUG_ENTER ();
#ifndef DBUG_OFF
    if (fundef != NULL) {

        if ((FUNDEF_BODY (fundef) == NULL) && (!FUNDEF_ISWRAPPERFUN (fundef))) {
            DSinitDeserialize (modnode);

            fundef = AFBdoAddFunctionBody (fundef);

            DSfinishDeserialize (modnode);
        }

        PrintLibStatCodeAddBodies (module, modnode, FUNDEF_NEXT (fundef));
    }
#endif
    DBUG_RETURN ();
}

static void
PrintLibStatCodeReadSymbols (module_t *module, stsymbol_t *symbol, const sttable_t *table)
{
    DBUG_ENTER ();

    DSaddSymbolByName (STsymbolName (symbol), SET_wrapperhead,
                       MODMgetModuleName (module));
    DSaddSymbolByName (STsymbolName (symbol), SET_typedef, MODMgetModuleName (module));
    DSaddSymbolByName (STsymbolName (symbol), SET_objdef, MODMgetModuleName (module));

    DBUG_RETURN ();
}

static void
PrintLibStatCode (module_t *module, const sttable_t *table)
{
    stsymboliterator_t *iterator;
    node *syntax_tree;

    DBUG_ENTER ();

    syntax_tree = TBmakeModule (NSgetNamespace (MODMgetModuleName (module)), FT_prog,
                                NULL, NULL, NULL, NULL, NULL);

    iterator = STsymbolIteratorGet (table);

    DSinitDeserialize (syntax_tree);

    while (STsymbolIteratorHasMore (iterator)) {
        PrintLibStatCodeReadSymbols (module, STsymbolIteratorNext (iterator), table);
    }

    DSfinishDeserialize (syntax_tree);

    iterator = STsymbolIteratorRelease (iterator);

    PrintLibStatCodeAddBodies (module, syntax_tree, MODULE_FUNS (syntax_tree));

    PRTdoPrint (syntax_tree);

    syntax_tree = FREEdoFreeTree (syntax_tree);

    DBUG_RETURN ();
}

void
LIBSprintLibStat (void)
{
    module_t *module;
    const sttable_t *table;

    DBUG_ENTER ();

    DBUG_PRINT ("Loading module `%s'", global.sacfilename);

    module = MODMloadModule (global.sacfilename);

    DBUG_PRINT ("Getting symbol table");

    table = MODMgetSymbolTable (module);

    DBUG_PRINT ("Printing LibStat header\n");

    PrintLibStatHeader (module);

    DBUG_PRINT ("Printing table information");

    PrintLibStatTable (table);

    DBUG_PRINT ("Printing dependencies");

    PrintLibStatDependencies (module);

    DBUG_PRINT ("Printing code");

    PrintLibStatCode (module, table);

    DBUG_PRINT ("Unloading module `%s'", global.sacfilename);

    module = MODMunLoadModule (module);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
