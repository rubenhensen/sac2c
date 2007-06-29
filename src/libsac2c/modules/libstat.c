/*
 *
 * $Id$
 *
 */

#include "libstat.h"
#include "dbug.h"
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
    DBUG_ENTER ("PrintLibStatHeader");

    printf ("\nLibrary Information for Module `%s':\n\n", MODMgetModuleName (module));

    DBUG_VOID_RETURN;
}

static void
PrintLibStatDependencies (module_t *module)
{
    DBUG_ENTER ("PrintLibStatDependencies");

    printf ("\n\nModule Dependencies:\n\n");

    STRSprint (MODMgetDependencyTable (module));

    DBUG_VOID_RETURN;
}

static void
PrintLibStatTable (const sttable_t *table)
{
    DBUG_ENTER ("PrintLibStatTable");
#ifndef DBUG_OFF
    printf ("\n\nModule Symbols:\n\n");

    STprint (table);
#endif
    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeAddBodies (module_t *module, node *modnode, node *fundef)
{
    DBUG_ENTER ("PrintLibStatCodeAddBodies");
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
    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadSymbols (module_t *module, stsymbol_t *symbol, const sttable_t *table)
{
    DBUG_ENTER ("PrintLibStatCodeReadSymbols");

    DSaddSymbolByName (STsymbolName (symbol), SET_wrapperhead,
                       MODMgetModuleName (module));
    DSaddSymbolByName (STsymbolName (symbol), SET_typedef, MODMgetModuleName (module));
    DSaddSymbolByName (STsymbolName (symbol), SET_objdef, MODMgetModuleName (module));

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCode (module_t *module, const sttable_t *table)
{
    stsymboliterator_t *iterator;
    node *syntax_tree;

    DBUG_ENTER ("PrintLibStatPrintCode");

    syntax_tree = TBmakeModule (NSgetNamespace (MODMgetModuleName (module)), F_prog, NULL,
                                NULL, NULL, NULL, NULL);

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

    DBUG_VOID_RETURN;
}

void
LIBSprintLibStat ()
{
    module_t *module;
    const sttable_t *table;

    DBUG_ENTER ("LIBSprintLibStat");

    if (global.libstat) {
        DBUG_PRINT ("LIBSTAT", ("Loading module `%s'", global.sacfilename));

        module = MODMloadModule (global.sacfilename);

        DBUG_PRINT ("LIBSTAT", ("Getting symbol table"));

        table = MODMgetSymbolTable (module);

        DBUG_PRINT ("LIBSTAT", ("Printing LibStat header\n"));

        PrintLibStatHeader (module);

        DBUG_PRINT ("LIBSTAT", ("Printing table information"));

        PrintLibStatTable (table);

        DBUG_PRINT ("LIBSTAT", ("Printing dependencies"));

        PrintLibStatDependencies (module);

        DBUG_PRINT ("LIBSTAT", ("Printing code"));

        PrintLibStatCode (module, table);

        DBUG_PRINT ("LIBSTAT", ("Unloading module `%s'", global.sacfilename));

        module = MODMunLoadModule (module);

        /*
         * exit compiler at this point, as we have printed
         * the libstat information
         */

        exit (0);
    }

    DBUG_VOID_RETURN;
}
