
/*
 *
 * $Log$
 * Revision 1.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 1.1  1999/06/16 17:18:13  rob
 * Initial revision
 *
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
    z = C_lastc;
    while ((i != C_lastc) && (z == C_lastc)) {
        if (0 == strncmp (nt + nc, nt_class_str[i], 3))
            z = i;
        i++;
    }
    DBUG_ASSERT ((z != C_lastc), "ICUNameClass did not find valid Name Class");

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   uniqueness_class_t ICUUniClass( char *nt)
 *
 * description:
 *   Returns the Uniqueness Class of an array or other object from an nt
 *
 ******************************************************************************/

uniqueness_class_t
ICUUniClass (char *nt)
{
    int nc, i;
    uniqueness_class_t z;
    DBUG_ENTER ("ICUUniClass");
    nc = 1 + FindParen (nt, 1 + NT_UNI_INDEX);
    i = 0;
    z = C_lastu;
    while ((i != C_lastu) && (z == C_lastu)) {
        if (0 == strncmp (nt + nc, nt_uni_str[i], 3))
            z = i;
        i++;
    }
    DBUG_ASSERT ((z != C_lastu), "ICUUniClass did not find valid Uniqueness Class");

    DBUG_RETURN (z);
}
