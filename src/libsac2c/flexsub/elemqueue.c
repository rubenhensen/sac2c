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
#include "elemqueue.h"
#include "elemlist.h"

void
EQenqueue (elemqueue *q, elem *e)
{

    DBUG_ENTER ("ELQenqueue");

    elemlist *head, *el;

    head = ELEMQUEUE_HEAD (q);

    el = MEMmalloc (sizeof (elemlist));
    ELinit (el);

    ELEMLIST_CURR (el) = e;
    ELEMLIST_NEXT (el) = head;

    ELEMLIST_PREV (head) = el;

    ELEMQUEUE_HEAD (q) = el;

    DBUG_VOID_RETURN;
}

elem *
EQdequeue (elemqueue *q)
{

    DBUG_ENTER ("ELQdequeue");

    elemlist *tail, *prev;
    elem *e;

    tail = ELEMQUEUE_TAIL (q);
    prev = ELEMLIST_PREV (tail);

    ELEMLIST_NEXT (prev) = NULL;
    e = ELEMLIST_CURR (tail);

    tail = ELfreeNonRecursive (tail);

    DBUG_RETURN (result);
}
