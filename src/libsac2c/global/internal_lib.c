/*
 */
#include "str.h"
#include "memory.h"

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "dbug.h"

#include "ctinfo.h"
#include "globals.h"
#include "traverse.h"
#include "filemgr.h"
#include "math_utils.h"

/*
 * experimental support for garbage collection
 */

#ifdef GC
#include <gc.h>
#define malloc(n) GC_malloc (n)
#endif /* GC */

/******************************************************************************
 *
 * Function:
 *   char *TRAVtmpVar( void)
 *
 * Description:
 *   Generates string to be used as artificial variable.
 *   The variable name is different in each call of TRAVtmpVar().
 *   The string has the form "__tmp_" ++ traversal ++ consecutive number.
 *
 ******************************************************************************/

char *
TRAVtmpVar (void)
{
    static int counter = 0;
    const char *prefix;
    char *result;

    DBUG_ENTER ("TRAVtmpVar");

    prefix = TRAVgetName ();
    result = (char *)MEMmalloc ((strlen (prefix) + MATHnumDigits (counter) + 3)
                                * sizeof (char));
    sprintf (result, "_%s_%d", prefix, counter);
    counter++;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *TRAVtmpVarName( char* postfix)
 *
 * description:
 *   creates a unique variable like TRAVtmpVar() and additionally appends
 *   an individual string.
 *
 ******************************************************************************/

char *
TRAVtmpVarName (char *postfix)
{
    const char *tmp;
    char *result, *prefix;

    DBUG_ENTER ("TRAVtmpVarName");

    /* avoid chains of same prefixes */
    tmp = TRAVgetName ();

    if ((strlen (postfix) > (strlen (tmp) + 1)) && (postfix[0] == '_')
        && (strncmp ((postfix + 1), tmp, strlen (tmp)) == 0)) {
        postfix = postfix + strlen (tmp) + 2;
        while (postfix[0] != '_') {
            postfix++;
        }
    }

    prefix = TRAVtmpVar ();

    result = STRcatn (3, prefix, "_", postfix);

    MEMfree (prefix);

    DBUG_RETURN (result);
}
