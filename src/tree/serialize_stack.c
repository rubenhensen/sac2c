/*
 * $Log$
 * Revision 1.1  2004/09/21 10:18:46  sah
 * Initial revision
 *
 *
 *
 */

#include "serialize_stack.h"
#include "dbug.h"
#include "internal_lib.h"

typedef struct SERENTRY_T serentry_t;

struct SERENTRY_T {
    node *val;
    serentry_t *next;
};

struct SERSTACK_T {
    serentry_t *head;
};

serstack_t *
SerStackInit ()
{
    serstack_t *result;

    DBUG_ENTER ("SerStackInit");

    result = Malloc (sizeof (serstack_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

serstack_t *
SerStackDestroy (serstack_t *stack)
{
    DBUG_ENTER ("SerStackDestroy");

    while (stack->head != NULL) {
        serentry_t *tmp = stack->head;
        stack->head = stack->head->next;

        tmp = Free (tmp);
    }

    stack = Free (stack);

    DBUG_RETURN (stack);
}

void
SerStackPush (node *val, serstack_t *stack)
{
    serentry_t *tmp;

    DBUG_ENTER ("SerStackPush");

    tmp = Malloc (sizeof (serentry_t));

    tmp->val = val;
    tmp->next = stack->head;
    stack->head = tmp;

    DBUG_VOID_RETURN;
}

node *
SerStackPop (serstack_t *stack)
{
    serentry_t *tmp;
    node *result;

    DBUG_ENTER ("SerStackPop");

    DBUG_ASSERT ((stack->head != NULL), "cannot pop element from empty stack");

    tmp = stack->head;
    stack->head = stack->head->next;

    result = tmp->val;
    tmp = Free (tmp);

    DBUG_RETURN (result);
}

int
SerStackFindPos (node *val, serstack_t *stack)
{
    int pos = 0;
    serentry_t *ptr;

    DBUG_ENTER ("SerStackFindPos");

    ptr = stack->head;

    while (ptr != NULL) {
        if (ptr->val == val)
            break;

        ptr = ptr->next;
        pos++;
    }

    DBUG_ASSERT ((ptr != NULL), "cannot find element on stack");

    DBUG_RETURN (pos);
}
