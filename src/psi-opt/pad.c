/*
 * $Log$
 * Revision 1.4  2000/06/08 11:14:49  mab
 * pad_info added
 *
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
#include "pad_info.h"
#include "pad_collect.h"
#include "pad_infer.h"
#include "pad_transform.h"

node *
ArrayPadding (node *arg_node)
{
    nums *tmp_old_shape;
    nums *tmp_new_shape;

    DBUG_ENTER ("ArrayPadding");

    DBUG_PRINT ("AP", ("Entering Array Padding"));

    tmp_old_shape = MakeNums (2, NULL);
    NUMS_NEXT (tmp_old_shape) = MakeNums (8, NULL);

    tmp_new_shape = MakeNums (3, NULL);
    NUMS_NEXT (tmp_new_shape) = MakeNums (9, NULL);

    /* constante fuer typ (arg2) einfuegen */
    PIadd (2, 0, tmp_old_shape, tmp_new_shape);

    tmp_old_shape = MakeNums (16, NULL);
    NUMS_NEXT (tmp_old_shape) = MakeNums (12, NULL);

    tmp_new_shape = MakeNums (19, NULL);
    NUMS_NEXT (tmp_new_shape) = MakeNums (13, NULL);

    /* constante fuer typ (arg2) einfuegen */
    PIadd (2, 0, tmp_old_shape, tmp_new_shape);

    /*  APcollect(arg_node); */
    APtransform (arg_node);

    /* free pad_info */

    DBUG_RETURN (arg_node);
}
