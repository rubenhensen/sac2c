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
PIHfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PrintAnnotate");

    fprintf (outfile, "/* generating function: \n");

    DBUG_RETURN (arg_node);
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
    fprintf (outfile, "\n");

    /*Generate fixed manual and usage hints in headerfile*/
    /* to be implemented */

    /* print all function headers */
    old_tab = act_tab;
    act_tab = print_tab;

    arg_info = MakeInfo ();

    INFO_PRINT_CONT (arg_info) = NULL;

    /*syntax_tree = Trav( syntax_tree, arg_info);*/

    FREE (arg_info);
    fclose (outfile);

    /* print all wrapper functions */
    /* do be implemented */

    /* beautify the headerfile */
    /* to be implemented */

    DBUG_RETURN (syntax_tree);
}
