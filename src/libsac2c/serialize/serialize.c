/*
 * $Id$
 */

#include "serialize.h"
#include "serialize_info.h"
#include "serialize_stack.h"
#include "dbug.h"
#include "build.h"
#include "str.h"
#include "memory.h"
#include "symboltable.h"
#include "stringset.h"
#include "traverse.h"
#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "filemgr.h"
#include "convert.h"
#include "namespaces.h"
#include "user_types.h"
#include "new_types.h"
#include "shape.h"
#include "map_fun_trav.h"
#include "serialize_symboltable.h"
#include "serialize_filenames.h"
#include "dbug.h"

#define MAX_FUN_NAME_LEN 255

/*
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SER_FILE (result) = NULL;
    INFO_SER_STACK (result) = NULL;
    INFO_SER_TABLE (result) = STinit ();
    INFO_SER_AST (result) = NULL;
    INFO_SER_CURRENT (result) = NULL;
    INFO_SER_ARGAVISDIRECT (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_SER_TABLE (info) = STdestroy (INFO_SER_TABLE (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

serstack_t *
SERbuildSerStack (node *arg_node)
{
    info *info;
    serstack_t *stack;

    DBUG_ENTER ("SERbuildSerStack");

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
GenerateSerFileHead (FILE *file, node *module)
{
    DBUG_ENTER ("GenerateSerFileHead");

    fprintf (file, "/* generated by sac2c %s */\n\n", global.version_id);
    fprintf (file, "#include \"sac_serialize.h\"\n\n");

    fprintf (file, "#include \"namespacemap.h\"\n");
    fprintf (file, "#include \"filenames.h\"\n\n");

    fprintf (file, "#define DROP( x, y) y\n");

    DBUG_VOID_RETURN;
}

