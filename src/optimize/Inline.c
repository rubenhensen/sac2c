/*
 *
 * $Log$
 * Revision 1.1  1995/05/26 14:22:18  asi
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
#include "Inline.h"

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
Inline (node *arg_node, node *info_node)
{
    DBUG_ENTER ("WorkReduction");
    act_tab = inline_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    DBUG_RETURN (arg_node);
}

node *
INLfundef (node *arg_node, node *info_node)
{
    DBUG_ENTER ("INLfundef");

    DBUG_RETURN (arg_node);
}
