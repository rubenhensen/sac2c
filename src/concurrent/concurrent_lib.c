/*
 *
 * $Log$
 * Revision 1.2  1999/07/30 13:45:44  jhs
 * Added initial macros and functions.
 *
 * Revision 1.1  1999/07/30 12:34:50  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   concurrent_lib.c
 *
 * prefix: CONL
 *
 * description:
 *   helper functions for concurrent-compilation
 *
 *****************************************************************************/

#include "dbug.h"

#include "DataFlowMask.h"

/******************************************************************************
 *
 * function:
 *   void CONLDisplayMask (char *tag, char *name, DFMmask_t mask)
 *
 * description:
 *   Prints out the names of variables contained in mask.
 *   The printout is coupled to DBUG_PRINT with tag, so prints happen only
 *   with this tag given for debugging.
 *   name is used for specify print out.
 ******************************************************************************/
void
CONLDisplayMask (char *tag, char *name, DFMmask_t mask)
{
    char *varname;

    DBUG_ENTER ("CONLDisplayMask");

    DBUG_PRINT (tag, ("%s-mask begin", name));
    varname = DFMGetMaskEntryNameSet (mask);
    while (varname != NULL) {
        DBUG_PRINT (tag, ("%s", varname));
        varname = DFMGetMaskEntryNameSet (NULL);
    }
    DBUG_PRINT (tag, ("%s-mask end", name));

    DBUG_VOID_RETURN;
}
