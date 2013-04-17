/*****************************************************************************
 *
 * file:   icm2c_utils.c
 *
 * prefix: ICU
 *
 * description:
 *   This file contains utility functions used by ICMs.
 *
 *****************************************************************************/

#include "globals.h"
#include "types.h"
#include "str.h"
#include "icm2c_utils.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

/******************************************************************************
 *
 * function:
 *   int FindParen( char *var_NT, int n)
 *
 * description:
 *   Returns the offset of the n-th left parenthesis within var_NT.
 *   It makes NO checks for right parentheses nor for quotes, etc.
 *
 ******************************************************************************/

static int
FindParen (char *var_NT, int n)
{
    int i;

    DBUG_ENTER ();

    DBUG_ASSERT (var_NT != NULL, "FindParen was called with NULL var_NT");

    for (i = 0; var_NT[i] != '\0'; i++) {
        if (var_NT[i] == '(') {
            n--;
            if (n == 0) {
                break;
            }
        }
    }

    DBUG_ASSERT (var_NT[i] != '\0', "FindParen() did not find the parenthesis");

    DBUG_RETURN (i);
}

/******************************************************************************
 *
 * function:
 *   shape_class_t ICUGetShapeClass( char *var_NT)
 *
 * description:
 *   Returns the class tag of an array or other object
 *
 ******************************************************************************/

shape_class_t
ICUGetShapeClass (char *var_NT)
{
    int nc, i;
    shape_class_t z;

    DBUG_ENTER ();

    nc = FindParen (var_NT, NT_SHAPE_INDEX + 1) + 1;
    i = 0;
    z = C_unknowns;
    while ((i != C_unknowns) && (z == C_unknowns)) {
        if (STReqn (var_NT + nc, global.nt_shape_string[i], 3)) {
            z = (shape_class_t)i;
        }
        i++;
    }

    DBUG_ASSERT (z != C_unknowns, "ICUGetShapeClass() did not find valid shape tag");

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   hidden_class_t ICUGetHiddenClass( char *var_NT)
 *
 * description:
 *   Returns the hidden tag of an array or other object
 *
 ******************************************************************************/

hidden_class_t
ICUGetHiddenClass (char *var_NT)
{
    int nc, i;
    hidden_class_t z;

    DBUG_ENTER ();

    nc = FindParen (var_NT, NT_HIDDEN_INDEX + 1) + 1;
    i = 0;
    z = C_unknownh;
    while ((i != C_unknownh) && (z == C_unknownh)) {
        if (STReqn (var_NT + nc, global.nt_hidden_string[i], 3)) {
            z = (hidden_class_t)i;
        }
        i++;
    }

    DBUG_ASSERT (z != C_unknownh, "ICUGetHiddenClass() did not find valid hidden tag");

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unique_class_t ICUGetUniqueClass( char *var_NT)
 *
 * description:
 *   Returns the uniqueness tag of an array or other object
 *
 ******************************************************************************/

unique_class_t
ICUGetUniqueClass (char *var_NT)
{
    int nc, i;
    unique_class_t z;

    DBUG_ENTER ();

    nc = FindParen (var_NT, NT_UNIQUE_INDEX + 1) + 1;
    i = 0;
    z = C_unknownu;
    while ((i != C_unknownu) && (z == C_unknownu)) {
        if (STReqn (var_NT + nc, global.nt_unique_string[i], 3)) {
            z = (unique_class_t)i;
        }
        i++;
    }

    DBUG_ASSERT (z != C_unknownu,
                 "ICUGetUniqueClass() did not find valid uniqueness tag");

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
