/*
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

/* interface identifier */
#define SACARGTYPE "SAC_arg"
#define SACPREFIX "SAC_"
#define NO_SIMPLE_RETURN -1

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
char *ctype_string[] = {
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
static int GetReturnPos (node *fundef);

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

    DBUG_ENTER ("PIHmodul");

    old_outfile = outfile; /* save, might be in use */

    /* open <module>.h in tmpdir for writing wrapper header*/
    outfile = WriteOpen ("%s/%s.h", tmp_dirname, MODUL_NAME (arg_node));
    fprintf (outfile, "/* Interface SAC <-> C for %s \n", MODUL_NAME (arg_node));
    fprintf (outfile, " * use this %s.h file with lib%s.a */\n", MODUL_NAME (arg_node),
             MODUL_NAME (arg_node));
    fprintf (outfile, "#include \"sac_cinterface.h\"\n");
    fprintf (outfile, "\n");

    /*Generate fixed manual and usage hints in headerfile*/
    /* to be implemented */

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
    fprintf (outfile, "\n/* function\n *   %s in %s\n * accepts arguments as follows:\n",
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
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
        } else {
            fprintf (outfile, "void");
        }

        fprintf (outfile, " -> ");

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

    if (INFO_PIH_FLAG (arg_info) == PIH_PRINT_COMMENT) {
        /* print internal accepted types of argument */

        typestring = Type2String (ARG_TYPE (arg_node), 0);

        fprintf (outfile, "%s %s", typestring, ARG_NAME (arg_node));
        FREE (typestring);

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (outfile, ", ");
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
        }
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

    if (INFO_PIH_FLAG (arg_info) == PIH_PRINT_COMMENT) {
        /* print resulting types */

        typestring = Type2String (arg_type, 0);

        fprintf (outfile, "%s", typestring);
        FREE (typestring);

        if (TYPES_NEXT (arg_type) != NULL) {
            fprintf (outfile, ", ");
            TYPES_NEXT (arg_type) = TravTH (TYPES_NEXT (arg_type), arg_info);
        }
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
             CWRAPPER_NAME (arg_node), CWRAPPER_ARGCOUNT (arg_node),
             CWRAPPER_RESCOUNT (arg_node));

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

    old_outfile = outfile; /* save, might be in use */

    /* open <module>_wrapper.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/%s_wrapper.c", tmp_dirname, MODUL_NAME (arg_node));
    fprintf (outfile, "/* Interface SAC <-> C for %s\n", MODUL_NAME (arg_node));
    fprintf (outfile,
             " * this file is only used when compiling the c-library lib%s.a */\n",
             modulename);
    fprintf (outfile, "#include <stdio.h>\n");
    fprintf (outfile, "#include \"sac_cinterface.h\"\n");
    fprintf (outfile, "#include \"sac_arg.h\"\n");
    fprintf (outfile, "#include \"sac_cwrapper.h\"\n");

    /* declarations for external SAC functions */
    fprintf (outfile, "#include \"header.h\"\n");
    fprintf (outfile, "\n");

    /* general preload for codefile */
    fprintf (outfile, "/* <insert some useful things here...> */\n");

    if (MODUL_CWRAPPER (arg_node) != NULL) {
        /* traverse list of wrappers */
        MODUL_CWRAPPER (arg_node) = Trav (MODUL_CWRAPPER (arg_node), arg_info);
    }

    fprintf (outfile, "/* generated codefile, please do not modify */\n");
    fclose (outfile);
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

    /* print checks for refcounts */
    fprintf (outfile, "/* refcount checks for arguments */\n");
    for (i = 1; i <= CWRAPPER_ARGCOUNT (arg_node); i++) {
        fprintf (outfile, "SAC_IW_CHECK_RC( in%d );\n", i);
    }

    /* print case switch for specialized functions */
    fprintf (outfile, "/* case switch for specialized functions */\n");
    funlist = CWRAPPER_FUNS (arg_node);
    DBUG_ASSERT (funlist != NULL, "PIWcwrapper: wrapper without fundef\n");

    while (funlist != NULL) {
        /* go for all fundefs in nodelist */
        NODELIST_NODE (funlist) = Trav (NODELIST_NODE (funlist), arg_info);

        funlist = NODELIST_NEXT (funlist);
    }

    /* no speacialized function found matching the args -> error */
    fprintf (outfile,
             "fprintf(stderr, \"ERROR - no matching specialized function!\\n\");\n");
    fprintf (outfile, "return(1); /* error - no matching specialized function */\n");
    fprintf (outfile, "\n}\n\n");

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

    /* print code  successful return from call */
    fprintf (outfile, "\n  return(0); /*call successful */\n");

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
    int i;

    DBUG_ENTER ("PIWArg");

    INFO_PIW_COUNTER (arg_info) = INFO_PIW_COUNTER (arg_info) + 1;

    switch (INFO_PIW_FLAG (arg_info)) {
    case PIW_SWITCH_ARGS:
        /* print check statement for argument */
        argtype = ARG_TYPE (arg_node);

        if (TYPES_DIM (argtype) < 0) {
            SYSERROR (("Unknown shapes cannot be exported!\n"));
        }

        fprintf (outfile, "SAC_CI_CmpSACArgType(in%d, %d, %d",
                 INFO_PIW_COUNTER (arg_info), TYPES_BASETYPE (argtype),
                 TYPES_DIM (argtype));

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
        argtype = ARG_TYPE (arg_node);
        DBUG_ASSERT ((TYPES_DIM (argtype) >= 0), "PIWarg: unknown shape dimension!\n");

        if (TYPES_DIM (argtype) == 0) {
            /* macro for simple type without refcounting */
            fprintf (outfile, "SAC_ARGCALL_SIMPLE");
        } else {
            /* macro for arraytype with refcounting */
            fprintf (outfile, "SAC_ARGCALL_REFCNT");
        }
        fprintf (outfile, "( in%d , %s )", INFO_PIW_COUNTER (arg_info),
                 ctype_string[TYPES_BASETYPE (argtype)]);

        if (ARG_NEXT (arg_node) != NULL) {
            fprintf (outfile, ", ");
        }
        break;

    case PIW_REFCOUNT_ARGS:
        /* create macro, dec-and-free SAC_arg */
        fprintf (outfile, "  SAC_DECANDFREERC( in%d );\n", INFO_PIW_COUNTER (arg_info));
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
    int i;

    DBUG_ENTER ("PIWtypes");

    INFO_PIW_COUNTER (arg_info) = INFO_PIW_COUNTER (arg_info) + 1;

    switch (INFO_PIW_FLAG (arg_info)) {
    case PIW_CREATE_RETTYPES:
        /* create vars for reference parameters */
        fprintf (outfile, "  *out%d=SAC_CI_CreateSACArg(%d, %d",
                 INFO_PIW_COUNTER (arg_info), TYPES_BASETYPE (arg_type),
                 TYPES_DIM (arg_type));

        /* write shape data*/
        for (i = 0; i < TYPES_DIM (arg_type); i++) {
            fprintf (outfile, "%d", TYPES_SHAPE (arg_type, i));
            if (i < (TYPES_DIM (arg_type)) - 1) {
                fprintf (outfile, ", ");
            }
        }
        fprintf (outfile, ");\n");
        break;

    case PIW_CALL_RESULTS:
        /* create macros for reference result types */
        if (INFO_PIW_RETPOS (arg_info) != INFO_PIW_COUNTER (arg_info)) {
            if (TYPES_DIM (arg_type) == 0) {
                /* macro for simple type without refcounting */
                fprintf (outfile, "SAC_RESULT_SIMPLE");
            } else {
                /* macro for arraytype with refcounting */
                fprintf (outfile, "SAC_RESULT_REFCNT");
            }
            fprintf (outfile, "( out%d , %s )", INFO_PIW_COUNTER (arg_info),
                     ctype_string[TYPES_BASETYPE (arg_type)]);

            INFO_PIW_COMMA (arg_info) = TRUE;
        }

        /* is there at least one more result?
         * check, if there is a comma needef */
        if ((TYPES_NEXT (arg_type) != NULL)
            && (INFO_PIW_RETPOS (arg_info) != INFO_PIW_COUNTER (arg_info))) {
            if (!((INFO_PIW_RETPOS (arg_info) == INFO_PIW_COUNTER (arg_info) + 1)
                  && (TYPES_NEXT (TYPES_NEXT (arg_type)) == NULL))) {
                fprintf (outfile, ", ");
            }
        }
        break;

    case PIW_CALL_RETPOS:
        /* create macro for simple direct return value */
        if (INFO_PIW_RETPOS (arg_info) == INFO_PIW_COUNTER (arg_info)) {
            fprintf (outfile, "SAC_ASSIGN_RESULT( out%d, %s)",
                     INFO_PIW_COUNTER (arg_info),
                     ctype_string[TYPES_BASETYPE (arg_type)]);
        }
        break;

    case PIW_REFCOUNT_RESULTS:
        /* create macro, setting result refcount to 1 */
        fprintf (outfile, "  SAC_SETREFCOUNT(out%d , 1 );\n",
                 INFO_PIW_COUNTER (arg_info));
        break;

    default:
        SYSERROR (("undefined case in PIWtypes!\n"));
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

    INFO_PIW_RETPOS (arg_info) = GetReturnPos (arg_node);

    if (INFO_PIW_RETPOS (arg_info) != NO_SIMPLE_RETURN) {
        /* print direct return parameter */
        INFO_PIW_FLAG (arg_info) = PIW_CALL_RETPOS;
        INFO_PIW_COUNTER (arg_info) = 0;
        FUNDEF_TYPES (arg_node) = TravTW (FUNDEF_TYPES (arg_node), arg_info);
        fprintf (outfile, "=");
    }
    fprintf (outfile, "  %s(", FUNDEF_NAME (arg_node));

    INFO_PIW_COMMA (arg_info) = FALSE;

    /* print reference return parameters */
    if (FUNDEF_TYPES (arg_node) != NULL) {
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
 *   int GetReturnPos( node *fundef)
 *
 * description:
 *   Looks for the one returntype that can be passed by the function
 *   return value. This must be a simple type without refcounting.
 *
 * return:
 *   the position (1 to n) in the functions return list or NO_SIMPLE_RETURN
 *
 * remark:
 *   this implemenatation depends on compile.h!
 *   should be changed to a flag based variant
 *
 ******************************************************************************/

static int
GetReturnPos (node *fundef)
{
    int count = 0;
    types *rettypes;

    DBUG_ENTER ("GetReturnPos");

    rettypes = FUNDEF_TYPES (fundef);

    while (rettypes != NULL) {
        count++;
        if (TYPES_DIM (rettypes) == 0) {
            /* found a simple type */
            DBUG_RETURN (count);
        }
        rettypes = TYPES_NEXT (rettypes);
    }

    /* no simple return type found */
    DBUG_RETURN (NO_SIMPLE_RETURN);
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

/*

sac2c -genlib c TestModule.sac

gcc -DSAC_FOR_LINUX_X86 -I ./Interface/ -I ~/sac/sac2c/src/runtime/ -Wall -c -o
Interface/SAC_sacinterface.o Interface/SAC_interface.c

gcc -DSAC_FOR_LINUX_X86 -I ./Interface/ -I ~/sac/sac2c/src/runtime/ -Wall -c -o
Interface/SAC_arg.o Interface/SAC_arg.c

*/
