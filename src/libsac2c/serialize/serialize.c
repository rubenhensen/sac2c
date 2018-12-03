#include "serialize.h"
#include "serialize_info.h"
#include "serialize_stack.h"

#define DBUG_PREFIX "SER"
#include "debug.h"

#include "build.h"
#include "str.h"
#include "str_buffer.h"
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

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

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
    DBUG_ENTER ();

    INFO_SER_TABLE (info) = STdestroy (INFO_SER_TABLE (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

serstack_t *
SERbuildSerStack (node *arg_node)
{
    info *info;
    serstack_t *stack;

    DBUG_ENTER ();

    info = MakeInfo ();
    stack = SSinit ();

    INFO_SER_STACK (info) = stack;

    TRAVpush (TR_sbt);

    arg_node = TRAVdo (arg_node, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_EXECUTE_TAG ("BST", SSdump (stack));

    DBUG_RETURN (stack);
}

static node *
StartSerializeTraversal (node *node, info *info)
{
    DBUG_ENTER ();

    TRAVpush (TR_set);

    node = TRAVdo (node, info);

    TRAVpop ();

    DBUG_RETURN (node);
}

static node *
StartSerializeLinkTraversal (node *node, info *info)
{
    DBUG_ENTER ();

    TRAVpush (TR_sel);

    node = TRAVdo (node, info);

    TRAVpop ();

    DBUG_RETURN (node);
}

static void
GenerateSerFileHead (FILE *file, node *module)
{
    DBUG_ENTER ();

    fprintf (file, "/* generated by sac2c %s */\n\n", global.version_id);
    fprintf (file, "#include \"sac_serialize.h\"\n\n");

    fprintf (file, "#include \"namespacemap.h\"\n");
    fprintf (file, "#include \"filenames.h\"\n\n");

    fprintf (file, "#ifdef __cplusplus\n"
                   "extern \"C\" {\n"
                   "#endif\n\n");

    fprintf (file, "#define DROP( x, y) y\n");
    
    /* This is an implementation of a static assert that checks the value of
       `e` at compile time and terminates compilation in case it is 0.
       If we ever switch to C11 or later such a funtionality is built-in.  */
    fprintf (file, "#define STATIC_ASSERT(e, x)  "
                   "((struct {const int junk[e ? 1 : -1];}){.junk={1}}.junk[0] ? x : x)\n");

    /* Check that the size of the value `v` matches the type of the type `t`.
       Terminate compilation if it doesn't and return `v` otherwise.
       For example:

          ASSERT_TYPESIZE (int,  5)    evaluates to 5
          ASSERT_TYPESIZE (char, 5)    terminates the compilation  */
    fprintf (file, "#define ASSERT_TYPESIZE(t, v) STATIC_ASSERT (sizeof (v) == sizeof (t), v)\n");

    DBUG_RETURN ();
}

static void
GenerateSerFileFooter (FILE *file)
{
    fprintf (file, "#ifdef __cplusplus\n"
                   "} /* extern C  */\n"
                   "#endif\n\n");
}

static void
GenerateSerFileModuleInfo (node *module, FILE *file)
{
    DBUG_ENTER ();

    char *uppercase;
    str_buf *sbuf;

    uppercase = STRcpy (NSgetName (MODULE_NAMESPACE (module)));

    STRtoupper (uppercase, 0, STRlen (uppercase));

    fprintf (file,
             "const char *__%s_MIXEDCASENAME( void) {\n"
             "  return( \"%s\"); \n}\n\n",
             uppercase, NSgetName (MODULE_NAMESPACE (module)));

    fprintf (file,
             "const char *__%s_ASTVERSION( void) {\n"
             "  return( \"%s\"); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), build_ast);

    fprintf (file,
             "int __%s_SERIALIZER( void) {\n"
             "  return( %d); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), SAC_SERIALIZE_VERSION);

    fprintf (file,
             "int __%s_USEDFLAGS( void) {\n"
             "  return( %d); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), GLOBALS_MODFLAGS);

    sbuf = SBUFcreate (1024);
    sbuf = SBUFprintf (sbuf, "NULL");
    sbuf = STRSfold (&STRStoSafeCEncodedStringFold, MODULE_HEADERS (module), sbuf);
    fprintf (file,
             "void * __%s_HEADERS( void) {\n"
             "  return( %s); \n}\n\n",
             NSgetName (MODULE_NAMESPACE (module)), SBUF2str (sbuf));
    sbuf = SBUFfree (sbuf);

    if (MODULE_DEPRECATED (module) == NULL) {
        fprintf (file,
                 "char * __%s_DEPRECATED( void) {\n"
                 "  return( (char *) 0); \n}\n\n",
                 NSgetName (MODULE_NAMESPACE (module)));
    } else {
        fprintf (file,
                 "char * __%s_DEPRECATED( void) {\n"
                 "  return( \"%s\"); \n}\n\n",
                 NSgetName (MODULE_NAMESPACE (module)), MODULE_DEPRECATED (module));
    }

    DBUG_RETURN ();
}

static str_buf *
AppendSerFunType (str_buf *funname, ntype *type)
{
    ntype *scalar;

    DBUG_ENTER ();

    if ((TYisAKS (type) || (TYisAKV (type)))) {
        char *shape = SHshape2String (0, TYgetShape (type));

        funname = SBUFprintf (funname, "K%s", shape);

        shape = MEMfree (shape);
        scalar = TYgetScalar (type);
    } else if (TYisAKD (type)) {
        funname = SBUFprintf (funname, "D%d", TYgetDim (type));

        scalar = TYgetScalar (type);
    } else if (TYisAUDGZ (type)) {
        funname = SBUFprintf (funname, "G");

        scalar = TYgetScalar (type);
    } else if (TYisAUD (type)) {
        funname = SBUFprintf (funname, "U");

        scalar = TYgetScalar (type);
    } else {
        DBUG_UNREACHABLE ("unknown shape class!");
        scalar = NULL;
    }

    if (TYisSimple (scalar)) {
        simpletype simple = TYgetSimpleType (scalar);

        funname = SBUFprintf (funname, "%s", CVbasetype2ShortString (simple));
    } else if (TYisUser (scalar)) {
        usertype user = TYgetUserType (scalar);

        funname = SBUFprintf (funname, "%s__%s", NSgetName (UTgetNamespace (user)),
                              UTgetName (user));

    } else if (TYisSymb (scalar)) {
        funname = SBUFprintf (funname, "%s__%s", NSgetName (TYgetNamespace (scalar)),
                              TYgetName (scalar));
    } else {
        DBUG_UNREACHABLE ("unknown scalar type found");
    }

    DBUG_RETURN (funname);
}

static str_buf *
AppendSerFunArgType (str_buf *funname, node *arg)
{
    DBUG_ENTER ();

    if (ARG_ISARTIFICIAL (arg)) {
        funname = SBUFprintf (funname, "_A");
    } else if (ARG_WASREFERENCE (arg)) {
        funname = SBUFprintf (funname, "_R");
    } else {
        funname = SBUFprintf (funname, "_N");
    }

    DBUG_RETURN (funname);
}

static str_buf *
AppendSerFunTypeSignature (str_buf *funname, node *fundef)
{
    node *args;

    DBUG_ENTER ();

    args = FUNDEF_ARGS (fundef);

    while (args != NULL) {
        funname = AppendSerFunArgType (funname, args);
        funname = AppendSerFunType (funname, AVIS_TYPE (ARG_AVIS (args)));

        args = ARG_NEXT (args);
    }

    DBUG_RETURN (funname);
}

static char *
GenerateSerFunName (stentrytype_t type, node *arg_node)
{
    static str_buf *buffer = NULL;
    char *tmp_name;
    char *result;

    DBUG_ENTER ();

    if (buffer == NULL) {
        buffer = SBUFcreate (255);
    }

    switch (type) {
    case SET_funbody:
    case SET_wrapperbody:
        DBUG_UNREACHABLE ("cannot generate names for function bodies!");

        break;
    case SET_funhead:
    case SET_wrapperhead:
        buffer = SBUFprintf (buffer, "SHDR_%s_%s_%d%d_", NSgetName (FUNDEF_NS (arg_node)),
                             FUNDEF_NAME (arg_node), FUNDEF_ISWRAPPERFUN (arg_node),
                             FUNDEF_ISOBJECTWRAPPER (arg_node));

        buffer = AppendSerFunTypeSignature (buffer, arg_node);

        break;
    case SET_typedef:
        buffer = SBUFprintf (buffer, "STD_%s_%s_", NSgetName (TYPEDEF_NS (arg_node)),
                             TYPEDEF_NAME (arg_node));
        break;
    case SET_objdef:
        buffer = SBUFprintf (buffer, "SOD_%s_%s", NSgetName (OBJDEF_NS (arg_node)),
                             OBJDEF_NAME (arg_node));
        break;
    default:
        DBUG_UNREACHABLE ("Unexpected symboltype found!");
        break;
    }

    tmp_name = SBUF2str (buffer);
    SBUFflush (buffer);

    DBUG_PRINT ("Generated new function name: %s", tmp_name);

    result = STRreplaceSpecialCharacters (tmp_name);
    tmp_name = MEMfree (tmp_name);

    DBUG_PRINT ("Final function name: %s", result);

    DBUG_RETURN (result);
}

char *
SERfundefHeadSymbol2BodySymbol (const char *symbol)
{
    char *result;

    /*
     * NOTE: this renaming has to be kept in line with the generation
     *       of serialize function names in SERgenerateSerFunName!
     */
    DBUG_ENTER ();

    DBUG_ASSERT (STRprefix ("SHDR", symbol),
                 "given symbol is not a function header symbol!");

    result = STRcpy (symbol);

    result[1] = 'B';
    result[2] = 'D';
    result[3] = 'Y';

    DBUG_RETURN (result);
}

static char *
GetSerFunName (stentrytype_t type, node *arg_node)
{
    char *result = NULL;

    DBUG_ENTER ();

    switch (type) {
    case SET_funbody:
    case SET_wrapperbody:
        /* reuse symbol for header to construct symbol for body */
        DBUG_ASSERT (FUNDEF_SYMBOLNAME (arg_node) != NULL,
                     "cannot produce name for fundef body before "
                     "fundef head has been serialized!");
        result = SERfundefHeadSymbol2BodySymbol (FUNDEF_SYMBOLNAME (arg_node));

        break;
    case SET_funhead:
    case SET_wrapperhead:
        if (FUNDEF_SYMBOLNAME (arg_node) == NULL) {
            FUNDEF_SYMBOLNAME (arg_node) = GenerateSerFunName (type, arg_node);
        }
        result = STRcpy (FUNDEF_SYMBOLNAME (arg_node));

        break;
    case SET_objdef:
        if (OBJDEF_SYMBOLNAME (arg_node) == NULL) {
            OBJDEF_SYMBOLNAME (arg_node) = GenerateSerFunName (type, arg_node);
        }
        result = STRcpy (OBJDEF_SYMBOLNAME (arg_node));

        break;
    case SET_typedef:
        if (TYPEDEF_SYMBOLNAME (arg_node) == NULL) {
            TYPEDEF_SYMBOLNAME (arg_node) = GenerateSerFunName (type, arg_node);
        }
        result = STRcpy (TYPEDEF_SYMBOLNAME (arg_node));

        break;
    default:
        DBUG_UNREACHABLE ("Unexpected symboltype found!");
        break;
    }

    DBUG_PRINT ("Grabbed function name: %s", result);

    DBUG_RETURN (result);
}

char *
SERgetSerFunName (node *arg_node)
{
    char *result;

    DBUG_ENTER ();

    switch (NODE_TYPE (arg_node)) {
    case N_fundef:
        if (FUNDEF_ISWRAPPERFUN (arg_node)) {
            result = GetSerFunName (SET_wrapperhead, arg_node);
        } else {
            result = GetSerFunName (SET_funhead, arg_node);
        }
        break;

    case N_objdef:
        result = GetSerFunName (SET_objdef, arg_node);
        break;

    case N_typedef:
        result = GetSerFunName (SET_typedef, arg_node);
        break;

    default:
        DBUG_UNREACHABLE ("unexpected node type.");
        result = NULL;
    }

    DBUG_RETURN (result);
}

static void
GenerateSerFunHead (node *elem, stentrytype_t type, info *info)
{
    char *funname;

    DBUG_ENTER ();

    funname = GetSerFunName (type, elem);

    fprintf (INFO_SER_FILE (info), "void *%s( void)", funname);
    fprintf (INFO_SER_FILE (info), "{\n");
    fprintf (INFO_SER_FILE (info), "void *result;\n");
    fprintf (INFO_SER_FILE (info), "void *stack;\n");
    fprintf (INFO_SER_FILE (info), "result = DROP( x");

    MEMfree (funname);

    DBUG_RETURN ();
}

static void
GenerateSerFunMiddle (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), ");\nstack = SERbuildSerStack( result);\n");

    DBUG_RETURN ();
}

static void
GenerateSerFunTail (node *elem, stentrytype_t type, info *info)
{
    DBUG_ENTER ();

    fprintf (INFO_SER_FILE (info), "return( result);\n}\n");

    DBUG_RETURN ();
}

static void
SerializeFundefBody (node *fundef, info *info)
{
    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

static void
SerializeFundefHead (node *fundef, info *info)
{
    char *funname;

    DBUG_ENTER ();

    INFO_SER_STACK (info) = SERbuildSerStack (fundef);

    funname = GetSerFunName (SET_funhead, fundef);

    /*
     * we do not store special funs (cond/loop)
     * in the symbol lookup-table as they
     * cannot be directly referenced.
     */

    if (!(FUNDEF_ISLOOPFUN (fundef) || FUNDEF_ISCONDFUN (fundef))) {
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
            unsigned argc = 0;

            if (!FUNDEF_HASDOTARGS (fundef) && !FUNDEF_HASDOTRETS (fundef)) {
                node *args = FUNDEF_ARGS (fundef);
                while (args) {
                    argc++; args = ARG_NEXT (args);
                }
            }

            STadd (FUNDEF_NAME (fundef), vis, funname,
                   FUNDEF_ISWRAPPERFUN (fundef) ? SET_wrapperhead : SET_funhead,
                   INFO_SER_TABLE (info), argc);
        }
    }

    GenerateSerFunHead (fundef, SET_funhead, info);

    fundef = StartSerializeTraversal (fundef, info);

    GenerateSerFunMiddle (fundef, SET_funhead, info);

    fundef = StartSerializeLinkTraversal (fundef, info);

    GenerateSerFunTail (fundef, SET_funhead, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));
    funname = MEMfree (funname);

    DBUG_RETURN ();
}

static void
SerializeTypedef (node *tdef, info *info)
{
    stvisibility_t vis;
    char *funname;

    DBUG_ENTER ();

    INFO_SER_STACK (info) = SERbuildSerStack (tdef);

    funname = GetSerFunName (SET_typedef, tdef);

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
        STadd (TYPEDEF_NAME (tdef), vis, funname, SET_typedef, INFO_SER_TABLE (info), 0);
    }

    GenerateSerFunHead (tdef, SET_typedef, info);

    tdef = StartSerializeTraversal (tdef, info);

    GenerateSerFunMiddle (tdef, SET_typedef, info);

    tdef = StartSerializeLinkTraversal (tdef, info);

    GenerateSerFunTail (tdef, SET_typedef, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));
    funname = MEMfree (funname);

    DBUG_RETURN ();
}

static void
SerializeObjdef (node *objdef, info *info)
{
    stvisibility_t vis;
    char *funname;

    DBUG_ENTER ();

    INFO_SER_STACK (info) = SERbuildSerStack (objdef);

    funname = GetSerFunName (SET_objdef, objdef);

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
        STadd (OBJDEF_NAME (objdef), vis, funname, SET_objdef, INFO_SER_TABLE (info), 0);
    }

    GenerateSerFunHead (objdef, SET_objdef, info);

    objdef = StartSerializeTraversal (objdef, info);

    GenerateSerFunMiddle (objdef, SET_objdef, info);

    objdef = StartSerializeLinkTraversal (objdef, info);

    GenerateSerFunTail (objdef, SET_objdef, info);

    INFO_SER_STACK (info) = SSdestroy (INFO_SER_STACK (info));
    funname = MEMfree (funname);

    DBUG_RETURN ();
}

