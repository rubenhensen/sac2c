/*
 * $Log$
 * Revision 1.5  2004/11/27 01:36:44  cg
 * Functions renamed.
 *
 * Revision 1.4  2004/11/27 01:21:40  ktr
 * ILIBreplaceSpecialCharacters.
 *
 * Revision 1.3  2004/11/26 21:18:50  sah
 * pour Bodo *<8-)
 *
 * Revision 1.2  2004/11/24 00:11:56  sah
 * COMPILES
 *
 * Revision 1.1  2004/11/23 22:40:47  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "serialize.h"
#include "serialize_info.h"
#include "dbug.h"
#include "internal_lib.h"
#include "symboltable.h"
#include "stringset.h"
#include "traverse.h"
#include "globals.h"
#include "tree_basic.h"
#include "filemgr.h"
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

    result = ILIBmalloc (sizeof (info));

    INFO_SER_FILE (result) = NULL;
    INFO_SER_STACK (result) = NULL;
    INFO_SER_TABLE (result) = STinit ();
    INFO_SER_AST (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_SER_TABLE (info) = STdestroy (INFO_SER_TABLE (info));

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

serstack_t *
SerializeBuildSerStack (node *arg_node)
{
    info *info;
    serstack_t *stack;

    DBUG_ENTER ("SerializeBuildSerStack");

    info = MakeInfo ();
    stack = SSinit ();

    INFO_SER_STACK (info) = stack;

    TRAVpush (TR_sbt);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_EXECUTE ("BST", SSdump (stack););

    DBUG_RETURN (stack);
}

static node *
StartSerializeTraversal (node *node, info *info)
{
    DBUG_ENTER ("StartSerializeTraversal");

    TRAVpush (TR_set);

    node = TRAVdo (node, info);

    TRAVpop ();

    DBUG_RETURN (node);
}

static node *
StartSerializeLinkTraversal (node *node, info *info)
{
    DBUG_ENTER ("StartSerializeLinkTraversal");

    TRAVpush (TR_sel);

    node = TRAVdo (node, info);

    TRAVpop ();

    DBUG_RETURN (node);
}

static void
GenerateSerFileHead (FILE *file)
{
    DBUG_ENTER ("GenerateSerFileHead");

    fprintf (file, "/* generated by sac2c %s */\n\n", global.version_id);
    fprintf (file, "#include \"sac_serialize.h\"\n\n");

    fprintf (file, "#define DROP( x, y) y\n");
    fprintf (file, "#define NULL (void *) 0\n");

    DBUG_VOID_RETURN;
}

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

    fprintf (file, "void *__%s__SYMTAB()\n", MODULE_NAME (module));

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

