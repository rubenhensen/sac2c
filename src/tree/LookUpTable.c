/*
 *
 * $Log$
 * Revision 3.3  2001/03/22 17:56:14  dkr
 * hash key functions implemented :-)
 *
 * Revision 3.2  2001/03/22 13:30:52  dkr
 * support for strings added
 *
 * Revision 3.1  2000/11/20 18:03:21  sacbase
 * new release made
 *
 * Revision 1.6  2000/07/12 13:37:26  dkr
 * lut->size counts the number of pointer-PAIRS now
 * Output of PrintLUT modified
 *
 * Revision 1.5  2000/07/12 12:24:34  dkr
 * DBUG_ASSERT in InsertIntoLUT added
 *
 * Revision 1.4  2000/03/17 18:31:54  dkr
 * traverse.h no longer included
 *
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

/*
 *  Look-Up-Table (LUT) for Pointers and Strings
 *  --------------------------------------------
 *
 *  Each entry of a LUT can hold a pair [old, new] of pointers (void*) or
 *  strings (char*).
 *
 *  To insert a pair of pointers or strings into the LUT use the function
 *  InsertIntoLUT_P()  or  InsertIntoLUT_S()  respectively.
 *
 *  The function  SearchInLUT_?( lut, old)  searches the LUT for an entry
 *  [old, new]. If the LUT contains such an entry the associated data
 *  'new' is returned. Otherwise the return value equals 'old'.
 *  SearchInLUT_P()  searches for a pointer (pointer compare),
 *  SearchInLUT_S()  searches for a string (string compare).
 *
 *  *** CAUTION ***
 *  - InsertIntoLUT_S() copies the strings before inserting them into the LUT.
 *  - RemoveLUT() removes all the stored strings from heap memory.
 *  - SearchInLUT_S() returns a pointer to the argument string (if no string
 *    entry of the same name is found in the LUT) or a *pointer* to the found
 *    look-up string. In the latter case the returned pointer will be undefined
 *    if RemoveLUT() has been called.
 *
 *
 *  Implementation
 *  --------------
 *
 *  The pairs of data are stored in hash tables. Pointers and strings are
 *  stored in disjoint hash tables. The pointers are stored in the tables with
 *  index [ 0 .. LUT_KEYS_POINTERS [, the strings are stored in the tables with
 *  index [ LUT_KEYS_POINTERS .. LUT_KEYS [.
 *  The functions GetHashKey_Pointer() and GetHashKey_String() compute for a
 *  pointer and string respectively the index of the hash table the pointer/
 *  string belongs to.
 *  Each hash table is made up of linked table fragments of fixed size
 *  (LUT_SIZE) in order to minimize the data overhead. Initially each hash
 *  table consists of a single table fragment. Each time the last entry of a
 *  hash table has been occupied another fragment is allocated and appended
 *  to the hash table.
 *
 *  The amount of memory (in byte) needed for all hash tables equals
 *
 *            (K - 1)
 *    MEM  =   SumOf  ( ( ( C[i] + S ) div S ) ( 2 S + 1 ) + 4 ) ,
 *             i = 0
 *
 *    where  K := LUT_KEYS ,  S := LUT_SIZE  and
 *           C[i] denotes the number of pairs the i-th hash table contains.
 *
 *  Let N be the number of pairs stored in the LUT. Suppose that
 *    C[i]  =  N / K
 *  is hold, i.e. we have an optimal hash key function.
 *  Then the best value for S is
 *    S  =  N / K + 1 ,
 *  thus the value for MEM equals approximately
 *    MEM  =  K ( ( ( N / K + N / K + 1 ) div ( N / K + 1 ) ) ( 2 S + 1 ) + 4 )
 *         =  K (                          1                  ( 2 S + 1 ) + 4 )
 *         =  K ( 2 S + 5 )
 *         =  K ( 2 ( N / K + 1 ) + 5 )
 *        ~=  2 N + 7 K .
 *  The time complexity for searching in the LUT is proportional to
 *    TC  =  N / K .
 */

#include <string.h>

#include "tree.h"
#include "free.h"
#include "dbug.h"

/*
 * size of a hash table fragment
 * (== # pairs of data that can be stored in the table)
 */
#define LUT_SIZE 4

/*
 * number of different hash keys (== number of hash tables)
 */
