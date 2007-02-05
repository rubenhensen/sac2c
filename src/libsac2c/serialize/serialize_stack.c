/*
 * $Log$
 * Revision 1.2  2004/11/26 21:18:50  sah
 * pour Bodo *<8-)
 *
 * Revision 1.1  2004/11/23 22:41:06  sah
 * Initial revision
 *
 *
 *
 */

#include "serialize_stack.h"
#include "dbug.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "globals.h"

typedef struct SERENTRY_T serentry_t;

struct SERENTRY_T {
    node *val;
    serentry_t *next;
};

struct SERSTACK_T {
    serentry_t *head;
};

serstack_t *
SSinit ()
{
    serstack_t *result;

    DBUG_ENTER ("SSinit");

    result = MEMmalloc (sizeof (serstack_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

serstack_t *
SSdestroy (serstack_t *stack)
{
    DBUG_ENTER ("SSdestroy");

    while (stack->head != NULL) {
        serentry_t *tmp = stack->head;
        stack->head = stack->head->next;

        tmp = MEMfree (tmp);
    }

    stack = MEMfree (stack);

    DBUG_RETURN (stack);
}

void
SSpush (node *val, serstack_t *stack)
{
    serentry_t *tmp;

    DBUG_ENTER ("SSpush");

    tmp = MEMmalloc (sizeof (serentry_t));

    tmp->val = val;
    tmp->next = stack->head;
    stack->head = tmp;

    DBUG_VOID_RETURN;
}

node *
SSpop (serstack_t *stack)
{
    serentry_t *tmp;
    node *result;

    DBUG_ENTER ("SSpop");

    DBUG_ASSERT ((stack->head != NULL), "cannot pop element from empty stack");

    tmp = stack->head;
    stack->head = stack->head->next;

    result = tmp->val;
    tmp = MEMfree (tmp);

    DBUG_RETURN (result);
}

int
SSfindPos (node *val, serstack_t *stack)
{
    int pos = 0;
    serentry_t *ptr;

    DBUG_ENTER ("SSfindPos");

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
SSlookup (int pos, serstack_t *stack)
{
    int cnt = 0;
    serentry_t *ptr = stack->head;
    node *result;

    DBUG_ENTER ("SSlookup");

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
SSdump (serstack_t *stack)
{
    serentry_t *ptr = stack->head;

    DBUG_ENTER ("SSdump");

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
