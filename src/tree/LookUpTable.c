/*
 *
 * $Log$
 * Revision 1.3  2000/02/03 08:35:20  dkr
 * GenLUT renamed to GenerateLUT
 *
 * Revision 1.2  2000/01/31 20:18:57  dkr
 * support for hashing added
 *
 * Revision 1.1  2000/01/28 12:33:14  dkr
 * Initial revision
 *
 */

#include <string.h>

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"

/*
 * Size of a single hash table.
 * (-> # pairs of data that can be stored in the table.)
 */
#define LUT_SIZE 5

/*
 * number of different hash values (-> tables)
 */
#define LUT_KEYS 1

/*
 * a single hash table
 *
 * 'first' points to the first item of the table.
 * 'act' points to the next free position of the table.
 * All elements of the table lie between these two addresses. The pairs of data
 * are stored as two consecutive table entries.
 * The table is of a static size. If the current table is filled, the address
 * of a newly allocated table is stored in the last entry of the current table.
 * 'size' contains the number of items stored in the whole chain of tables.
 */
typedef struct LUT_T {
    void **first;
    void **act;
    long size;
} single_lut_t;

/*
 * an array of size LUT_KEYS containing the hash tables.
 */
typedef single_lut_t *lut_t;

/******************************************************************************
 *
 * function:
 *   long GetHashKey( void *data)
 *
 * description:
 *   Calculates the hash key for a given object.
 *   For the return value the condition  0 <= return < LUT_KEYS  must be hold.
 *
 ******************************************************************************/

static long
GetHashKey (void *data)
{
    long hash_key = 0;

    DBUG_ENTER ("GetHashKey");

    DBUG_ASSERT ((data != NULL), "NULL has no hash key!");
    DBUG_ASSERT (((hash_key >= 0) && (hash_key < LUT_KEYS)), "hash key out of bounds!");

    DBUG_RETURN (hash_key);
}

/******************************************************************************
 *
 * function:
 *   lut_t *GenerateLUT( void)
 *
 * description:
 *   Generates a new LUT: All the needed hash tables are created and the
 *   internal data structures are initialized.
 *
 ******************************************************************************/

