/*
 *
 * $Log$
 * Revision 3.8  2004/11/25 10:26:46  jhb
 * compile SACdevCamp 2k4
 *
 * Revision 3.7  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.6  2003/09/19 15:32:11  dkr
 * postfix _nt of varnames renamed into _NT
 *
 * Revision 3.5  2002/07/31 15:37:19  dkr
 * new hidden tag added
 *
 * Revision 3.4  2002/06/04 08:35:45  dkr
 * C_unknownc renamed into C_unknownd
 *
 * Revision 3.3  2002/06/02 21:36:56  dkr
 * functions renamed
 *
 * Revision 3.2  2002/05/31 17:21:29  dkr
 * functions renamed
 *
 * Revision 3.1  2000/11/20 18:01:19  sacbase
 * new release made
 *
 * Revision 1.6  2000/10/23 12:46:15  dkr
 * FindParen(): In the 2nd DBUG_ASSERT 'NULL' is replaced by '\0'.
 *
 * Revision 1.5  2000/09/13 15:05:09  dkr
 * C_last? renamed into C_unknown?
 *
 * Revision 1.4  2000/08/17 10:19:10  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 1.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 1.1  1999/06/16 17:18:13  rob
 * Initial revision
 *
 */

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

#include <string.h>

#include "globals.h"
#include "types.h"
#include "icm2c_utils.h"
#include "dbug.h"

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

    DBUG_ENTER ("FindParen");

    DBUG_ASSERT ((var_NT != NULL), "FindParen was called with NULL var_NT");

    for (i = 0; var_NT[i] != '\0'; i++) {
        if (var_NT[i] == '(') {
            n--;
            if (n == 0) {
                break;
            }
        }
    }

    DBUG_ASSERT ((var_NT[i] != '\0'), "FindParen() did not find the parenthesis");

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

    DBUG_ENTER ("ICUGetShapeClass");

    nc = FindParen (var_NT, NT_SHAPE_INDEX + 1) + 1;
    i = 0;
    z = C_unknowns;
    while ((i != C_unknowns) && (z == C_unknowns)) {
        if (!strncmp (var_NT + nc, global.nt_shape_string[i], 3)) {
            z = i;
        }
        i++;
    }

    DBUG_ASSERT ((z != C_unknowns), "ICUGetShapeClass() did not find valid shape tag");

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

    DBUG_ENTER ("ICUGetHiddenClass");

    nc = FindParen (var_NT, NT_HIDDEN_INDEX + 1) + 1;
    i = 0;
    z = C_unknownh;
    while ((i != C_unknownh) && (z == C_unknownh)) {
        if (!strncmp (var_NT + nc, global.nt_hidden_string[i], 3)) {
            z = i;
        }
        i++;
    }

    DBUG_ASSERT ((z != C_unknownh), "ICUGetHiddenClass() did not find valid hidden tag");

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

    DBUG_ENTER ("ICUGetUniqueClass");

    nc = FindParen (var_NT, NT_UNIQUE_INDEX + 1) + 1;
    i = 0;
    z = C_unknownu;
    while ((i != C_unknownu) && (z == C_unknownu)) {
        if (!strncmp (var_NT + nc, global.nt_unique_string[i], 3)) {
            z = i;
        }
        i++;
    }

    DBUG_ASSERT ((z != C_unknownu),
                 "ICUGetUniqueClass() did not find valid uniqueness tag");

    DBUG_RETURN (z);
}
