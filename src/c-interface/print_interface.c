/*
 * $Log$
 * Revision 1.17  2000/08/01 13:25:24  nmw
 * startup-code generation modified to handle PHM in c-library
 *
 * Revision 1.16  2000/07/28 14:47:07  nmw
 * handling of void functions added
 * handling of T_users types added
 *
 * Revision 1.15  2000/07/24 15:00:37  nmw
 * refcount check in cwrapper added
 * generation of separate c-files with object initflags added
 * code beautifying
 *
 * Revision 1.14  2000/07/20 12:09:49  nmw
 * debug print generated code removed
 *
 * Revision 1.13  2000/07/20 11:40:15  nmw
 * wrapperfunctions check for initialized module and register themselves
 * at the free_interface_hanlder to be freed at cleanup
 *
 * Revision 1.12  2000/07/19 16:44:21  nmw
 * made a variable a bit more static...
 *
 * Revision 1.11  2000/07/19 08:32:33  nmw
 * link order in comment adjusted
 *
 * Revision 1.10  2000/07/13 15:34:09  nmw
 * add comments in generated headerfile
 *
 * Revision 1.9  2000/07/13 14:52:39  nmw
 * handling for global objects and startup code generation added
 *
 * Revision 1.8  2000/07/12 15:52:35  nmw
 * add comment which libraries to link with
 *
 * Revision 1.7  2000/07/12 10:09:08  nmw
 * RCS-header added
 *
 * Revision 1.6  2000/07/12 09:24:46  nmw
 * traversal of returntypes modified. now using TYPE_STATUS()
 *
 * Revision 1.5  2000/07/07 15:34:56  nmw
 * beautyfing of generated code
 *
 * Revision 1.4  2000/07/06 15:56:10  nmw
 * debugging in generated cwrapper code
 *
 * Revision 1.3  2000/07/05 15:33:25  nmw
 * filenames modified according to cccalls.c
 *
 * Revision 1.2  2000/07/05 12:54:30  nmw
 * filenames for printed includes adjusted
 *
 * Revision 1.1  2000/07/05 11:39:33  nmw
 * Initial revision
 *
 *
 * The c-interface allows you to generate a c-library from your SAC-module.
 * at this time, only functions with a fixed shape and a c-compatible datatype
 * can be exported.
 * When you compile your SAC-module with sac2c and the option '-genlib c'
 * you get an headerfile (myMod.h) with comments to all exported functions
 * (the shapes they accept and return) and list of library files to link with.
 * The libary is named libmyMod.a.
 *
 * Involved compiler parts for this c-interface:
 * map_cwrapper.[ch]: traversing the AST, looking for overloaded SAC-functions
 *                    and building up a set of wrapper functions, witch simulate
 *                    this overloading.
 *
 * print_interface.[ch]: generating the c-code for the module headerfile and
 *                    the wrapper functions (calles cwrapper.c). The generated
 *                    code uses macros, defined in runtime/sac_cwrapper.h
 *
 * libsac/sac_arg.c, runtime/sac_arg.h: implements the abstract datatype used
 *                    be the c-interface functions and the wrapper functions
 *                    these functions are included in the sac-runtime-system
 *
 * libsac/cinterface.c, runtime/sac_cinterface.c: implements the interface
 *                    functions used by the c-programmer for converting c-datatypes
 *                    to sac-datatypes and vice versa
 *
 * modules/cccall.c:  compiles and generates the c-library.
 *
 * known limitations:
 *   no user defined types at the interface level (but you might use them internally)
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "print_interface.h"
#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "resource.h"
#include "gen_startup_code.h"

/* interface identifier */
#define SACARGTYPE "SAC_arg"
#define SACPREFIX "SAC_"

#define PIH_PRINT_COMMENT 1
#define PIH_PRINT_PROTOTYPE 2

#define PIW_CREATE_RETTYPES 1
#define PIW_SWITCH_ARGS 2
#define PIW_CALL_ARGS 3
#define PIW_CALL_RESULTS 4
#define PIW_CALL_RETPOS 5
#define PIW_REFCOUNT_ARGS 6
#define PIW_REFCOUNT_RESULTS 7

/* basic c-type-strings for sac types */
#define TYP_IFpr_str(str) str
static char *ctype_string[] = {
#include "type_info.mac"
};