static node *
TagLocalAsSticky (node *fundef, info *arg_info)
{
    DBUG_ENTER ();

    if ((!FUNDEF_ISLACFUN (fundef)) && FUNDEF_ISLOCAL (fundef)) {
        FUNDEF_ISSTICKY (fundef) = TRUE;
    }

    DBUG_RETURN (fundef);
}

node *
SERdoSerialize (node *module)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting serialization run");

    info = MakeInfo ();

    INFO_SER_FILE (info) = FMGRwriteOpen ("%s/serialize.c", global.tmp_dirname);

    GenerateSerFileHead (INFO_SER_FILE (info), module);

    GenerateSerFileModuleInfo (module, INFO_SER_FILE (info));

    TRAVpush (TR_ser);

    TRAVdo (module, info);

    TRAVpop ();

    GenerateSerFileFooter (INFO_SER_FILE (info));

    fclose (INFO_SER_FILE (info));
    INFO_SER_FILE (info) = NULL;

    SSTserializeSymbolTable (module, INFO_SER_TABLE (info));

    NSgenerateNamespaceMap ();
    SFNgenerateFilenameTable ();

    info = FreeInfo (info);

    /*
     * we have to disable DFR now, as for modules every function that
     * has been serialised, has to be present in the binary module
     * as well. Especially when specialising one instance from this
     * module in a later context, all dependent functions need to be
     * present! To do so, we tag all local functions as sticky.
     */
    module = MFTdoMapFunTrav (module, NULL, TagLocalAsSticky);

    DBUG_RETURN (module);
}

