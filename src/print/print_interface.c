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
 *   char *truncFunName( char *funname)
 *
 * description:
 *   truncates the actual function name after some renaming by removing
 *   SAC-prefix and modulename. returns the offset-pointer in the funname string
 *   NOT a new copy of that string!
 *
 ******************************************************************************/
char *
truncFunName (char *funname)
{
    int offset;
    char *result;
    DBUG_ENTER ("truncFunName");

    /* behind the modulename in the current funname skip additional inserted "__" */
    offset = strlen (modulename) + strlen ("__");

    result = strstr (funname, modulename) + offset;

    DBUG_RETURN (result);
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

    DBUG_ENTER ("PrintInterface");
    NOTE (("Generating c library interface files\n"));

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
    syntax_tree
      = Trav (syntax_tree,
              arg_info); /*start travesal of tree, looking for fundef & arg nodes*/
    FREE (arg_info);

    fprintf (outfile,
             "/* generated headerfile, please do not modify function prototypes */\n");
    fclose (outfile);

    /* print all wrapper functions */
    /* do be implemented */

    /* beautify the headerfile, removing ICMs */
    /* to be implemented */

    /* Systemcall to resolve ICM Makros in a file using the cpp */
    /* SystemCall("%s -E -H %s -o %s/%s.h %s/%s.h",
       config.cc, config.ccdir, tmp_dirname, modulename, tmp_dirname, modulename); */

    DBUG_RETURN (syntax_tree);
}
