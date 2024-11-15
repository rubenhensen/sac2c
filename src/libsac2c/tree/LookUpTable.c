/*
 *
 *  Look-Up-Table (LUT) for Pointers and Strings
 *  --------------------------------------------
 *
 *  Each entry of a LUT can hold a pair [old, new] where 'old' is either a
 *  pointer (void*) or a string (char*) and 'new' is always a pointer (void*).
 *
 *  To insert a pair [old, new] into the LUT where 'old' represents a pointer
 *  or a string, use the function  InsertIntoLUT_P( lut, old, new)  or
 *  InsertIntoLUT_S( lut, old, new)  respectively.
 *  Note here, that it is possible to put doubles (pairs with identical
 *  compare-data) into the LUT, e.g. pairs (a,b), (a,c).
 *  If doubles are not welcome, it may be better to use the function
 *  UpdateLUT_P() instead.
 *
 *  The function  UpdateLUT_?( lut, old, new, &act_new)  checks whether there
 *  is already a pair [old, act_new] in the LUT. If so, 'act_new' is replaced
 *  by 'new', otherwise  InsertIntoLUT_?()  is called to insert the pair into
 *  the LUT.
 *
 *  The function  SearchInLUT_?( lut, old)  searches the LUT for an entry
 *  [old, new]. If the LUT contains such an entry, a pointer to the associated
 *  data 'new' is returned. Otherwise the return value equals NULL.
 *  Note here, that always the *first* matching entry is returned. If the LUT
 *  contains multiple matching entries, use  SearchInLUT_Next?()  to get
 *  the next match.
 *  For example, let the LUT contain pairs (a,b) and (a,c). When looking-up 'a'
 *  now, the return value is 'b' for the first, 'c' for the second, and NULL
 *  for all subsequent look-ups.
 *  SearchInLUT_P()  searches for a pointer (pointer compare),
 *  SearchInLUT_S()  searches for a string (string compare).
 *
 *  If both values of the pairs [old, new] have the same type (both strings or
 *  both pointers), you may want to use the function  SearchInLUT_SS( lut, old)
 *  or  SearchInLUT_PP( lut, old)  instead.
 *  If the LUT contains an entry [old, new] the associated data 'new' itself
 *  is returned (not a pointer to it). Otherwise the return value equals 'old'
 *  (not NULL).
 *  SearchInLUT_PP()  searches for a pointer (pointer compare) and expects
 *                    to find a pointer,
 *  SearchInLUT_SS()  searches for a string (string compare) and expects
 *                    to find a string.
 *
 *  In the following we refer to "old" and "new" as Key and Value.
 *  You can map a function 'fun' to all Values present in the LUT
 *  by means of  MapLUT( lut, fun). If ass_t is the type of the Value
 *  in the LUT, and ass_k is type of Key, then 'fun' should have
 *  the signature:
 *
 *    ass_t fun( ass_t Value, ass_k Key)
 *
 *  Moreover, the Value can be folded by using the function
 *  FoldLUT( lut, init, fun)  , where 'init' is the initial value for the
 *  fold operation. If init_t is the type of 'init, then the
 *  fun should have the signature:
 *
 *     init_t fun( init_t redu, ass_t Value, ass_k, Key)
 *
 *  *** CAUTION ***
 *  - InsertIntoLUT_S()  copies the compare-string ('old') before inserting
 *    it into the LUT. But the associated data is never copied!!
 *    If the associated data is also a string, you may want to duplicate it
 *    with  StringCopy()  first.
 *  - RemoveLUT()  removes all the stored compare-strings from heap memory.
 *  - SearchInLUT_?()  returns a *pointer* to the found associated data. Thus,
 *    the returned pointer will be undefined if  RemoveLUT()  has been called.
 *    Therefore you should not forget to duplicate the data first ... :-/
 *  - The support for multiple entries with identical compare-data is
 *    implemented rather inefficiently :-(
 *
 *
 *  Implementation
 *  --------------
 *
 *  The pairs of data are stored in a hash table. Pairs with identical hash
 *  keys are stored in a collision table (i.e. collision resolution by
 *  chaining).
 *  The hash key spaces for pointers and strings are disjunct. The hash keys
 *  for pointers are elements of the intervall [ 0 .. LUT_KEYS_POINTERS [, the
 *  hash keys for strings are elements of [ LUT_KEYS_POINTERS .. LUT_KEYS [.
 *  Thus, we need an array of size LUT_KEYS which elements are the collision
 *  tables.
 *  The functions GetHashKey_Pointer() and GetHashKey_String() compute for a
 *  pointer and string respectively the index of the collision table the
 *  pointer/string belongs to.
 *  Each collision table is made up of linked table fragments of fixed size
 *  (LUT_SIZE) in order to minimize the data overhead. Initially each collision
 *  table consists of a single table fragment. Each time the last entry of a
 *  collision table has been occupied another fragment is allocated and appended
 *  to the table.
 *
 *  The amount of memory (in words) needed for the whole LUT equals
 *
 *            (K - 1)
 *    MEM  =   SumOf  ( ( ( C[i] + S ) div S ) ( 2 S + 1 ) + 3 ) ,
 *             i = 0
 *
 *    where  K := HASH_KEYS ,  S := LUT_SIZE  and
 *           C[i] denotes the number of pairs the i-th collision table contains.
 *
 *  Let N be the number of pairs stored in the LUT. Suppose that
 *    C[i]  =  N / K
 *  is hold, i.e. we have an optimal hash key function.
 *
 *  Then the value for MEM equals
 *    MEM  =  K ( ( ( N / K + S ) div S ) ( 2 S + 1 ) + 3 )
 *  Thus, for the worst case (S = 1) we get
 *    MEM  =  K ( ( N / K + 1 ) 3 + 3 )
 *         =  3 N + 6 K .
 *  and for the best case (S = N / K + 1) we get
 *    MEM  =  K ( ( ( N / K + N / K + 1 ) div ( N / K + 1 ) ) ( 2 S + 1 ) + 3 )
 *         =  K (                          1                  ( 2 S + 1 ) + 3 )
 *         =  K ( 2 S + 4 )
 *         =  K ( 2 ( N / K + 1 ) + 4 )
 *         =  2 N + 6 K .
 *
 *  The time complexity for searching in the LUT is proportional to
 *    TC  =  N / K .
 *
 *
 *  DBUG strings
 *  ------------
 *
 *  LUT:       debug output for lut
 *  LUT_CHECK: perform some runtime checks for lut
 *
 */

