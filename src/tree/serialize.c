/*
 * $Log$
 * Revision 1.8  2004/10/01 08:13:10  sah
 * fixed include to avoid warning
 * on linux platform
 *
 * Revision 1.7  2004/09/30 20:15:52  sah
 * removed some compiler warnings
 *
 * Revision 1.6  2004/09/27 13:18:12  sah
 * implemented new serialization scheme
 *
 * Revision 1.5  2004/09/24 20:21:53  sah
 * intermediate version
 *
 * Revision 1.4  2004/09/23 21:12:25  sah
 * ongoing implementation
 *
 * Revision 1.3  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.2  2004/09/21 09:49:34  sah
 * fixed missing ;
 *
 * Revision 1.1  2004/09/20 19:55:13  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "tree_basic.h"
#include "filemgr.h"
#include "stdio.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "traverse.h"
#include "symboltable.h"
#include "precompile.h"
#include "convert.h"
#include <string.h>

#define MAX_FUN_NAME_LEN 255

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_SER_FILE (result) = NULL;
    INFO_SER_STACK (result) = NULL;
    INFO_SER_TABLE (result) = SymbolTableInit ();

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_SER_TABLE (info) = SymbolTableDestroy (INFO_SER_TABLE (info));

    info = Free (info);

    DBUG_RETURN (info);
}

node *
SerializeLookupFunction (const char *module, const char *name)
{
    DBUG_ENTER ("SerializeLookupFunction");

    DBUG_RETURN ((node *)NULL);
}

static node *
StartSerializeTraversal (node *node, info *info)
{
    funtab *store_tab;

    DBUG_ENTER ("StartSerializeTraversal");

    store_tab = act_tab;
    act_tab = set_tab;

    node = Trav (node, info);

    act_tab = store_tab;

    DBUG_RETURN (node);
}

static void
GenerateSerSymbolTableAdd (const char *symbol, symbolentry_t *entry, FILE *file)
{
    DBUG_ENTER ("GenerateSerSymbolTableAdd");

    fprintf (file, "SymbolTableAdd( \"%s\", \"%s\", %d, result);\n", symbol,
             SymbolTableEntryName (entry), SymbolTableEntryType (entry));

    DBUG_VOID_RETURN;
}

static void
GenerateSerSymbolTableHead (node *module, FILE *file)
{
    DBUG_ENTER ("GenerateSerSymbolTableHead");

    fprintf (file, "/* generated by sac2c %s */\n\n", version_id);
    fprintf (file, "#include \"symboltable.h\"\n\n");

    fprintf (file, "symboltable_t *__%s__SYMTAB()\n", MODDEC_NAME (module));

    fprintf (file, "{\nsymboltable_t *result;\n");
    fprintf (file, "result = SymbolTableInit();\n");

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
SerializeSymbolTableSymbol (const char *symbol, symboltable_t *table, FILE *file)
{
    symbolentrychain_t *chain;

    DBUG_ENTER ("SerializeSymbolTableSymbol");

    chain = SymbolTableEntryChainGet (symbol, table);

    while (SymbolTableEntryChainHasMore (chain)) {
        GenerateSerSymbolTableAdd (symbol, SymbolTableEntryChainNext (chain), file);
    }

    chain = SymbolTableEntryChainRelease (chain);

    DBUG_VOID_RETURN;
}

static void
SerializeSymbolTable (node *module, symboltable_t *table)
{
    symbolchain_t *chain;
    FILE *file;

    DBUG_ENTER ("SerializeSymbolTable");

    file = WriteOpen (TempFileName (tmp_dirname, "STAB_", ".c"));

    GenerateSerSymbolTableHead (module, file);

    chain = SymbolTableSymbolChainGet (table);

    while (SymbolTableSymbolChainHasMore (chain)) {
        SerializeSymbolTableSymbol (SymbolTableSymbolChainNext (chain), table, file);
    }

    chain = SymbolTableSymbolChainRelease (chain);

    GenerateSerSymbolTableTail (module, file);

    DBUG_VOID_RETURN;
}

