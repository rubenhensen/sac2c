/*
 *
 * $Log$
 * Revision 1.7  2004/11/24 17:57:18  sah
 * COMPILES.
 *
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
    strstype_t kind;
    stringset_t *next;
};

bool
STRScontains (const char *string, stringset_t *set)
{
    bool result;

    DBUG_ENTER ("STRScontains");

    if (set == NULL) {
        result = FALSE;
    } else {
        if (!strcmp (set->val, string)) {
            result = TRUE;
        } else {
            result = STRScontains (string, set->next);
        }
    }

    DBUG_RETURN (result);
}

stringset_t *
STRSadd (const char *string, strstype_t kind, stringset_t *set)
{
    DBUG_ENTER ("STRSadd");

    if (!STRScontains (string, set)) {
        stringset_t *new = ILIBmalloc (sizeof (stringset_t));

        DBUG_PRINT ("STRS", ("adding %s.", string));

        new->val = ILIBstringCopy (string);
        new->kind = kind;
        new->next = set;

        set = new;
    }

    DBUG_RETURN (set);
}

void *
STRSfold (strsfoldfun_p fun, stringset_t *set, void *init)
{
    DBUG_ENTER ("STRSfold");

    while (set != NULL) {
        init = fun (set->val, set->kind, init);
        set = set->next;
    }

    DBUG_RETURN (init);
}

stringset_t *
STRSjoin (stringset_t *one, stringset_t *two)
{
    stringset_t *result;

    DBUG_ENTER ("STRSjoin");

    result = one;

    while (two != NULL) {
        stringset_t *act = two;
        two = two->next;

        if (STRScontains (act->val, result)) {
            act->next = NULL;
            act = STRSfree (act);
        } else {
            act->next = result;
            result = act;
        }
    }

    DBUG_RETURN (result);
}

stringset_t *
STRSfree (stringset_t *set)
{
    DBUG_ENTER ("STRSfree");

    if (set != NULL) {
        set->val = ILIBfree (set->val);
        set->next = STRSfree (set->next);
        set = ILIBfree (set);
    }

    DBUG_RETURN (set);
}

void *
STRSprintFoldFun (const char *entry, strstype_t kind, void *rest)
{
    DBUG_ENTER ("STRSprintFoldFun");

    printf ("%s ", entry);

    switch (kind) {
    case STRS_saclib:
        printf ("(sac library)\n");
        break;
    case STRS_extlib:
        printf ("(external library)\n");
        break;
    default:
        printf ("(unknown)\n");
        break;
    }

    DBUG_RETURN ((void *)0);
}

void
STRSprint (stringset_t *set)
{
    DBUG_ENTER ("STRSprint");

    STRSfold (&STRSprintFoldFun, set, NULL);

    DBUG_VOID_RETURN;
}