#include "LookUpTable.h"

#include <math.h>

#include "str.h"
#include "memory.h"
#include "free.h"

#define DBUG_PREFIX "LUT"
#include "debug.h"

#include "ctinfo.h"
#include "check_mem.h"
#include "tree_basic.h"

/*
 * size of a collision table fragment
 * (== # pairs of data that can be stored in the table)
 */
#define LUT_SIZE 5

/*
 * number of different hash keys
 * (== size of hash table == # collision tables)
 */
#define HASH_KEYS_POINTER 32 /* 2^5 */
#define HASH_KEYS_STRING 17  /* should be a prime number */
#define HASH_KEYS ((HASH_KEYS_POINTER) + (HASH_KEYS_STRING))

#define HASH_KEY_T unsigned long int
#define HASH_KEY_CONV "%lu"
#define HASH_KEY_CONVT "%4lu"

#define LUT_SIZE_T int
#define LUT_SIZE_CONV "%d"

typedef LUT_SIZE_T lut_size_t;
typedef HASH_KEY_T hash_key_t;

/*
 * collision table fragment:
 *
 * 'first' points to the first item of the table.
 * 'next' points to the next free position of the table.
 * All elements of the table lie between these two addresses. The pairs of data
 * are stored as two consecutive table entries.
 * The table is of static size. If the current table is filled, the address
 * of a newly allocated table is stored in the last entry of the current table.
 * 'size' contains the number of items stored in the whole chain of tables.
 */
struct LUT_T {
    void **first;
    void **next;
    lut_size_t size;
};

typedef hash_key_t (*hash_key_fun_t) (void *);
typedef bool (*is_equal_fun_t) (const void *, const void *);

/*
 * static functions
 */

/******************************************************************************
 *
 * function:
 *   hash_key_t GetHashKey_Pointer( void *data)
 *
 * description:
 *   Calculates the hash key for a given pointer.
 *   For the return value the condition
 *     0  <=  return  <  HASH_KEYS_POINTER
 *   must be hold.
 *
 ******************************************************************************/

static hash_key_t
GetHashKey_Pointer (void *data)
{
    hash_key_t hash_key;

    DBUG_ENTER ();

    /*
     * hash key: bits 9 .. 5
     *  ->  0 <= key < 2^5
     */
    hash_key = (((hash_key_t)data >> 5) & 0x1f);

    DBUG_ASSERT (hash_key < (HASH_KEYS_POINTER), "hash key for pointers out of bounds!");

    DBUG_RETURN (hash_key);
}

/******************************************************************************
 *
 * function:
 *   hash_key_t GetHashKey_String( void *data)
 *
 * description:
 *   Calculates the hash key for a given string.
 *   For the return value the condition
 *     HASH_KEYS_POINTERS  <=  return  <  HASH_KEYS
 *   must be hold.
 *
 ******************************************************************************/