#define LUT_KEYS_POINTER 256 /* 2^8 */
#define LUT_KEYS_STRING 101  /* should be a prime number */
#define LUT_KEYS ((LUT_KEYS_POINTER) + (LUT_KEYS_STRING))

/*
 * hash table fragment
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

typedef long (*hash_key_fun_t) (void *);
typedef bool (*is_equal_fun_t) (void *, void *);

/******************************************************************************
 *
 * function:
 *   long GetHashKey_Pointer( void *data)
 *
 * description:
 *   Calculates the hash key for a given pointer.
 *   For the return value the condition
 *     0  <=  return  <  LUT_KEYS_POINTER
 *   must be hold.
 *
 ******************************************************************************/

static long
GetHashKey_Pointer (void *data)
{
    long hash_key;

    DBUG_ENTER ("GetHashKey_Pointer");

    DBUG_ASSERT ((data != NULL), "NULL has no hash key!");

    /*
     * hash key: bits 11 .. 4
     *  ->  0 <= key < 2^8
     */
    hash_key = (((int)data) & 0xff0) >> 4;

    DBUG_ASSERT (((hash_key >= 0) && (hash_key < (LUT_KEYS_POINTER))),
                 "hash key for pointers out of bounds!");

    DBUG_RETURN (hash_key);
}

/******************************************************************************
 *
 * function:
 *   long GetHashKey_String( void *data)
 *
 * description:
 *   Calculates the hash key for a given string.
 *   For the return value the condition
 *     LUT_KEYS_POINTERS  <=  return  <  LUT_KEYS
 *   must be hold.
 *
 ******************************************************************************/

static long
GetHashKey_String (void *data)
{
    char *str;
    long hash_key;

    DBUG_ENTER ("GetHashKey_String");

    DBUG_ASSERT ((data != NULL), "NULL has no hash key!");

    hash_key = 0;
    for (str = (char *)data; ((*str) != '\0'); str++) {
        hash_key += (*str);
    }
    hash_key %= LUT_KEYS_STRING;

    DBUG_ASSERT (((hash_key >= 0) && (hash_key < (LUT_KEYS_STRING))),
                 "hash key for strings out of bounds!");

    hash_key += LUT_KEYS_POINTER;

    DBUG_ASSERT (((hash_key >= (LUT_KEYS_POINTER)) && (hash_key < (LUT_KEYS))),
                 "hash key for strings out of bounds!");

    DBUG_RETURN (hash_key);
}

/******************************************************************************
 *
 * Function:
 *   bool IsEqual_Pointer( void *data1, void *data2)
 *
 * Description:
 *   Compares the given pointers and returns TRUE iff they contain the same
 *   address.
 *
 ******************************************************************************/

