/*
 * $Log$
 * Revision 1.4  2004/11/01 21:51:43  sah
 * added SerStackDump
 *
 * Revision 1.3  2004/09/23 21:12:25  sah
 * ongoing implementation
 *
 * Revision 1.2  2004/09/21 16:34:27  sah
 * ongoing implementation of
 * serialize traversal
 *
 * Revision 1.1  2004/09/21 10:18:46  sah
 * Initial revision
 *
 *
 *
 */

#include "serialize_stack.h"
#include "dbug.h"
#include "internal_lib.h"
#include "tree_basic.h"

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

    if (ptr == NULL)
        pos = SERSTACK_NOT_FOUND;

    DBUG_RETURN (pos);
}

node *
SerStackLookup (int pos, serstack_t *stack)
{
    int cnt = 0;
    serentry_t *ptr = stack->head;
    node *result;

    DBUG_ENTER ("SerStackLookup");

    while ((cnt < pos) && (ptr != NULL)) {
        ptr = ptr->next;
        cnt++;
    }

    DBUG_ASSERT ((cnt == pos), "stack selection out of bounds.");

    if (ptr == NULL) {
        result = NULL;
    } else {
        result = ptr->val;
    }

    DBUG_RETURN (result);
}

void
SerStackDump (serstack_t *stack)
{
    serentry_t *ptr = stack->head;

    DBUG_ENTER ("SerStackDump");

    printf ("StackDump:\n\n");

    while (ptr != NULL) {
        printf ("- " F_PTR " (", ptr->val);

        if (ptr->val != NULL) {
            printf ("%s )\n", NODE_TEXT (ptr->val));
        } else {
            printf ("-- )\n");
        }

        ptr = ptr->next;
    }

    DBUG_VOID_RETURN;
}
