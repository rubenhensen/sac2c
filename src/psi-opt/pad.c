/*
 * $Log$
 * Revision 1.9  2000/07/21 14:39:20  mab
 * *** empty log message ***
 *
 * Revision 1.8  2000/07/19 12:39:28  mab
 * added dummy values for testing purposes
 *
 * Revision 1.7  2000/07/07 12:07:51  mab
 * *** empty log message ***
 *
 * Revision 1.6  2000/07/05 09:12:34  mab
 * fixed memory problem
 *
 * Revision 1.5  2000/06/14 10:41:31  mab
 * comments added
 *
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

/*****************************************************************************
 *
 * file:   pad.c
 *
 * prefix: AP
 *
 * description:
 *
 *   This compiler module infers new array shapes and applies array padding
 *   to improve cache performance.
 *
 *
 *****************************************************************************/

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

/*****************************************************************************
 *
 * function:
 *   node *ArrayPadding( node * arg_node)
 *
 * description:
 *   main function for array padding,
 *   calls APcollect, APinfer and APtransform
 *
 *****************************************************************************/

node *
ArrayPadding (node *arg_node)
{
    shpseg *tmp_old_shape;
    shpseg *tmp_new_shape;

    DBUG_ENTER ("ArrayPadding");

    PIinit ();

    DBUG_PRINT ("AP", ("Entering Array Padding"));

    /* collect information for inference phase */

    /* APcollect(); */

    /* apply array padding */

    /* APinfer(); */

    /* dummies only !!! */

    tmp_old_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_old_shape, 0) = 2;
    SHPSEG_SHAPE (tmp_old_shape, 1) = 3;
    SHPSEG_SHAPE (tmp_old_shape, 2) = 4;

    tmp_new_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_new_shape, 0) = 5;
    SHPSEG_SHAPE (tmp_new_shape, 1) = 7;
    SHPSEG_SHAPE (tmp_new_shape, 2) = 4;

    PIaddInferredShape (3, T_int, tmp_old_shape, tmp_new_shape);

    tmp_old_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_old_shape, 0) = 2;
    SHPSEG_SHAPE (tmp_old_shape, 1) = 3;

    tmp_new_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_new_shape, 0) = 5;
    SHPSEG_SHAPE (tmp_new_shape, 1) = 7;

    PIaddInferredShape (2, T_int, tmp_old_shape, tmp_new_shape);

    tmp_old_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_old_shape, 0) = 3;
    SHPSEG_SHAPE (tmp_old_shape, 1) = 4;

    tmp_new_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_new_shape, 0) = 5;
    SHPSEG_SHAPE (tmp_new_shape, 1) = 6;

    PIaddInferredShape (2, T_int, tmp_old_shape, tmp_new_shape);

    tmp_old_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_old_shape, 0) = 3;
    SHPSEG_SHAPE (tmp_old_shape, 1) = 4;

    tmp_new_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_new_shape, 0) = 6;
    SHPSEG_SHAPE (tmp_new_shape, 1) = 7;

    PIaddInferredShape (2, T_float, tmp_old_shape, tmp_new_shape);

    tmp_old_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_old_shape, 0) = 3;
    SHPSEG_SHAPE (tmp_old_shape, 1) = 4;

    tmp_new_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_new_shape, 0) = 8;
    SHPSEG_SHAPE (tmp_new_shape, 1) = 9;

    PIaddInferredShape (2, T_double, tmp_old_shape, tmp_new_shape);

    tmp_old_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_old_shape, 0) = 8;
    SHPSEG_SHAPE (tmp_old_shape, 1) = 9;

    tmp_new_shape = MakeShpseg (NULL);
    SHPSEG_SHAPE (tmp_new_shape, 0) = 10;
    SHPSEG_SHAPE (tmp_new_shape, 1) = 11;

    PIaddInferredShape (2, T_int, tmp_old_shape, tmp_new_shape);

    /* apply array padding */

    /* APtransform(arg_node); */

    /* free pad_info structure */
    PIfree ();

    DBUG_RETURN (arg_node);
}
