/*
 *
 * $Log$
 * Revision 1.20  2005/08/09 10:06:52  sah
 * using CTIterminateCompilation instead of exit
 * to make sure the tmpdir is removed and all
 * the cleanup is done correctly
 *
 * Revision 1.19  2005/07/27 13:40:40  sah
 * added missing include
 *
 * Revision 1.18  2005/07/22 13:11:39  sah
 * interface changes
 *
 * Revision 1.17  2005/07/17 21:14:59  sah
 * wrapper bodies are no more deserialized
 *
 * Revision 1.16  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
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
#include "add_function_body.h"
#include "tree_basic.h"
#include "print.h"
#include "new2old.h"
#include "stringset.h"
#include "internal_lib.h"
#include "namespaces.h"
#include "ctinfo.h"
#include "free.h"

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

    CTIterminateCompilation (PH_final, NULL, NULL);

    DBUG_VOID_RETURN;
}