/* function for only local usage */
static types *TravTH (types *arg_type, node *arg_info);
static types *TravTW (types *arg_type, node *arg_info);
static types *PIHtypes (types *arg_type, node *arg_info);
static types *PIWtypes (types *arg_type, node *arg_info);
static node *PIHcwrapperPrototype (node *wrapper, node *arg_info);
static node *PIWfundefSwitch (node *arg_node, node *arg_info);
static node *PIWfundefCall (node *arg_node, node *arg_info);
static node *PIWfundefRefcounting (node *arg_node, node *arg_info);
static strings *PrintDepEntry (deps *depends, statustype stat, strings *done);
static void PIWModuleInitFlag (char *modname);
static void PIWModuleInitFunction (char *modname);
static void PIWModuleFreeFunction (char *modname);
static void PrintInternalRuntimeInit (node *arg_node);

/******************************************************************************
 *
 * function:
 *   node *PIHmodule(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses only in wrappers of module, generate c headerfile for module
 *
 ******************************************************************************/

node *
PIHmodul (node *arg_node, node *arg_info)
{
    FILE *old_outfile;
    strings *done;

    DBUG_ENTER ("PIHmodul");

    old_outfile = outfile; /* save, might be in use */

    /* open <module>.h in tmpdir for writing wrapper header*/
    outfile = WriteOpen ("%s/%s.h", tmp_dirname, MODUL_NAME (arg_node));
    fprintf (outfile, "/* Interface SAC <-> C for %s \n", MODUL_NAME (arg_node));
    fprintf (outfile, " * use this %s.h file with lib%s.a\n", MODUL_NAME (arg_node),
             MODUL_NAME (arg_node));

    /*Generate list of modules to link with */
    fprintf (outfile,
             " * For compiling with this module you need to set up\n"
             " * your c compiler with searchpaths to:\n"
             " * -L$SACBASE/runtime (for sac runtime-system)\n"
             " * -L.                (or where you place 'lib%s.a')\n"
             " *\n"
             " * -I$SACBASE/runtime (for include of 'sac_cinterface.h')\n"
             " *\n"
             " * when compiling your code add the following files to link with:\n"
             " * -l%s\n",
             MODUL_NAME (arg_node), MODUL_NAME (arg_node));
    done = PrintDepEntry (dependencies, ST_external, NULL);
    done = PrintDepEntry (dependencies, ST_system, NULL);

    if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) {
        if (optimize & OPT_PHM) {
            if (runtimecheck & RUNTIMECHECK_HEAP) {
                fprintf (outfile, " * -lsac_heapmgr_mt_diag\n");
            } else {
                fprintf (outfile, " * -lsac_heapmgr_mt\n");
            }
        }
    } else {
        if (optimize & OPT_PHM) {
            if (runtimecheck & RUNTIMECHECK_HEAP) {
                fprintf (outfile, " * -lsac_heapmgr_diag\n");
            } else {
                fprintf (outfile, " * -lsac_heapmgr\n");
            }
        }
    }

    fprintf (outfile, " * -lsac\n"
                      " *\n */\n\n");

    fprintf (outfile, "#include \"sac_cinterface.h\"\n");

    if (MODUL_CWRAPPER (arg_node) != NULL) {
        /* traverse list of wrappers */
        MODUL_CWRAPPER (arg_node) = Trav (MODUL_CWRAPPER (arg_node), arg_info);
    }

    fprintf (outfile,
             "\n/* generated headerfile, please do not modify function prototypes */\n");
    fclose (outfile);

    outfile = old_outfile; /* restore old filehandle */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIHcwrapper(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses and print cwrapper interface header and protoypes
 *
 ******************************************************************************/

node *
PIHcwrapper (node *arg_node, node *arg_info)
{
    nodelist *funlist;

    DBUG_ENTER ("PIHcwrapper");

    /* print general comment header for this wrapper */
    fprintf (outfile,
             "\n"
             "/* function %s\n"
             " * defined in module %s\n"
             " * accepts arguments as follows:\n",
             CWRAPPER_NAME (arg_node), CWRAPPER_MOD (arg_node));

    /* print accepted shapes for each spezialized function */
    funlist = CWRAPPER_FUNS (arg_node);
    DBUG_ASSERT (funlist != NULL, ("PIHcwrapper: wrapper node without fundef\n"));

    while (funlist != NULL) {
        /* go for all fundefs in nodelist */
        INFO_PIH_FLAG (arg_info) = PIH_PRINT_COMMENT;

        NODELIST_NODE (funlist) = Trav (NODELIST_NODE (funlist), arg_info);

        funlist = NODELIST_NEXT (funlist);
    }

    fprintf (outfile, " */\n");
    fprintf (outfile, "extern ");
    arg_node = PIHcwrapperPrototype (arg_node, arg_info);
    fprintf (outfile, ";\n\n");

    if (CWRAPPER_NEXT (arg_node) != NULL) {
        CWRAPPER_NEXT (arg_node) = Trav (CWRAPPER_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIHfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints Interface Header for Wrapper-function c->SAC
 *   also prints the return types
 * flag PIH_PRINT_COMMENT: prints comment line for specialized fundef
 *
 *
 ******************************************************************************/

node *
PIHfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIHfundef");

    if (INFO_PIH_FLAG (arg_info) == PIH_PRINT_COMMENT) {
        /* print comment line in headerfile */
        fprintf (outfile, " * ");

        /* first print accepted arguments */
        INFO_PIH_COUNTER (arg_info) = 0;
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        } else {
            fprintf (outfile, " () ");
        }

        fprintf (outfile, " -> ");

        INFO_PIH_COUNTER (arg_info) = 0;
        /* then print resulting types */
        if (FUNDEF_TYPES (arg_node) != NULL) {
            FUNDEF_TYPES (arg_node) = TravTH (FUNDEF_TYPES (arg_node), arg_info);
        } else {
            fprintf (outfile, "void");
        }
        fprintf (outfile, "\n");
    } else {
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIHarg(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints function arguments for Interface Header for Wrapper-function c->SAC
 *
 *
 ******************************************************************************/

node *
PIHarg (node *arg_node, node *arg_info)
{
    char *typestring;

    DBUG_ENTER ("PIHArg");

    switch (INFO_PIH_FLAG (arg_info)) {
    case PIH_PRINT_COMMENT:
        /* print internal accepted types of argument */
        typestring = Type2String (ARG_TYPE (arg_node), 0);

        fprintf (outfile, "%s %s", typestring, ARG_NAME (arg_node));
        FREE (typestring);
        break;

    default:
        SYSERROR (("undefined case in PIWtypes!\n"));
    }

    /* traverse to next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ", ");
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   types *PIHtypes(types *arg_type, node *arg_info)
 *
 * description:
 *   Prints results of functions
 *
 * remark: simulation of the syntax of the Trav technology
 *
 ******************************************************************************/

static types *
PIHtypes (types *arg_type, node *arg_info)
{
    char *typestring;

    DBUG_ENTER ("PIHtypes");

    INFO_PIH_COUNTER (arg_info) = INFO_PIH_COUNTER (arg_info) + 1;

    switch (INFO_PIH_FLAG (arg_info)) {
    case PIH_PRINT_COMMENT:
        if (TYPES_BASETYPE (arg_type) == T_void) {
            /* if void, do not print varname */
            fprintf (outfile, "void");
        } else {
            typestring = Type2String (arg_type, 0 | 4);

            fprintf (outfile, "%s out%d", typestring, INFO_PIH_COUNTER (arg_info));
            FREE (typestring);
        }

        if (TYPES_NEXT (arg_type) != NULL) {
            fprintf (outfile, ", ");
        }
        break;

    default:
        SYSERROR (("undefined case in PIWtypes!\n"));
    }

    /* traverse to next returntype */
    if (TYPES_NEXT (arg_type) != NULL) {
        TYPES_NEXT (arg_type) = TravTH (TYPES_NEXT (arg_type), arg_info);
    }

    DBUG_RETURN (arg_type);
}

/******************************************************************************
 *
 * function:
 *   node *PIHcwrapperPrototype(node *wrapper, node *arg_info)
 *
 * description:
 *   Prints one cwrapper prototype with SAC_arg parameters and returntypes
 *
 ******************************************************************************/

static node *
PIHcwrapperPrototype (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("PIHcwrapperPrototype");

    /* print declaration */
    fprintf (outfile, "int %s%s_%s_%d_%d(", SACPREFIX, CWRAPPER_MOD (arg_node),
             CWRAPPER_NAME (arg_node), CWRAPPER_RESCOUNT (arg_node),
             CWRAPPER_ARGCOUNT (arg_node));

    /* print return reference parameters */
    for (i = 1; i <= CWRAPPER_RESCOUNT (arg_node); i++) {
        fprintf (outfile, "%s *out%d", SACARGTYPE, i);
        if (i < CWRAPPER_RESCOUNT (arg_info) || CWRAPPER_ARGCOUNT (arg_node) > 0) {
            fprintf (outfile, ", ");
        }
    }

    /* print arguments */
    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile, "%s in%d", SACARGTYPE, i);
        if (i < CWRAPPER_ARGCOUNT (arg_node)) {
            fprintf (outfile, ", ");
        }
    }

    /* print End of prototype */
    fprintf (outfile, ")");
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWmodul(node *arg_node, node *arg_info)
 *
 * description:
 *   generated wrapper implementation file, traverses all wrappers
 *
 ******************************************************************************/

node *
PIWmodul (node *arg_node, node *arg_info)
{
    FILE *old_outfile;

    DBUG_ENTER ("PIWmodul");

    PrintInternalRuntimeInit (arg_node);

    old_outfile = outfile; /* save, might be in use */

    /* open <module>_wrapper.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/cwrapper.c", tmp_dirname);
    fprintf (outfile, "/* Interface SAC <-> C for %s\n", MODUL_NAME (arg_node));
    fprintf (outfile,
             " * this file is only used when compiling"
             " the c-library lib%s.a */\n\n",
             modulename);

    /* general preload for codefile */
    fprintf (outfile, "/* startup functions and global code */\n");

    /* declarations for external SAC functions  */
    fprintf (outfile,
             "#include \"header.h\"\n"
             "#include \"sac.h\"\n"
             "#include \"sac_cwrapper.h\"\n"
             "#include \"sac_cinterface.h\"\n"
             "#include \"sac_free_interface_handler.h\"\n\n"
             "static void SAC_Free%s();"
             "\n",
             MODUL_NAME (arg_node));

    PIWModuleInitFlag (MODUL_NAME (arg_node));
    PIWModuleInitFunction (MODUL_NAME (arg_node));
    PIWModuleFreeFunction (MODUL_NAME (arg_node));

    if (MODUL_CWRAPPER (arg_node) != NULL) {
        /* traverse list of wrappers */
        MODUL_CWRAPPER (arg_node) = Trav (MODUL_CWRAPPER (arg_node), arg_info);
    }

    fprintf (outfile, "/* generated codefile, please do not modify */\n");
    fclose (outfile);

    /* generate init flags for all global objects */
    if (MODUL_OBJS (arg_node) != NULL) {
        INFO_PIW_COUNTER (arg_info) = 0;
        arg_node = Trav (MODUL_OBJS (arg_node), arg_info);

        /* set global var for object counting - needed in cccall.c */
        object_counter = INFO_PIW_COUNTER (arg_info);
    }

    outfile = old_outfile; /* restore old filehandle */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWcwrapper(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses and print cwrapper interface implementation
 *
 ******************************************************************************/

node *
PIWcwrapper (node *arg_node, node *arg_info)
{
    nodelist *funlist;
    int i;

    DBUG_ENTER ("PIWcwrapper");

    fprintf (outfile, "\n\n");
    /* print standard function prototype for wrapper */
    arg_node = PIHcwrapperPrototype (arg_node, arg_info);
    fprintf (outfile, "\n{\n");

    /* print check for module initialization */
    fprintf (outfile, "/* check for proper module initialization */\n");
    fprintf (outfile, "SAC_IW_CHECK_MODINIT( SAC_Initflag%s , SAC_Init%s );\n\n",
             modulename, modulename);

    /* print checks for refcounts
     * decrement refcount to check rc if arg is used more than one time
     */
    fprintf (outfile, "/* refcount checks for arguments */\n");
    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile, "SAC_IW_CHECKDEC_RC( in%d );\n", i);
    }
    /* restore original refcount */
    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile, "SAC_IW_INC_RC( in%d );\n", i);
    }

    /* print case switch for specialized functions */
    fprintf (outfile, "/* case switch for specialized functions */\n");
    funlist = CWRAPPER_FUNS (arg_node);
    DBUG_ASSERT (funlist != NULL, "PIWcwrapper: wrapper without fundef\n");

    while (funlist != NULL) {
        /* go for all fundefs in nodelist */
        NODELIST_NODE (funlist) = Trav (NODELIST_NODE (funlist), arg_info);
        fprintf (outfile, "else ");

        funlist = NODELIST_NEXT (funlist);
    }

    /* no specialized function found matching the args -> error */
    fprintf (outfile,
             "{\n"
             "  SAC_RuntimeError(\"ERROR - no matching specialized function!\\n\");\n"
             "  return(1); /* error - code */\n"
             "}\n"
             "return(0);\n}\n\n");

    if (CWRAPPER_NEXT (arg_node) != NULL) {
        CWRAPPER_NEXT (arg_node) = Trav (CWRAPPER_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints Wrapper-function c->SAC
 *   with argument checks, return values, refcounter handling
 *
 *
 ******************************************************************************/

node *
PIWfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWfundef");

    fprintf (outfile, "/* function: %s */\n", FUNDEF_NAME (arg_node));

    /* print code for functions switch */
    arg_node = PIWfundefSwitch (arg_node, arg_info);
    fprintf (outfile, " {\n");

    /* print code creating all return SAC_args */
    fprintf (outfile, "  /* create return type structs */\n");
    INFO_PIW_FLAG (arg_info) = PIW_CREATE_RETTYPES;
    INFO_PIW_COUNTER (arg_info) = 0;
    if (FUNDEF_TYPES (arg_node) != NULL) {
        FUNDEF_TYPES (arg_node) = TravTW (FUNDEF_TYPES (arg_node), arg_info);
    }

    /* print makros creating function call to specialized function */
    fprintf (outfile, "\n  /* call native SAC-function %s */\n", FUNDEF_NAME (arg_node));
    arg_node = PIWfundefCall (arg_node, arg_info);

    /* print makros for dec local refcounters and maybe free SAC_arg */
    fprintf (outfile, "\n  /* modify local refcounters */\n");
    arg_node = PIWfundefRefcounting (arg_node, arg_info);

    /* print code end of switch */
    fprintf (outfile, "}\n");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWarg(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints function arguments for use in Wrapper-function c->SAC
 *
 *
 ******************************************************************************/

node *
PIWarg (node *arg_node, node *arg_info)
{
    types *argtype;
    char *quote; /* optional quote character in string */
    char *tname; /* user type name */
    int i;

    DBUG_ENTER ("PIWArg");

    INFO_PIW_COUNTER (arg_info) = INFO_PIW_COUNTER (arg_info) + 1;

    /* analyse argument type: T_user or T_<simple> */
    if (TYPES_BASETYPE (ARG_TYPE (arg_node)) == T_user) {
        /* internal user type from N_typedef */
        argtype = TYPEDEF_TYPE (TYPES_TDEF (ARG_TYPE (arg_node)));
        tname = TYPEDEF_NAME (TYPES_TDEF (ARG_TYPE (arg_node)));
        quote = "\"";
    } else {
        /* standard type */
        argtype = ARG_TYPE (arg_node);
        tname = "NULL";
        quote = "";
    }

    switch (INFO_PIW_FLAG (arg_info)) {
    case PIW_SWITCH_ARGS:
        /* print check statement for argument */
        if (TYPES_DIM (argtype) < 0) {
            SYSERROR (("Unknown shapes cannot be exported!\n"));
        }

        fprintf (outfile, "SAC_CI_CmpSACArgType(in%d, %d, %s%s%s, %d",
                 INFO_PIW_COUNTER (arg_info), TYPES_BASETYPE (argtype), quote, tname,
                 quote, TYPES_DIM (argtype));

        if (TYPES_DIM (argtype) > 0) {
            /* arraytype with fixed shape */
            for (i = 0; i < TYPES_DIM (argtype); i++) {
                fprintf (outfile, ", %d", TYPES_SHAPE (argtype, i));
            }
        }
        fprintf (outfile, ")");

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (outfile, " && ");
        }
        break;

    case PIW_CALL_ARGS:
        /* print macro for arg in SAC-function call */
        DBUG_ASSERT ((TYPES_DIM (argtype) >= 0), "PIWarg: unknown shape dimension!\n");

        if (ARG_ATTRIB (arg_node) == ST_regular) {
            if (TYPES_DIM (argtype) == 0) {
                /* macro for simple type without refcounting */
                fprintf (outfile, "SAC_ARGCALL_SIMPLE");
            } else {
                /* macro for arraytype with refcounting */
                fprintf (outfile, "SAC_ARGCALL_REFCNT");
            }
        } else if (ARG_ATTRIB (arg_node) == ST_inout) {
            /* these args are handled like out parameters */
            if (TYPES_DIM (argtype) == 0) {
                /* macro for simple type without refcounting */
                fprintf (outfile, "SAC_ARGCALL_INOUT_SIMPLE");
            } else {
                /* macro for arraytype with refcounting */
                fprintf (outfile, "SAC_ARGCALL_INOUT_REFCNT");
            }
        } else {
            DBUG_ASSERT ((1 == 0), "PIWarg: unable to handle types attribute!");
        }

        fprintf (outfile, "( in%d , %s )", INFO_PIW_COUNTER (arg_info),
                 ctype_string[TYPES_BASETYPE (argtype)]);

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (outfile, ", ");
        }
        break;

    case PIW_REFCOUNT_ARGS:
        /* create macro, dec-and-free SAC_arg */
        if (TYPES_DIM (argtype) > 0) {
            /* refcounting only for array types */
            fprintf (outfile, "  SAC_DECLOCALRC( in%d );\n", INFO_PIW_COUNTER (arg_info));
        }
        break;

    default:
        SYSERROR (("undefined case in PIWarg!\n"));
    }

    /*traverse to next argument */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   types *PIWtypes(types *arg_type, node *arg_info)
 *
 * description:
 *   Prints results of functions in different formats
 *
 * remark: simulation of the syntax of the Trav technology
 *
 ******************************************************************************/

static types *
PIWtypes (types *arg_type, node *arg_info)
{
    types *atype;
    /* reference to implemented type - direct if simple
     * type in tdef if T_user
     */
    char *quote; /* optional quote character in string */
    char *tname; /* user type name */
    int i;

    DBUG_ENTER ("PIWtypes");

    /* skip void returntypes */
    if (!(TYPES_BASETYPE (arg_type) == T_void)) {
        INFO_PIW_COUNTER (arg_info) = INFO_PIW_COUNTER (arg_info) + 1;

        /* analyse argument type: T_user or T_<simple> */
        if (TYPES_BASETYPE (arg_type) == T_user) {
            /* internal user type from N_typedef */
            atype = TYPEDEF_TYPE (TYPES_TDEF (arg_type));
            tname = TYPEDEF_NAME (TYPES_TDEF (arg_type));
            quote = "\"";
        } else {
            /* standard type */
            atype = arg_type;
            tname = "NULL";
            quote = "";
        }

        switch (INFO_PIW_FLAG (arg_info)) {
        case PIW_CREATE_RETTYPES:
            /* create vars for reference parameters */
            fprintf (outfile, "  *out%d=SAC_CI_CreateSACArg(%d, %s%s%s, %d",
                     INFO_PIW_COUNTER (arg_info), TYPES_BASETYPE (atype), quote, tname,
                     quote, TYPES_DIM (atype));

            /* write shape data*/
            for (i = 0; i < TYPES_DIM (atype); i++) {
                fprintf (outfile, ", %d", TYPES_SHAPE (atype, i));
            }
            fprintf (outfile, ");\n");

            /* for simple types, alloc data memory */
            if (TYPES_DIM (atype) == 0) {
                fprintf (outfile, "  SAC_CI_INIT_SIMPLE_RESULT(out%d, %d );\n",
                         INFO_PIW_COUNTER (arg_info), TYPES_BASETYPE (atype));
            }
            break;

        case PIW_CALL_RESULTS:
            /* create macros for reference result types */
            if (TYPES_STATUS (arg_type) != ST_crettype) {
                if (TYPES_DIM (atype) == 0) {
                    /* macro for simple type without refcounting */
                    fprintf (outfile, "SAC_RESULT_SIMPLE");
                } else {
                    /* macro for arraytype with refcounting */
                    fprintf (outfile, "SAC_RESULT_REFCNT");
                }
                fprintf (outfile, "( out%d , %s )", INFO_PIW_COUNTER (arg_info),
                         ctype_string[TYPES_BASETYPE (atype)]);

                INFO_PIW_COMMA (arg_info) = TRUE;
            }

            /* is there at least one more result?
             * check, if there is a comma needef */
            if ((TYPES_NEXT (arg_type) != NULL)
                && (TYPES_STATUS (arg_type) != ST_crettype)) {
                if (!((TYPES_STATUS (TYPES_NEXT (arg_type)) == ST_crettype)
                      && (TYPES_NEXT (TYPES_NEXT (arg_type)) == NULL))) {
                    fprintf (outfile, ", ");
                }
            }
            break;

        case PIW_CALL_RETPOS:
            /* create macro for simple direct return value */
            if (TYPES_STATUS (arg_type) == ST_crettype) {
                fprintf (outfile,
                         "SAC_ASSIGN_RESULT( out%d, %s) = ", INFO_PIW_COUNTER (arg_info),
                         ctype_string[TYPES_BASETYPE (atype)]);
            }
            break;

        case PIW_REFCOUNT_RESULTS:
            /* init refcounts with 1 */
            fprintf (outfile, "  SAC_SETLOCALRC(out%d , 1 );\n",
                     INFO_PIW_COUNTER (arg_info));
            break;

        default:
            SYSERROR (("undefined case in PIWtypes!\n"));
        }
    }

    /* traverse to next returntype */
    if (TYPES_NEXT (arg_type) != NULL) {
        TYPES_NEXT (arg_type) = TravTW (TYPES_NEXT (arg_type), arg_info);
    }

    DBUG_RETURN (arg_type);
}

/******************************************************************************
 *
 * function:
 *   node *PIWfundefSwitch(node *wrapper, node *arg_info)
 *
 * description:
 *   Prints one fundef switch in a cwrapper function
 *
 ******************************************************************************/

static node *
PIWfundefSwitch (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWfundefSwitch");

    fprintf (outfile, "if(");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /*traverse all arguments */
        INFO_PIW_FLAG (arg_info) = PIW_SWITCH_ARGS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    } else {
        /* no args -> always true */
        fprintf (outfile, " 1 ");
    }

    fprintf (outfile, ")");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWfundefCall(node *wrapper, node *arg_info)
 *
 * description:
 *   Prints one fundef call in a cwrapper function
 *
 ******************************************************************************/

static node *
PIWfundefCall (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWfundefCall");

    /* print reference return parameters */

    if (FUNDEF_TYPES (arg_node) != NULL) {
        /* print direct return parameter */
        INFO_PIW_FLAG (arg_info) = PIW_CALL_RETPOS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_TYPES (arg_node) = TravTW (FUNDEF_TYPES (arg_node), arg_info);
    }

    fprintf (outfile, "  %s(", FUNDEF_NAME (arg_node));
    INFO_PIW_COMMA (arg_info) = FALSE;

    if (FUNDEF_TYPES (arg_node) != NULL) {
        /* print reference return parameters */

        INFO_PIW_FLAG (arg_info) = PIW_CALL_RESULTS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_TYPES (arg_node) = TravTW (FUNDEF_TYPES (arg_node), arg_info);
    }

    /* print args */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        if (INFO_PIW_COMMA (arg_info) == TRUE) {
            fprintf (outfile, ", ");
        }
        INFO_PIW_FLAG (arg_info) = PIW_CALL_ARGS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    fprintf (outfile, ");\n");
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWfundefRefcounting(node *arg_node, node *arg_info)
 *
 * description:
 *   Prints refcounter modifications in a cwrapper function
 *
 ******************************************************************************/

static node *
PIWfundefRefcounting (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PIWfundefRefcounting");

    /* print refcount modifications for results */
    if (FUNDEF_TYPES (arg_node) != NULL) {
        INFO_PIW_FLAG (arg_info) = PIW_REFCOUNT_RESULTS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_TYPES (arg_node) = TravTW (FUNDEF_TYPES (arg_node), arg_info);
    }

    /*  print refcount modifications for arguments */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        INFO_PIW_FLAG (arg_info) = PIW_REFCOUNT_ARGS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    fprintf (outfile, "\n");
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PIWobjdef(node *arg_node, node *arg_info)
 *
 * description:
 *   generates a c-file with definition of a global init flag for this object
 *
 *
 ******************************************************************************/

node *
PIWobjdef (node *arg_node, node *arg_info)
{
    FILE *old_outfile;

    DBUG_ENTER ("PIWobjdef");

    INFO_PIW_COUNTER (arg_info) = INFO_PIW_COUNTER (arg_info) + 1;
    old_outfile = outfile; /* save, might be in use */

    /* open obj<i>.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/objinitflag%d.c", tmp_dirname, INFO_PIW_COUNTER (arg_info));
    fprintf (outfile,
             "/* global object init flag for %s */ \n"
             "#include \"sac_bool.h\" \n",
             OBJDEF_NAME (arg_node));
    fprintf (outfile, "bool SAC_INIT_FLAG_%s = false;\n", OBJDEF_NAME (arg_node));
    fclose (outfile);
    outfile = old_outfile;

    if (OBJDEF_NEXT (arg_node) != NULL) {
        arg_node = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PrintInterface( node *syntax_tree)
 *
 * description:
 *   Prints the whole Module-Interface files for c library.
 *
 *
 ******************************************************************************/

node *
PrintInterface (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("PrintInterface");
    NOTE (("Generating c library interface files\n"));

    arg_info = MakeInfo ();
    old_tab = act_tab;

    /* do PIH traversal */
    act_tab = pih_tab;
    syntax_tree = Trav (syntax_tree, arg_info);

    /* do PIW traversal */
    act_tab = piw_tab;
    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;
    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   types *TravTH(types *arg_type, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for PIH
 *
 *
 ******************************************************************************/

static types *
TravTH (types *arg_type, node *arg_info)
{
    DBUG_ENTER ("TravTH");

    DBUG_ASSERT (arg_type != NULL, "TravTH: traversal in NULL type\n");
    arg_type = PIHtypes (arg_type, arg_info);

    DBUG_RETURN (arg_type);
}

/******************************************************************************
 *
 * function:
 *   types *TravTW(types *arg_type, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used dor PIW
 *
 ******************************************************************************/

static types *
TravTW (types *arg_type, node *arg_info)
{
    DBUG_ENTER ("TravTW");

    DBUG_ASSERT (arg_type != NULL, "TravTW: traversal in NULL type\n");
    arg_type = PIWtypes (arg_type, arg_info);

    DBUG_RETURN (arg_type);
}

/******************************************************************************
 *
 * function:
 *   strings *PrintDepEntry(deps *depends, statustype stat, strings *done)
 *
 * description:
 *   prints the dependencies to library files in the header comment.
 *
 ******************************************************************************/

static strings *
PrintDepEntry (deps *depends, statustype stat, strings *done)
{
    strings *tmp_done;
    deps *tmp;

    DBUG_ENTER ("PrintDepEntry");

    tmp = depends;

    while (tmp != NULL) {
        if (DEPS_STATUS (tmp) == stat) {
            tmp_done = done;

            while ((tmp_done != NULL)
                   && (0 != strcmp (DEPS_NAME (tmp), STRINGS_STRING (tmp_done)))) {
                tmp_done = STRINGS_NEXT (tmp_done);
            }
            if (tmp_done == NULL) {
                done = MakeStrings (DEPS_NAME (tmp), done);
                fprintf (outfile, " * %s\n", DEPS_LIBNAME (tmp));
            }
        }
        tmp = DEPS_NEXT (tmp);
    }

    tmp = depends;
    while (tmp != NULL) {
        if (DEPS_SUB (tmp) != NULL) {
            done = PrintDepEntry (DEPS_SUB (tmp), stat, done);
        }
        tmp = DEPS_NEXT (tmp);
    }

    DBUG_RETURN (done);
}

/******************************************************************************
 *
 * function:
 *    void PIHModuleInitFlag(char *modname)
 *
 * description:
 *   prints code with comment for SAC_Initflag<mod>
 *
 ******************************************************************************/

static void
PIWModuleInitFlag (char *modname)
{
    DBUG_ENTER ("PIHModuleInitFlag");
    fprintf (outfile,
             "/* initflag */\n"
             "static bool SAC_Initflag%s=false;\n\n",
             modname);
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void PIWModuleInitFunction(char *modname)
 *
 * description:
 *   prints code in cwrapper.c for SAC_Free<mod>()
 *
 ******************************************************************************/

static void
PIWModuleInitFunction (char *modname)
{
    DBUG_ENTER ("PIWModuleInitFunction");
    fprintf (outfile,
             "static void SAC_Init%s()\n"
             "{\n"
             "  SAC_FIH_AddFreeFunction( &SAC_Free%s );\n",
             modname, modname);
    GSCPrintMainBegin ();
    fprintf (outfile, "\n}\n\n\n");
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void PIWModuleFreeFunction(char *modname)
 *
 * description:
 *   prints code in cwrapper.c for SAC_Free<mod>()
 *
 ******************************************************************************/

static void
PIWModuleFreeFunction (char *modname)
{
    DBUG_ENTER ("PIWModuleExitFunction");
    fprintf (outfile, "static void SAC_Free%s()\n{\n", modname);
    /* unfortunately there is nothing to free for a module, jet*/
    fprintf (outfile, "\n}\n\n\n");
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *    void PrintInternalRuntimeInit(node *arg_node)
 *
 * description:
 *   prints code in internal_runtime_init.c which inits the heapmanager
 *   and the multithreading parts of the runtime system
 *   this function is called by SAC_InitRuntimeSystem
 *
 ******************************************************************************/
static void
PrintInternalRuntimeInit (node *arg_node)
{
    FILE *old_outfile;

    DBUG_ENTER ("PrintInternalRuntimeInit");

    old_outfile = outfile; /* save, might be in use */

    /* open internal_runtime_init.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/internal_runtime_init.c", tmp_dirname);
    fprintf (outfile, "/* Interface SAC <-> C \n");
    fprintf (outfile, " * this code initializes the internal data structures\n"
                      " * of the SAC runtime system */\n\n");

    /* general preload for codefile */
    fprintf (outfile, "/* startup functions and global code */\n\n");
    GSCPrintInternalInitFileHeader (arg_node);
    fprintf (outfile, "void SAC_InternalRuntimeInit(int __argc, char **__argv)\n"
                      "{\n"
                      "  SAC_MT_SETUP_INITIAL();\n"
                      "  SAC_PF_SETUP();\n"
                      "  SAC_HM_SETUP();\n"
                      "  SAC_MT_SETUP();\n"
                      "  SAC_CS_SETUP();\n"
                      "}\n\n");
    fprintf (outfile, "void SAC_InternalRuntimeExit()\n"
                      "{\n");
    GSCPrintMainEnd ();
    fprintf (outfile, "\n}\n\n");

    fprintf (outfile, "/* generated codefile, please do not modify */\n");
    fclose (outfile);

    outfile = old_outfile; /* restore old filehandle */

    DBUG_VOID_RETURN;
}
