/** <!--********************************************************************-->
 *
 * @defgroup cfwh Create Foreign-function wrapper header
 *
 * Creates a wrapper header file from a given syntax tree for a specified
 * target language, currently supported:
 *
 *  - C
 *  - Fortran
 *
 * @ingroup cfwh
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file create_f_wrapper_header.c
 *
 * Prefix: CFWH
 *
 *****************************************************************************/
#include "create_f_wrapper_header.h"

#define DBUG_PREFIX "CFWH"
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
 * @name LANGS enum
 * @{
 *
 * This enum contains the currently supported languages for this interface and
 * is used in various calls to generate the correct code.
 *
 *****************************************************************************/
typedef enum LANGS { CLANG = 0, FORTRAN = 1 } langs;

/** <!--********************************************************************-->
 *
 * @name HOLDER structure
 * @{
 *
 * This structure is used to pass both a str_buf object as well as a comment
 * symbol to static functions - saves us having to overhaul the TY-related
 * stuff.
 *
 *****************************************************************************/
struct HOLDER {
    char *com_sym;
    str_buf *buffer;
};

/**
 * Macros to access the HOLDER structure fields
 */
#define HOLDER_COMSYM(n) ((n)->com_sym)
#define HOLDER_BUFFER(n) ((n)->buffer)

/**
 * static functions to malloc and free HOLDER structure
 */
static holder *
MakeHolder (char *com_sym)
{
    holder *result;

    DBUG_ENTER ();

    result = (holder *)MEMmalloc (sizeof (holder));

    HOLDER_BUFFER (result) = NULL;
    HOLDER_COMSYM (result) = com_sym;

    DBUG_RETURN (result);
}

static holder *
FreeHolder (holder *holdr)
{
    DBUG_ENTER ();

    holdr = MEMfree (holdr);

    DBUG_RETURN (holdr);
}

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool inbundle;
    FILE *file;
    langs lang;
    char *lang_com_sym;
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
#define INFO_LANG(n) ((n)->lang)
#define INFO_LANGSYM(n) ((n)->lang_com_sym)

static info *
MakeInfo (langs type, char *lang_com_sym)
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
    INFO_LANG (result) = type;
    INFO_LANGSYM (result) = lang_com_sym;

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

    DBUG_PRINT ("Generating Fortran-interface...");

    info = MakeInfo (FORTRAN, "!");

    TRAVpush (TR_cfwh);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHdoCreateCWrapperHeader( node *syntax_tree)
 *
 *****************************************************************************/
node *
CFWHdoCreateCWrapperHeader (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Generating C-interface...");

    info = MakeInfo (CLANG, " *");

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
 * @param holder a container that holds both a comment symbol and a
 *               str_buf object
 *
 * @return the extended string buffer
 ******************************************************************************/
static holder *
PrintModuleNames (const char *module, strstype_t kind, holder *holdr)
{
    DBUG_ENTER ();

    HOLDER_BUFFER (holdr)
      = SBUFprintf (HOLDER_BUFFER (holdr), "%s   - %s\n", HOLDER_COMSYM (holdr), module);

    DBUG_RETURN (holdr);
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
    holder *holdr;
    char *modules;
    char *header;

    holdr = MakeHolder (INFO_LANGSYM (arg_info));

    DBUG_ENTER ();

    HOLDER_BUFFER (holdr) = SBUFcreate (100);
    holdr = (holder *)STRSfold ((strsfoldfun_p)PrintModuleNames, global.exported_modules,
                                holdr);
    modules = SBUF2str (HOLDER_BUFFER (holdr));
    HOLDER_BUFFER (holdr) = SBUFfree (HOLDER_BUFFER (holdr));
    holdr = FreeHolder (holdr);

    switch (INFO_LANG (arg_info)) {
    case CLANG:
        header = "/*\n"
                 " * C interface header file for module(s):\n"
                 " *\n"
                 "%s"
                 " *\n"
                 " * generated by sac4c %s (%s)\n"
                 " */\n\n"
                 "#include \"sacinterface.h\"\n\n";
        break;
    case FORTRAN:
        header
          = "!\n"
            "! Fortran interface header file for modules:\n"
            "!\n"
            "%s"
            "!\n"
            "! To make use of the Fortran interface, place the following at the top of\n"
            "! the Fortran `program` block:\n"
            "!\n"
            "!    use, intrinsic :: iso_c_binding\n"
            "!    use fwrapper\n"
            "!\n"
            "! And to compile, generate the Fortran fwrapper.mod by doing:\n"
            "!\n"
            "!    gfortran -c fwrapper.f `sac4c -fortran -ccflags MOD`\n"
            "!      where MOD is the SAC module to which the interface is bound.\n"
            "!\n"
            "! Make sure to have the fwrapper.mod as well as the cwrapper.h files "
            "within\n"
            "! your include path when compiling the Fortran application.\n"
            "!\n"
            "! NOTE: this requires the use of at least Fortran 2003!\n"
            "!\n"
            "! generated by sac4c %s (%s)\n"
            "!\n"
            "      module fwrapper\n"
            "        use, intrinsic :: iso_c_binding\n"
            "        implicit none\n\n"
            "        interface\n";
        break;
    default:
        DBUG_UNREACHABLE ("Unknown header comment specified -> LANG: %d.\n",
                          INFO_LANG (arg_info));
    }

    /* Print the header comment */
    fprintf (INFO_FILE (arg_info), header, modules, global.version_id, build_style);

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
    DBUG_ENTER ();

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

        switch (INFO_LANG (arg_info)) {
        case CLANG:
            fprintf (INFO_FILE (arg_info),
                     "/******************************************************************"
                     "***********\n"
                     " * C declaration of function %s.\n"
                     " *\n"
                     " * defined instances:\n"
                     " *\n",
                     CTIitemName (FUNBUNDLE_FUNDEF (arg_node)));

            FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

            fprintf (INFO_FILE (arg_info), " ********************************************"
                                           "*********************************/\n"
                                           "\n");
            break;
        case FORTRAN:
            fprintf (INFO_FILE (arg_info),
                     "!\n"
                     "! Fortran declaration of function %s.\n"
                     "!\n"
                     "! defined instances:\n"
                     "!\n",
                     CTIitemName (FUNBUNDLE_FUNDEF (arg_node)));

            FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

            fprintf (INFO_FILE (arg_info), "!\n");
            break;
        default:
            DBUG_UNREACHABLE (
              "Unknown Foreign-function interface used, uses type number %d.\n",
              INFO_LANG (arg_info));
        }

        INFO_COMMENT (arg_info) = FALSE;

        /*
         * next the subroutine dummy valus
         */
        if (INFO_LANG (arg_info) == FORTRAN) {
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
        }

        /*
         * next the subroutine declarations
         */
        INFO_DECL (arg_info) = TRUE;

        switch (INFO_LANG (arg_info)) {
        case CLANG:
            fprintf (INFO_FILE (arg_info), "extern void %s(",
                     FUNBUNDLE_EXTNAME (arg_node));

            //         if ( global.xtmode) {
            //           fprintf( INFO_FILE( arg_info), "SAC_XT_RESOURCE_ARG()");
            //           fprintf( INFO_FILE( arg_info),
            //                    ((FUNDEF_RETS( FUNBUNDLE_FUNDEF( arg_node)) != NULL) ||
            //                     (FUNDEF_ARGS( FUNBUNDLE_FUNDEF( arg_node)) != NULL))
            //                    ? ", " : "");
            //         }
            FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

            fprintf (INFO_FILE (arg_info), ");\n\n");
            break;
        case FORTRAN:
            FUNBUNDLE_FUNDEF (arg_node) = TRAVdo (FUNBUNDLE_FUNDEF (arg_node), arg_info);

            fprintf (INFO_FILE (arg_info), "\n          end subroutine %s\n",
                     CTIitemNameDivider (FUNBUNDLE_FUNDEF (arg_node), "_"));
            break;
        default:
            DBUG_UNREACHABLE (
              "Unknown Foreign-function interface used, uses type number %d.\n",
              INFO_LANG (arg_info));
        }

        INFO_DECL (arg_info) = FALSE;
        INFO_INBUNDLE (arg_info) = FALSE;
    }

    FUNBUNDLE_NEXT (arg_node) = TRAVopt(FUNBUNDLE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CFWHfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static holder *
FunctionToComment (node *fundef, holder *holdr)
{
    ntype *rets, *args;
    char *retstr, *argstr;

    DBUG_ENTER ();

    rets = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
    args = TUmakeProductTypeFromArgs (FUNDEF_ARGS (fundef));
    retstr = TYtype2String (rets, FALSE, 0);
    argstr = TYtype2String (args, FALSE, 0);

    SBUFprintf (HOLDER_BUFFER (holdr), "%s  %s -> %s\n", HOLDER_COMSYM (holdr), argstr,
                retstr);

    rets = TYfreeType (rets);
    args = TYfreeType (args);
    retstr = MEMfree (retstr);
    argstr = MEMfree (argstr);

    DBUG_RETURN (holdr);
}

node *
CFWHfundef (node *arg_node, info *arg_info)
{
    holder *holdr;
    char *str;

    holdr = MakeHolder (INFO_LANGSYM (arg_info));

    DBUG_ENTER ();

    if (INFO_INBUNDLE (arg_info)) {
        if (INFO_COMMENT (arg_info)) {
            HOLDER_BUFFER (holdr) = SBUFcreate (255);

            if (TYisFun (FUNDEF_WRAPPERTYPE (arg_node))) {
                holdr = (holder *)
                  TYfoldFunctionInstances (FUNDEF_WRAPPERTYPE (arg_node),
                                           (void *(*)(node *, void *))FunctionToComment,
                                           holdr);

            } else {
                holdr = FunctionToComment (FUNDEF_IMPL (arg_node), holdr);
            }

            str = SBUF2str (HOLDER_BUFFER (holdr));
            HOLDER_BUFFER (holdr) = SBUFfree (HOLDER_BUFFER (holdr));
            fprintf (INFO_FILE (arg_info), "%s%s\n", str, INFO_LANGSYM (arg_info));
            str = MEMfree (str);

            FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
        } else if (INFO_DECL (arg_info) || INFO_DUMMY (arg_info)) {
            FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);

            if ((FUNDEF_RETS (arg_node) != NULL) && (FUNDEF_ARGS (arg_node) != NULL)) {
                switch (INFO_LANG (arg_info)) {
                case CLANG:
                    fprintf (INFO_FILE (arg_info), ", ");
                    break;
                case FORTRAN:
                    fprintf (INFO_FILE (arg_info), INFO_DECL (arg_info) ? "\n" : ", ");
                    break;
                default:
                    DBUG_UNREACHABLE ("Unknown language type -> %d.\n",
                                      INFO_LANG (arg_info));
                }
            }

            FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);
        }
    } else {
        FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);
    }

    holdr = FreeHolder (holdr);

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

    switch (INFO_LANG (arg_info)) {
    case CLANG:
        if (INFO_DECL (arg_info)) {
            fprintf (INFO_FILE (arg_info), "SACarg *arg%d", INFO_COUNTER (arg_info));

            if (ARG_NEXT (arg_node) != NULL) {
                fprintf (INFO_FILE (arg_info), ", ");
            }
        }
        break;
    case FORTRAN:
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
        break;
    default:
        DBUG_UNREACHABLE (
          "Unknown Foreign-function interface used, uses type number %d.\n",
          INFO_LANG (arg_info));
    }

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

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

    switch (INFO_LANG (arg_info)) {
    case CLANG:
        fprintf (INFO_FILE (arg_info), "SACarg **ret%d", INFO_COUNTER (arg_info));

        if (RET_NEXT (arg_node) != NULL) {
            fprintf (INFO_FILE (arg_info), ", ");
            RET_NEXT (arg_node) = TRAVdo (RET_NEXT (arg_node), arg_info);
        }
        break;
    case FORTRAN:
        if (INFO_DECL (arg_info)) {
            fprintf (INFO_FILE (arg_info),
                     "            type(c_ptr), intent(out) :: ret%d",
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
        RET_NEXT (arg_node) = TRAVopt(RET_NEXT (arg_node), arg_info);
        break;
    default:
        DBUG_UNREACHABLE (
          "Unknown Foreign-function interface used, uses type number %d.\n",
          INFO_LANG (arg_info));
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
    switch (INFO_LANG (arg_info)) {
    case CLANG:
        INFO_FILE (arg_info)
          = FMGRwriteOpen ("%s/%s.h", STRonNull (".", global.inc_dirname),
                           global.outfilename);
        break;
    case FORTRAN:
        INFO_FILE (arg_info)
          = FMGRwriteOpen ("%s/%s.f", STRonNull (".", global.inc_dirname), "fwrapper");
        break;
    }

    PrintFileHeader (arg_info);

    /*
     * 2) print udt defines
     */
    MODULE_TYPES (arg_node) = TRAVopt(MODULE_TYPES (arg_node), arg_info);

    /*
     * 3) print function headers
     */
    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    /*
     * 4) print the file footer
     */
    if (INFO_LANG (arg_info) == FORTRAN) {
        PrintFileFooter (arg_info);
    }

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
