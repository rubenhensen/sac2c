/*
 *
 * $Log$
 * Revision 1.6  2004/11/07 18:06:28  sah
 * added support for different stringkinds
 *
 * Revision 1.5  2004/11/04 14:53:43  sah
 * implemented dependencies between modules
 *
 * Revision 1.4  2004/11/01 21:49:59  sah
 * added SSPrint
 *
 * Revision 1.3  2004/10/28 22:07:43  sah
 * added missing include
 *
 * Revision 1.2  2004/10/28 17:22:37  sah
 * now, internally a copy of the string is used
 *
 * Revision 1.1  2004/10/27 13:10:38  sah
 * Initial revision
 *
 *
 *
 */

#include "stringset.h"
#include "dbug.h"
#include "internal_lib.h"

#include <string.h>

/*
 * stringset routines
 */

struct STRINGSET_T {
    char *val;
    SStype_t kind;
    stringset_t *next;
};

bool
SSContains (const char *string, stringset_t *set)
{
    bool result;

    DBUG_ENTER ("SSContains");

    if (set == NULL) {
        result = FALSE;
    } else {
        if (!strcmp (set->val, string)) {
            result = TRUE;
        } else {
            result = SSContains (string, set->next);
        }
    }

    DBUG_RETURN (result);
}

stringset_t *
SSAdd (const char *string, SStype_t kind, stringset_t *set)
{
    DBUG_ENTER ("SSAdd");

    if (!SSContains (string, set)) {
        stringset_t *new = Malloc (sizeof (stringset_t));

        DBUG_PRINT ("SS", ("adding %s.", string));

        new->val = StringCopy (string);
        new->kind = kind;
        new->next = set;

        set = new;
    }

    DBUG_RETURN (set);
}

void *
SSFold (SSfoldfun_p fun, stringset_t *set, void *init)
{
    DBUG_ENTER ("SSFold");

    while (set != NULL) {
        init = fun (set->val, set->kind, init);
        set = set->next;
    }

    DBUG_RETURN (init);
}

stringset_t *
SSJoin (stringset_t *one, stringset_t *two)
{
    stringset_t *result;

    DBUG_ENTER ("SSJoin");

    result = one;

    while (two != NULL) {
        stringset_t *act = two;
        two = two->next;

        if (SSContains (act->val, result)) {
            act->next = NULL;
            act = SSFree (act);
        } else {
            act->next = result;
            result = act;
        }
    }

    DBUG_RETURN (result);
}

stringset_t *
SSFree (stringset_t *set)
{
    DBUG_ENTER ("SSFree");

    if (set != NULL) {
        set->val = Free (set->val);
        set->next = SSFree (set->next);
        set = Free (set);
    }

    DBUG_RETURN (set);
}

void *
SSPrintFoldFun (const char *entry, SStype_t kind, void *rest)
{
    DBUG_ENTER ("SSPrintFoldFun");

    printf ("%s ", entry);

    switch (kind) {
    case SS_saclib:
        printf ("(sac library)\n");
        break;
    case SS_extlib:
        printf ("(external library)\n");
        break;
    default:
        printf ("(unknown)\n");
        break;
    }

    DBUG_RETURN ((void *)0);
}

void
SSPrint (stringset_t *set)
{
    DBUG_ENTER ("SSPrint");

    SSFold (&SSPrintFoldFun, set, NULL);

    DBUG_VOID_RETURN;
}
