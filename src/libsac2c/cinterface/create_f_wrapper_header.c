/** <!--********************************************************************-->
 *
 * @defgroup cfwh Create Fortran wrapper header
 *
 * Creates a Fortran wrapper header file from a given syntax tree.
 *
 * @ingroup cfwh
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file creater_f_wrapper_header.c
 *
 * Prefix: CFWH
 *
 *****************************************************************************/
#include "create_f_wrapper_header.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
    bool dummy;
    bool body;
};

/**
 * INFO macros
 */
#define INFO_INBUNDLE(n) ((n)->inbundle)
#define INFO_FILE(n) ((n)->file)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_COMMENT(n) ((n)->comment)
#define INFO_DECL(n) ((n)->decl)
#define INFO_DUMMY(n) ((n)->dummy)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_INBUNDLE (result) = FALSE;
    INFO_FILE (result) = NULL;
    INFO_COUNTER (result) = 0;
    INFO_COMMENT (result) = FALSE;
    INFO_DECL (result) = FALSE;
    INFO_DUMMY (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn node *CFWHdoCreateFWrapperHeader( node *syntax_tree)
 *
 *****************************************************************************/
node *
CFWHdoCreateFWrapperHeader (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_cfwh);
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
    DBUG_ENTER ();

    buffer = SBUFprintf (buffer, "!   - %s\n", module);

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

    DBUG_ENTER ();

    buffer = SBUFcreate (100);
    buffer = (str_buf *)STRSfold ((strsfoldfun_p)PrintModuleNames,
                                  global.exported_modules, buffer);
    modules = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    fprintf (INFO_FILE (arg_info),
             "!\n"
             "! Fortran interface header file for modules:\n"
             "!\n"
             "%s"
             "!\n"
             "! To make use of the Fortran interface, simply place the following at the "
             "top\n"
             "! of the Fortran program block:\n"
             "!\n"
             "!    use, intrinsic :: iso_c_binding\n"
             "!    use fwrapper\n"
             "!\n"
             "! And to compile, generate the Fortran fwrapper.mod by doing:\n"
             "!\n"
             "!    gfortran -c fwrapper.f `sac4c -fortran -ccflags MOD`\n"
             "!      where MOD is the SAC module to which the interface is bound.\n"
             "!\n"
             "! Make sure to have the fwrapper.mod file within your include path when\n"
             "! compiling the Fortran application.\n"
             "!\n"
             "! NOTE: this requires the use of at least Fortran 2003!\n"
             "!\n"
             "! generated by sac4c %s (%s)\n"
             "!\n"
             "      module fwrapper\n"
             "        use, intrinsic :: iso_c_binding\n"
             "        implicit none\n\n"
             "        interface\n",
             modules, global.version_id, build_style);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void PrintFileFooter(FILE *file);
 *
 * @brief Prints the footer for the file to be generated.
 *
 *****************************************************************************/
static void
PrintFileFooter (info *arg_info)
{
    str_buf *buffer;
    char *modules;

    DBUG_ENTER ();

    buffer = SBUFcreate (100);
    buffer = (str_buf *)STRSfold ((strsfoldfun_p)PrintModuleNames,
                                  global.exported_modules, buffer);
    modules = SBUF2str (buffer);
    buffer = SBUFfree (buffer);

    fprintf (INFO_FILE (arg_info),
             "\n          include 'sacinterface.f' ! SAC Runtime Functions !\n\n"
             "        end interface\n"
             "      end module fwrapper\n");

    DBUG_RETURN ();
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
 * @fn node *CFWHfunbundle(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CFWHfunbundle (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_INBUNDLE (arg_info) = TRUE;

    DBUG_ASSERT (FUNBUNDLE_FUNDEF (arg_node) != NULL, "empty funbundle found!");

    /* we only use non-{x,s}t funbundle here and manually do a dispatch between
       the versions to call */
    if (!FUNBUNDLE_ISXTBUNDLE (arg_node) && !FUNBUNDLE_ISSTBUNDLE (arg_node)) {
        /*
         * first print the descriptive comment
         */
        INFO_COMMENT (arg_info) = TRUE;

        fprintf (INFO_FILE (arg_info),
                 "!\n"
                 "! Fortran declaration of function %s.\n"
                 "!\n"
                 "! defined instances:\n"
                 "!\n",
                 CTIitemName (FUNBUNDLE_FUNDEF (arg_node)));

        FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

        fprintf (INFO_FILE (arg_info), "!\n");
        INFO_COMMENT (arg_info) = FALSE;

        /*
         * next the subroutine dummy valus
         */
        INFO_DUMMY (arg_info) = TRUE;

        fprintf (INFO_FILE (arg_info),
                 "          subroutine %s\n"
                 "     &        (",
                 CTIitemNameDivider (FUNBUNDLE_FUNDEF (arg_node), "_"));

        FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

        fprintf (INFO_FILE (arg_info),
                 ")\n"
                 "     &        bind(c, name = '%s')\n"
                 "            import\n",
                 FUNBUNDLE_EXTNAME (arg_node));

        INFO_DUMMY (arg_info) = FALSE;

        /*
         * next the subroutine declarations
         */
        INFO_DECL (arg_info) = TRUE;

        //     if ( global.xtmode) {
        //       fprintf( INFO_FILE( arg_info), "SAC_XT_RESOURCE_ARG()");
        //       fprintf( INFO_FILE( arg_info),
        //                ((FUNDEF_RETS( FUNBUNDLE_FUNDEF( arg_node)) != NULL) ||
        //                 (FUNDEF_ARGS( FUNBUNDLE_FUNDEF( arg_node)) != NULL))
        //                ? ", " : "");
        //     }
        FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

        fprintf (INFO_FILE (arg_info), "\n          end subroutine %s\n",
                 CTIitemNameDivider (FUNBUNDLE_FUNDEF (arg_node), "_"));

        INFO_DECL (arg_info) = FALSE;
        INFO_INBUNDLE (arg_info) = FALSE;
    }

    if (FUNBUNDLE_NEXT (arg_node) != NULL) {
        FUNBUNDLE_NEXT (arg_node) = TRAVdo (FUNBUNDLE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/

static str_buf *
FunctionToComment (node *fundef, str_buf *buffer)
{
    ntype *rets, *args;
    char *retstr, *argstr;

    DBUG_ENTER ();

    rets = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
    args = TUmakeProductTypeFromArgs (FUNDEF_ARGS (fundef));
    retstr = TYtype2String (rets, FALSE, 0);
    argstr = TYtype2String (args, FALSE, 0);

    SBUFprintf (buffer, "!  %s -> %s\n", argstr, retstr);

    rets = TYfreeType (rets);
    args = TYfreeType (args);
    retstr = MEMfree (retstr);
    argstr = MEMfree (argstr);

    DBUG_RETURN (buffer);
}

node *
CFWHfundef (node *arg_node, info *arg_info)
{
    str_buf *buffer = NULL;
    char *str;

    DBUG_ENTER ();

    if (INFO_INBUNDLE (arg_info)) {
        if (INFO_COMMENT (arg_info)) {
            buffer = SBUFcreate (255);

            if (TYisFun (FUNDEF_WRAPPERTYPE (arg_node))) {
                buffer = (str_buf *)
                  TYfoldFunctionInstances (FUNDEF_WRAPPERTYPE (arg_node),
                                           (void *(*)(node *, void *))FunctionToComment,
                                           buffer);

            } else {
                buffer = FunctionToComment (FUNDEF_IMPL (arg_node), buffer);
            }

            str = SBUF2str (buffer);
            fprintf (INFO_FILE (arg_info), "%s!\n", str);
            str = MEMfree (str);
            buffer = SBUFfree (buffer);

            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        } else if (INFO_DECL (arg_info) || INFO_DUMMY (arg_info)) {
            if (FUNDEF_RETS (arg_node) != NULL) {
                FUNDEF_RETS (arg_node) = TRAVdo (FUNDEF_RETS (arg_node), arg_info);
            }

            if ((FUNDEF_RETS (arg_node) != NULL) && (FUNDEF_ARGS (arg_node) != NULL)) {
                fprintf (INFO_FILE (arg_info), INFO_DECL (arg_info) ? "\n" : ", ");
            }

            if (FUNDEF_ARGS (arg_node) != NULL) {
                FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
            }
        }
    } else if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHarg(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CFWHarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info)++;

    if (INFO_DECL (arg_info)) {
        fprintf (INFO_FILE (arg_info),
                 "            type(c_ptr), value, intent(in) :: arg%d",
                 INFO_COUNTER (arg_info));

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (INFO_FILE (arg_info), "\n");
        }
    } else if (INFO_DUMMY (arg_info)) {
        fprintf (INFO_FILE (arg_info), "arg%d", INFO_COUNTER (arg_info));

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (INFO_FILE (arg_info), ", ");
        }
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    INFO_COUNTER (arg_info)--;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHret(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CFWHret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_COUNTER (arg_info)++;

    if (INFO_DECL (arg_info)) {
        fprintf (INFO_FILE (arg_info), "            type(c_ptr), intent(out) :: ret%d",
                 INFO_COUNTER (arg_info));

        if (RET_NEXT (arg_node) != NULL) {
            fprintf (INFO_FILE (arg_info), "\n");
        }
    } else if (INFO_DUMMY (arg_info)) {
        fprintf (INFO_FILE (arg_info), "ret%d", INFO_COUNTER (arg_info));

        if (RET_NEXT (arg_node) != NULL) {
            fprintf (INFO_FILE (arg_info), ", ");
        }
    }

    if (RET_NEXT (arg_node) != NULL) {
        RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
    }

    INFO_COUNTER (arg_info)--;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHtypedef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CFWHtypedef (node *arg_node, info *arg_info)
{
    usertype udt;

    DBUG_ENTER ();

    /*
     * we filter out prelude types
     */
    if (!NSequals (TYPEDEF_NS (arg_node), NSgetNamespace (global.preludename))) {
        udt = UTfindUserType (TYPEDEF_NAME (arg_node), TYPEDEF_NS (arg_node));

        DBUG_ASSERT (udt != UT_NOT_DEFINED, "cannot find udt!");

        udt = UTgetUnAliasedType (udt);

        fprintf (INFO_FILE (arg_info), "\n#define SACTYPE_%s_%s %d",
                 NSgetName (TYPEDEF_NS (arg_node)), TYPEDEF_NAME (arg_node),
                 udt + global.sac4c_udt_offset);
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    } else {
        fprintf (INFO_FILE (arg_info), "\n\n");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHmodule(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CFWHmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * 1) create the file header
     */
    INFO_FILE (arg_info)
      = FMGRwriteOpen ("%s/%s.f", STRonNull (".", global.inc_dirname), "fwrapper");

    PrintFileHeader (arg_info);

    /*
     * 2) print udt defines
     */
    if (MODULE_TYPES (arg_node) != NULL) {
        MODULE_TYPES (arg_node) = TRAVdo (MODULE_TYPES (arg_node), arg_info);
    }

    /*
     * 3) print function headers
     */
    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    /*
     * 4) print the file footer
     */

    PrintFileFooter (arg_info);

    /*
     * 5) close files
     */
    INFO_FILE (arg_info) = FMGRclose (INFO_FILE (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Create Wrapper Headers -->
 *****************************************************************************/

#undef DBUG_PREFIX