lut_t *
GenerateLUT (void)
{
    lut_t *lut;
    long k;

    DBUG_ENTER ("GenerateLUT");

    lut = (lut_t *)MALLOC (LUT_KEYS * sizeof (lut_t));
    for (k = 0; k < LUT_KEYS; k++) {
        lut[k] = (single_lut_t *)MALLOC (sizeof (single_lut_t));
        lut[k]->first = (void **)MALLOC ((2 * LUT_SIZE + 1) * sizeof (void *));
        lut[k]->act = lut[k]->first;
        lut[k]->size = 0;
    }
    DBUG_PRINT ("LUT", ("finished"));

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *RemoveLUT( lut_t *lut)
 *
 * description:
 *   Removes the given LUT from memory.
 *
 ******************************************************************************/

lut_t *
RemoveLUT (lut_t *lut)
{
    void **tmp;
    long k, i;

    DBUG_ENTER ("RemoveLUT");

    for (k = 0; k < LUT_KEYS; k++) {
        DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
        for (i = 0; i <= lut[k]->size / (2 * LUT_SIZE); i++) {
            tmp = lut[k]->first;
            lut[k]->first = lut[k]->first[2 * LUT_SIZE];
            FREE (tmp);
        }
        FREE (lut[k]);
    }
    FREE (lut);
    DBUG_PRINT ("LUT", ("finished"));

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT( lut_t *lut, void *old_entry, void *new_entry)
 *
 * description:
 *   Inserts the given pair of data (old_entry, new_entry) into the correct
 *   hash table of the LUT.
 *
 ******************************************************************************/

lut_t *
InsertIntoLUT (lut_t *lut, void *old_entry, void *new_entry)
{
    long k;

    DBUG_ENTER ("InsertIntoLUT");

    if (lut != NULL) {
        k = GetHashKey (old_entry);
        *(lut[k]->act++) = old_entry;
        *(lut[k]->act++) = new_entry;
        lut[k]->size += 2;
        DBUG_PRINT ("LUT", ("new nodes inserted (hash key %li) -> [ 0x%p , 0x%p ]", k,
                            old_entry, new_entry));

        if (lut[k]->size % (2 * LUT_SIZE) == 0) {
            /*
             * the last table entry has been used -> allocate a new one.
             */
            *lut[k]->act = (void **)MALLOC (((2 * LUT_SIZE) + 1) * sizeof (void *));
            DBUG_PRINT ("LUT", ("new LUT created -> 0x%p", lut[k]->act));
            /*
             * move 'act' to the first entry of the new table.
             */
            lut[k]->act = *lut[k]->act;
        }

        DBUG_PRINT ("LUT", ("finished: new LUT size -> %li", lut[k]->size));
    } else {
        DBUG_PRINT ("LUT", ("FAILED"));
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   void *SearchInLUT( lut_t *lut, void *old_entry)
 *
 * description:
 *   Searches for the given data in the LUT.
 *   If the given data is *not* found in the LUT, the same data is returned.
 *   Otherwise the data associated with the found entry is returned.
 *
 ******************************************************************************/

void *
SearchInLUT (lut_t *lut, void *old_entry)
{
    void *new_entry;
    void **tmp;
    long k, i;

    DBUG_ENTER ("SearchInLUT");

    new_entry = old_entry;
    if (lut != NULL) {
        if (old_entry != NULL) {
            k = GetHashKey (old_entry);
            tmp = lut[k]->first;
            for (i = 0; i < lut[k]->size; i += 2) {
                if (tmp[0] == old_entry) {
                    new_entry = tmp[1];
                    break;
                }
                tmp += 2;
                if ((i + 2) % (2 * LUT_SIZE) == 0) {
                    /*
                     * the last table entry is reached
                     *  -> enter the next table of the chain
                     */
                    tmp = *tmp;
                }
            }
            if (new_entry == old_entry) {
                DBUG_PRINT ("LUT", ("finished: node 0x%p (hash key %li) *not* found :-(",
                                    old_entry, k));
            } else {
                DBUG_PRINT ("LUT", ("finished: node 0x%p (hash key %li) found -> 0x%p",
                                    old_entry, k, new_entry));
            }
        } else {
            DBUG_PRINT ("LUT", ("finished: node is NULL"));
        }
    } else {
        DBUG_PRINT ("LUT", ("FAILED"));
    }

    DBUG_RETURN (new_entry);
}

/******************************************************************************
 *
 * function:
 *   void PrintLUT( FILE *handle, lut_t *lut)
 *
 * description:
 *   Prints the contents of the given LUT.
 *
 ******************************************************************************/

void
PrintLUT (FILE *handle, lut_t *lut)
{
    long k, i;
    void **tmp;

    DBUG_ENTER ("PrintLUT");

    if (handle == NULL) {
        handle = stderr;
    }

    if (lut != NULL) {
        for (k = 0; k < LUT_KEYS; k++) {
            fprintf (handle, "*** hash key %li ***\n", k);
            tmp = lut[k]->first;
            for (i = 0; i < lut[k]->size; i += 2) {
                fprintf (handle, "%li %li: [ 0x%p ] - [ 0x%p ]\n", i, i + 1, tmp[0],
                         tmp[1]);
                tmp += 2;
                if ((i + 2) % (2 * LUT_SIZE) == 0) {
                    /*
                     * the last table entry is reached
                     *  -> enter the next table of the chain
                     */
                    tmp = *tmp;
                }
            }
            fprintf (handle, "number of entries: %li\n", lut[k]->size);
        }
        DBUG_PRINT ("LUT", ("finished"));
    } else {
        DBUG_PRINT ("LUT", ("FAILED"));
    }

    DBUG_VOID_RETURN;
}