static void
SerializeSymbolTable (node *module, sttable_t *table)
{
    stsymboliterator_t *iterator;
    FILE *file;

    DBUG_ENTER ("SerializeSymbolTable");

    file = FMGRwriteOpen ("%s/symboltable.c", global.tmp_dirname);

    GenerateSerFileHead (file);

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

static void
GenerateSerDependencyTableHead (node *module, FILE *file)
{
    DBUG_ENTER ("GenerateSerDependencyTableHead");

    fprintf (file, "/* generated by sac2c %s */\n\n", global.version_id);
    fprintf (file, "void *__%s__DEPTAB() {\n", MODULE_NAME (module));
    fprintf (file, "void *result = (void *) 0;\n");

    DBUG_VOID_RETURN;
}

static void
GenerateSerDependencyTableTail (node *module, FILE *file)
{
    DBUG_ENTER ("GenerateSerDependencyTableTail");

    fprintf (file, "return(result);\n}\n");

    DBUG_VOID_RETURN;
}

static void *
SerTableEntriesFoldFun (const char *val, strstype_t kind, void *rest)
{
    str_buf *result;

    DBUG_ENTER ("SerTableEntriesFoldFun");

    switch (kind) {
    case STRS_saclib:
    case STRS_extlib:
        result = ILIBstrBufPrintf ((str_buf *)rest,
                                   "result = STRSadd( \"%s\", %d, result);\n", val, kind);
        break;
    default:
        break;
    }

    DBUG_RETURN (result);
}

static void
GenerateSerDependencyTableEntries (node *module, FILE *file)
{
    str_buf *buffer;
    char *string;

    DBUG_ENTER ("GenerateSerDependencyTableEntries");

    buffer = ILIBstrBufCreate (4096);

    buffer = STRSfold (&SerTableEntriesFoldFun, MODULE_DEPENDENCIES (module), buffer);

    string = ILIBstrBuf2String (buffer);

    fprintf (file, string);

    string = ILIBfree (string);
    buffer = ILIBstrBufFree (buffer);

    DBUG_VOID_RETURN;
}

static void
SerializeDependencyTable (node *module)
{
    FILE *file;

    DBUG_ENTER ("SerializeDependencyTable");

    file = FMGRwriteOpen ("%s/dependencytable.c", global.tmp_dirname);

    GenerateSerFileHead (file);

    GenerateSerDependencyTableHead (module, file);

    GenerateSerDependencyTableEntries (module, file);

    GenerateSerDependencyTableTail (module, file);

    fclose (file);

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunTypeSignature (char *funname, node *args)
{
    DBUG_ENTER ("GenerateSerFunTypeSignature");

    while (args != NULL) {
        strcat (funname, "_");
        strcat (funname, CVtype2String (ARG_TYPE (args), 2, TRUE));

        args = ARG_NEXT (args);
    }

    DBUG_VOID_RETURN;
}

const char *
SERgenerateSerFunName (stentrytype_t type, node *node)
{
    static char result[MAX_FUN_NAME_LEN];
    char *tmp;

    DBUG_ENTER ("SERgenerateSerFunName");

    switch (type) {
    case SET_funbody:
    case SET_wrapperbody:
        snprintf (result, MAX_FUN_NAME_LEN, "SBDY_%s_%s_%d_", FUNDEF_MOD (node),
                  FUNDEF_NAME (node), type);

        GenerateSerFunTypeSignature (result, FUNDEF_ARGS (node));

        break;
    case SET_funhead:
    case SET_wrapperhead:
        snprintf (result, MAX_FUN_NAME_LEN, "SHD_%s_%s_%d_", FUNDEF_MOD (node),
                  FUNDEF_NAME (node), type);

        GenerateSerFunTypeSignature (result, FUNDEF_ARGS (node));

        break;
    case SET_typedef:
        snprintf (result, MAX_FUN_NAME_LEN, "STD_%s_%s_", TYPEDEF_MOD (node),
                  TYPEDEF_NAME (node));
        break;
    case SET_objdef:
        snprintf (result, MAX_FUN_NAME_LEN, "SOD_%s_%s_", OBJDEF_MOD (node),
                  OBJDEF_NAME (node));
        break;
    default:
        DBUG_ASSERT (0, "Unexpected symboltype found!");
        break;
    }

    DBUG_PRINT ("SER", ("Generated new function name: %s", result));

    tmp = ILIBreplaceSpecialCharacters (result);
    strcpy (result, tmp);

    tmp = ILIBfree (tmp);

    DBUG_PRINT ("SER", ("Final function name: %s", result));

    DBUG_RETURN (result);
}

static void
GenerateSerFunHead (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunBodyHead");

    fprintf (INFO_SER_FILE (info), "void *%s()", SERgenerateSerFunName (type, elem));
    fprintf (INFO_SER_FILE (info), "{\n");
    fprintf (INFO_SER_FILE (info), "void *result;\n");
    fprintf (INFO_SER_FILE (info), "void *stack;\n");
    fprintf (INFO_SER_FILE (info), "result = DROP( x");

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunMiddle (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunMiddle");

    fprintf (INFO_SER_FILE (info), ");\nstack = SerializeBuildSerStack( result);\n");

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunTail (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunBodyTail");

    fprintf (INFO_SER_FILE (info), "return( result);\n}\n");

    DBUG_VOID_RETURN;
}

static void
SerializeFundefBody (node *fundef, info *info)
{
    stvisibility_t vis;
    DBUG_ENTER ("SerializeFundefBody");

    INFO_SER_STACK (info) = SerializeBuildSerStack (FUNDEF_BODY (fundef));

    if (FUNDEF_ISEXPORTED (fundef)) {
        vis = SVT_exported;
    } else if (FUNDEF_ISPROVIDED (fundef)) {
        vis = SVT_provided;
    } else {
        vis = SVT_local;
    }

    STadd (FUNDEF_NAME (fundef), vis, SERgenerateSerFunName (SET_funbody, fundef),
           FUNDEF_ISWRAPPERFUN (fundef) ? SET_wrapperbody : SET_funbody,
           INFO_SER_TABLE (info));

    GenerateSerFunHead (fundef, SET_funbody, info);

    FUNDEF_BODY (fundef) = StartSerializeTraversal (FUNDEF_BODY (fundef), info);

    GenerateSerFunMiddle (fundef, SET_funbody, info);

    FUNDEF_BODY (fundef) = StartSerializeLinkTraversal (FUNDEF_BODY (fundef), info);

    GenerateSerFunTail (fundef, SET_funbody, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

static void
SerializeFundefHead (node *fundef, info *info)
{
    stvisibility_t vis;

    DBUG_ENTER ("SerializeFundefHead");

    INFO_SER_STACK (info) = SerializeBuildSerStack (fundef);

    if (FUNDEF_ISEXPORTED (fundef)) {
        vis = SVT_exported;
    } else if (FUNDEF_ISPROVIDED (fundef)) {
        vis = SVT_provided;
    } else {
        vis = SVT_local;
    }

    FUNDEF_SYMBOLNAME (fundef)
      = ILIBstringCopy (SERgenerateSerFunName (SET_funhead, fundef));

    STadd (FUNDEF_NAME (fundef), vis, FUNDEF_SYMBOLNAME (fundef),
           FUNDEF_ISWRAPPERFUN (fundef) ? SET_wrapperhead : SET_funhead,
           INFO_SER_TABLE (info));

    GenerateSerFunHead (fundef, SET_funhead, info);

    fundef = StartSerializeTraversal (fundef, info);

    GenerateSerFunMiddle (fundef, SET_funhead, info);

    fundef = StartSerializeLinkTraversal (fundef, info);

    GenerateSerFunTail (fundef, SET_funhead, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

static void
SerializeTypedef (node *tdef, info *info)
{
    stvisibility_t vis;

    DBUG_ENTER ("SerializeTypedef");

    INFO_SER_STACK (info) = SerializeBuildSerStack (tdef);

    TYPEDEF_SYMBOLNAME (tdef)
      = ILIBstringCopy (SERgenerateSerFunName (SET_typedef, tdef));

    if (TYPEDEF_ISEXPORTED (tdef)) {
        vis = SVT_exported;
    } else if (TYPEDEF_ISPROVIDED (tdef)) {
        vis = SVT_provided;
    } else {
        vis = SVT_local;
    }

    STadd (TYPEDEF_NAME (tdef), vis, TYPEDEF_SYMBOLNAME (tdef), SET_typedef,
           INFO_SER_TABLE (info));

    GenerateSerFunHead (tdef, SET_typedef, info);

    tdef = StartSerializeTraversal (tdef, info);

    GenerateSerFunMiddle (tdef, SET_typedef, info);

    tdef = StartSerializeLinkTraversal (tdef, info);

    GenerateSerFunTail (tdef, SET_typedef, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

static void
SerializeObjdef (node *objdef, info *info)
{
    stvisibility_t vis;

    DBUG_ENTER ("SerializeObjdef");

    INFO_SER_STACK (info) = SerializeBuildSerStack (objdef);

    OBJDEF_SYMBOLNAME (objdef)
      = ILIBstringCopy (SERgenerateSerFunName (SET_objdef, objdef));

    if (OBJDEF_ISEXPORTED (objdef)) {
        vis = SVT_exported;
    } else if (OBJDEF_ISPROVIDED (objdef)) {
        vis = SVT_provided;
    } else {
        vis = SVT_local;
    }

    STadd (OBJDEF_NAME (objdef), vis, OBJDEF_SYMBOLNAME (objdef), SET_objdef,
           INFO_SER_TABLE (info));

    GenerateSerFunHead (objdef, SET_objdef, info);

    objdef = StartSerializeTraversal (objdef, info);

    GenerateSerFunMiddle (objdef, SET_objdef, info);

    objdef = StartSerializeLinkTraversal (objdef, info);

    GenerateSerFunTail (objdef, SET_objdef, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

node *
SerializeModule (node *module)
{
    info *info;

    DBUG_ENTER ("SerializeModule");

    DBUG_PRINT ("SER", ("Starting serialization run"));

    info = MakeInfo ();

    INFO_SER_FILE (info) = FMGRwriteOpen ("%s/serialize.c", global.tmp_dirname);

    GenerateSerFileHead (INFO_SER_FILE (info));

    TRAVpush (TR_ser);

    TRAVdo (module, info);

    TRAVpop ();

    fclose (INFO_SER_FILE (info));
    INFO_SER_FILE (info) = NULL;

    SerializeSymbolTable (module, INFO_SER_TABLE (info));
    SerializeDependencyTable (module);

    info = FreeInfo (info);

    DBUG_RETURN (module);
}

void
SerializeFundefLink (node *fundef, FILE *file)
{
    DBUG_ENTER ("SerializeFundefLink");

    if (fundef == NULL) {
        fprintf (file, "NULL");
    } else {
        fprintf (file, "DeserializeLookupFunction( \"%s\", \"%s\")", FUNDEF_MOD (fundef),
                 SERgenerateSerFunName (SET_funhead, fundef));
    }

    DBUG_VOID_RETURN;
}

node *
SERFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SERFundef");

    DBUG_PRINT ("SER", ("Serializing function %s", FUNDEF_NAME (arg_node)));

    /*
     * only serialize functions that are not available
     * in another module
     */
    if (FUNDEF_ISLOCAL (arg_node)) {
        SerializeFundefHead (arg_node, arg_info);

        /*
         * there might be functions without a body
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            SerializeFundefBody (arg_node, arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SERTypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SERTypedef");

    DBUG_PRINT ("SER", ("Serializing typedef %s", TYPEDEF_NAME (arg_node)));

    /*
     * Only serialize typedefs that are not available in another
     * module
     */
    if (TYPEDEF_ISLOCAL (arg_node)) {
        SerializeTypedef (arg_node, arg_info);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
SERObjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SERObjdef");

    DBUG_PRINT ("SER", ("Serializing objdef %s", OBJDEF_NAME (arg_node)));

    /*
     * only serialize objdefs that are not available in another
     * module
     */
    if (OBJDEF_SYMBOLNAME (arg_node) == NULL) {
        SerializeObjdef (arg_node, arg_info);
    }

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = TRAVdo (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