static void
GenerateSerFileHead (info *info)
{
    DBUG_ENTER ("GenerateSerFileHead");

    fprintf (INFO_SER_FILE (info), "/* generated by sac2c %s */\n\n", version_id);
    fprintf (INFO_SER_FILE (info), "#include \"serialize_helper.h\"\n");
    fprintf (INFO_SER_FILE (info), "#include \"serialize_stack.h\"\n");
    fprintf (INFO_SER_FILE (info), "#include \"tree_basic.h\"\n\n");

    fprintf (INFO_SER_FILE (info), "#define PUSH( x) SerStackPush( x, stack)\n");
    fprintf (INFO_SER_FILE (info), "#define LOOKUP( x) SerStackLookup( x, stack)\n");
    fprintf (INFO_SER_FILE (info), "#define DROP( x, y) y\n");

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunTypeSignature (char *funname, node *args)
{
    DBUG_ENTER ("GenerateSerFunTypeSignature");

    while (args != NULL) {
        strcat (funname, "_");
        strcat (funname, Type2String (ARG_TYPE (args), 2, TRUE));

        args = ARG_NEXT (args);
    }

    DBUG_VOID_RETURN;
}

const char *
GenerateSerFunName (symbolentrytype_t type, node *node)
{
    static char result[MAX_FUN_NAME_LEN];
    char *tmp;

    DBUG_ENTER ("GenerateSerFunName");

    switch (type) {
    case STE_funbody:
        snprintf (result, MAX_FUN_NAME_LEN, "SBDY_%s_%s_%d_", FUNDEF_MOD (node),
                  FUNDEF_NAME (node), FUNDEF_STATUS (node));

        GenerateSerFunTypeSignature (result, FUNDEF_ARGS (node));

        break;
    case STE_funhead:
        snprintf (result, MAX_FUN_NAME_LEN, "SHD_%s_%s_%d_", FUNDEF_MOD (node),
                  FUNDEF_NAME (node), FUNDEF_STATUS (node));

        GenerateSerFunTypeSignature (result, FUNDEF_ARGS (node));

        break;
    case STE_typedef:
        result[0] = '\0';
        break;
    case STE_objdef:
        result[0] = '\0';
        break;
    }

    DBUG_PRINT ("SER", ("Generated new function name: %s", result));

    tmp = ReplaceSpecialCharacters (result);
    strcpy (result, tmp);

    tmp = Free (tmp);

    DBUG_PRINT ("SER", ("Final function name: %s", result));

    DBUG_RETURN (result);
}

static void
GenerateSerFunHead (node *fundef, symbolentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunBodyHead");

    fprintf (INFO_SER_FILE (info), "node *%s()", GenerateSerFunName (type, fundef));
    fprintf (INFO_SER_FILE (info), "{ serstack_t *stack = SerStackInit();\n");
    fprintf (INFO_SER_FILE (info), "node *result;\n");
    fprintf (INFO_SER_FILE (info), "result = DROP( x");

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunTail (node *fundef, symbolentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunBodyTail");

    fprintf (INFO_SER_FILE (info), ");\n");

    fprintf (INFO_SER_FILE (info), "stack = SerStackDestroy( stack);\n");
    fprintf (INFO_SER_FILE (info), "return( result);\n}\n");

    DBUG_VOID_RETURN;
}

static void
SerializeFundefBody (node *fundef, info *info)
{
    char *filename;

    DBUG_ENTER ("SerializeFundefBody");

    filename = TempFileName (tmp_dirname, "SER_", ".c");
    INFO_SER_FILE (info) = WriteOpen (filename);

    INFO_SER_STACK (info) = SerStackInit ();

    SymbolTableAdd (FUNDEF_NAME (fundef), GenerateSerFunName (STE_funbody, fundef),
                    STE_funbody, INFO_SER_TABLE (info));

    GenerateSerFileHead (info);
    GenerateSerFunHead (fundef, STE_funbody, info);

    FUNDEF_BODY (fundef) = StartSerializeTraversal (FUNDEF_BODY (fundef), info);

    GenerateSerFunTail (fundef, STE_funbody, info);

    fclose (INFO_SER_FILE (info));
    Free (filename);

    INFO_SER_FILE (info) = NULL;

    INFO_SER_STACK (info) = SerStackDestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

static void
SerializeFundefHead (node *fundef, info *info)
{
    char *filename;

    DBUG_ENTER ("SerializeFundefHead");

    filename = TempFileName (tmp_dirname, "SER_", ".c");
    INFO_SER_FILE (info) = WriteOpen (filename);

    INFO_SER_STACK (info) = SerStackInit ();

    SymbolTableAdd (FUNDEF_NAME (fundef), GenerateSerFunName (STE_funhead, fundef),
                    STE_funhead, INFO_SER_TABLE (info));

    GenerateSerFileHead (info);
    GenerateSerFunHead (fundef, STE_funhead, info);

    fundef = StartSerializeTraversal (fundef, info);

    GenerateSerFunTail (fundef, STE_funhead, info);

    fclose (INFO_SER_FILE (info));
    Free (filename);

    INFO_SER_FILE (info) = NULL;

    INFO_SER_STACK (info) = SerStackDestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

void
SerializeModule (node *module)
{
    funtab *store_tab;
    info *info;

    DBUG_ENTER ("SerializeModule");

    DBUG_PRINT ("SER", ("Starting serialization run"));

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = ser_tab;

    Trav (module, info);

    act_tab = store_tab;

    SerializeSymbolTable (module, INFO_SER_TABLE (info));

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}

node *
SERFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SERFundef");

    DBUG_PRINT ("SER", ("Serializing function %s", FUNDEF_NAME (arg_node)));

    SerializeFundefHead (arg_node, arg_info);
    SerializeFundefBody (arg_node, arg_info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
