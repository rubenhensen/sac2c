/*         $Id$
 *
 * $Log$
 * Revision 1.1  1998/05/13 13:47:44  srs
 * Initial revision
 *
 *
 */

/*******************************************************************************

  This file make the following functions available:
  - Check whether WL (genarray, modarray, fold) can be unrolled
  - Execution of WL unrolling (genarray, modarray, fold)

  Theses functions are called from Unroll.c

 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "tree.h"
#include "free.h"
#include "Error.h"
#include "dbug.h"
#include "traverse.h"
#include "internal_lib.h"

#include "optimize.h"
#include "DupTree.h"

/******************************************************************************
 *
 * function:
 *   int CheckUnrollModarray(node *wln)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

int
CheckUnrollModarray (node *wln)
{
    DBUG_ENTER ("CheckUnrollModarray");

    DBUG_RETURN (1);
}

/******************************************************************************
 *
 * function:
 *   node *DoUnrollModarray(node *wln)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
DoUnrollModarray (node *wln)
{
    DBUG_ENTER ("DoUnrollModarray");

    DBUG_RETURN (wln);
}

int
CheckUnrollGenarray (node *wln)
{
    return 1;
}
node *
DoUnrollGenarray (node *wln)
{
    return wln;
}

int
CheckUnrollFold (node *wln)
{
    return 1;
}
node *
DoUnrollFold (node *wln)
{
    return wln;
}
