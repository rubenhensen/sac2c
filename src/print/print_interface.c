#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"

#include "print.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"

/******************************************************************************
 *
 * function:
 *   node *PrintInterface( node *syntax_tree)
 *
 * description:
 *   Prints the whole Module-Interface files for c library.
 *
 ******************************************************************************/

node *
PrintInterface (node *syntax_tree)
{
    node *arg_info;

    DBUG_ENTER ("PrintInterface");

    arg_info = MakeInfo ();
    /* we want to duplicate all sons */
    INFO_PRINT_CONT (arg_info) = NULL;

    syntax_tree = PrintTrav (syntax_tree, arg_info);

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}
