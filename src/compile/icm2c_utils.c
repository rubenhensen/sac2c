/*
 *
 * $Log$
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
 *
 *   This file contains utility functions used by ICMs.
 *
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
 *   int FindParen(char *nt,int n)
 * description:
 *  Returns the offset of the nTH left parenthesis within nt.
 *  It makes NO checks for right parentheses nor for quotes, etc.
 *
 ******************************************************************************/

static int
FindParen (char *nt, int n)
{
    int i;

    DBUG_ENTER ("FindParen");

    DBUG_ASSERT ((NULL != nt), "FindParen was called with NULL nt");
    for (i = 0; '\0' != nt[i]; i++) {
        if ('(' == nt[i]) {
            n--;
            if (0 == n)
                break;
        }
    }
    DBUG_ASSERT ((NULL != nt[i]), "FindParen did not find the paren");

    DBUG_RETURN (i);
}

/******************************************************************************
 *
 * function:
 *   data_class_t ICUNameClass( char *nt)
 *
 * description:
 *   Returns the Data Class of an array or other object from an nt
 *
 ******************************************************************************/

data_class_t
ICUNameClass (char *nt)
{
    int nc, i;
    data_class_t z;

    DBUG_ENTER ("ICUNameClass");

    nc = 1 + FindParen (nt, 1 + NT_CLASS_INDEX);
    i = 0;
    z = C_unknownc;
    while ((i != C_unknownc) && (z == C_unknownc)) {
        if (0 == strncmp (nt + nc, nt_class_string[i], 3))
            z = i;
        i++;
    }
    DBUG_ASSERT ((z != C_unknownc), "ICUNameClass did not find valid Name Class");

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   unq_class_t ICUUnqClass( char *nt)
 *
 * description:
 *   Returns the Uniqueness Class of an array or other object from an nt
 *
 ******************************************************************************/

unq_class_t
ICUUnqClass (char *nt)
{
    int nc, i;
    unq_class_t z;

    DBUG_ENTER ("ICUUnqClass");

    nc = 1 + FindParen (nt, 1 + NT_UNQ_INDEX);
    i = 0;
    z = C_unknownu;
    while ((i != C_unknownu) && (z == C_unknownu)) {
        if (0 == strncmp (nt + nc, nt_unq_string[i], 3))
            z = i;
        i++;
    }
    DBUG_ASSERT ((z != C_unknownu), "ICUUnqClass did not find valid Uniqueness Class");

    DBUG_RETURN (z);
}
