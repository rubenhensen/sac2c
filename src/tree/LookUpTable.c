/*
 *
 * $Log$
 * Revision 1.1  2000/01/28 12:33:14  dkr
 * Initial revision
 *
 *
 */

#include <string.h>

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

#define LUT_MAX (2 * 5)

typedef struct LUT_T {
    void **first;
    void **act;
    long size;
} lut_t;

lut_t *
GenLUT (void)
{
    lut_t *lut;

    DBUG_ENTER ("GenLUT");

    lut = (lut_t *)MALLOC (sizeof (lut_t));
    lut->first = (void **)MALLOC ((LUT_MAX + 1) * sizeof (void *));
    lut->act = lut->first;
    lut->size = 0;
    DBUG_PRINT ("LUT", ("finished"));

    DBUG_RETURN (lut);
}

lut_t *
RemoveLUT (lut_t *lut)
{
    void **tmp;
    long i;

    DBUG_ENTER ("RemoveLUT");

    DBUG_ASSERT ((lut != NULL), "lut is NULL!");
    for (i = 0; i <= lut->size / LUT_MAX; i++) {
        tmp = lut->first;
        lut->first = lut->first[LUT_MAX];
        FREE (tmp);
    }
    FREE (lut);
    DBUG_PRINT ("LUT", ("finished"));

    DBUG_RETURN (lut);
}

lut_t *
InsertIntoLUT (lut_t *lut, void *old_entry, void *new_entry)
{
    DBUG_ENTER ("InsertIntoLUT");

    if (lut != NULL) {
        *(lut->act++) = old_entry;
        *(lut->act++) = new_entry;
        lut->size += 2;
        DBUG_PRINT ("LUT",
                    ("new nodes inserted -> [ 0x%p , 0x%p ]", old_entry, new_entry));

        if (lut->size % LUT_MAX == 0) {
            *lut->act = (void **)MALLOC ((LUT_MAX + 1) * sizeof (void *));
            DBUG_PRINT ("LUT", ("new LUT created -> 0x%p", lut->act));
            lut->act = *lut->act;
        }

        DBUG_PRINT ("LUT", ("finished: new LUT size -> %i", lut->size));
    } else {
        DBUG_PRINT ("LUT", ("FAILED"));
    }

    DBUG_RETURN (lut);
}

void *
SearchInLUT (lut_t *lut, void *old_entry)
{
    void *new_entry;
    void **tmp;
    long i;

    DBUG_ENTER ("SearchInLUT");

    new_entry = old_entry;
    if (lut != NULL) {
        if (old_entry != NULL) {
            tmp = lut->first;
            for (i = 0; i < lut->size; i += 2) {
                if (tmp[0] == old_entry) {
                    new_entry = tmp[1];
                    break;
                }
                tmp += 2;
                if ((i + 2) % LUT_MAX == 0) {
                    tmp = *tmp;
                }
            }
            if (new_entry == old_entry) {
                DBUG_PRINT ("LUT", ("finished: node 0x%p *not* found :-(", old_entry));
            } else {
                DBUG_PRINT ("LUT",
                            ("finished: node 0x%p found -> 0x%p", old_entry, new_entry));
            }
        } else {
            DBUG_PRINT ("LUT", ("finished: node is NULL"));
        }
    } else {
        DBUG_PRINT ("LUT", ("FAILED"));
    }

    DBUG_RETURN (new_entry);
}

void
PrintLUT (FILE *handle, lut_t *lut)
{
    long i;
    void **tmp;

    DBUG_ENTER ("PrintLUT");

    if (handle == NULL) {
        handle = stderr;
    }

    if (lut != NULL) {
        tmp = lut->first;
        for (i = 0; i < lut->size; i += 2) {
            fprintf (handle, "%li %li: [ 0x%p ] - [ 0x%p ]\n", i, i + 1, tmp[0], tmp[1]);
            tmp += 2;
            if ((i + 2) % LUT_MAX == 0) {
                tmp = *tmp;
            }
        }
        fprintf (handle, "number of entries: %li\n", lut->size);
        DBUG_PRINT ("LUT", ("finished"));
    } else {
        DBUG_PRINT ("LUT", ("FAILED"));
    }

    DBUG_VOID_RETURN;
}