static bool
IsEqual_Pointer (void *data1, void *data2)
{
    bool ret;

    DBUG_ENTER ("IsEqual_Pointer");

    ret = (data1 == data2);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   bool IsEqual_String( void *data1, void *data2)
 *
 * Description:
 *   Compares the given strings and returns TRUE iff they are equal.
 *
 ******************************************************************************/

static bool
IsEqual_String (void *data1, void *data2)
{
    char *str1, *str2;
    bool ret;

    DBUG_ENTER ("IsEqual_String");

    str1 = (char *)data1;
    str2 = (char *)data2;
    ret = (!strcmp (str1, str2));

    DBUG_RETURN (ret);
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

    lut = (lut_t *)MALLOC ((LUT_KEYS) * sizeof (lut_t));
    for (k = 0; k < (LUT_KEYS); k++) {
        lut[k] = (single_lut_t *)MALLOC (sizeof (single_lut_t));
        lut[k]->first = (void **)MALLOC ((2 * (LUT_SIZE) + 1) * sizeof (void *));
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
    void **first, **tmp;
    long k, i;

    DBUG_ENTER ("RemoveLUT");

    if (lut != NULL) {
        /* remove LUT for pointers */
        for (k = 0; k < (LUT_KEYS_POINTER); k++) {
            DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
            for (i = 0; i <= lut[k]->size / (LUT_SIZE); i++) {
                tmp = lut[k]->first;
                lut[k]->first = lut[k]->first[2 * (LUT_SIZE)];
                FREE (tmp);
            }
            FREE (lut[k]);
        }
        /* remove LUT for strings */
        for (k = (LUT_KEYS_POINTER); k < (LUT_KEYS); k++) {
            DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
            tmp = lut[k]->first;
            first = tmp;
            for (i = 0; i < lut[k]->size; i++) {
                FREE (tmp[0]);
                FREE (tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /*
                     * the last table entry is reached
                     *  -> enter the next table of the chain
                     */
                    tmp = *tmp;
                    FREE (first);
                    first = tmp;
                }
            }
            FREE (first);
            FREE (lut[k]);
        }
        FREE (lut);
        DBUG_PRINT ("LUT", ("finished"));
    } else {
        DBUG_PRINT ("LUT", ("FAILED: lut is NULL"));
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT( lut_t *lut, void *old_item, void *new_item,
 *                         hash_key_fun_t hash_key_fun)
 *
 * description:
 *   Inserts the given pair of data (old_item, new_item) into the correct
 *   hash table of the LUT.
 *
 ******************************************************************************/

static lut_t *
InsertIntoLUT (lut_t *lut, void *old_item, void *new_item, hash_key_fun_t hash_key_fun)
{
    long k;

    DBUG_ENTER ("InsertIntoLUT");

    if (lut != NULL) {
        DBUG_ASSERT ((old_item != NULL), "Key of LUT entry must be != NULL!");

        k = hash_key_fun (old_item);
        DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
        *(lut[k]->act++) = old_item;
        *(lut[k]->act++) = new_item;
        lut[k]->size++;

        if (k < LUT_KEYS_POINTER) {
            DBUG_PRINT ("LUT", ("new pointers inserted (hash key %li)"
                                " -> [ 0x%p , 0x%p ]",
                                k, old_item, new_item));
        } else {
            DBUG_PRINT ("LUT",
                        ("new strings inserted (hash key %li)"
                         " -> [ \"%s\" , \"%s\" ]",
                         k - LUT_KEYS_POINTER, (char *)old_item, (char *)new_item));
        }

        if (lut[k]->size % (LUT_SIZE) == 0) {
            /*
             * the last table entry has been used -> allocate a new one.
             */
            *lut[k]->act = (void **)MALLOC ((2 * (LUT_SIZE) + 1) * sizeof (void *));
            DBUG_PRINT ("LUT", ("new LUT segment created -> 0x%p", lut[k]->act));
            /*
             * move 'act' to the first entry of the new table.
             */
            lut[k]->act = *lut[k]->act;
        }

        DBUG_PRINT ("LUT", ("finished: new LUT size -> %li", lut[k]->size));
    } else {
        DBUG_PRINT ("LUT", ("FAILED: lut is NULL"));
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT_P( lut_t *lut, void *old_item, void *new_item)
 *
 * description:
 *   Inserts the given pair of pointers (old_item, new_item) into the correct
 *   hash table of the LUT.
 *
 ******************************************************************************/

lut_t *
InsertIntoLUT_P (lut_t *lut, void *old_item, void *new_item)
{
    DBUG_ENTER ("InsertIntoLUT_P");

    lut = InsertIntoLUT (lut, old_item, new_item, GetHashKey_Pointer);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT_S( lut_t *lut, char *old_item, char *new_item)
 *
 * description:
 *   Inserts the given pair of strings (old_item, new_item) into the correct
 *   hash table of the LUT.
 *
 ******************************************************************************/

lut_t *
InsertIntoLUT_S (lut_t *lut, char *old_item, char *new_item)
{
    DBUG_ENTER ("InsertIntoLUT_S");

    lut = InsertIntoLUT (lut, StringCopy (old_item), StringCopy (new_item),
                         GetHashKey_String);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   void *SearchInLUT( lut_t *lut, void *old_item,
 *                      hash_key_fun_t hash_key_fun,
 *                      is_equal_fun_t is_equal_fun)
 *
 * description:
 *   Searches for the given data in the LUT.
 *   If the given data is *not* found in the LUT, the same data is returned.
 *   Otherwise the data associated with the found item is returned.
 *
 ******************************************************************************/

static void *
SearchInLUT (lut_t *lut, void *old_item, hash_key_fun_t hash_key_fun,
             is_equal_fun_t is_equal_fun)
{
    void *new_item;
    void **tmp;
    long k, i;

    DBUG_ENTER ("SearchInLUT");

    new_item = old_item;
    if (lut != NULL) {
        if (old_item != NULL) {
            k = hash_key_fun (old_item);
            DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
            tmp = lut[k]->first;
            for (i = 0; i < lut[k]->size; i++) {
                if (is_equal_fun (tmp[0], old_item)) {
                    new_item = tmp[1];
                    break;
                }
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /*
                     * the last table entry is reached
                     *  -> enter the next table of the chain
                     */
                    tmp = *tmp;
                }
            }

            if (new_item == old_item) {
                if (k < LUT_KEYS_POINTER) {
                    DBUG_PRINT ("LUT", ("finished:"
                                        " pointer 0x%p (hash key %li) *not* found :-(",
                                        old_item, k));
                } else {
                    DBUG_PRINT ("LUT", ("finished:"
                                        " string \"%s\" (hash key %li) *not* found :-(",
                                        (char *)old_item, k - LUT_KEYS_POINTER));
                }
            } else {
                if (k < LUT_KEYS_POINTER) {
                    DBUG_PRINT ("LUT", ("finished:"
                                        " pointer 0x%p (hash key %li) found -> 0x%p",
                                        old_item, k, new_item));
                } else {
                    DBUG_PRINT ("LUT", ("finished:"
                                        " string \"%s\" (hash key %li) found -> \"%s\"",
                                        (char *)old_item, k - LUT_KEYS_POINTER,
                                        (char *)new_item));
                }
            }
        } else {
            DBUG_PRINT ("LUT", ("finished: pointer/string is NULL"));
        }
    } else {
        DBUG_PRINT ("LUT", ("FAILED: lut is NULL"));
    }

    DBUG_RETURN (new_item);
}

/******************************************************************************
 *
 * function:
 *   void *SearchInLUT_P( lut_t *lut, void *old_item)
 *
 * description:
 *   Searches for the given pointer in the LUT.
 *   If the pointer is *not* found in the LUT, the same pointer is returned.
 *   Otherwise the pointer associated with the found pointer is returned.
 *
 ******************************************************************************/

void *
SearchInLUT_P (lut_t *lut, void *old_item)
{
    void *new_item;

    DBUG_ENTER ("SearchInLUT_P");

    new_item = SearchInLUT (lut, old_item, GetHashKey_Pointer, IsEqual_Pointer);

    DBUG_RETURN (new_item);
}

/******************************************************************************
 *
 * function:
 *   char *SearchInLUT_S( lut_t *lut, char *old_item)
 *
 * description:
 *   Searches for the given string in the LUT.
 *   If the string is *not* found in the LUT, the same string is returned.
 *   Otherwise the string associated with the found string is returned.
 *
 ******************************************************************************/

char *
SearchInLUT_S (lut_t *lut, char *old_item)
{
    char *new_item;

    DBUG_ENTER ("SearchInLUT_S");

    new_item = (char *)SearchInLUT (lut, old_item, GetHashKey_String, IsEqual_String);

    DBUG_RETURN (new_item);
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
        for (k = 0; k < (LUT_KEYS_POINTER); k++) {
            DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
            fprintf (handle, "*** pointers: hash key %li ***\n", k);
            tmp = lut[k]->first;
            for (i = 0; i < lut[k]->size; i++) {
                fprintf (handle, "%li: [ 0x%p -> 0x%p ]\n", i, tmp[0], tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /*
                     * the last table entry is reached
                     *  -> enter the next table of the chain
                     */
                    tmp = *tmp;
                }
            }
            fprintf (handle, "number of entries: %li\n", lut[k]->size);
        }
        for (k = LUT_KEYS_POINTER; k < (LUT_KEYS); k++) {
            DBUG_ASSERT ((lut[k] != NULL), "lut is NULL!");
            fprintf (handle, "*** strings: hash key %li ***\n", k - LUT_KEYS_POINTER);
            tmp = lut[k]->first;
            for (i = 0; i < lut[k]->size; i++) {
                fprintf (handle, "%li: [ \"%s\" -> \"%s\" ]\n", i, (char *)(tmp[0]),
                         (char *)(tmp[1]));
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    tmp = *tmp;
                }
            }
            fprintf (handle, "number of entries: %li\n", lut[k]->size);
        }
        DBUG_PRINT ("LUT", ("finished"));
    } else {
        DBUG_PRINT ("LUT", ("FAILED: lut is NULL"));
    }

    DBUG_VOID_RETURN;
}
