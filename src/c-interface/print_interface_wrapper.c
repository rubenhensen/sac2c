/*
 *
 * $Log$
 * Revision 3.8  2002/03/04 13:19:25  dkr
 * no changes done
 *
 * Revision 3.7  2001/04/27 08:47:22  nmw
 * PIWarg can handle unique args now
 *
 * Revision 3.6  2001/04/26 17:07:18  dkr
 * bug in PIWarg() detected but unfortunately not fixed yet :-(
 *
 * Revision 3.5  2001/03/22 18:54:50  dkr
 * include of tree.h eliminated
 *
 * Revision 3.4  2001/03/15 11:59:28  dkr
 * ST_inout replaced by ST_reference
 *
 * Revision 3.3  2000/12/05 14:35:52  nmw
 * handling of T_hidden fixed, macro calls adjusted
 *
 * Revision 3.2  2000/11/29 16:25:04  nmw
 * detailed runtime error messages for missing
 * specializations added
 *
 * Revision 3.1  2000/11/20 18:03:44  sacbase
 * new release made
 *
 * Revision 1.2  2000/08/03 14:17:26  nmw
 * handling macro for T_hidden added
 *
 * Revision 1.1  2000/08/02 14:26:17  nmw
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "print_interface_header.h"
#include "print_interface_wrapper.h"
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
static types *TravTW (types *arg_type, node *arg_info);
static types *PIWtypes (types *arg_type, node *arg_info);
static node *PIWfundefSwitch (node *arg_node, node *arg_info);
static node *PIWfundefCall (node *arg_node, node *arg_info);
static node *PIWfundefRefcounting (node *arg_node, node *arg_info);
static void PIWModuleInitFlag (char *modname);
static void PIWModuleInitFunction (char *modname);

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
    fprintf (outfile, "#include \"header.h\"\n"
                      "#include \"sac.h\"\n"
                      "#include \"sac_cwrapper.h\"\n"
                      "#include \"sac_cinterface.h\"\n\n");

    PIWModuleInitFlag (MODUL_NAME (arg_node));
    PIWModuleInitFunction (MODUL_NAME (arg_node));

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
        fprintf (outfile, "SAC_IW_CHECKDEC_RC( in%d , %d );\n", i, T_hidden);
    }
    /* restore original refcount */
    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile, "SAC_IW_INC_RC( in%d , %d );\n", i, T_hidden);
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

    /* no specialized function found matching the args -> generate error */
    fprintf (outfile,
             "{\n"
             "  char SAC_CI_tempcharbuffer[256];\n"
             "  SAC_Print(\"\\n\\n\\n*** function called with:\\n\");\n"
             "  SAC_Print(\"*** %s(\");\n",
             CWRAPPER_NAME (arg_node));

    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile,
                 "  SAC_CI_SACArg2string(in%d, SAC_CI_tempcharbuffer);\n"
                 "  SAC_Print(\"%%s\", SAC_CI_tempcharbuffer);\n",
                 i);
        if (i < CWRAPPER_ARGCOUNT (arg_node)) {
            fprintf (outfile, "  SAC_Print(\", \");\n");
        }
    }

    fprintf (outfile,
             "  SAC_Print(\")\\n\");\n"
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

        switch (ARG_ATTRIB (arg_node)) {
        case ST_regular:
        case ST_unique:
            if ((TYPES_DIM (argtype) == 0) && (TYPES_BASETYPE (argtype) != T_hidden)) {
                /* macro for simple type without refcounting */
                fprintf (outfile, "SAC_ARGCALL_SIMPLE");
            } else {
                /* macro for arraytype with refcounting */
                fprintf (outfile, "SAC_ARGCALL_REFCNT");
            }
            break;

        case ST_reference:
            /* these args are handled like out parameters */
            if (TYPES_DIM (argtype) == 0) {
                /* macro for simple type without refcounting */
                fprintf (outfile, "SAC_ARGCALL_INOUT_SIMPLE");
            } else {
                /* macro for arraytype with refcounting */
                fprintf (outfile, "SAC_ARGCALL_INOUT_REFCNT");
            }
            break;

        default:
            DBUG_ASSERT ((0), "PIWarg: unable to handle types attribute!");
        }

        fprintf (outfile, "( in%d , %s )", INFO_PIW_COUNTER (arg_info),
                 ctype_string[TYPES_BASETYPE (argtype)]);

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (outfile, ", ");
        }
        break;

    case PIW_REFCOUNT_ARGS:
        /* create macro, dec-and-free SAC_arg */
        if ((TYPES_DIM (argtype) > 0) || (TYPES_BASETYPE (argtype) == T_hidden)) {
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
                if (TYPES_BASETYPE (atype) != T_hidden) {
                    if ((TYPES_DIM (atype) == 0)) {
                        /* macro for simple type without refcounting */
                        fprintf (outfile, "SAC_RESULT_SIMPLE");
                    } else {
                        /* macro for arraytype with refcounting */
                        fprintf (outfile, "SAC_RESULT_REFCNT");
                    }
                } else {
                    fprintf (outfile, "SAC_RESULT_HIDDEN_RC");
                }
                fprintf (outfile, "( out%d , %s )", INFO_PIW_COUNTER (arg_info),
                         ctype_string[TYPES_BASETYPE (atype)]);

                INFO_PIW_COMMA (arg_info) = TRUE;
            }

            /* is there at least one more result?
             * check, if there is a comma needed */
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
 *   node *PIWfundefCall(node *arg_node, node *arg_info)
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
 *    void PIWModuleInitFlag(char *modname)
 *
 * description:
 *   prints code with comment for SAC_Initflag<mod>
 *
 *************************************************  ***************************/

static void
PIWModuleInitFlag (char *modname)
{
    DBUG_ENTER ("PIWModuleInitFlag");
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
             "{\n",
             modname);
    GSCPrintMainBegin ();
    fprintf (outfile, "\n}\n\n\n");
    DBUG_VOID_RETURN;
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
