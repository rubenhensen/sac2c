/*
 *
 * $Log$
 * Revision 1.2  1994/12/20 17:34:35  hw
 * bug fixed in FreeIdsOnly
 * exchanged stdio.h with stdlib.h
 *
 * Revision 1.1  1994/12/20  15:42:10  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>

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

    DBUG_ENTER ("FreeIdsOnly");

    if (ids->next != NULL)
        FreeIdsOnly (ids->next);
    free (ids);

    DBUG_VOID_RETURN;
}
