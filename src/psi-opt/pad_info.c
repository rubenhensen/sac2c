/*
 * $Log$
 * Revision 1.1  2000/06/08 11:20:16  mab
 * Initial revision
 *
 */

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"

#include "pad_info.h"

#define PI_DIM(p) p->dim
#define PI_TYPE(p) p->type
#define PI_OLD_SHAPE(p) p->old_shape
#define PI_NEW_SHAPE(p) p->new_shape
#define PI_NEXT(p) p->next

/* to do:

*/

/* private */
int
PIequalShapes (pad_info_t *pi, nums *old_shape)
{
    nums *tmp1 = old_shape;
    nums *tmp2 = PI_OLD_SHAPE (pi);
    int equalShapes = TRUE;

    DBUG_ENTER ("PIequalShapes");

    while (tmp1 != NULL && tmp2 != NULL) {
        if (NUMS_NUM (tmp1) != NUMS_NUM (tmp2)) {
            equalShapes = FALSE;
            tmp1 = NULL;
            tmp2 = NULL;
        } else {
            tmp1 = NUMS_NEXT (tmp1);
            tmp2 = NUMS_NEXT (tmp2);
            if (tmp1 == NULL || tmp2 == NULL) {
                equalShapes = FALSE;
            }
        }
    }

    DBUG_RETURN (equalShapes);
}

/* public */
int
PIgetDim (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetDim");

    DBUG_RETURN (PI_DIM (pi));
}

/* public */
simpletype
PIgetType (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetType");

    DBUG_RETURN (PI_TYPE (pi));
}

/* public */
nums *
PIgetOldShape (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetOldShape");

    DBUG_RETURN (PI_OLD_SHAPE (pi));
}

/* public */
nums *
PIgetNewShape (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetNewShape");

    DBUG_RETURN (PI_NEW_SHAPE (pi));
}

/* public */
void
PIadd (simpletype type, int dim, nums *old_shape, nums *new_shape)
{

    pad_info_t *tmp;

    DBUG_ENTER ("PIadd");

    tmp = (pad_info_t *)malloc (sizeof (pad_info_t));
    PI_DIM (tmp) = dim;
    PI_TYPE (tmp) = type;
    PI_OLD_SHAPE (tmp) = old_shape;
    PI_NEW_SHAPE (tmp) = new_shape;
    PI_NEXT (tmp) = pad_info;
    pad_info = tmp;

    DBUG_VOID_RETURN;
}

/* public */
int
PIvalid (pad_info_t *pi)
{

    int result;

    DBUG_ENTER ("PIvalid");

    result = FALSE;
    if (pi != NULL)
        result = TRUE;

    DBUG_RETURN (result);
}

/* public */
pad_info_t *
PInewShape (int dim, simpletype type, nums *old_shape)
{
    pad_info_t *tmp = pad_info;
    pad_info_t *res = NULL;

    DBUG_ENTER ("PInewShape");

    while (tmp != NULL) {
        if ((PI_DIM (tmp) == dim) && (PI_TYPE (tmp) == type)
            && PIequalShapes (tmp, old_shape) == TRUE) {
            res = tmp;
            tmp = NULL;
        }
        tmp = PI_NEXT (tmp);
    }

    DBUG_RETURN (res);
}
