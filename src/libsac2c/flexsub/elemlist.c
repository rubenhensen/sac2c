#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "str.h"
#include "dbug.h"
#include "memory.h"
#include "tree_compound.h"
#include "globals.h"
#include "dynelem.h"
#include "elemlist.h"

void
ELinit (elemlist *el)
{

    DBUG_ENTER ("ELinit");

    ELEMLIST_CURR (el) = NULL;
    ELEMLIST_PREV (el) = NULL;
    ELEMLIST_NEXT (el) = NULL;

    DBUG_VOID_RETURN;
}

elemlist *
ELfreeNonRecursive (elemlist *el)
{

    DBUG_ENTER ("ELfreeNonRecursive");

    if (el != NULL) {
        el = MEMfree (el);
    }

    DBUG_RETURN (el);
}

elemlist *
ELfreeRecursive (elemlist *el)
{

    DBUG_ENTER ("ELfreeRecursive");

    if (ELEMLIST_CURR (el) == NULL) {
        freeElem (ELEMLIST_CURR (el));
    }

    el = ELfreeNonRecursive (el);

    DBUG_RETURN (el);
}
