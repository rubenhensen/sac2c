/*
 * $Log$
 * Revision 1.2  2000/05/26 14:24:29  sbs
 * dummy function ArrayPadding added.
 *
 * Revision 1.1  2000/05/26 13:41:35  sbs
 * Initial revision
 *
 *
 */

#include "dbug.h"
#include "tree_basic.h"
#include "pad.h"

node *
ArrayPadding (node *arg_node)
{
    DBUG_ENTER ("ArrayPadding");

    DBUG_PRINT ("AP", ("Entering Array Padding"));

    DBUG_RETURN (arg_node);
}
