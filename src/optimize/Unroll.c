/*
 *
 * $Log$
 * Revision 1.1  1995/05/26 14:22:26  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "Unroll.h"

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */
node *
Unroll (node *arg_node, node *info_node)
{
    DBUG_ENTER ("Unroll");
    act_tab = unroll_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    DBUG_RETURN (arg_node);
}

node *
UNRfundef (node *arg_node, node *info_node)
{
    DBUG_ENTER ("UNRfundef");

    DBUG_RETURN (arg_node);
}
