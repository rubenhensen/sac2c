/*
 * $Log$
 * Revision 1.1  1999/10/19 13:02:59  sacbase
 * Initial revision
 *
 *
 */

#include "tree_basic.h"
#include "dbug.h"

#include "cv2scalar.h"

#define TYP_IFcv2scal(fun) fun
cv2scalarfunptr cv2scalar[] = {
#include "type_info.mac"
};

/*
 * Functions for converting a single element from a Constant-Vector (void *)
 * into a scalar node from the AST!
 */

node *
COCv2Num (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Num");

    DBUG_RETURN (MakeNum (((int *)elems)[offset]));
}

node *
COCv2Double (void *elems, int offset)
{
    DBUG_ENTER ("COCv2Num");

    DBUG_RETURN (MakeDouble (((double *)elems)[offset]));
}

node *
COCv2ScalarDummy (void *elems, int offset)
{
    DBUG_ASSERT ((1 == 0), "COCv2SCalarDummy called!");
    return (NULL);
}
