/*
 *
 * $Log$
 * Revision 1.2  1995/06/14 13:31:23  asi
 * added Unroll, UNRdo, UNRwhile and UNRassign
 *
 * Revision 1.1  1995/05/26  14:22:26  asi
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
#include "ConstantFolding.h"
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
    funptr *tmp_tab;

    DBUG_ENTER ("Unroll");
    tmp_tab = act_tab;
    act_tab = unroll_tab;

    arg_node = Trav (arg_node, info_node);

    act_tab = tmp_tab;
    DBUG_RETURN (arg_node);
}

node *
UNRdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRdo");

    DBUG_RETURN (arg_node);
}

node *
UNRwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRwhile");

    DBUG_RETURN (arg_node);
}

node *
UNRassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("UNRassign");

    DBUG_RETURN (arg_node);
}
