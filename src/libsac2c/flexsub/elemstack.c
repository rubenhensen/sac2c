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
#include "elemstack.h"

int
isElemstackEmpty (elemstack *s)
{

    DBUG_ENTER ("isElemstackEmpty");

    int result = 0;

    if (ELEMSTACK_CURR (s) == NULL) {
        result = 1;
    }

    DBUG_RETURN (result);
}

void
initElemstack (elemstack *s)
{

    ELEMSTACK_CURR (s) = NULL;
    ELEMSTACK_NEXT (s) = NULL;
}

void
pushElemstack (elemstack **s, elem *e)
{

    elemstack *top = MEMmalloc (sizeof (elemstack));
    ELEMSTACK_CURR (top) = e;
    ELEMSTACK_NEXT (top) = *s;
    *s = top;
}

elem *
popElemstack (elemstack **s)
{

    elemstack *top = NULL;
    elem *e;

    if (*s == NULL) {
        CTIabort ("Trying to pop from empty elemstack\n");
    } else {
        top = *s;
        *s = ELEMSTACK_NEXT (top);
    }

    e = ELEMSTACK_CURR (top);
    top = MEMfree (top);

    return e;
}
