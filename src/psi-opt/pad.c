/*
 * $Log$
 * Revision 1.3  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.2  2000/05/26 14:24:29  sbs
 * dummy function ArrayPadding added.
 *
 * Revision 1.1  2000/05/26 13:41:35  sbs
 * Initial revision
 *
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"

#include "pad.h"
#include "pad_collect.h"
#include "pad_infer.h"
#include "pad_transform.h"

node *
ArrayPadding (node *arg_node)
{
    DBUG_ENTER ("ArrayPadding");

    DBUG_PRINT ("AP", ("Entering Array Padding"));

    APcollect (arg_node);
    APtransform (arg_node);

    DBUG_RETURN (arg_node);
}
