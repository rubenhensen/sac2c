/*
 *
 * $Log$
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
 * Revision 1.3  1999/06/25 15:22:32  rob
 * Don't gen if not TAGGED_ARRAYS
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
 *   int FindParen( char *nt, int n)
 *
 * description:
 *   Returns the offset of the n-th left parenthesis within nt.
 *   It makes NO checks for right parentheses nor for quotes, etc.
 *
 ******************************************************************************/

static int
FindParen (char *nt, int n)
{
    int i;

    DBUG_ENTER ("FindParen");

    DBUG_ASSERT ((nt != NULL), "FindParen was called with NULL nt");

    for (i = 0; nt[i] != '\0'; i++) {
        if (nt[i] == '(') {
            n--;
            if (n == 0) {
                break;
            }
        }
    }

    DBUG_ASSERT ((nt[i] != '\0'), "FindParen() did not find the parenthesis");

    DBUG_RETURN (i);
}

/******************************************************************************
 *
 * function:
 *   data_class_t ICUGetDataClass( char *nt)
 *
 * description:
 *   Returns the class tag of an array or other object
 *
 ******************************************************************************/

data_class_t
ICUGetDataClass (char *nt)
{
    int nc, i;
    data_class_t z;

    DBUG_ENTER ("ICUGetDataClass");

    nc = FindParen (nt, NT_DATA_INDEX + 1) + 1;
    i = 0;
    z = C_unknownd;
    while ((i != C_unknownd) && (z == C_unknownd)) {
        if (!strncmp (nt + nc, nt_data_string[i], 3)) {
            z = i;
        }
        i++;
    }

    DBUG_ASSERT ((z != C_unknownd), "ICUGetDataClass() did not find valid class tag");

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unq_class_t ICUGetUnqClass( char *nt)
 *
 * description:
 *   Returns the uniqueness tag of an array or other object
 *
 ******************************************************************************/

unq_class_t
ICUGetUnqClass (char *nt)
{
    int nc, i;
    unq_class_t z;

    DBUG_ENTER ("ICUGetUnqClass");

    nc = FindParen (nt, NT_UNQ_INDEX + 1) + 1;
    i = 0;
    z = C_unknownu;
    while ((i != C_unknownu) && (z == C_unknownu)) {
        if (!strncmp (nt + nc, nt_unq_string[i], 3)) {
            z = i;
        }
        i++;
    }

    DBUG_ASSERT ((z != C_unknownu), "ICUGetUnqClass() did not find valid uniqueness tag");

    DBUG_RETURN (z);
}
