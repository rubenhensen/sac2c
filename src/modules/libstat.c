/*
 *
 * $Log$
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

    if (fundef != NULL) {
        if (FUNDEF_BODY (fundef) == NULL) {
            AddFunctionBodyToHead (fundef, modnode);
        }

        PrintLibStatCodeAddBodies (module, modnode, FUNDEF_NEXT (fundef));
    }

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadEntry (module_t *module, node *modnode, STentry_t *entry,
                           STtable_t *table)
{
    serfun_p serfun;
    node *tmp;

    DBUG_ENTER ("PrintLibStatCodeReadEntry");

    switch (STEntryType (entry)) {
    case SET_funbody:
        /* these are ignored right now, they will
         * be added as soon as all others have been
         * read in
         */
        break;
    case SET_funhead:
        serfun = GetDeSerializeFunction (STEntryName (entry), module);

        tmp = serfun ();

        MODUL_FUNS (modnode) = AppendFundef (MODUL_FUNS (modnode), tmp);

        break;
    case SET_typedef:
    case SET_objdef:
        break;
    default:
        DBUG_ASSERT (0, "unsupported symbol within a modules symboltable!");
        break;
    }

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadSymbols (module_t *module, node *modnode, const char *symbol,
                             STtable_t *table)
{
    STentryiterator_t *iterator;

    DBUG_ENTER ("PrintLibStatCodeReadSymbols");

    iterator = STEntryIteratorGet (symbol, table);

    while (STEntryIteratorHasMore (iterator)) {
        PrintLibStatCodeReadEntry (module, modnode, STEntryIteratorNext (iterator),
                                   table);
    }

    iterator = STEntryIteratorRelease (iterator);

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

    while (STSymbolIteratorHasMore (iterator)) {
        PrintLibStatCodeReadSymbols (module, syntax_tree, STSymbolIteratorNext (iterator),
                                     table);
    }

    iterator = STSymbolIteratorRelease (iterator);

    /* Add Old Types */

    syntax_tree = NT2OTTransform (syntax_tree);

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

    STPrint (table);

    DBUG_PRINT ("LIBSTAT", ("Printing code"));

    PrintLibStatCode (module, table);

    DBUG_PRINT ("LIBSTAT", ("Destroying table"));

    table = STDestroy (table);

    DBUG_PRINT ("LIBSTAT", ("Unloading module `%s'", libname));

    module = UnLoadModule (module);

    DBUG_VOID_RETURN;
}
