/*
 *
 * $Log$
 * Revision 1.4  1994/12/30 16:59:33  sbs
 * added FreeIds
 *
 * Revision 1.3  1994/12/20  17:42:23  hw
 * added includes dbug.h & stdio.h
 *
 * Revision 1.2  1994/12/20  17:34:35  hw
 * bug fixed in FreeIdsOnly
 * exchanged stdio.h with stdlib.h
 *
 * Revision 1.1  1994/12/20  15:42:10  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>
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
FreeIds (ids *ids)

{

    DBUG_ENTER ("FreeIds");

    if (ids->next != NULL)
        FreeIds (ids->next);
    free (ids->id);
    free (ids);

    DBUG_VOID_RETURN;
}

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
FreeImplist (node *implist)

{
    int i;

    DBUG_ENTER ("FreeImplist");

    if (implist->node[0] != NULL)
        FreeImplist (implist->node[0]);
    for (i = 1; i < 4; i++)
        if (implist->node[i] != NULL)
            FreeIds ((ids *)implist->node[i]);
    free (implist);

    DBUG_VOID_RETURN;
}
