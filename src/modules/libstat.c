/*
 *
 * $Log$
 * Revision 1.11  2004/11/25 20:22:11  sah
 * COMPILES
 *
 *
 * Revision 1.1  2004/10/04 11:35:10  sah
 * Initial revision
 *
 *
 *
 */

#include "libstat.h"
#include "dbug.h"
#include "modulemanager.h"
#include "serialize.h"
#include "deserialize.h"
#include "tree_basic.h"
#include "print.h"
#include "new2old.h"
#include "stringset.h"
#include "internal_lib.h"
#include "free.h"

static void
PrintLibStatHeader (module_t *module)
{
    DBUG_ENTER ("PrintLibStatHeader");

#ifndef PRODUCTION
    printf ("\nThe SaC DevCamp DK Team presents the ");
#endif
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
PrintLibStatTable (sttable_t *table)
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

        if (FUNDEF_BODY (fundef) == NULL) {
            DSinitDeserialize (modnode);

            DSdoDeserialize (fundef);

            DSfinishDeserialize (modnode);
        }

        PrintLibStatCodeAddBodies (module, modnode, FUNDEF_NEXT (fundef));
    }
#endif
    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadSymbols (module_t *module, stsymbol_t *symbol, sttable_t *table)
{
    DBUG_ENTER ("PrintLibStatCodeReadSymbols");

    DSaddSymbolByName (STsymbolName (symbol), SET_wrapperhead,
                       MODMgetModuleName (module));
    DSaddSymbolByName (STsymbolName (symbol), SET_typedef, MODMgetModuleName (module));
    DSaddSymbolByName (STsymbolName (symbol), SET_objdef, MODMgetModuleName (module));

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCode (module_t *module, sttable_t *table)
{
    stsymboliterator_t *iterator;
    node *syntax_tree;

    DBUG_ENTER ("PrintLibStatPrintCode");

    syntax_tree = TBmakeModule (ILIBstringCopy (MODMgetModuleName (module)), F_prog, NULL,
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
PrintLibStat (char *libname)
{
    module_t *module;
    sttable_t *table;

    DBUG_ENTER ("PrintLibStat");

    DBUG_PRINT ("LIBSTAT", ("Loading module `%s'", libname));

    module = MODMloadModule (libname);

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

    DBUG_PRINT ("LIBSTAT", ("Destroying table"));

    table = STdestroy (table);

    DBUG_PRINT ("LIBSTAT", ("Unloading module `%s'", libname));

    module = MODMunLoadModule (module);

    DBUG_VOID_RETURN;
}
