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

/* ??? copied from print.c */
#define TYPE_LENGTH 256      /* dimension of array of char */
#define INT_STRING_LENGTH 16 /* dimension of array of char */

/* strings for primitve types */

extern char *type_string[];

/* strings for primitve types used for renaming of functions*/
#define TYP_IFfunr_str(str) str
static char *rename_type[] = {
#include "type_info.mac"
};

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

    fprintf (outfile, " %s *%s", SACARGTYPE, truncArgName (ARG_NAME (arg_node)));
    FREE (typestring);

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        arg_node = Trav (ARG_NEXT (arg_node), arg_info);
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
 *
 *
 ******************************************************************************/
node *
PIHfundef (node *arg_node, node *arg_info)
{
    types *rettypes;
    int separator_needed;
    int i;
    char *typestring;

    DBUG_ENTER ("PIHfundef");

    if (FUNDEF_STATUS (arg_node)
        == ST_regular) { /* export only functions defined in this module */
        if ((FUNDEF_BODY (arg_node) == NULL)
            || ((NULL != FUNDEF_RETURN (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node))))) {
            /* print wrapper-prototype to headerfile */
            /* comment header */
            fprintf (outfile, "/* function declaration for %s */\n",
                     truncFunName (FUNDEF_NAME (arg_node)));

            fprintf (outfile, "extern ");

            /* simple return type */
            fprintf (outfile, "int ");

            /* function name */
            fprintf (outfile, "%s(", truncFunName (FUNDEF_NAME (arg_node)));

            /* SAC-return-types */
            rettypes = FUNDEF_TYPES (arg_node);
            i = 0;
            separator_needed = FALSE;
            while (rettypes != NULL) {
                i++;
                fprintf (outfile, "%s **out%d", SACARGTYPE, i);
                rettypes = TYPES_NEXT (rettypes);
                if (rettypes != NULL)
                    fprintf (outfile, ", ");
                separator_needed = TRUE;
            }

            /* args */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                if (separator_needed)
                    fprintf (outfile, ", ");
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
            }

            /* EOL */
            fprintf (outfile, ");\n\n");
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

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
    char *typestring;
    DBUG_ENTER ("PIWArg");

    typestring = Type2CTypeString (ARG_TYPE (arg_node), 0);
    fprintf (outfile, " %s%s", typestring, truncArgName (ARG_NAME (arg_node)));
    FREE (typestring);

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        arg_node = Trav (ARG_NEXT (arg_node), arg_info);
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
    types *rettypes;
    node *params;
    int separator_needed;
    int i;
    char *typestring;

    DBUG_ENTER ("PIWfundef");

    if (FUNDEF_STATUS (arg_node)
        == ST_regular) { /* export only functions defined in this module */
        if ((FUNDEF_BODY (arg_node) == NULL)
            || ((NULL != FUNDEF_RETURN (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_RETURN (arg_node))))) {

            /*some debug comment*/
            NOTE (("working on function: %s\n", FUNDEF_NAME (arg_node)));

            /*first the returntypes*/
            rettypes = FUNDEF_TYPES (arg_node);
            i = 0;
            while (rettypes != NULL) {
                i++;

                NOTE (("  %d. return type: %s\n", i, Type2String (rettypes, 0 | 4)));
                rettypes = TYPES_NEXT (rettypes);
            }

            /*next the parameters*/
            params = FUNDEF_ARGS (arg_node);
            i = 0;
            while (params != NULL) {
                i++;
                NOTE (("  %d. arg type: %s %s\n", i,
                       Type2String (ARG_TYPE (params), 0 | 4), ARG_NAME (params)));
                params = ARG_NEXT (params);
            }

            NOTE (("\n"));

            /* print wrapper-prototype to headerfile */
            /* comment header */
            fprintf (outfile, "/* wrapper function for %s */\n", FUNDEF_NAME (arg_node));

            /* simple return type */
            fprintf (outfile, "int ");

            /* function name */
            fprintf (outfile, "%s(", truncFunName (FUNDEF_NAME (arg_node)));

            /* SAC-return-types */
            rettypes = FUNDEF_TYPES (arg_node);
            i = 0;
            separator_needed = FALSE;
            while (rettypes != NULL) {
                i++;
                typestring = Type2CTypeString (rettypes, 0);
                fprintf (outfile, "%s*out%d", typestring, i);
                FREE (typestring);
                rettypes = TYPES_NEXT (rettypes);
                if (rettypes != NULL)
                    fprintf (outfile, ", ");
                separator_needed = TRUE;
            }

            /* args */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                if (separator_needed)
                    fprintf (outfile, ", ");
                FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
            }

            /* End Of Header */
            fprintf (outfile, "){\n");

            /* wrapper body */
            if ((NULL != FUNDEF_ICM (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))
                && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
                Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
            }

            /* end of wrapper */
            fprintf (outfile, "\n}\n\n");
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   char *truncFunName( char *funname)
 *
 * description:
 *   truncates the actual function name after some renaming by removing
 *   SAC-type postfixes.
 * returns new allocated string
 *
 ******************************************************************************/
char *
truncFunName (char *funname)
{
    int offset;
    char *endpos;
    char *result;
    char *tmp_string;
    DBUG_ENTER ("truncFunName");

    tmp_string = (char *)Malloc (sizeof (char) * strlen (funname));
    tmp_string[0] = '\0';
    strcat (tmp_string, "SAC_");     /* set standard Prefix */
    strcat (tmp_string, modulename); /* add modulename*/
    strcat (tmp_string, "_");
    /* add funname */
    /* behind the modulename in the current funname skip additional inserted "__" */
    offset = strlen (modulename) + strlen ("__");
    result = strstr (funname, modulename) + offset;
    endpos = strstr (result, "__");
    strncat (tmp_string, result, strlen (result) - strlen (endpos)); /* isolate funname */

    DBUG_RETURN (tmp_string);
}

/******************************************************************************
 *
 * function:
 *   char *truncArgName( char *argname)
 *
 * description:
 *   truncates the actual argument name after some renaming by removing
 *   SAC-prefix. returns the offset-pointer in the funname string
 *   NOT a new copy of that string!
 *
 ******************************************************************************/

char *
truncArgName (char *argname)
{
    int offset;
    char *result;
    DBUG_ENTER ("truncArgName");

    /* skip additional inserted "SACl_" */
    offset = strlen ("SACl_");

    result = argname + offset;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *Type2CTypeString(types *type , int flag)
 *
 * description:
 *   returns as string the c-type to use in the c-interface for sac-type type
 *   example: array-types int[] -> sa_i, double -> sa_d ...
 *            int -> int
 *            usertype -> ???
 *   flag:    not used
 *
 *
 ******************************************************************************/
char *
Type2CTypeString (types *type, int flag)
{
    char *tmp_string;

    DBUG_ENTER ("Type2CTypeString");

    tmp_string = (char *)Malloc (sizeof (char) * TYPE_LENGTH);
    tmp_string[0] = '\0';

    if (TYPES_DIM (type) > 0) { /* arraytype with dim>0 */
        /* uses prefix sa_ for SAC-ARRAY-TYPE */
        strcat (tmp_string, "sa_");
        if (TYPES_BASETYPE (type) == T_user) {
            /* user defined types are not handled yet */
            SYSERROR (("User defined types are not handled yet.\n"));
            strcat (tmp_string, TYPES_NAME (type));
        } else {
            /* simple basetype */
            strcat (tmp_string, rename_type[TYPES_BASETYPE (type)]);
        }
        strcat (tmp_string, " *"); /* add pointer * for struct arg */
    } else {                       /* scalartype */
        /* no additional prefix neede */
        if (TYPES_BASETYPE (type) == T_user) {
            /* user defined types are not handled yet */
            SYSERROR (("User defined types are not handled yet.\n"));
            strcat (tmp_string, "__");
            strcat (tmp_string, TYPES_NAME (type));
        } else {
            /* simple basetype */
            strcat (tmp_string, type_string[TYPES_BASETYPE (type)]);
        }
        strcat (tmp_string, " "); /* add space to arg-name */
    }

    DBUG_RETURN (tmp_string);
}
/******************************************************************************
 *
 * function:
 *  extern node *MapFunctionToWrapper(node *syntaxtree)
 *
 * description:
 *  builds a simple tree of N_info nodes which mapps all exportable functions
 *  to their wrapper function. uses fundefs from syntaxtree.
 * returns root of mappedtree
 *
 ******************************************************************************/
node *
MapFunctionToWrapper (node *syntax_tree)
{
    node *mappingtree = NULL;
    node *funs;
    node *wrapper;

    funs = MODUL_FUNS (syntax_tree);
    while (funs != NULL) { /* look for all fundefs in tree */

        NOTE (("found a fundef node (%s)...\n", FUNDEF_NAME (funs)));
        funs = FUNDEF_NEXT (funs);
    }

    return mappingtree;
}

/******************************************************************************
 *
 * function:
 *   node *PrintInterface( node *syntax_tree)
 *
 * description:
 *   Prints the whole Module-Interface files for c library.
 *   generates <modulename>.h as headerfile with wrapper defs and datatypes
 *             <modulename>_wrapper.c implementing the wrapper functions
 *             ...
 *
 ******************************************************************************/
node *
PrintInterface (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    node *mappingtree;

    DBUG_ENTER ("PrintInterface");
    NOTE (("Generating c library interface files\n"));

    /* sort specialized functions to get needed wrapper functions */
    mappingtree = MapFunctionToWrapper (syntax_tree);

    /* open <module>.h in tmpdir for writing*/
    outfile = WriteOpen ("%s/%s.h", tmp_dirname, modulename);
    fprintf (outfile, "/* Interface SAC-C for %s */\n", modulename);
    fprintf (outfile, "/* use this file %s.h with lib%s.a\n", modulename, modulename);
    fprintf (outfile, "#include \"sacinterface.h\"\n");
    fprintf (outfile, "\n");

    /*Generate fixed manual and usage hints in headerfile*/
    /* to be implemented */

    /* print all function headers */
    /* only printing selected functions: to be implemented */
    old_tab = act_tab;
    act_tab = pih_tab;

    arg_info = MakeInfo ();

    INFO_PRINT_CONT (arg_info) = NULL;

    /*start travesal of tree, looking for fundef & arg nodes*/
    syntax_tree = Trav (syntax_tree, arg_info);
    FREE (arg_info);

    fprintf (outfile,
             "/* generated headerfile, please do not modify function prototypes */\n");
    fclose (outfile);

    /* header file finished - now generating code for wrapper functions */

    /* open <module>_wrapper.c in tmpdir for writing*/
    outfile = WriteOpen ("%s/%s_wrapper.c", tmp_dirname, modulename);
    fprintf (outfile, "/* Interface SAC-C for %s */\n", modulename);
    fprintf (outfile, "/* this file is only used when compiling the c-library lib%s.a\n",
             modulename);
    fprintf (outfile, "#include \"sacinterface.h\"\n");
    fprintf (outfile, "\n");

    /* general preload for codefile */
    /* to be implemented */
    fprintf (outfile, "/* <insert some useful things here...> */");

    /* print all function headers */
    /* only printing selected functions: to be implemented */
    act_tab = piw_tab;

    arg_info = MakeInfo ();

    INFO_PRINT_CONT (arg_info) = NULL;

    /*start travesal of tree, looking for fundef & arg nodes*/
    syntax_tree = Trav (syntax_tree, arg_info);
    FREE (arg_info);
    act_tab = old_tab;

    fprintf (outfile,
             "/* generated headerfile, please do not modify function prototypes */\n");
    fclose (outfile);

    /* print all wrapper functions */
    /* do be implemented */

    /* resolving ICMs */
    /* to be implemented */

    /* Systemcall to resolve ICM Makros in a file using the cpp */
    /* SystemCall("%s -E -H %s -o %s/%s.h %s/%s.h",
       config.cc, config.ccdir, tmp_dirname, modulename, tmp_dirname, modulename); */

    DBUG_RETURN (syntax_tree);
}

/* when used in print_interface.c  (sorting of fundefs -> wrapper funs)
#define INFO_PIW_FUNDEF(n)       (n->node[0])
#define INFO_PIW_NEXT_FUNDEF(n)  (n->node[1])
#define INFO_PIW_NEXT_WRAPPER(n) (n->node[2])
#define INFO_PIW_RETPOS(n)       (n->int_data)
#define INFO_PIW_ARGCOUNT(n)     (n->counter)
#define INFO_PIW_WRAPPERNAME(n)  (n->src_file)
*/