node *
SERfundef (node *arg_node, info *arg_info)
{
    node *last;

    DBUG_ENTER ();

    DBUG_PRINT ("Serializing function %s", FUNDEF_NAME (arg_node));

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

    DBUG_ENTER ();

    DBUG_PRINT ("Serializing typedef %s", TYPEDEF_NAME (arg_node));

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

    DBUG_ENTER ();

    DBUG_PRINT ("Serializing objdef %s", OBJDEF_NAME (arg_node));

    last = INFO_SER_CURRENT (arg_info);
    INFO_SER_CURRENT (arg_info) = arg_node;

    /*
     * only serialize objdefs that are not available in another
     * module
     */
    if (OBJDEF_ISLOCAL (arg_node)) {
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

    DBUG_ENTER ();

    funname = GetSerFunName (SET_funhead, fundef);

    fprintf (file, "DSlookupFunction( \"%s\", \"%s\")", NSgetModule (FUNDEF_NS (fundef)),
             funname);

    funname = MEMfree (funname);

    DBUG_RETURN ();
}

void
SERserializeObjdefLink (node *objdef, FILE *file)
{
    char *funname;

    DBUG_ENTER ();

    funname = GetSerFunName (SET_objdef, objdef);

    fprintf (file, "DSlookupObject( \"%s\", \"%s\")", NSgetModule (OBJDEF_NS (objdef)),
             funname);

    funname = MEMfree (funname);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