static hash_key_t
GetHashKey_String (void *data)
{
    char *str;
    hash_key_t hash_key;

    DBUG_ENTER ();

    hash_key = 0;
    if (data != NULL) {
        for (str = (char *)data; ((*str) != '\0'); str++) {
            hash_key += (hash_key_t)(*str);
        }
        hash_key %= (HASH_KEYS_STRING);
    }

    DBUG_ASSERT (hash_key < (HASH_KEYS_STRING), "hash key for strings out of bounds!");

    /*
     * use the offset HASH_KEYS_POINTERS in order to get disjoint hash key spaces
     * for pointers and strings
     */
    hash_key += (HASH_KEYS_POINTER);

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
IsEqual_Pointer (const void *data1, const void *data2)
{
    bool ret;

    DBUG_ENTER ();

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
IsEqual_String (const void *data1, const void *data2)
{
    DBUG_ENTER ();

    DBUG_RETURN (STReq ((const char *)data1, (const char *)data2));
}

#ifndef DBUG_OFF

/******************************************************************************
 *
 * Function:
 *   void ComputeHashStat( lut_t *lut, char *note,
 *                         hash_key_t min_key, hash_key_t max_key)
 *
 * Description:
 *   This function is used from DBUG_EXECUTE only!
 *
 ******************************************************************************/

static void
ComputeHashStat (lut_t *lut, char *note, hash_key_t min_key, hash_key_t max_key)
{
    hash_key_t min_k, max_k, k;
    lut_size_t size, sum_size;
    lut_size_t min_size, max_size, diff_size;
    double mean_size, sdev_size, sdev_mean;

    DBUG_ENTER ();

    if (lut != NULL) {
        DBUG_PRINT ("lut " F_PTR ", %s ---", (void *)lut, note);
        DBUG_EXECUTE (fprintf (stderr, "  key:  "); for (k = min_key; k < max_key; k++) {
            fprintf (stderr, HASH_KEY_CONVT " ", k);
        } fprintf (stderr, "\n");
                      fprintf (stderr, "  size: "); for (k = min_key; k < max_key; k++) {
                          DBUG_EXECUTE (fprintf (stderr, "%4i ", lut[k].size));
                      } fprintf (stderr, "\n"));

        /*
         * compute sum_size, min_size, max_size, diff_size, mean_size
         */
        sum_size = 0;
        min_size = max_size = lut[min_key].size;
        min_k = max_k = min_key;
        for (k = min_key; k < max_key; k++) {
            sum_size += size = lut[k].size;
            if (min_size > size) {
                min_size = size;
                min_k = k;
            }
            if (max_size < size) {
                max_size = size;
                max_k = k;
            }
        }
        diff_size = max_size - min_size;
        mean_size = ((double)sum_size) / (double)(max_key - min_key);

        /*
         * compute sdev_size
         */
        sdev_size = 0;
        for (k = min_key; k < max_key; k++) {
            double diff_size = lut[k].size - mean_size;
            sdev_size += (diff_size * diff_size);
        }
        sdev_size = sqrt (sdev_size / (double)(max_key - min_key));
        sdev_mean = (sum_size > 0) ? (sdev_size / mean_size) : 0;

        DBUG_EXECUTE (
          fprintf (stderr, "  sum = %i, LUTsize = %i\n", sum_size, LUT_SIZE);
          fprintf (stderr,
                   "  min (key " HASH_KEY_CONV ") = %i, max (key " HASH_KEY_CONV ") = %i,"
                   " mean = %1.1f, sdev = %1.1f, sdev/mean^2 = %1.2f\n",
                   min_k, min_size, max_k, max_size, mean_size, sdev_size, sdev_mean));

        if ((diff_size > LUT_SIZE) && (sdev_mean > 0.8)) {
            CTIwarn (EMPTY_LOC, "LUT: unbalanced lut (%s) detected:\n"
                     "(range = %i..%i,\n"
                     " mean = %1.1f, sdev = %1.1f, sdev/mean^2 = %1.2f)",
                     note, min_size, max_size, mean_size, sdev_size, sdev_mean);
        }
    } else {
        diff_size = 0;
    }

    DBUG_RETURN ();
}

#endif /* !DBUG_OFF */

/******************************************************************************
 *
 * Function:
 *   void **SearchInLUT_( lut_size_t size, lut_size_t i, void **entry,
 *                        void *old_item,
 *                        hash_key_t hash_key,
 *                        is_equal_fun_t is_equal_fun,
 *                        char *old_format, char *new_format)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void **
SearchInLUT_ (lut_size_t size, lut_size_t *i, void ***entry, void *old_item,
              hash_key_t hash_key, is_equal_fun_t is_equal_fun, char *old_format,
              char *new_format)
{
    void **new_item_p = NULL;

    DBUG_ENTER ();

    /* search in the collision table for 'old_item' */
    for (; (*i) < size; (*i)++) {
        if (is_equal_fun ((*entry)[0], old_item)) {
            new_item_p = (*entry) + 1;
            break;
        }
        (*entry) += 2;
        if (((*i) + 1) % (LUT_SIZE) == 0) {
            /* last table entry is reached -> enter next table of the chain */
            (*entry) = (void **)*(*entry);
        }
    }

    if (new_item_p == NULL) {
        DBUG_EXECUTE (
          fprintf (stderr, "  data (hash key " HASH_KEY_CONV ") *not* found: ", hash_key);
          fprintf (stderr, old_format, old_item); fprintf (stderr, "\n"));
    } else {
        DBUG_EXECUTE (
          fprintf (stderr, "  data (hash key " HASH_KEY_CONV ", pos " LUT_SIZE_CONV 
                           " ) found: [ ", hash_key, (*i));
          fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
          fprintf (stderr, new_format, *new_item_p); fprintf (stderr, " ]\n"));
    }

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * Function:
 *   void **SearchInLUT( lut_t *lut, void *old_item,
 *                       hash_key_t hash_key,
 *                       is_equal_fun_t is_equal_fun,
 *                       char *old_format, char *new_format)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void **
SearchInLUT (lut_t *lut, void *old_item, hash_key_t hash_key, is_equal_fun_t is_equal_fun,
             char *old_format, char *new_format)
{
    void **new_item_p = NULL;
    void **entry = NULL;
    lut_size_t i = 0;

    DBUG_ENTER ();

    entry = lut[hash_key].first;

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        if (old_item != NULL) {
            new_item_p
              = SearchInLUT_ (lut[hash_key].size, &i, &entry, old_item,
                              hash_key, is_equal_fun, old_format, new_format);

            DBUG_PRINT ("< finished");
        } else {
            DBUG_PRINT ("< finished: data is NULL");
        }
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * Function:
 *   void **SearchInLUT_state( lut_t *lut, void *old_item,
 *                             hash_key_t hash_key,
 *                             is_equal_fun_t is_equal_fun,
 *                             bool init,
 *                             char *old_format, char *new_format)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void **
SearchInLUT_state (lut_t *lut, void *old_item, hash_key_t hash_key,
                   is_equal_fun_t is_equal_fun, bool init, char *old_format,
                   char *new_format)
{
    static lut_t *store_lut = NULL;
    static void *store_old_item = NULL;
    static hash_key_t store_hash_key = 0;
    static lut_size_t store_size = 0;
    static lut_size_t store_i = 0;
    static void **store_entry = NULL;

    void **new_item_p = NULL;

    DBUG_ENTER ();

    if (init) {
        /*
         * store current parameters for subsequent calls
         */
        store_lut = lut;

        DBUG_PRINT ("> lut (" F_PTR "), initial search", (void *)store_lut);

        if (store_lut != NULL) {
            store_old_item = old_item;
            if (store_old_item != NULL) {
                store_hash_key = hash_key;
                store_size = store_lut[hash_key].size;
                DBUG_ASSERT (store_size >= 0, "illegal LUT size found!");
                store_i = 0;
                store_entry = store_lut[hash_key].first;

                new_item_p
                  = SearchInLUT_ (store_size, &store_i, &store_entry, store_old_item,
                                  hash_key, is_equal_fun, old_format, new_format);

                DBUG_PRINT ("< finished");
            } else {
                DBUG_PRINT ("< finished: data is NULL");
            }
        } else {
            DBUG_PRINT ("< FAILED: lut is NULL");
        }
    } else {
        /*
         * go to next entry in LUT
         */
        DBUG_PRINT ("> lut (" F_PTR "), search for doubles", (void *)store_lut);

        if (store_lut != NULL) {
            if (store_old_item != NULL) {
                store_entry += 2;
                if ((store_i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    store_entry = (void **)*store_entry;
                }
                store_i++;

                new_item_p
                  = SearchInLUT_ (store_size, &store_i, &store_entry, store_old_item,
                                  store_hash_key, is_equal_fun, old_format, new_format);

                DBUG_PRINT ("< finished");
            } else {
                DBUG_PRINT ("< finished: data is NULL");
            }
        } else {
            DBUG_PRINT ("< FAILED: lut is NULL");
        }
    }

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT_noDBUG( lut_t *lut, void *old_item, void *new_item,
 *                                hash_key_t hash_key)
 *
 * description:
 *
 *
 ******************************************************************************/

static lut_t *
InsertIntoLUT_noDBUG (lut_t *lut, void *old_item, void *new_item, hash_key_t hash_key)
{
    DBUG_ENTER ();

    DBUG_ASSERT (lut != NULL, "no LUT found!");

    *(lut[hash_key].next++) = old_item;
    *(lut[hash_key].next++) = new_item;
    lut[hash_key].size++;
    DBUG_ASSERT (lut[hash_key].size >= 0, "illegal LUT size found!");

    if (lut[hash_key].size % (LUT_SIZE) == 0) {
        /* the last table entry has been used -> allocate a new one */
        *lut[hash_key].next = (void **)MEMmalloc ((2 * (LUT_SIZE) + 1) * sizeof (void *));

        DBUG_PRINT ("new LUT segment created: " F_PTR, (void *)lut[hash_key].next);

        /* move 'next' to the first entry of the new table */
        lut[hash_key].next = (void **)*lut[hash_key].next;
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT( lut_t *lut, void *old_item, void *new_item,
 *                         hash_key_t hash_key,
 *                         char *old_format, char *new_format)
 *
 * description:
 *
 *
 ******************************************************************************/

static lut_t *
InsertIntoLUT (lut_t *lut, void *old_item, void *new_item, hash_key_t hash_key,
               char *old_format, char *new_format)
{
    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        DBUG_ASSERT (old_item != NULL, "NULL not allowed in LUT");
        lut = InsertIntoLUT_noDBUG (lut, old_item, new_item, hash_key);

        DBUG_EXECUTE (fprintf (stderr, "  new data inserted: [ ");
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, new_item); fprintf (stderr, " ]\n"));

        DBUG_PRINT ("< finished: new LUT size (hash key %lu) == %i", hash_key,
                    lut[hash_key].size);

        DBUG_EXECUTE_TAG ("LUT_CHECK", /* check quality of hash key function */
                          ComputeHashStat (lut, "pointers", 0, HASH_KEYS_POINTER);
                          ComputeHashStat (lut, "strings", HASH_KEYS_POINTER, HASH_KEYS));
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *UpdateLUT( lut_t *lut, void *old_item, void *new_item,
 *                     hash_key_t hash_key,
 *                     is_equal_fun_t is_equal_fun,
 *                     char *old_format, char *new_format,
 *                     void **found_item)
 *
 * description:
 *
 *
 ******************************************************************************/

lut_t *
UpdateLUT (lut_t *lut, void *old_item, void *new_item, hash_key_t hash_key,
           is_equal_fun_t is_equal_fun, char *old_format, char *new_format,
           void **found_item)
{
    void **found_item_p;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    found_item_p
      = SearchInLUT (lut, old_item, hash_key, is_equal_fun, old_format, new_format);

    if (found_item_p == NULL) {
        lut = InsertIntoLUT (lut,
                             (hash_key < (HASH_KEYS_POINTER)) ? old_item
                                                              : STRcpy ((char *)old_item),
                             new_item, hash_key, old_format, new_format);

        if (found_item != NULL) {
            (*found_item) = NULL;
        }
    } else {
        DBUG_EXECUTE (fprintf (stderr, "  data replaced: [ ");
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, *found_item_p);
                      fprintf (stderr, " ] =>> [ ");
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, new_item); fprintf (stderr, " ]\n"));

        if (found_item != NULL) {
            (*found_item) = (*found_item_p);
        }

        (*found_item_p) = new_item;
    }

    DBUG_PRINT ("< finished");

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   lut_t *MapLUT( lut_t *lut, void *(*fun)( void *, void *),
 *                  hash_key_t start, hash_key_t stop)
 *
 * Description:
 *  Invoke fun, passing each Value in LUT as first argument, and
 *  Key as second argument.
 *
 *
 ******************************************************************************/

static lut_t *
MapLUT (lut_t *lut, void *(*fun) (void *, void *), hash_key_t start, hash_key_t stop)
{
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        for (k = start; k < stop; k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                tmp[1] = fun (tmp[1], tmp[0]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
        }

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   void *FoldLUT( lut_t *lut, void *init, void *(*fun)( void *, void *, void *),
 *                  hash_key_t start, hash_key_t stop)
 *
 * Description:
 *  Invoke fun, passing each Value in LUT as first argument, and
 *  Key as second argument.
 *
 *
 ******************************************************************************/

static void *
FoldLUT (lut_t *lut, void *init, void *(*fun) (void *, void *, void *), hash_key_t start,
         hash_key_t stop)
{
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        for (k = start; k < stop; k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                init = fun (init, tmp[1], tmp[0]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
        }

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (init);
}

/*
 * exported functions
 */

/******************************************************************************
 *
 * function:
 *   lut_t *LUTgenerateLut()
 *
 * description:
 *   Generates a new LUT: All the needed collision tables are created and the
 *   internal data structures are initialized.
 *
 ******************************************************************************/

lut_t *
LUTgenerateLut (void)
{
    lut_t *lut;
    hash_key_t k;

    DBUG_ENTER ();

    lut = (lut_t *)MEMmalloc ((HASH_KEYS) * sizeof (lut_t));

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    for (k = 0; k < (HASH_KEYS); k++) {
        lut[k].first = (void **)MEMmalloc ((2 * (LUT_SIZE) + 1) * sizeof (void *));
        lut[k].next = lut[k].first;
        lut[k].size = 0;
    }

    DBUG_PRINT ("< finished");

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTduplicateLut( lut_t *lut)
 *
 * description:
 *   Duplicates the given LUT.
 *
 ******************************************************************************/

lut_t *
LUTduplicateLut (lut_t *lut)
{
    lut_t *new_lut;
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        new_lut = LUTgenerateLut ();

        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                new_lut = InsertIntoLUT_noDBUG (new_lut, tmp[0], tmp[1], k);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
        }
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                new_lut
                  = InsertIntoLUT_noDBUG (new_lut, STRcpy ((char *)(tmp[0])), tmp[1], k);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
        }

        DBUG_PRINT ("< finished");
    } else {
        new_lut = NULL;

        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (new_lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTremoveContentLut( lut_t *lut)
 *
 * description:
 *   Removes the content of the given LUT from memory.
 *
 ******************************************************************************/

lut_t *
LUTremoveContentLut (lut_t *lut)
{
    void **first, **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        /* init LUT for pointers */
        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            /* remove all but the first collision table fragments */
            for (i = 1; i <= lut[k].size / (LUT_SIZE); i++) {
                tmp = lut[k].first;
                lut[k].first = (void **)lut[k].first[2 * (LUT_SIZE)];
                tmp = MEMfree (tmp);
            }
            lut[k].next = lut[k].first;
            lut[k].size = 0;
        }
        /* init LUT for strings */
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            first = tmp;
            /* remove all strings and all but the first collision table fragments */
            for (i = 0; i < lut[k].size; i++) {
                tmp[0] = MEMfree (tmp[0]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                    first = MEMfree (first);
                    first = tmp;
                }
            }
            lut[k].first = lut[k].next = first;
            lut[k].size = 0;
        }

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTremoveLut( lut_t *lut)
 *
 * description:
 *   Removes the given LUT from memory.
 *
 ******************************************************************************/

lut_t *
LUTremoveLut (lut_t *lut)
{
    hash_key_t k;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        /* remove content of LUT */
        lut = LUTremoveContentLut (lut);

        /* remove empty LUT */
        for (k = 0; k < (HASH_KEYS); k++) {
            DBUG_ASSERT (lut[k].size == 0, "LUT not empty!");
            lut[k].first = MEMfree (lut[k].first);
        }
        lut = MEMfree (lut);

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTtouchContentLut( lut_t *lut, arg_info)
 *
 * description:
 *   touches the content of the given LUT from memory.
 *
 ******************************************************************************/

void
LUTtouchContentLut (lut_t *lut, info *arg_info)
{
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        /* init LUT for pointers */
        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                CHKMtouch (tmp, arg_info);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
        }
        /* init LUT for strings */
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            /* touch all strings and all but the first collision table fragments */
            for (i = 0; i < lut[k].size; i++) {
                CHKMtouch (tmp, arg_info);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
        }

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTtouchLut( lut_t *lut, info *arg_info)
 *
 * description:
 *   touches the given LUT from memory.
 *
 ******************************************************************************/

void
LUTtouchLut (lut_t *lut, info *arg_info)
{
    hash_key_t k;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (lut != NULL) {
        /* touch content of LUT */
        LUTtouchContentLut (lut, arg_info);

        /* touch LUT */
        for (k = 0; k < (HASH_KEYS); k++) {
            CHKMtouch (lut[k].first, arg_info);
        }
        CHKMtouch (lut, arg_info);

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   bool LUTisEmptyLut( lut_t *lut)
 *
 * Description:
 *   Returns TRUE iff the given LUT is empty.
 *
 ******************************************************************************/

bool
LUTisEmptyLut (lut_t *lut)
{
    hash_key_t k;
    bool empty = TRUE;

    DBUG_ENTER ();

    if (lut != NULL) {
        for (k = 0; k < (HASH_KEYS); k++) {
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");

            if (lut[k].size > 0) {
                empty = FALSE;
                break;
            }
        }
    }

    DBUG_RETURN (empty);
}

/******************************************************************************
 *
 * function:
 *   void **LUTsearchInLutP( lut_t *lut, void *old_item)
 *
 * description:
 *   Searches for the given pointer in the LUT.
 *   If the pointer is not found in the LUT, NULL is returned.
 *   Otherwise the *address* of the data associated with the found pointer is
 *   returned.
 *
 * caution:
 *   If the LUT contains multiple entries for the given string, the *first*
 *   match is returned.
 *   To get in touch with the other entries, the function SearchInLUT_NextS()
 *   should be used.
 *
 ******************************************************************************/

void **
LUTsearchInLutP (lut_t *lut, void *old_item)
{
    void **new_item_p;

    DBUG_ENTER ();

    new_item_p = SearchInLUT_state (lut, old_item, GetHashKey_Pointer (old_item),
                                    IsEqual_Pointer, TRUE, F_PTR, F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void **LUTsearchInLutS( lut_t *lut, char *old_item)
 *
 * description:
 *   Searches for the given string in the LUT.
 *   If the string is not found in the LUT, NULL is returned.
 *   Otherwise the *address* of the data associated with the found string is
 *   returned.
 *
 * caution:
 *   If the LUT contains multiple entries for the given string, the *first*
 *   match is returned.
 *   To get in touch with the other entries, the function SearchInLUT_NextS()
 *   should be used.
 *
 ******************************************************************************/

void **
LUTsearchInLutS (lut_t *lut, char *old_item)
{
    void **new_item_p;

    DBUG_ENTER ();

    new_item_p = SearchInLUT_state (lut, old_item, GetHashKey_String (old_item),
                                    IsEqual_String, TRUE, "\"%s\"", F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void **LUTsearchInLutNextP( void)
 *
 * description:
 *   Searches for the next entry matching for the given pointer.
 *   Iff no more matching entries are found in the LUT, NULL is returned.
 *
 * caution:
 *   The function LUTsearchInLutP() must be called first!!
 *
 ******************************************************************************/

void **
LUTsearchInLutNextP (void)
{
    void **new_item_p;

    DBUG_ENTER ();

    new_item_p = SearchInLUT_state (NULL, NULL, 0, IsEqual_Pointer, FALSE, F_PTR, F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void **LUTsearchInLutNextS( void)
 *
 * description:
 *   Searches for the next entry matching for the given string.
 *   Iff no more matching entries are found in the LUT, NULL is returned.
 *
 * caution:
 *   The function LUTsearchInLutS() must be called first!!
 *
 ******************************************************************************/

void **
LUTsearchInLutNextS (void)
{
    void **new_item_p;

    DBUG_ENTER ();

    new_item_p
      = SearchInLUT_state (NULL, NULL, 0, IsEqual_String, FALSE, "\"%s\"", F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void *LUTsearchInLutPp( lut_t *lut, void *old_item)
 *
 * description:
 *   Searches for the given pointer in the LUT.
 *   If the pointer is not found in the LUT, the same pointer is returned.
 *   Otherwise the pointer associated with the found pointer is returned.
 *
 * caution:
 *   If the LUT contains multiple entries for the given string, the *first*
 *   match is returned.
 *   To get in touch with the other entries, the function SearchInLUT_NextP()
 *   should be used.
 *
 ******************************************************************************/

void *
LUTsearchInLutPp (lut_t *lut, void *old_item)
{
    void **new_item_p;
    void *new_item;

    DBUG_ENTER ();

    new_item_p = LUTsearchInLutP (lut, old_item);

    new_item = (new_item_p == NULL) ? old_item : *new_item_p;

    DBUG_RETURN (new_item);
}

/******************************************************************************
 *
 * function:
 *   char *LUTsearchInLutSs( lut_t *lut, char *old_item)
 *
 * description:
 *   Searches for the given string in the LUT.
 *   If the string is *not* found in the LUT, the same string is returned.
 *   Otherwise the string associated with the found string is returned.
 *
 * caution:
 *   If the LUT contains multiple entries for the given string, the *first*
 *   match is returned.
 *   To get in touch with the other entries, the function SearchInLUT_NextS()
 *   should be used.
 *
 *   This function should only be used iff the associated data is indead a
 *   string!!
 *
 ******************************************************************************/

char *
LUTsearchInLutSs (lut_t *lut, char *old_item)
{
    char **new_item_p;
    char *new_item;

    DBUG_ENTER ();

    new_item_p = (char **)SearchInLUT_state (lut, old_item, GetHashKey_String (old_item),
                                             IsEqual_String, TRUE, "\"%s\"", "\"%s\"");

    new_item = (new_item_p == NULL) ? old_item : *new_item_p;

    DBUG_RETURN (new_item);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTinsertIntoLutP( lut_t *lut, void *old_item, void *new_item)
 *
 * description:
 *   Inserts the given pair of pointers (old_item, new_item) into the correct
 *   collision table of the LUT.
 *
 * remark:
 *   It is possible to put doubles (pairs with identical compare-data) into the
 *   LUT, e.g. pairs (a,b), (a,c). When looking-up 'a' now, the return value
 *   is 'b' for the first, 'c' for the second, and NULL for all subsequent
 *   look-ups.
 *   If doubles are not welcome, it may be better to use the function
 *   UpdateLUT_P() instead!
 *
 *****************************************************************************/

lut_t *
LUTinsertIntoLutP (lut_t *lut, void *old_item, void *new_item)
{
    DBUG_ENTER ();

    lut = InsertIntoLUT (lut, old_item, new_item, GetHashKey_Pointer (old_item), F_PTR,
                         F_PTR);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTinsertIntoLutS( lut_t *lut, char *old_item, void *new_item)
 *
 * description:
 *   Inserts the given pair of strings (old_item, new_item) into the correct
 *   collision table of the LUT.
 *
 * remark:
 *   It is possible to put doubles (pairs with identical compare-data) into the
 *   LUT, e.g. pairs (a,b), (a,c). When looking-up 'a' now, the return value
 *   is 'b' for the first, 'c' for the second, and NULL for all subsequent
 *   look-ups.
 *   If doubles are not welcome, it may be better to use the function
 *   UpdateLUT_S() instead!
 *
 ******************************************************************************/

lut_t *
LUTinsertIntoLutS (lut_t *lut, char *old_item, void *new_item)
{
    DBUG_ENTER ();

    lut = InsertIntoLUT (lut, STRcpy (old_item), new_item, GetHashKey_String (old_item),
                         "\"%s\"", F_PTR);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTupdateLutP( lut_t *lut, void *old_item, void *new_item,
 *                         void **found_item)
 *
 * description:
 *   Inserts the given pair of pointers (old_item, new_item) into the correct
 *   collision table of the LUT.
 *   If a pair (old_item, old_new_item) is already present in the table, the
 *   data represented by 'old_new_item' is saved in '*found_item' and then
 *   overwritten.
 *   Otherwise a new entry is put into the collision table and NULL is stored
 *   in '*found_item'.
 *
 *****************************************************************************/

lut_t *
LUTupdateLutP (lut_t *lut, void *old_item, void *new_item, void **found_item)
{
    DBUG_ENTER ();

    lut = UpdateLUT (lut, old_item, new_item, GetHashKey_Pointer (old_item),
                     IsEqual_Pointer, F_PTR, F_PTR, found_item);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *LUTupdateLutS( lut_t *lut, char *old_item, void *new_item,
 *                         void **found_item)
 *
 * description:
 *   Inserts the given pair of strings (old_item, new_item) into the correct
 *   collision table of the LUT.
 *   If a pair (old_item, old_new_item) is already present in the table, the
 *   data represented by 'old_new_item' is saved in '*found_item' and then
 *   overwritten.
 *   Otherwise a new entry is put into the collision table and NULL is stored
 *   in '*found_item'.
 *
 ******************************************************************************/

lut_t *
LUTupdateLutS (lut_t *lut, char *old_item, void *new_item, void **found_item)
{
    DBUG_ENTER ();

    lut = UpdateLUT (lut, old_item, new_item, GetHashKey_String (old_item),
                     IsEqual_String, "\"%s\"", F_PTR, found_item);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   lut_t *LUTmapLutS( lut_t *lut, void *(*fun)( void *, void *))
 *
 * Description:
 *   Applies 'fun' to all string Values in the LUT.
 *
 ******************************************************************************/

lut_t *
LUTmapLutS (lut_t *lut, void *(*fun) (void *, void *))
{
    DBUG_ENTER ();

    lut = MapLUT (lut, fun, HASH_KEYS_POINTER, HASH_KEYS);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   lut_t *LUTmapLutP( lut_t *lut, void *(*fun)( void *, void *))
 *
 * Description:
 *   Applies 'fun' to all pointer Values in the LUT.
 *
 ******************************************************************************/

lut_t *
LUTmapLutP (lut_t *lut, void *(*fun) (void *, void *))
{
    DBUG_ENTER ();

    lut = MapLUT (lut, fun, 0, HASH_KEYS_POINTER);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   void *LUTfoldLutS( lut_t *lut, void *init, void *(*fun)( void *, void *,
 *                      void *))
 *
 * Description:
 *   Applies 'fun' to all string Values in the LUT.
 *
 ******************************************************************************/

void *
LUTfoldLutS (lut_t *lut, void *init, void *(*fun) (void *, void *, void *))
{
    DBUG_ENTER ();

    init = FoldLUT (lut, init, fun, HASH_KEYS_POINTER, HASH_KEYS);

    DBUG_RETURN (init);
}

/******************************************************************************
 *
 * Function:
 *   void *LUTfoldLutP( lut_t *lut, void *init, void *(*fun)( void *, void *
 *                      void *))
 *
 * Description:
 *   Applies 'fun' to all pointer values in the LUT.
 *
 ******************************************************************************/

void *
LUTfoldLutP (lut_t *lut, void *init, void *(*fun) (void *, void *, void *))
{
    DBUG_ENTER ();

    init = FoldLUT (lut, init, fun, 0, HASH_KEYS_POINTER);

    DBUG_RETURN (init);
}

/******************************************************************************
 *
 * function:
 *   void LUTprintLut( FILE *handle, lut_t *lut)
 *
 * description:
 *   Prints the contents of the given LUT.
 *
 ******************************************************************************/

void
LUTprintLut (FILE *handle, lut_t *lut)
{
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("> lut (" F_PTR ")", (void *)lut);

    if (handle == NULL) {
        handle = stderr;
    }

    if (lut != NULL) {
        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            fprintf (handle, "*** pointers: hash key " HASH_KEY_CONV " ***\n", k);
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                fprintf (handle, "%i: [ " F_PTR " -> " F_PTR " ]\n", i, tmp[0], tmp[1]);
                if (N_avis == NODE_TYPE ((node *)tmp[0])) {
                    fprintf (handle, "%s  ->  ", AVIS_NAME ((node *)tmp[0]));
                }

                if (N_avis == NODE_TYPE ((node *)tmp[1])) {
                    fprintf (handle, "%s\n", AVIS_NAME ((node *)tmp[1]));
                } else {
                    if (N_fundef == NODE_TYPE ((node *)tmp[1])) {
                        fprintf (handle, "%s\n", FUNDEF_NAME ((node *)tmp[1]));
                    }
                }

                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
            fprintf (handle, "number of entries: %i\n", lut[k].size);
        }
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            fprintf (handle, "*** strings: hash key " HASH_KEY_CONV " ***\n", k);
            DBUG_ASSERT (lut[k].size >= 0, "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                fprintf (handle, "%i: [ \"%s\" -> " F_PTR " ]\n", i, (char *)(tmp[0]),
                         tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = (void **)*tmp;
                }
            }
            fprintf (handle, "number of entries: %i\n", lut[k].size);
        }

        DBUG_PRINT ("< finished");
    } else {
        DBUG_PRINT ("< FAILED: lut is NULL");
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
