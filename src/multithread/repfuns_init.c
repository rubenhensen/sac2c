/*
 *
 * $Log$
 * Revision 1.1  2000/02/04 13:48:36  jhs
 * Initial revision
 *
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"

#include "internal_lib.h"

node *
RepfunsInit (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("BuildRepfuns");

    DBUG_RETURN (arg_node);
}
