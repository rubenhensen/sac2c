/*
 *
 * $Log$
 * Revision 1.1  1994/12/20 15:42:10  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>

#include "dbug.h"
#include "tree.h"

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
FreeIdsOnly (ids *ids)

{

    DBUG_ENTER ("FreeIds");

    if (ids->next != NULL)
        FreeIds (ids->next);
    free (ids);

    DBUG_VOID_RETURN;
}
