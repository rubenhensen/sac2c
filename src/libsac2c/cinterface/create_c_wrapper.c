/* $Id$ */

/** <!--********************************************************************-->
 *
 * @defgroup ccw Create C wrapper
 *
 * Creates a C wrapper header and implementation file from a given syntax tree.
 *
 * @ingroup ccw
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file creater_wrapper_header.c
 *
 * Prefix: CCW
 *
 *****************************************************************************/
#include "create_c_wrapper.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "filemgr.h"
#include "globals.h"
#include "build.h"
#include "stringset.h"
#include "str_buffer.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "user_types.h"
#include "ctinfo.h"
#include "str.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool inbundle;
    FILE *file;
    int counter;
    bool comment;
    bool decl;
    bool body;
    enum { CCI_unknown, CCI_header, CCI_impl } mode;
};

/**
 * INFO macros
 */
#define INFO_INBUNDLE(n) ((n)->inbundle)
#define INFO_FILE(n) ((n)->file)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_COMMENT(n) ((n)->comment)
#define INFO_MODE(n) ((n)->mode)
#define INFO_DECL(n) ((n)->decl)
#define INFO_BODY(n) ((n)->body)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_INBUNDLE (result) = FALSE;
    INFO_FILE (result) = NULL;
    INFO_COUNTER (result) = 0;
    INFO_COMMENT (result) = FALSE;
    INFO_MODE (result) = CCI_unknown;
    INFO_DECL (result) = FALSE;
    INFO_BODY (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CCWdoCreateCWrapper( node *syntax_tree)
 *
 *****************************************************************************/
node *
CCWdoCreateCWrapper (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CCWdoCreateCWrapper");

    info = MakeInfo ();

    TRAVpush (TR_ccw);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 * @fn str_buf *PrintModuleNames( const char *module, strstype_t kind,
 *                                str_buf *buffer)
 *
 * @brief strsfoldfun that appends a module name with prepended C
 *        comment * to a string buffer.
 *
 * @param module name of module
 * @param kind   ignored
 * @param buffer buffer to print to
 *
 * @return the extended string buffer
 ******************************************************************************/
static str_buf *
PrintModuleNames (const char *module, strstype_t kind, str_buf *buffer)
{
    DBUG_ENTER ("PrintModuleNames");

    buffer = SBUFprintf (buffer, " *   %s\n", module);

    DBUG_RETURN (buffer);
}

/** <!--********************************************************************-->
 *
 * @fn void PrintFileHeader(FILE *file);
 *
 * @brief Prints the header for the file to be generated.
 *
 *****************************************************************************/
static void
PrintFileHeader (info *arg_info)
{
    str_buf *buffer;
    char *modules;

    DBUG_ENTER ("PrintFileHeader");

    buffer = SBUFcreate (100);
    buffer = STRSfold ((strsfoldfun_p)PrintModuleNames, global.exported_modules, buffer);
    modules = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    fprintf (INFO_FILE (arg_info),
             "/*\n"
             " * C interface %s file for modules:\n"
             " *\n"
             "%s"
             " *\n"
             " * generated by sac4c %s (%s) rev %s\n"
             " */\n\n"
             "#include \"sac4c.h\"\n\n",
             (INFO_MODE (arg_info) == CCI_header) ? "header" : "implementation", modules,
             global.version_id, build_style, build_rev);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CCWfunbundle(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CCWfunbundle (node *arg_node, info *arg_info)
{
    char *safename, *safens;

    DBUG_ENTER ("CCWfunbundle");

    INFO_INBUNDLE (arg_info) = TRUE;

    DBUG_ASSERT ((FUNBUNDLE_FUNDEF (arg_node) != NULL), "empty funbundle found!");

    /*
     * first print the descriptive comment
     */
    INFO_COMMENT (arg_info) = TRUE;

    fprintf (INFO_FILE (arg_info),
             "/**************************************************************************"
             "***\n"
             " * C declaration of function %s.\n"
             " *\n"
             " * defined instances:\n"
             " *\n",
             CTIitemName (FUNBUNDLE_FUNDEF (arg_node)));

    FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), " ****************************************************"
                                   "*************************/\n"
                                   "\n");
    INFO_COMMENT (arg_info) = FALSE;

    /*
     * next the function declaration
     */
    INFO_DECL (arg_info) = TRUE;

    safename = STRreplaceSpecialCharacters (FUNBUNDLE_NAME (arg_node));
    safens = STRreplaceSpecialCharacters (NSgetName (FUNBUNDLE_NS (arg_node)));

    /*
     * add an extern if in header mode...
     */
    if (INFO_MODE (arg_info) == CCI_header) {
        fprintf (INFO_FILE (arg_info), "extern ");
    }

    fprintf (INFO_FILE (arg_info), "void %s__%s%d(", safens, safename,
             FUNBUNDLE_ARITY (arg_node));

    safens = MEMfree (safens);
    safename = MEMfree (safename);

    FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

    fprintf (INFO_FILE (arg_info), ")");

    INFO_DECL (arg_info) = FALSE;

    /*
     * add the body if in impl mode, or a ';' to end the declaration
     * otherwise
     */
    if (INFO_MODE (arg_info) == CCI_header) {
        fprintf (INFO_FILE (arg_info), ";");
    } else {
        INFO_BODY (arg_info) = TRUE;

        fprintf (INFO_FILE (arg_info), "{\n");
        fprintf (INFO_FILE (arg_info), "SAC4C_DISPATCH_BEGIN()\n");

        FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

        fprintf (INFO_FILE (arg_info), "SAC4C_DISPATCH_END()\n");
        fprintf (INFO_FILE (arg_info), "}\n");

        INFO_BODY (arg_info) = FALSE;
    }

    fprintf (INFO_FILE (arg_info), "\n\n");

    INFO_INBUNDLE (arg_info) = FALSE;

    if (FUNBUNDLE_NEXT (arg_node) != NULL) {
        FUNBUNDLE_NEXT (arg_node) = TRAVdo (FUNBUNDLE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCWfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/

static str_buf *
FunctionToComment (node *fundef, str_buf *buffer)
{
    ntype *rets, *args;
    char *retstr, *argstr;

    DBUG_ENTER ("FunctionToComment");

    rets = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
    args = TUmakeProductTypeFromArgs (FUNDEF_ARGS (fundef));
    retstr = TYtype2String (rets, FALSE, 0);
    argstr = TYtype2String (args, FALSE, 0);

    SBUFprintf (buffer, " * %s -> %s\n", argstr, retstr);

    rets = TYfreeType (rets);
    args = TYfreeType (args);
    retstr = MEMfree (retstr);
    argstr = MEMfree (argstr);

    DBUG_RETURN (buffer);
}

node *
CCWfundef (node *arg_node, info *arg_info)
{
    str_buf *buffer = NULL;
    char *str;

    DBUG_ENTER ("CCWfundef");

    if (INFO_INBUNDLE (arg_info)) {
        if (INFO_COMMENT (arg_info)) {
            buffer = SBUFcreate (255);

            if (TYisFun (FUNDEF_WRAPPERTYPE (arg_node))) {
                buffer
                  = TYfoldFunctionInstances (FUNDEF_WRAPPERTYPE (arg_node),
                                             (void *(*)(node *, void *))FunctionToComment,
                                             buffer);

            } else {
                buffer = FunctionToComment (FUNDEF_IMPL (arg_node), buffer);
            }

            str = SBUF2str (buffer);
            fprintf (INFO_FILE (arg_info), "%s *\n", str);
            str = MEMfree (str);
            buffer = SBUFfree (buffer);

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        } else if (INFO_DECL (arg_info)) {
            if (FUNDEF_RETS (arg_node) != NULL) {
                FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
            }

            if ((FUNDEF_RETS (arg_node) != NULL) && (FUNDEF_ARGS (arg_node) != NULL)) {
                fprintf (INFO_FILE (arg_info), ", ");
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            }
        } else if (INFO_BODY (arg_info)) {
            fprintf (INFO_FILE (arg_info), "SAC4C_DISPATCH_COND(");

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            }

            fprintf (INFO_FILE (arg_info), ", ");

            /*
             * TODO: call the actual function
             */
            fprintf (INFO_FILE (arg_info), "SAC4C_NOOP()");

            fprintf (INFO_FILE (arg_info), ")\n");

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        }
    } else if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCWarg(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CCWarg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ("CCWarg");

    INFO_COUNTER (arg_info)++;

    if (INFO_DECL (arg_info)) {
        fprintf (INFO_FILE (arg_info), "SACarg *arg%d", INFO_COUNTER (arg_info));
    } else if (INFO_BODY (arg_info)) {
        type = AVIS_TYPE (ARG_AVIS (arg_node));
        if (TUisArrayOfUser (type)) {
            usertype udt = TYgetUserType (TYgetScalar (type));
            udt = UTgetUnAliasedType (udt);

            fprintf (INFO_FILE (arg_info), "SAC4C_ARGMATCHES_UDT( %d, \"%s\", \"%s\")",
                     INFO_COUNTER (arg_info), NSgetName (UTgetNamespace (udt)),
                     UTgetName (udt));
        } else {
            simpletype basetype = TYgetSimpleType (TYgetScalar (type));

            fprintf (INFO_FILE (arg_info), "SAC4C_ARGMATCHES( %d, %d)",
                     INFO_COUNTER (arg_info), basetype);
        }
    }

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (INFO_FILE (arg_info), ", ");
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    INFO_COUNTER (arg_info)--;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCWret(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CCWret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CCWret");

    INFO_COUNTER (arg_info)++;

    fprintf (INFO_FILE (arg_info), "SACarg **ret%d", INFO_COUNTER (arg_info));

    if (RET_NEXT (arg_node) != NULL) {
        fprintf (INFO_FILE (arg_info), ", ");
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    INFO_COUNTER (arg_info)--;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CCWmodule(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CCWmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CCWmodule");

    /*
     * ROUND 1: create the header file
     */
    INFO_MODE (arg_info) = CCI_header;
    INFO_FILE (arg_info) = FMGRwriteOpen ("%s.h", global.outfilename);

    PrintFileHeader (arg_info);

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_FILE (arg_info) = FMGRclose (INFO_FILE (arg_info));

    /*
     * ROUND 2: create the C file
     */
    INFO_MODE (arg_info) = CCI_impl;
    INFO_FILE (arg_info) = FMGRwriteOpen ("%s.c", global.outfilename);

    PrintFileHeader (arg_info);

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_FILE (arg_info) = FMGRclose (INFO_FILE (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Create Wrapper Headers -->
 *****************************************************************************/
