#include <string.h>

#define DBUG_PREFIX "MEM_ALLOC"
#include "debug.h"
#include "memory.h"

#include "ctinfo.h"
#include "globals.h"
#include "convert.h"
#include "uthash.h"
#include "phase_info.h"

#undef malloc
#undef realloc
#undef free

#include <stdlib.h>

mallocinfo_t *malloctable = 0;
mallocphaseinfo_t phasetable[PH_final + 1] = {{0, 0, 0, 0, 0, 0, 0, 0}};

static FILE *mreport = 0;

void *
_MEMmalloc (size_t size, const char *file, int line, const char *func)
{
#ifndef DBUG_OFF
    void *ptr;
    mallocinfo_t *info;

    DBUG_ENTER ();

    if (!(size > 0)) {
        ptr = NULL;
    } else {
        ptr = malloc (size);
        if (!ptr) {
            CTIabortOutOfMemory (size);
        }

        if (global.memcheck) {
            info = malloc (sizeof (mallocinfo_t));
            if (!info) {
                CTIabortOutOfMemory (sizeof (mallocinfo_t));
            }

            global.current_allocated_mem += size;
            global.max_allocated_mem
              = global.current_allocated_mem > global.max_allocated_mem
                  ? global.current_allocated_mem
                  : global.max_allocated_mem;

            info->size = size;
            info->phase = global.compiler_anyphase;
            info->isfreed = FALSE;
            info->isnode = FALSE;
            info->wasintree = FALSE;
            info->isreachable = FALSE;
            info->line = line;
            info->file = file;
            info->callingfunc = func;
            info->occurrence = 1;

            phasetable[info->phase].nmallocd++;

            global.memcheck = FALSE;

            /*
             * Store the info in a ascosiative hashtable with the malloc adress as key
             * and info adress as value
             */
            info->key = ptr;
            HASH_ADD_PTR (malloctable, key, info);
            global.memcheck = TRUE;
        }
    }

    DBUG_RETURN (ptr);
#else
    return malloc (size);
#endif
}

void *
_MEMrealloc (void *ptr, size_t size)
{
#ifndef DBUG_OFF
    mallocinfo_t *info, *newinfo;
    void *newptr;

    if (global.memcheck) {
        HASH_FIND_PTR (malloctable, &ptr, info);
        if (info) {
            newptr = MEMmalloc (size);
            memcpy (newptr, ptr, info->size);
            HASH_FIND_PTR (malloctable, &newptr, newinfo);
            newinfo->phase = info->phase;
            MEMfree (ptr);
        } else {
            newptr = realloc (ptr, size);
        }
    } else {
        newptr = realloc (ptr, size);
    }

    if (!newptr) {
        CTIabortOutOfMemory (sizeof (mallocinfo_t));
    }

    return newptr;
#else
    return realloc (ptr, size);
#endif
}

void *
_MEMfree (void *ptr)
{
#ifndef DBUG_OFF
    mallocinfo_t *info;

    if (global.memcheck) {
        HASH_FIND_PTR (malloctable, &ptr, info);
        if (info) {
            phasetable[info->phase].nfreed++;
            global.current_allocated_mem -= info->size;
            HASH_DEL (malloctable, info);
            free (info);
        }
    }
#endif
    free (ptr);
    return NULL;
}

void *
foldmallocreport (void *init, void *key, void *value)
{
    mallocinfo_t *info = value;
    mallocinfo_t *iterator;
    bool ispresent = FALSE;
    if (info->phase > PH_final + 1) {
        CTInote ("corrupted mallocinfo, ignoring ...");
    } else {

        iterator = phasetable[info->phase].notfreed;
        while (iterator) {
            if ((!strcmp (iterator->file, info->file)) && iterator->line == info->line) {
                iterator->occurrence++;
                iterator->size += info->size;
                ispresent = TRUE;
                break;
            }
            iterator = iterator->next;
        }

        if (!ispresent) {
            info->next = phasetable[info->phase].notfreed;
            phasetable[info->phase].notfreed = info;
        }
        phasetable[info->phase].notfreedsize += info->size;
    }

    return NULL;
}

/* 
 * This needs to return int as used as __compare_fn_t for qsort which has def:
 * typedef int (*__compar_fn_t) (const void *, const void *); 
 */
static int
SortMemreport (const void *a, const void *b)
{
    const mallocphaseinfo_t *aa = a;
    const mallocphaseinfo_t *bb = b;
    size_t a_size = aa->leakedsize, b_size = bb->leakedsize;

    return  a_size < b_size 
            ? -1 
            : a_size > b_size
              ? 1
              : 0;
}

node *
MEMreport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    char *name = 0;
    mallocinfo_t *iterator;

    if (mreport == NULL) {
        name = MEMmalloc ((strlen (global.outfilename) + 9) * sizeof (char));
        sprintf (name, "%s.mreport", global.outfilename);
        mreport = fopen (name, "w");
    }

    global.memcheck = FALSE;
    for (mallocinfo_t *iter = malloctable; iter != NULL; iter = iter->hh.next) {
        foldmallocreport (NULL, NULL, iter);
    }
    global.memcheck = TRUE;
    for (int i = 0; i < PH_final + 1; i++) {
        phasetable[i].phase = i;
    }
    qsort (phasetable, PH_final + 1, sizeof (mallocphaseinfo_t), SortMemreport);

    for (int i = 0; i <= PH_final; i++) {
        fprintf (mreport, "** description: %s\n", PHIphaseText (phasetable[i].phase));
        fprintf (mreport, "     ident: %s, leaked: %d, total bytes leaked %zu\n",
                 PHIphaseIdent (phasetable[i].phase), phasetable[i].nleaked,
                 phasetable[i].leakedsize);
        iterator = phasetable[i].leaked;
        if (iterator) {
            fprintf (mreport, "\n  ** The following mallocs where leaked during the "
                              "traversal of this phase\n");
        }
        while (iterator) {
            fprintf (mreport,
                     "     ** file: %s, line: %d, occurrence: %d, size: %zu, from phase: "
                     "%s, from func: %s\n",
                     iterator->file, iterator->line, iterator->occurrence, iterator->size,
                     PHIphaseIdent (iterator->phase), iterator->callingfunc);
            iterator = iterator->next;
        }
        iterator = phasetable[i].notfreed;
        if (iterator) {
            fprintf (mreport,
                     "\n  ** Total malloced in this phase: %d, Total freed from this "
                     "phase: %d\n",
                     phasetable[i].nmallocd, phasetable[i].nleaked);
            fprintf (mreport,
                     "  ** The following mallocs from this phase where not freed\n");
        }
        while (iterator) {
            fprintf (mreport, "     ** file: %s, line: %d, occurrence: %d, size: %zu\n",
                     iterator->file, iterator->line, iterator->occurrence,
                     iterator->size);
            iterator = iterator->next;
        }
        fprintf (mreport, "\n");
    }

    DBUG_RETURN (arg_node);
}

/*
 * XXX Why this function (argument-wise) is different from the memcpy?
 * Can we unify them?
 */
void *
MEMcopy (size_t size, void *mem)
{
    void *result;

    DBUG_ENTER ();

    result = MEMmalloc (size);

    result = memcpy (result, mem, size);

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
