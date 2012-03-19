/*
 * $Id$
 */

#include "stringset.h"

#define DBUG_PREFIX "STRS"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "check_mem.h"

/*
 * stringset routines
 */

bool
STRScontains (const char *string, stringset_t *set)
{
    bool result;

    DBUG_ENTER ();

    if (set == NULL) {
        result = FALSE;
    } else {
        if (STReq (set->val, string)) {
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
    DBUG_ENTER ();

    if (!STRScontains (string, set)) {
        stringset_t *new = MEMmalloc (sizeof (stringset_t));

        DBUG_PRINT ("adding %s.", string);

        new->val = STRcpy (string);
        new->kind = kind;
        new->next = set;

        set = new;
    }

    DBUG_RETURN (set);
}

void *
STRSfold (strsfoldfun_p fun, stringset_t *set, void *init)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (set != NULL) {
        set->val = MEMfree (set->val);
        set->next = STRSfree (set->next);
        set = MEMfree (set);
    }

    DBUG_RETURN (set);
}

void
STRStouch (stringset_t *set, info *arg_info)
{
    DBUG_ENTER ();

    if (set != NULL) {
        CHKMtouch (set->val, arg_info);
        STRStouch (set->next, arg_info);
        CHKMtouch (set, arg_info);
    }

    DBUG_RETURN ();
}

void *
STRSprintFoldFun (const char *entry, strstype_t kind, void *rest)
{
    DBUG_ENTER ();

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

stringset_t *
STRSduplicate (stringset_t *src)
{
    stringset_t *result = NULL;

    DBUG_ENTER ();

    if (src != NULL) {
        result = MEMmalloc (sizeof (stringset_t));

        result->val = STRcpy (src->val);
        result->kind = src->kind;
        result->next = STRSduplicate (src->next);
    }

    DBUG_RETURN (result);
}

void
STRSprint (stringset_t *set)
{
    DBUG_ENTER ();

    STRSfold (&STRSprintFoldFun, set, NULL);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
