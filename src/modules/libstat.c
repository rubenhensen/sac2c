/*
 *
 * $Log$
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
PrintLibStatCodeAddBodies (module_t *module, node *fundef)
{
    DBUG_ENTER ("PrintLibStatCodeAddBodies");

    if (fundef != NULL) {
        if (FUNDEF_BODY (fundef) == NULL) {
            serfun_p serfun;

            serfun
              = GetDeSerializeFunction (GenerateSerFunName (STE_funbody, fundef), module);

            CombineFunctionHeadAndBody (fundef, serfun ());
        }

        PrintLibStatCodeAddBodies (module, FUNDEF_NEXT (fundef));
    }

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadEntry (module_t *module, node *modnode, symbolentry_t *entry,
                           symboltable_t *table)
{
    serfun_p serfun;
    node *tmp;

    DBUG_ENTER ("PrintLibStatCodeReadEntry");

    switch (SymbolTableEntryType (entry)) {
    case STE_funbody:
        /* these are ignored right now, they will
         * be added as soon as all others have been
         * read in
         */
        break;
    case STE_funhead:
        serfun = GetDeSerializeFunction (SymbolTableEntryName (entry), module);

        tmp = serfun ();

        MODUL_FUNS (modnode) = AppendFundef (MODUL_FUNS (modnode), tmp);

        break;
    case STE_typedef:
    case STE_objdef:
        break;
    default:
        DBUG_ASSERT (0, "unsupported symbol within a modules symboltable!");
        break;
    }

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCodeReadSymbols (module_t *module, node *modnode, const char *symbol,
                             symboltable_t *table)
{
    symbolentrychain_t *chain;

    DBUG_ENTER ("PrintLibStatSymbolsCodbStatCodeReadSymbolsi");

    chain = SymbolTableEntryChainGet (symbol, table);

    while (SymbolTableEntryChainHasMore (chain)) {
        PrintLibStatCodeReadEntry (module, modnode, SymbolTableEntryChainNext (chain),
                                   table);
    }

    chain = SymbolTableEntryChainRelease (chain);

    DBUG_VOID_RETURN;
}

static void
PrintLibStatCode (module_t *module, symboltable_t *table)
{
    symbolchain_t *chain;
    node *syntax_tree;

    DBUG_ENTER ("PrintLibStatPrintCode");

    syntax_tree = MakeModul (StringCopy (GetModuleName (module)), F_prog, NULL, NULL,
                             NULL, NULL, NULL);

    chain = SymbolTableSymbolChainGet (table);

    while (SymbolTableSymbolChainHasMore (chain)) {
        PrintLibStatCodeReadSymbols (module, syntax_tree,
                                     SymbolTableSymbolChainNext (chain), table);
    }

    chain = SymbolTableSymbolChainRelease (chain);

    syntax_tree = NT2OTTransform (syntax_tree);

    PrintLibStatCodeAddBodies (module, MODUL_FUNS (syntax_tree));

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
    symboltable_t *table;

    DBUG_ENTER ("PrintLibStat");

    DBUG_PRINT ("LIBSTAT", ("Loading module `%s'", libname));

    module = LoadModule (libname);

    DBUG_PRINT ("LIBSTAT", ("Getting symbol table"));

    table = GetSymbolTable (module);

    DBUG_PRINT ("LIBSTAT", ("Printing LibStat header\n"));

    PrintLibStatHeader (module);

    DBUG_PRINT ("LIBSTAT", ("Printing table information"));

    SymbolTablePrint (table);

    DBUG_PRINT ("LIBSTAT", ("Printing code"));

    PrintLibStatCode (module, table);

    DBUG_PRINT ("LIBSTAT", ("Destroying table"));

    table = SymbolTableDestroy (table);

    DBUG_PRINT ("LIBSTAT", ("Unloading module `%s'", libname));

    module = UnLoadModule (module);

    DBUG_VOID_RETURN;
}
