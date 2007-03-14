/*
 * $Log$
 * Revision 1.1  2005/07/16 18:34:03  sah
 * Initial revision
 *
 *
 *
 */

#include "serialize_info.h"
#include "dbug.h"
#include "symboltable.h"
#include "filemgr.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "serialize_symboltable.h"
#include "globals.h"

static void
GenerateSerSymbolTableAdd (stsymbol_t *symbol, stentry_t *entry, FILE *file)
{
    DBUG_ENTER ("GenerateSerSymbolTableAdd");

    fprintf (file, "STadd( \"%s\", %d, \"%s\", %d, result);\n", STsymbolName (symbol),
             STsymbolVisibility (symbol), STentryName (entry), STentryType (entry));

    DBUG_VOID_RETURN;
}

static void
GenerateSerSymbolTableHead (node *module, FILE *file)
{
    DBUG_ENTER ("GenerateSerSymbolTableHead");

    fprintf (file, "/* generated by sac2c %s */\n\n", global.version_id);

    fprintf (file, "#include \"sac_serialize.h\"\n\n");

    fprintf (file, "void *__%s__SYMTAB()\n", NSgetName (MODULE_NAMESPACE (module)));

    fprintf (file, "{\nvoid *result;\n");
    fprintf (file, "result = STinit();\n");

    DBUG_VOID_RETURN;
}

static void
GenerateSerSymbolTableTail (node *module, FILE *file)
{
    DBUG_ENTER ("GenerateSerSymbolTableTail");

    fprintf (file, "return( result);\n");
    fprintf (file, "}\n");

    DBUG_VOID_RETURN;
}

static void
SerializeSymbolTableSymbol (stsymbol_t *symbol, sttable_t *table, FILE *file)
{
    stentryiterator_t *iterator;

    DBUG_ENTER ("SerializeSymbolTableSymbol");

    iterator = STentryIteratorGet (STsymbolName (symbol), table);

    while (STentryIteratorHasMore (iterator)) {
        GenerateSerSymbolTableAdd (symbol, STentryIteratorNext (iterator), file);
    }

    iterator = STentryIteratorRelease (iterator);

    DBUG_VOID_RETURN;
}

void
SSTserializeSymbolTable (node *module, sttable_t *table)
{
    stsymboliterator_t *iterator;
    FILE *file;

    DBUG_ENTER ("SSTserializeSymbolTable");

    file = FMGRwriteOpen ("%s/symboltable.c", global.tmp_dirname);

    GenerateSerSymbolTableHead (module, file);

    iterator = STsymbolIteratorGet (table);

    while (STsymbolIteratorHasMore (iterator)) {
        SerializeSymbolTableSymbol (STsymbolIteratorNext (iterator), table, file);
    }

    iterator = STsymbolIteratorRelease (iterator);

    GenerateSerSymbolTableTail (module, file);

    fclose (file);

    DBUG_VOID_RETURN;
}