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
static void
PrintRC (int rc, int nrc, int show_rc)
{
    DBUG_ENTER ("PrintRC");

    if ((rc != -1) && show_rc) {
        fprintf (outfile, ":%d", rc);
    }
    DBUG_EXECUTE ("PRINT_NRC",
                  if ((nrc != -1) && show_rc) {
                      fprintf (outfile, "::%d", nrc);
                  } if ((!(optimize & OPT_RCO)) && show_rc && (rc != -1) && (rc != nrc)) {
                      fprintf (outfile, "**");
                  });

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("PIHArg");

    DBUG_EXECUTE ("PRINT_MASKS", fprintf (outfile, "**%d: ", ARG_VARNO (arg_node)););

    fprintf (outfile, " %s",
             Type2String (ARG_TYPE (arg_node),
                          INFO_PRINT_OMIT_FORMAL_PARAMS (arg_info) ? 0 : 1));

    /*PrintRC( ARG_REFCNT( arg_node), ARG_NAIVE_REFCNT( arg_node), show_refcnt);*/

    if (ARG_COLCHN (arg_node) && show_idx) {
        Trav (ARG_COLCHN (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        fprintf (outfile, ",");
        Trav (ARG_NEXT (arg_node), arg_info);
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
    node *params;
    int dim;
    int i, j;

    DBUG_ENTER ("PIHfundef");

    fprintf (outfile, "/* function declaration for %s */\n", FUNDEF_NAME (arg_node));

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

            fprintf (outfile, "extern ");
            /* simple return type */
            fprintf (outfile, "<returntype> ");

            /* function name */
            fprintf (outfile, "%s(", truncFunName (FUNDEF_NAME (arg_node)));

            /* rest of return types */
            fprintf (outfile, "<ret.types>, ");

            /* args */
            if (FUNDEF_ARGS (arg_node) != NULL) {
                Trav (FUNDEF_ARGS (arg_node), arg_info);
            }

            /* EOL */
            fprintf (outfile, ");\n");

            if ((NULL != FUNDEF_ICM (arg_node))
                && (N_icm == NODE_TYPE (FUNDEF_ICM (arg_node)))
                && (FUNDEF_STATUS (arg_node) != ST_spmdfun)) {
                Trav (FUNDEF_ICM (arg_node), arg_info); /* print N_icm ND_FUN_DEC */
            } else {
                NOTE (("Printed function header!!!\n"));
                PrintFunctionHeader (arg_node, arg_info);
            }

            fprintf (outfile, ";\n");

            fprintf (outfile, "\n");
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) { /* traverse next function */
        Trav (FUNDEF_NEXT (arg_node), arg_info);
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
    fprintf (outfile, "/*Interface SAC-C for %s */\n", modulename);
    fprintf (outfile, "#include \"sac_std.h\"\n");
    fprintf (outfile, "\n");

    /*Generate fixed manual and usage hints in headerfile*/
    /* to be implemented */

    /* print all function headers */
    old_tab = act_tab;
    act_tab = pih_tab;

    arg_info = MakeInfo ();

    INFO_PRINT_CONT (arg_info) = NULL;

    syntax_tree = Trav (syntax_tree, arg_info);

    FREE (arg_info);

    fclose (outfile);

    /* print all wrapper functions */
    /* do be implemented */

    /* beautify the headerfile, removing ICMs */
    /* to be implemented */
    SystemCall ("%s -E -H %s -o %s/%s.h %s/%s.h", config.cc, config.ccdir, tmp_dirname,
                modulename, tmp_dirname, modulename);

    DBUG_RETURN (syntax_tree);
}
