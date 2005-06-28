/*
 *
 * $Log$
 * Revision 1.15  2005/06/28 16:29:46  sah
 * removed some warning messages
 *
 * Revision 1.14  2005/05/18 13:56:51  sah
 * enabled caching of symboltables which
 * leads to a huge speedup when analysing use and import
 * from big modules
 *
 * Revision 1.13  2005/04/24 15:20:03  sah
 * libstat now exits the entire compiler
 *
 * Revision 1.12  2004/11/26 23:19:54  jhb
 * PrintLibStat changed to LIBSprintStat
 *
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
LIBSprintLibStat (char *libname)
{
    module_t *module;
    const sttable_t *table;

    DBUG_ENTER ("LIBSprintLibStat");

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

    DBUG_PRINT ("LIBSTAT", ("Unloading module `%s'", libname));

    module = MODMunLoadModule (module);

    /*
     * exit compiler at this point, as we have printed
     * the libstat information
     */

    exit (0);

    DBUG_VOID_RETURN;
}
