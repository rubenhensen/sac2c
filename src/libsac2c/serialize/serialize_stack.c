#include "serialize_stack.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
SSinit (void)
{
    serstack_t *result;

    DBUG_ENTER ();

    result = (serstack_t *)MEMmalloc (sizeof (serstack_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

serstack_t *
SSdestroy (serstack_t *stack)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    tmp = (serentry_t *)MEMmalloc (sizeof (serentry_t));

    tmp->val = val;
    tmp->next = stack->head;
    stack->head = tmp;

    DBUG_RETURN ();
}

node *
SSpop (serstack_t *stack)
{
    serentry_t *tmp;
    node *result;

    DBUG_ENTER ();

    DBUG_ASSERT (stack->head != NULL, "cannot pop element from empty stack");

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    while ((cnt < pos) && (ptr != NULL)) {
        ptr = ptr->next;
        cnt++;
    }

    DBUG_ASSERT (cnt == pos, "stack selection out of bounds.");

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

    DBUG_ENTER ();

    printf ("StackDump:\n\n");

    while (ptr != NULL) {
        printf ("- " F_PTR " (", (void *)ptr->val);

        if (ptr->val != NULL) {
            printf ("%s )\n", NODE_TEXT (ptr->val));
        } else {
            printf ("-- )\n");
        }

        ptr = ptr->next;
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
