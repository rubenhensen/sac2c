/*
 * $Log$
 * Revision 1.2  2000/06/14 10:45:04  mab
 * added methods for accessing data structure
 *
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
#define PI_FUNDEF_PAD(p) p->fundef_pad
#define PI_FUNDEF_UNPAD(p) p->fundef_unpad
#define PI_NEXT(p) p->next

/*****************************************************************************
 *
 * file:   pad_info.c
 *
 * prefix: PI
 *
 * description:
 *
 *   This is an abstract data structure for storing information needed by
 *   the array padding.
 *
 *
 *****************************************************************************/

/*****************************************************************************
 *
 * function:
 *   int PIequalShapes(int dim, shpseg* shape2, shpseg* shape1)
 *
 * description:
 *   compares two shapes, result is TRUE, if shapes are equal
 *   !internal function only!
 *
 *****************************************************************************/

int
PIequalShapes (int dim, shpseg *shape2, shpseg *shape1)
{
    int equalShapes = TRUE;
    int i;

    DBUG_ENTER ("PIequalShapes");

    i = 0;
    while (i < dim && equalShapes == TRUE) {
        if (SHPSEG_SHAPE (shape1, i) != SHPSEG_SHAPE (shape2, i)) {
            equalShapes = FALSE;
        }
        i++;
    }

    DBUG_RETURN (equalShapes);
}

/*****************************************************************************
 *
 * function:
 *   int PIgetDim(pad_info_t* pi)
 *
 * description:
 *   return dimension of a pad_info_t*
 *
 *****************************************************************************/

int
PIgetDim (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetDim");

    DBUG_RETURN (PI_DIM (pi));
}

/*****************************************************************************
 *
 * function:
 *   simpletype PIgetType(pad_info_t* pi)
 *
 * description:
 *   return simpletype of a pad_info_t*
 *
 *****************************************************************************/

simpletype
PIgetType (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetType");

    DBUG_RETURN (PI_TYPE (pi));
}

/*****************************************************************************
 *
 * function:
 *   shpseg* PIgetOldShape(pad_info_t* pi)
 *
 * description:
 *   return pointer to original shape of a pad_info_t*
 *
 *****************************************************************************/

shpseg *
PIgetOldShape (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetOldShape");

    DBUG_RETURN (PI_OLD_SHAPE (pi));
}

/*****************************************************************************
 *
 * function:
 *   shpseg* PIgetNewShape(pad_info_t* pi)
 *
 * description:
 *   return pointer to newly inferred shape of a pad_info_t*
 *
 *****************************************************************************/

shpseg *
PIgetNewShape (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetNewShape");

    DBUG_RETURN (PI_NEW_SHAPE (pi));
}

/*****************************************************************************
 *
 * function:
 *   shpseg* PIcopyNewShape(pad_info_t* pi)
 *
 * description:
 *   return copy of new shape contained in pad_info_t
 *
 *****************************************************************************/

shpseg *
PIcopyNewShape (pad_info_t *pi)
{
    shpseg *res;
    int i;

    DBUG_ENTER ("PIcopyNewShape");

    res = (shpseg *)malloc (sizeof (shpseg *));
    SHPSEG_NEXT (res) = NULL;
    for (i = 0; i < SHP_SEG_SIZE; i++) {
        SHPSEG_SHAPE (res, i) = SHPSEG_SHAPE (PI_NEW_SHAPE (pi), i);
    }

    DBUG_RETURN (res);
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFUndefPad(pad_info_t* pi)
 *
 * description:
 *   return pointer to fundef-node of padding-function
 *
 *****************************************************************************/

node *
PIgetFundefPad (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetFundefPad");

    DBUG_RETURN (PI_FUNDEF_PAD (pi));
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFUndefUnpad(pad_info_t* pi)
 *
 * description:
 *   return pointer to fundef-node of unpadding-function
 *
 *****************************************************************************/

node *
PIgetFUndefUnpad (pad_info_t *pi)
{
    DBUG_ENTER ("PIgetFundefUnpad");

    DBUG_RETURN (PI_FUNDEF_UNPAD (pi));
}

/*****************************************************************************
 *
 * function:
 *   void PIinit()
 *
 * description:
 *   initialize abstract data structure
 *   !!! need to call this before use of any other functions !!!
 *
 *****************************************************************************/

void
PIinit ()
{
    DBUG_ENTER ("PIinit");

    pad_info = NULL;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIfree()
 *
 * description:
 *   free abstract data structure
 *   call this after finishing work mith pad_info
 *
 *****************************************************************************/

void
PIfree ()
{
    pad_info_t *next;
    pad_info_t *current;

    DBUG_ENTER ("PIfree");

    current = pad_info;

    while (current != NULL) {
        next = PI_NEXT (current);

        PI_OLD_SHAPE (current) = NULL;
        PI_NEW_SHAPE (current) = NULL;
        free (current);

        current = next;
    }
    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   void PIadd(types* old_type, shpseg* new_shape)
 *
 * description:
 *   add a new entry to the data structure for a newly inferred type
 *
 *****************************************************************************/

void
PIadd (types *old_type, shpseg *new_shape)
{

    pad_info_t *tmp;

    DBUG_ENTER ("PIadd");

    tmp = (pad_info_t *)malloc (sizeof (pad_info_t));
    PI_DIM (tmp) = TYPES_DIM (old_type);
    PI_TYPE (tmp) = TYPES_BASETYPE (old_type);
    PI_OLD_SHAPE (tmp) = TYPES_SHPSEG (old_type);
    PI_NEW_SHAPE (tmp) = new_shape;
    PI_FUNDEF_PAD (tmp) = NULL;
    PI_FUNDEF_UNPAD (tmp) = NULL;
    PI_NEXT (tmp) = pad_info;
    pad_info = tmp;

    DBUG_VOID_RETURN;
}

/*****************************************************************************
 *
 * function:
 *   int PIvalid(pad_info_t* pi)
 *
 * description:
 *   check, if a pad_info_t* points to a valid structure
 *   you may want to call this after use of PInewShape
 *
 *****************************************************************************/

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

/*****************************************************************************
 *
 * function:
 *   pad_info_t* PInewShape(types* old_type)
 *
 * description:
 *   return newly inferred shape for a given old shape
 *
 *****************************************************************************/

pad_info_t *
PInewShape (types *old_type)
{
    pad_info_t *tmp = pad_info;
    pad_info_t *res = NULL;

    DBUG_ENTER ("PInewShape");

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TYPES_BASETYPE (old_type))
            && (PI_DIM (tmp) == TYPES_DIM (old_type))
            && PIequalShapes (PI_DIM (tmp), PI_OLD_SHAPE (tmp), TYPES_SHPSEG (old_type))
                 == TRUE) {
            res = tmp;
            tmp = NULL;
        } else {
            tmp = PI_NEXT (tmp);
        }
    }

    DBUG_RETURN (res);
}
