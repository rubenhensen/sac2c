/*
 *
 * $Log$
 * Revision 1.1  2004/10/27 13:10:38  sah
 * Initial revision
 *
 *
 *
 */

#include "stringset.h"
#include "dbug.h"
#include "internal_lib.h"

/*
 * stringset routines
 */

struct STRINGSET_T {
    const char *val;
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
SSAdd (const char *string, stringset_t *set)
{
    DBUG_ENTER ("SSAdd");

    if (!SSContains (string, set)) {
        stringset_t *new = Malloc (sizeof (stringset_t));

        new->val = string;
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
        init = fun (set->val, init);
        set = set->next;
    }

    DBUG_RETURN (init);
}

stringset_t *
SSFree (stringset_t *set)
{
    DBUG_ENTER ("SSFree");

    if (set != NULL) {
        set->next = SSFree (set->next);
        set = Free (set);
    }

    DBUG_RETURN (set);
}
