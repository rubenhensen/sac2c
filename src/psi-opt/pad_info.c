/*
 * $Log$
 * Revision 1.6  2000/07/06 12:44:25  mab
 * added PIgetOldTypes
 *
 * Revision 1.5  2000/07/05 09:13:10  mab
 * fixed problem with global data structure pad_info
 *
 * Revision 1.4  2000/06/30 15:21:01  mab
 * *** empty log message ***
 *
 * Revision 1.3  2000/06/28 10:43:10  mab
 * made some code modifications according to code review
 *
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
#include "DupTree.h"

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

pad_info_t *pad_info;

/*****************************************************************************
 *
 * function:
 *   static int EqualShapes(int dim, shpseg* shape2, shpseg* shape1)
 *
 * description:
 *   compares two shapes, result is TRUE, if shapes are equal
 *   !internal function only!
 *
 *****************************************************************************/

static int
EqualShapes (int dim, shpseg *shape2, shpseg *shape1)
{

    int equalShapes = TRUE;
    int i;

    DBUG_ENTER ("EqualShapes");

    i = 0;
    while (i < dim && equalShapes) {
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
 *   static pad_info_t *GetNewTableEntry(types *old_type)
 *
 * description:
 *   returns pointer to table entry corrsponding to old_type or NULL
 *   !internal function only!
 *
 *****************************************************************************/

static pad_info_t *
GetNewTableEntry (types *old_type)
{

    pad_info_t *tmp = pad_info;
    pad_info_t *matching_entry = NULL;

    DBUG_ENTER ("GetNewTableEntry");

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TYPES_BASETYPE (old_type))
            && (PI_DIM (tmp) == TYPES_DIM (old_type))
            && EqualShapes (PI_DIM (tmp), PI_OLD_SHAPE (tmp), TYPES_SHPSEG (old_type))) {

            matching_entry = tmp;
            tmp = NULL;
        } else {
            tmp = PI_NEXT (tmp);
        }
    }

    DBUG_RETURN (matching_entry);
}

/*****************************************************************************
 *
 * function:
 *   static pad_info_t *GetOldTableEntry(types *new_type)
 *
 * description:
 *   returns pointer to table entry corrsponding to new_type or NULL
 *   !internal function only!
 *
 *****************************************************************************/

static pad_info_t *
GetOldTableEntry (types *new_type)
{

    pad_info_t *tmp = pad_info;
    pad_info_t *matching_entry = NULL;

    DBUG_ENTER ("GetOldTableEntry");

    while (tmp != NULL) {
        if ((PI_TYPE (tmp) == TYPES_BASETYPE (new_type))
            && (PI_DIM (tmp) == TYPES_DIM (new_type))
            && EqualShapes (PI_DIM (tmp), PI_NEW_SHAPE (tmp), TYPES_SHPSEG (new_type))) {

            matching_entry = tmp;
            tmp = NULL;
        } else {
            tmp = PI_NEXT (tmp);
        }
    }

    DBUG_RETURN (matching_entry);
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

    tmp = (pad_info_t *)MALLOC (sizeof (pad_info_t));
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
 *   types* PIgetNewType(types* old_type)
 *
 * description:
 *   returns type-structure with padded shape
 *   or NULL, if old_type can't be padded
 *   renaming has to be done separately, because only N_arg and N_vardec
 *   store their names in types-structure, N_id and ids-attribute do not!
 *   !!! old_type is set free here !!!
 *
 *****************************************************************************/

types *
PIgetNewType (types *old_type)
{

    types *new_type = NULL;
    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetNewType");

    table_entry = GetNewTableEntry (old_type);

    if (table_entry != NULL) {
        new_type = DupTypes (old_type);
        FreeShpseg (TYPES_SHPSEG (new_type));
        TYPES_SHPSEG (new_type) = DupShpSeg (PI_NEW_SHAPE (table_entry));
        FreeOneTypes (old_type);
    }

    DBUG_RETURN (new_type);
}

/*****************************************************************************
 *
 * function:
 *   types* PIgetOldType(types* new_type)
 *
 * description:
 *   returns type-structure with unpadded shape
 *   or NULL, if new_type has no padded shape
 *   renaming has to be done separately, because only N_arg and N_vardec
 *   store their names in types-structure, N_id and ids-attribute do not!
 *   !!! new_type is set free here !!!
 *
 *****************************************************************************/

types *
PIgetOldType (types *new_type)
{

    types *old_type = NULL;
    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetOldType");

    table_entry = GetOldTableEntry (new_type);

    if (table_entry != NULL) {
        old_type = DupTypes (new_type);
        FreeShpseg (TYPES_SHPSEG (old_type));
        TYPES_SHPSEG (old_type) = DupShpSeg (PI_OLD_SHAPE (table_entry));
        FreeOneTypes (new_type);
    }

    DBUG_RETURN (old_type);
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFUndefPad(types *old_type)
 *
 * description:
 *   return pointer to fundef-node of padding-function
 *
 *****************************************************************************/

node *
PIgetFundefPad (types *old_type)
{
    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetFundefPad");

    table_entry = GetNewTableEntry (old_type);

    DBUG_RETURN ((table_entry != NULL) ? PI_FUNDEF_PAD (table_entry) : NULL);
}

/*****************************************************************************
 *
 * function:
 *   node* PIgetFundefUnpad(pad_info_t* pi)
 *
 * description:
 *   return pointer to fundef-node of unpadding-function
 *
 *****************************************************************************/

node *
PIgetFundefUnpad (types *old_type)
{

    pad_info_t *table_entry;

    DBUG_ENTER ("PIgetFundefUnpad");

    table_entry = GetNewTableEntry (old_type);

    DBUG_RETURN ((table_entry != NULL) ? PI_FUNDEF_UNPAD (table_entry) : NULL);
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
        FREE (current);

        current = next;
    }

    DBUG_VOID_RETURN;
}