static void
GenerateSerFileModuleInfo (node *module, FILE *file)
{
    DBUG_ENTER ("GenerateSerFileModuleInfo");

    fprintf (file,
             "const char *__%s_ASTVERSION() {\n"
             "  return( \"%s\"); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), build_ast);

    fprintf (file,
             "int __%s_SERIALIZER() {\n"
             "  return( %d); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), SAC_SERIALIZE_VERSION);

    fprintf (file,
             "int __%s_USEDFLAGS() {\n"
             "  return( %d); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), GLOBALS_MODFLAGS);

    if (MODULE_DEPRECATED (module) == NULL) {
        fprintf (file,
                 "char * __%s_DEPRECATED() {\n"
                 "  return( (char *) 0); \n}\n\n",
                 NSgetName (MODULE_NAMESPACE (module)));
    } else {
        fprintf (file,
                 "char * __%s_DEPRECATED() {\n"
                 "  return( \"%s\"); \n}\n\n",
                 NSgetName (MODULE_NAMESPACE (module)), MODULE_DEPRECATED (module));
    }

    DBUG_VOID_RETURN;
}

static int
AppendSerFunType (char *funname, ntype *type, int size)
{
    char *pos;
    int written;
    ntype *scalar;

    DBUG_ENTER ("AppendSerFunType");

    pos = &funname[STRlen (funname)];

    DBUG_ASSERT ((size > 1), "fundef name buffer to small!");

    if (TYisScalar (type)) {
        *pos = 'S';
        pos++;
        size--;
        scalar = type;
    } else if ((TYisAKS (type) || (TYisAKV (type)))) {
        char *shape = SHshape2String (0, TYgetShape (type));
        written = 0;

        *pos = 'K';
        pos++;
        size--;

        written = snprintf (pos, size, "%s", shape);
        pos += written;
        size -= written;

        shape = MEMfree (shape);
        scalar = TYgetScalar (type);
    } else if (TYisAKD (type)) {
        int written;
        *pos = 'D';
        pos++;
        size--;

        written = snprintf (pos, size, "%d", TYgetDim (type));
        pos += written;
        size -= written;

        scalar = TYgetScalar (type);
    } else if (TYisAUDGZ (type)) {
        *pos = 'G';
        pos++;
        size--;
        scalar = TYgetScalar (type);
    } else if (TYisAUD (type)) {
        *pos = 'U';
        pos++;
        size--;
        scalar = TYgetScalar (type);
    } else {
        DBUG_ASSERT (0, "unknown shape class!");
        scalar = NULL;
    }

    if (TYisSimple (scalar)) {
        simpletype simple = TYgetSimpleType (scalar);

        written = snprintf (pos, size, "%s", CVbasetype2ShortString (simple));

        pos += written;
        size -= written;
    } else if (TYisUser (scalar)) {
        usertype user = TYgetUserType (scalar);

        written = snprintf (pos, size, "%s__%s", NSgetName (UTgetNamespace (user)),
                            UTgetName (user));

        pos += written;
        size -= written;
    } else if (TYisSymb (scalar)) {
        written = snprintf (pos, MAX_FUN_NAME_LEN - size, "%s__%s",
                            NSgetName (TYgetNamespace (scalar)), TYgetName (scalar));

        pos += written;
        size -= written;
    } else {
        DBUG_ASSERT (0, "unknown scalar type found");
    }

    DBUG_ASSERT ((size > 0), "funname buffer to small");

    *pos = '\0';

    DBUG_RETURN (size);
}

static int
AppendSerFunArgType (char *funname, node *arg, int size)
{
    char *pos;

    DBUG_ENTER ("AppendSerFunArgType");

    DBUG_ASSERT ((size > 2), "funname buffer to small");

    pos = &funname[STRlen (funname)];

    if (ARG_ISARTIFICIAL (arg)) {
        size -= snprintf (pos, size, "_A");
    } else if (ARG_WASREFERENCE (arg)) {
        size -= snprintf (pos, size, "_R");
    } else {
        size -= snprintf (pos, size, "_N");
    }

    DBUG_RETURN (size);
}

static int
AppendSerFunTypeSignature (char *funname, node *fundef, int size)
{
    node *args;

    DBUG_ENTER ("AppendSerFunTypeSignature");

    args = FUNDEF_ARGS (fundef);

    while (args != NULL) {
        size = AppendSerFunArgType (funname, args, size);
        size = AppendSerFunType (funname, AVIS_TYPE (ARG_AVIS (args)), size);

        args = ARG_NEXT (args);
    }

    DBUG_RETURN (size);
}

char *
SERgenerateSerFunName (stentrytype_t type, node *arg_node)
{
    static char buffer[MAX_FUN_NAME_LEN + 1];
    int size = MAX_FUN_NAME_LEN;
    char *result;

    DBUG_ENTER ("SERgenerateSerFunName");

    switch (type) {
    case SET_funbody:
    case SET_wrapperbody:
        size
          -= snprintf (buffer, size, "SBDY_%s_%s_%d_", NSgetName (FUNDEF_NS (arg_node)),
                       FUNDEF_NAME (arg_node), FUNDEF_ISWRAPPERFUN (arg_node));

        size = AppendSerFunTypeSignature (buffer, arg_node, size);

        break;
    case SET_funhead:
    case SET_wrapperhead:
        size -= snprintf (buffer, size, "SHD_%s_%s_%d_", NSgetName (FUNDEF_NS (arg_node)),
                          FUNDEF_NAME (arg_node), FUNDEF_ISWRAPPERFUN (arg_node));

        size = AppendSerFunTypeSignature (buffer, arg_node, size);

        break;
    case SET_typedef:
        size -= snprintf (buffer, size, "STD_%s_%s_", NSgetName (TYPEDEF_NS (arg_node)),
                          TYPEDEF_NAME (arg_node));
        break;
    case SET_objdef:
        size -= snprintf (buffer, size, "SOD_%s_%s", NSgetName (OBJDEF_NS (arg_node)),
                          OBJDEF_NAME (arg_node));
        break;
    default:
        DBUG_ASSERT (0, "Unexpected symboltype found!");
        break;
    }

    DBUG_ASSERT ((size > 0), "internal buffer in SERgenerateSerFunName too small!");

    DBUG_PRINT ("SER", ("Generated new function name: %s", buffer));

    result = STRreplaceSpecialCharacters (buffer);

    DBUG_PRINT ("SER", ("Final function name: %s", result));

    DBUG_RETURN (result);
}

static void
GenerateSerFunHead (node *elem, stentrytype_t type, info *info)
{
    char *funname;

    DBUG_ENTER ("GenerateSerFunHead");

    funname = SERgenerateSerFunName (type, elem);

    fprintf (INFO_SER_FILE (info), "void *%s()", funname);
    fprintf (INFO_SER_FILE (info), "{\n");
    fprintf (INFO_SER_FILE (info), "void *result;\n");
    fprintf (INFO_SER_FILE (info), "void *stack;\n");
    fprintf (INFO_SER_FILE (info), "result = DROP( x");

    MEMfree (funname);

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunMiddle (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunMiddle");

    fprintf (INFO_SER_FILE (info), ");\nstack = SERbuildSerStack( result);\n");

    DBUG_VOID_RETURN;
}

static void
GenerateSerFunTail (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ("GenerateSerFunTail");

    fprintf (INFO_SER_FILE (info), "return( result);\n}\n");

    DBUG_VOID_RETURN;
}

static void
SerializeFundefBody (node *fundef, info *info)
{
    DBUG_ENTER ("SerializeFundefBody");

    INFO_SER_STACK (info) = SERbuildSerStack (FUNDEF_BODY (fundef));

    /*
     * this flag indicates, whether we should link to the avis of an arg
     * directly using DSfetchArgAvis or by using the usual link
     * mechanism.
     * As we serialize the body, the head (including the args) is not in
     * scope, so we have to use the DSfetchArgAvis which gets the right
     * avis by looking it up in the fundef node that has already been
     * deserialized.
     */
    INFO_SER_ARGAVISDIRECT (info) = TRUE;

    /*
     * we do not store function bodies in the symbol
     * table as they are never looked up by symbol
     * name. Instead, the symbolid is always known
     * by looking at the fundefs's signature
     */

    GenerateSerFunHead (fundef, SET_funbody, info);

    FUNDEF_BODY (fundef) = StartSerializeTraversal (FUNDEF_BODY (fundef), info);

    GenerateSerFunMiddle (fundef, SET_funbody, info);

    FUNDEF_BODY (fundef) = StartSerializeLinkTraversal (FUNDEF_BODY (fundef), info);

    GenerateSerFunTail (fundef, SET_funbody, info);

    INFO_SER_ARGAVISDIRECT (info) = FALSE;
    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

static void
SerializeFundefHead (node *fundef, info *info)
{
    DBUG_ENTER ("SerializeFundefHead");

    INFO_SER_STACK (info) = SERbuildSerStack (fundef);

    FUNDEF_SYMBOLNAME (fundef) = SERgenerateSerFunName (SET_funhead, fundef);

    /*
     * we do not store special funs (cond/loop)
     * in the symbol lookup-table as they
     * cannot be directly referenced.
     */

    if (!(FUNDEF_ISDOFUN (fundef) || FUNDEF_ISCONDFUN (fundef))) {
        stvisibility_t vis;

        if (FUNDEF_ISEXPORTED (fundef)) {
            vis = SVT_exported;
        } else if (FUNDEF_ISPROVIDED (fundef)) {
            vis = SVT_provided;
        } else {
            vis = SVT_local;
        }

        /*
         * only store those functions that are
         * visible!
         */
        if (vis != SVT_local) {
            STadd (FUNDEF_NAME (fundef), vis, FUNDEF_SYMBOLNAME (fundef),
                   FUNDEF_ISWRAPPERFUN (fundef) ? SET_wrapperhead : SET_funhead,
                   INFO_SER_TABLE (info));
        }
    }

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

    INFO_SER_STACK (info) = SERbuildSerStack (tdef);

    TYPEDEF_SYMBOLNAME (tdef) = SERgenerateSerFunName (SET_typedef, tdef);

    if (TYPEDEF_ISEXPORTED (tdef)) {
        vis = SVT_exported;
    } else if (TYPEDEF_ISPROVIDED (tdef)) {
        vis = SVT_provided;
    } else {
        vis = SVT_local;
    }

    /*
     * only add those typedefs to the
     * table that are visible
     */
    if (vis != SVT_local) {
        STadd (TYPEDEF_NAME (tdef), vis, TYPEDEF_SYMBOLNAME (tdef), SET_typedef,
               INFO_SER_TABLE (info));
    }

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

    INFO_SER_STACK (info) = SERbuildSerStack (objdef);

    OBJDEF_SYMBOLNAME (objdef) = SERgenerateSerFunName (SET_objdef, objdef);

    if (OBJDEF_ISEXPORTED (objdef)) {
        vis = SVT_exported;
    } else if (OBJDEF_ISPROVIDED (objdef)) {
        vis = SVT_provided;
    } else {
        vis = SVT_local;
    }

    /*
     * only add those objdefs to the table
     * that are indeed visible
     */
    if (vis != SVT_local) {
        STadd (OBJDEF_NAME (objdef), vis, OBJDEF_SYMBOLNAME (objdef), SET_objdef,
               INFO_SER_TABLE (info));
    }

    GenerateSerFunHead (objdef, SET_objdef, info);

    objdef = StartSerializeTraversal (objdef, info);

    GenerateSerFunMiddle (objdef, SET_objdef, info);

    objdef = StartSerializeLinkTraversal (objdef, info);

    GenerateSerFunTail (objdef, SET_objdef, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));

    DBUG_VOID_RETURN;
}

static node *
TagLocalAsSticky (node *fundef, info *arg_info)
{
    DBUG_ENTER ("TagLocalAsSticky");

    if ((!FUNDEF_ISLACFUN (fundef)) && FUNDEF_ISLOCAL (fundef)) {
        FUNDEF_ISSTICKY (fundef) = TRUE;
    }

    DBUG_RETURN (fundef);
}

node *
SERdoSerialize (node *module)
{
    info *info;

    DBUG_ENTER ("SERdoSerialize");

    /*
     * we have to disable DFR now, as for modules every function that
     * has been serialised, has to be present in the binary module
     * as well. Especially when specialising one instance from this
     * module in a later context, all dependent functions need to be
     * present! To do so, we tag all local functions as sticky.
     */
    MODULE_FUNS (module) = MFTdoMapFunTrav (MODULE_FUNS (module), NULL, TagLocalAsSticky);

    DBUG_PRINT ("SER", ("Starting serialization run"));

    info = MakeInfo ();

    INFO_SER_FILE (info) = FMGRwriteOpen ("%s/serialize.c", global.tmp_dirname);

    GenerateSerFileHead (INFO_SER_FILE (info), module);

    GenerateSerFileModuleInfo (module, INFO_SER_FILE (info));

    TRAVpush (TR_ser);

    TRAVdo (module, info);

    TRAVpop ();

    fclose (INFO_SER_FILE (info));
    INFO_SER_FILE (info) = NULL;

    SSTserializeSymbolTable (module, INFO_SER_TABLE (info));

    NSgenerateNamespaceMap ();
    SFNgenerateFilenameTable ();

    info = FreeInfo (info);

    DBUG_RETURN (module);
}

node *
SERfundef (node *arg_node, info *arg_info)
{
    node *last;

    DBUG_ENTER ("SERfundef");

    DBUG_PRINT ("SER", ("Serializing function %s", FUNDEF_NAME (arg_node)));

    last = INFO_SER_CURRENT (arg_info);
    INFO_SER_CURRENT (arg_info) = arg_node;

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

    INFO_SER_CURRENT (arg_info) = last;

    DBUG_RETURN (arg_node);
}

node *
SERtypedef (node *arg_node, info *arg_info)
{
    node *last;

    DBUG_ENTER ("SERtypedef");

    DBUG_PRINT ("SER", ("Serializing typedef %s", TYPEDEF_NAME (arg_node)));

    last = INFO_SER_CURRENT (arg_info);
    INFO_SER_CURRENT (arg_info) = arg_node;

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

    INFO_SER_CURRENT (arg_info) = last;

    DBUG_RETURN (arg_node);
}

node *
SERobjdef (node *arg_node, info *arg_info)
{
    node *last;

    DBUG_ENTER ("SERobjdef");

    DBUG_PRINT ("SER", ("Serializing objdef %s", OBJDEF_NAME (arg_node)));

    last = INFO_SER_CURRENT (arg_info);
    INFO_SER_CURRENT (arg_info) = arg_node;

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

    INFO_SER_CURRENT (arg_info) = last;

    DBUG_RETURN (arg_node);
}

void
SERserializeFundefLink (node *fundef, FILE *file)
{
    char *funname;

    DBUG_ENTER ("SERserializeFundefLink");

    funname = SERgenerateSerFunName (SET_funhead, fundef);

    fprintf (file, "DSlookupFunction( \"%s\", \"%s\")", NSgetModule (FUNDEF_NS (fundef)),
             funname);

    MEMfree (funname);

    DBUG_VOID_RETURN;
}

void
SERserializeObjdefLink (node *objdef, FILE *file)
{
    char *funname;

    DBUG_ENTER ("SERserializeObjdefLink");

    funname = SERgenerateSerFunName (SET_objdef, objdef);

    fprintf (file, "DSlookupObject( \"%s\", \"%s\")", NSgetModule (OBJDEF_NS (objdef)),
             funname);

    MEMfree (funname);

    DBUG_VOID_RETURN;
}
