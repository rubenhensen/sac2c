/*
 *
 * $Log$
 * Revision 1.9  2004/11/17 19:48:01  sah
 * interface changes
 *
 * Revision 1.8  2004/11/14 15:22:42  sah
 * switched to AddSymbolByName
 *
 * Revision 1.7  2004/11/01 21:49:10  sah
 * dependencises are printed now as well
 *
 * Revision 1.6  2004/10/28 17:18:33  sah
 * now deserialisation has an internal state
 *
 * Revision 1.5  2004/10/26 09:33:09  sah
 * uses deserialize.h now
 *
 * Revision 1.4  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.3  2004/10/22 13:24:09  sah
 * added a default case
 *
 * Revision 1.2  2004/10/15 10:03:58  sah
 * removed old module system nodes in
 * new ast mode
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
#include "free.h"

static void
PrintLibStatHeader (module_t *module)
{
    DBUG_ENTER ("PrintLibStatHeader");

    printf ("\nLibrary Information for Module `%s':\n\n", GetModuleName (module));

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeAddBodies (module_t *module, node *modnode, node *fundef)
{
    DBUG_ENTER ("PrintLibStatCodeAddBodies");
#ifndef DBUG_OFF
    if (fundef != NULL) {

        if (FUNDEF_BODY (fundef) == NULL) {
            InitDeserialize (modnode);

            AddFunctionBodyToHead (fundef);

            FinishDeserialize (modnode);
        }

        PrintLibStatCodeAddBodies (module, modnode, FUNDEF_NEXT (fundef));
    }
#endif
    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadSymbols (module_t *module, STsymbol_t *symbol, STtable_t *table)
{
    DBUG_ENTER ("PrintLibStatCodeReadSymbols");

    AddSymbolByName (STSymbolName (symbol), SET_wrapperhead, GetModuleName (module));
    AddSymbolByName (STSymbolName (symbol), SET_typedef, GetModuleName (module));

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCode (module_t *module, STtable_t *table)
{
    STsymboliterator_t *iterator;
    node *syntax_tree;

    DBUG_ENTER ("PrintLibStatPrintCode");

    syntax_tree = MakeModul (StringCopy (GetModuleName (module)), F_prog, NULL, NULL,
                             NULL, NULL, NULL);

    iterator = STSymbolIteratorGet (table);

    InitDeserialize (syntax_tree);

    while (STSymbolIteratorHasMore (iterator)) {
        PrintLibStatCodeReadSymbols (module, STSymbolIteratorNext (iterator), table);
    }

    FinishDeserialize (syntax_tree);

    iterator = STSymbolIteratorRelease (iterator);

    PrintLibStatCodeAddBodies (module, syntax_tree, MODUL_FUNS (syntax_tree));

    Print (syntax_tree);

    /*

    syntax_tree = FreeTree( syntax_tree);

    */

    DBUG_VOID_RETURN;
}

void
PrintLibStat (char *libname)
{
    module_t *module;
    STtable_t *table;

    DBUG_ENTER ("PrintLibStat");

    DBUG_PRINT ("LIBSTAT", ("Loading module `%s'", libname));

    module = LoadModule (libname);

    DBUG_PRINT ("LIBSTAT", ("Getting symbol table"));

    table = GetSymbolTable (module);

    DBUG_PRINT ("LIBSTAT", ("Printing LibStat header\n"));

    PrintLibStatHeader (module);

    DBUG_PRINT ("LIBSTAT", ("Printing table information"));
#ifndef DBUG_OFF
    STPrint (table);
#endif
    DBUG_PRINT ("LIBSTAT", ("Printing dependencies"));

    SSPrint (GetDependencyTable (module));

    DBUG_PRINT ("LIBSTAT", ("Printing code"));

    PrintLibStatCode (module, table);

    DBUG_PRINT ("LIBSTAT", ("Destroying table"));

    table = STDestroy (table);

    DBUG_PRINT ("LIBSTAT", ("Unloading module `%s'", libname));

    module = UnLoadModule (module);

    DBUG_VOID_RETURN;
}
