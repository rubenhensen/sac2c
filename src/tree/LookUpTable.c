/*
 *
 * $Log$
 * Revision 3.26  2002/09/13 19:04:35  dkr
 * bug in UpdateLUT() fixed: 'old_item' is duplicated if it is a string!
 *
 * Revision 3.25  2002/08/15 20:58:30  dkr
 * comment about implementation modified
 *
 * Revision 3.24  2002/08/15 18:46:45  dkr
 * functions SearchInLUT_Next?() added
 *
 * Revision 3.23  2002/08/15 11:45:40  dkr
 * - functions ApplyToEach_?() renamed into MapLUT_?()
 * - functions FoldLUT_?() added
 *
 * Revision 3.22  2002/08/14 15:03:54  dkr
 * ComputeHashDiff() expanded and renamed into ComputeHashStat()
 *
 * Revision 3.21  2002/08/14 13:43:44  dkr
 * hash key calculation for pointers optimized
 *
 * Revision 3.20  2002/08/14 11:59:51  dkr
 * DBUG-output in ComputeHashDiff() modified
 *
 * Revision 3.18  2002/08/13 13:21:15  dkr
 * - !!!! string support modified !!!!
 *   Now, only the compare-data is a string, the associated-data is always
 *   a (void*) pointer!
 * - functions ApplyToEach_?() added
 *
 * Revision 3.17  2001/11/22 08:48:56  sbs
 * ComputeHashDiff compiled only if DBUG is active
 *
 * Revision 3.16  2001/05/18 11:40:11  dkr
 * function IsEmptyLUT() added
 *
 * Revision 3.15  2001/05/17 11:39:01  dkr
 * MALLOC FREE aliminated
 *
 * Revision 3.14  2001/05/17 09:41:36  nmw
 * some bugs in UpdateLUT functions fixed
 * wrong hashkeytype, illegal pointer sharing resolved
 *
 * Revision 3.13  2001/04/19 07:47:25  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 3.12  2001/04/11 09:43:38  dkr
 * fixed a bug in DuplicateLUT(): StringCopy() used :-/
 *
 * Revision 3.11  2001/04/10 09:59:34  dkr
 * DuplicateLUT added
 *
 * Revision 3.10  2001/04/06 15:55:31  dkr
 * number of hash keys reduced
 *
 * Revision 3.9  2001/04/06 15:29:09  dkr
 * function RemoveContentLUT added
 *
 * Revision 3.8  2001/04/04 14:55:09  sbs
 * pointer casted into long rather than int
 * (for generating hash-keys). This pleases
 * SUN LINUX and ALPHA as well 8-)
 *
 * [...]
 *
 * Revision 1.1  2000/01/28 12:33:14  dkr
 * Initial revision
 *
 */

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
 *  You can map a function 'fun' to all associated data present in the LUT
 *  by means of  MapLUT( lut, fun). Let  ass_t  be the type of the associated
 *  data in the LUT, then 'fun' should have the signature  ass_t -> ass_t  .
 *  Moreover, the associated data can be folded by using the function
 *  FoldLUT( lut, init, fun)  , where 'init' is the initial value for the
 *  fold operation. Let again  ass_t  be the type of the associated data in
 *  the LUT and let  init_t  be the type of 'init'. Then 'fun' should have
 *  the signature  ( init_t , ass_t ) -> init_t  and the return value of
 *  FoldLUT()  has the type  init_t  .
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

#include <string.h>
#include <math.h>

#include "internal_lib.h"
#include "free.h"
#include "dbug.h"
#include "Error.h"

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

typedef int lut_size_t;
typedef int hash_key_t;

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
typedef struct LUT_T {
    void **first;
    void **next;
    lut_size_t size;
} lut_t;

typedef hash_key_t (*hash_key_fun_t) (void *);
typedef bool (*is_equal_fun_t) (void *, void *);

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

    DBUG_ENTER ("GetHashKey_Pointer");

    /*
     * hash key: bits 9 .. 5
     *  ->  0 <= key < 2^5
     */
    hash_key = (((hash_key_t)data >> 5) & 0x1f);

    DBUG_ASSERT (((hash_key >= 0) && (hash_key < (HASH_KEYS_POINTER))),
                 "hash key for pointers out of bounds!");

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

    DBUG_ENTER ("GetHashKey_String");

    hash_key = 0;
    if (data != NULL) {
        for (str = (char *)data; ((*str) != '\0'); str++) {
            hash_key += (*str);
        }
        hash_key %= (HASH_KEYS_STRING);
    }

    DBUG_ASSERT (((hash_key >= 0) && (hash_key < (HASH_KEYS_STRING))),
                 "hash key for strings out of bounds!");

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
    bool ret;

    DBUG_ENTER ("IsEqual_String");

    ret = (!strcmp ((char *)data1, (char *)data2));

    DBUG_RETURN (ret);
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

    DBUG_ENTER ("ComputeHashStat");

    if (lut != NULL) {
        DBUG_PRINT ("LUT", ("lut " F_PTR ", %s ---", lut, note));
        DBUG_EXECUTE ("LUT", fprintf (stderr, "  key:  ");
                      for (k = min_key; k < max_key;
                           k++) { fprintf (stderr, "%4i ", k); } fprintf (stderr, "\n");
                      fprintf (stderr, "  size: "); for (k = min_key; k < max_key; k++) {
                          DBUG_EXECUTE ("LUT", fprintf (stderr, "%4i ", lut[k].size););
                      } fprintf (stderr, "\n"););

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
        mean_size = ((double)sum_size) / (max_key - min_key);

        /*
         * compute sdev_size
         */
        sdev_size = 0;
        for (k = min_key; k < max_key; k++) {
            double diff_size = lut[k].size - mean_size;
            sdev_size += (diff_size * diff_size);
        }
        sdev_size = sqrt (sdev_size / (max_key - min_key));
        sdev_mean = (sum_size > 0) ? (sdev_size / mean_size) : 0;

        DBUG_EXECUTE ("LUT",
                      fprintf (stderr, "  sum = %i, LUTsize = %i\n", sum_size, LUT_SIZE);
                      fprintf (stderr,
                               "  min (key %i) = %i, max (key %i) = %i,"
                               " mean = %1.1f, sdev = %1.1f, sdev/mean^2 = %1.2f\n",
                               min_k, min_size, max_k, max_size, mean_size, sdev_size,
                               sdev_mean););

        if ((diff_size > LUT_SIZE) && (sdev_mean > 0.8)) {
            SYSWARN (("LUT: unballanced lut (%s) detected", note));
            CONT_WARN (("(range = %i..%i,"
                        " mean = %1.1f, sdev = %1.1f, sdev/mean^2 = %1.2f)",
                        min_size, max_size, mean_size, sdev_size, sdev_mean));
        }
    } else {
        diff_size = 0;
    }

    DBUG_VOID_RETURN;
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
SearchInLUT_ (lut_size_t size, lut_size_t i, void **entry, void *old_item,
              hash_key_t hash_key, is_equal_fun_t is_equal_fun, char *old_format,
              char *new_format)
{
    void **new_item_p = NULL;

    DBUG_ENTER ("SearchInLUT_");

#if 0
  /* lut, hash_key -> size, i, entry */
  size = lut[ hash_key].size;
  DBUG_ASSERT( (size >= 0), "illegal LUT size found!");
  i = 0;
  entry = lut[ hash_key].first;
#endif

    /* search in the collision table for 'old_item' */
    for (; i < size; i++) {
        if (is_equal_fun (entry[0], old_item)) {
            new_item_p = entry + 1;
            break;
        }
        entry += 2;
        if ((i + 1) % (LUT_SIZE) == 0) {
            /* last table entry is reached -> enter next table of the chain */
            entry = *entry;
        }
    }

    if (new_item_p == NULL) {
        DBUG_EXECUTE ("LUT",
                      fprintf (stderr, "  data (hash key %i) *not* found: ", hash_key);
                      fprintf (stderr, old_format, old_item); fprintf (stderr, "\n"););
    } else {
        DBUG_EXECUTE ("LUT", fprintf (stderr, "  data (hash key %i) found: [ ", hash_key);
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, *new_item_p);
                      fprintf (stderr, " ]\n"););
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

    DBUG_ENTER ("SearchInLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        if (old_item != NULL) {
            new_item_p
              = SearchInLUT_ (lut[hash_key].size, 0, lut[hash_key].first, old_item,
                              hash_key, is_equal_fun, old_format, new_format);

            DBUG_PRINT ("LUT", ("< finished"));
        } else {
            DBUG_PRINT ("LUT", ("< finished: data is NULL"));
        }
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
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

    DBUG_ENTER ("SearchInLUT_state");

    if (init) {
        /*
         * store current parameters for subsequent calls
         */
        store_lut = lut;

        DBUG_PRINT ("LUT", ("> lut (" F_PTR "), initial search", store_lut));

        if (store_lut != NULL) {
            store_old_item = old_item;
            if (store_old_item != NULL) {
                store_hash_key = hash_key;
                store_size = store_lut[hash_key].size;
                DBUG_ASSERT ((store_size >= 0), "illegal LUT size found!");
                store_i = 0;
                store_entry = store_lut[hash_key].first;

                new_item_p
                  = SearchInLUT_ (store_size, store_i, store_entry, store_old_item,
                                  hash_key, is_equal_fun, old_format, new_format);

                DBUG_PRINT ("LUT", ("< finished"));
            } else {
                DBUG_PRINT ("LUT", ("< finished: data is NULL"));
            }
        } else {
            DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
        }
    } else {
        /*
         * go to next entry in LUT
         */
        DBUG_PRINT ("LUT", ("> lut (" F_PTR "), search for doubles", store_lut));

        if (store_lut != NULL) {
            if (store_old_item != NULL) {
                store_entry += 2;
                if ((store_i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    store_entry = *store_entry;
                }
                store_i++;

                new_item_p
                  = SearchInLUT_ (store_size, store_i, store_entry, store_old_item,
                                  store_hash_key, is_equal_fun, old_format, new_format);

                DBUG_PRINT ("LUT", ("< finished"));
            } else {
                DBUG_PRINT ("LUT", ("< finished: data is NULL"));
            }
        } else {
            DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
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
    DBUG_ENTER ("InsertIntoLUT_noDBUG");

    DBUG_ASSERT ((lut != NULL), "no LUT found!");

    *(lut[hash_key].next++) = old_item;
    *(lut[hash_key].next++) = new_item;
    lut[hash_key].size++;
    DBUG_ASSERT ((lut[hash_key].size >= 0), "illegal LUT size found!");

    if (lut[hash_key].size % (LUT_SIZE) == 0) {
        /* the last table entry has been used -> allocate a new one */
        *lut[hash_key].next = (void **)Malloc ((2 * (LUT_SIZE) + 1) * sizeof (void *));

        DBUG_PRINT ("LUT", ("new LUT segment created: " F_PTR, lut[hash_key].next));

        /* move 'next' to the first entry of the new table */
        lut[hash_key].next = *lut[hash_key].next;
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
    DBUG_ENTER ("InsertIntoLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        DBUG_ASSERT ((old_item != NULL), "NULL not allowed in LUT");
        lut = InsertIntoLUT_noDBUG (lut, old_item, new_item, hash_key);

        DBUG_EXECUTE ("LUT", fprintf (stderr, "  new data inserted: [ ");
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, new_item); fprintf (stderr, " ]\n"););

        DBUG_PRINT ("LUT", ("< finished: new LUT size (hash key %i) == %i", hash_key,
                            lut[hash_key].size));

        DBUG_EXECUTE ("LUT_CHECK",
                      /* check quality of hash key function */
                      ComputeHashStat (lut, "pointers", 0, HASH_KEYS_POINTER);
                      ComputeHashStat (lut, "strings", HASH_KEYS_POINTER, HASH_KEYS););
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
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

    DBUG_ENTER ("UpdateLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    found_item_p
      = SearchInLUT (lut, old_item, hash_key, is_equal_fun, old_format, new_format);

    if (found_item_p == NULL) {
        lut = InsertIntoLUT (lut,
                             (hash_key < (HASH_KEYS_POINTER)) ? old_item
                                                              : StringCopy (old_item),
                             new_item, hash_key, old_format, new_format);

        if (found_item != NULL) {
            (*found_item) = NULL;
        }
    } else {
        DBUG_EXECUTE ("LUT", fprintf (stderr, "  data replaced: [ ");
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, *found_item_p);
                      fprintf (stderr, " ] =>> [ ");
                      fprintf (stderr, old_format, old_item); fprintf (stderr, " -> ");
                      fprintf (stderr, new_format, new_item); fprintf (stderr, " ]\n"););

        (*found_item_p) = new_item;

        if (found_item != NULL) {
            (*found_item) = (*found_item_p);
        }
    }

    DBUG_PRINT ("LUT", ("< finished"));

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   lut_t *MapLUT( lut_t *lut, void *(*fun)( void *),
 *                  hash_key_t start, hash_key_t stop)
 *
 * Description:
 *
 *
 ******************************************************************************/

static lut_t *
MapLUT (lut_t *lut, void *(*fun) (void *), hash_key_t start, hash_key_t stop)
{
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ("MapLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        for (k = start; k < stop; k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                tmp[1] = fun (tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                }
            }
        }

        DBUG_PRINT ("LUT", ("< finished"));
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   void *FoldLUT( lut_t *lut, void *init, void *(*fun)( void *, void *),
 *                  hash_key_t start, hash_key_t stop)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void *
FoldLUT (lut_t *lut, void *init, void *(*fun) (void *, void *), hash_key_t start,
         hash_key_t stop)
{
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ("FoldLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        for (k = start; k < stop; k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                init = fun (init, tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                }
            }
        }

        DBUG_PRINT ("LUT", ("< finished"));
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
    }

    DBUG_RETURN (init);
}

/*
 * exported functions
 */

/******************************************************************************
 *
 * function:
 *   lut_t *GenerateLUT( void)
 *
 * description:
 *   Generates a new LUT: All the needed collision tables are created and the
 *   internal data structures are initialized.
 *
 ******************************************************************************/

lut_t *
GenerateLUT (void)
{
    lut_t *lut;
    hash_key_t k;

    DBUG_ENTER ("GenerateLUT");

    lut = (lut_t *)Malloc ((HASH_KEYS) * sizeof (lut_t));

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    for (k = 0; k < (HASH_KEYS); k++) {
        lut[k].first = (void **)Malloc ((2 * (LUT_SIZE) + 1) * sizeof (void *));
        lut[k].next = lut[k].first;
        lut[k].size = 0;
    }

    DBUG_PRINT ("LUT", ("< finished"));

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *DuplicateLUT( lut_t *lut)
 *
 * description:
 *   Duplicates the given LUT.
 *
 ******************************************************************************/

lut_t *
DuplicateLUT (lut_t *lut)
{
    lut_t *new_lut;
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ("DuplicateLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        new_lut = GenerateLUT ();

        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                new_lut = InsertIntoLUT_noDBUG (new_lut, tmp[0], tmp[1], k);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                }
            }
        }
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                new_lut = InsertIntoLUT_noDBUG (new_lut, StringCopy ((char *)(tmp[0])),
                                                tmp[1], k);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                }
            }
        }

        DBUG_PRINT ("LUT", ("< finished"));
    } else {
        new_lut = NULL;

        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
    }

    DBUG_RETURN (new_lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *RemoveContentLUT( lut_t *lut)
 *
 * description:
 *   Removes the content of the given LUT from memory.
 *
 ******************************************************************************/

lut_t *
RemoveContentLUT (lut_t *lut)
{
    void **first, **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ("RemoveContentLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        /* init LUT for pointers */
        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            /* remove all but the first collision table fragments */
            for (i = 1; i <= lut[k].size / (LUT_SIZE); i++) {
                tmp = lut[k].first;
                lut[k].first = lut[k].first[2 * (LUT_SIZE)];
                tmp = Free (tmp);
            }
            lut[k].next = lut[k].first;
            lut[k].size = 0;
        }
        /* init LUT for strings */
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            first = tmp;
            /* remove all strings and all but the first collision table fragments */
            for (i = 0; i < lut[k].size; i++) {
                tmp[0] = Free (tmp[0]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                    first = Free (first);
                    first = tmp;
                }
            }
            lut[k].first = lut[k].next = first;
            lut[k].size = 0;
        }

        DBUG_PRINT ("LUT", ("< finished"));
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
    }

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
    hash_key_t k;

    DBUG_ENTER ("RemoveLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (lut != NULL) {
        /* remove content of LUT */
        lut = RemoveContentLUT (lut);

        /* remove empty LUT */
        for (k = 0; k < (HASH_KEYS); k++) {
            DBUG_ASSERT ((lut[k].size == 0), "LUT not empty!");
            lut[k].first = Free (lut[k].first);
        }
        lut = Free (lut);

        DBUG_PRINT ("LUT", ("< finished"));
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
    }

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   bool IsEmptyLUT( lut_t *lut)
 *
 * Description:
 *   Returns TRUE iff the given LUT is empty.
 *
 ******************************************************************************/

bool
IsEmptyLUT (lut_t *lut)
{
    hash_key_t k;
    bool empty = TRUE;

    DBUG_ENTER ("IsEmptyLUT");

    if (lut != NULL) {
        for (k = 0; k < (HASH_KEYS); k++) {
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");

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
 *   void **SearchInLUT_P( lut_t *lut, void *old_item)
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
SearchInLUT_P (lut_t *lut, void *old_item)
{
    void **new_item_p;

    DBUG_ENTER ("SearchInLUT_P");

    new_item_p = SearchInLUT_state (lut, old_item, GetHashKey_Pointer (old_item),
                                    IsEqual_Pointer, TRUE, F_PTR, F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void **SearchInLUT_S( lut_t *lut, char *old_item)
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
SearchInLUT_S (lut_t *lut, char *old_item)
{
    void **new_item_p;

    DBUG_ENTER ("SearchInLUT_S");

    new_item_p = SearchInLUT_state (lut, old_item, GetHashKey_String (old_item),
                                    IsEqual_String, TRUE, "\"%s\"", F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void **SearchInLUT_NextP( void)
 *
 * description:
 *   Searches for the next entry matching for the given pointer.
 *   Iff no more matching entries are found in the LUT, NULL is returned.
 *
 * caution:
 *   The function SearchInLUT_P() must be called first!!
 *
 ******************************************************************************/

void **
SearchInLUT_NextP (void)
{
    void **new_item_p;

    DBUG_ENTER ("SearchInLUT_NextP");

    new_item_p = SearchInLUT_state (NULL, NULL, 0, IsEqual_Pointer, FALSE, F_PTR, F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void **SearchInLUT_NextS( void)
 *
 * description:
 *   Searches for the next entry matching for the given string.
 *   Iff no more matching entries are found in the LUT, NULL is returned.
 *
 * caution:
 *   The function SearchInLUT_S() must be called first!!
 *
 ******************************************************************************/

void **
SearchInLUT_NextS (void)
{
    void **new_item_p;

    DBUG_ENTER ("SearchInLUT_NextS");

    new_item_p
      = SearchInLUT_state (NULL, NULL, 0, IsEqual_String, FALSE, "\"%s\"", F_PTR);

    DBUG_RETURN (new_item_p);
}

/******************************************************************************
 *
 * function:
 *   void *SearchInLUT_PP( lut_t *lut, void *old_item)
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
SearchInLUT_PP (lut_t *lut, void *old_item)
{
    void **new_item_p;
    void *new_item;

    DBUG_ENTER ("SearchInLUT_PP");

    new_item_p = SearchInLUT_P (lut, old_item);

    new_item = (new_item_p == NULL) ? old_item : *new_item_p;

    DBUG_RETURN (new_item);
}

/******************************************************************************
 *
 * function:
 *   char *SearchInLUT_SS( lut_t *lut, char *old_item)
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
SearchInLUT_SS (lut_t *lut, char *old_item)
{
    char **new_item_p;
    char *new_item;

    DBUG_ENTER ("SearchInLUT_SS");

    new_item_p = (char **)SearchInLUT_state (lut, old_item, GetHashKey_String (old_item),
                                             IsEqual_String, TRUE, "\"%s\"", "\"%s\"");

    new_item = (new_item_p == NULL) ? old_item : *new_item_p;

    DBUG_RETURN (new_item);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT_P( lut_t *lut, void *old_item, void *new_item)
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
 ******************************************************************************/

lut_t *
InsertIntoLUT_P (lut_t *lut, void *old_item, void *new_item)
{
    DBUG_ENTER ("InsertIntoLUT_P");

    lut = InsertIntoLUT (lut, old_item, new_item, GetHashKey_Pointer (old_item), F_PTR,
                         F_PTR);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *InsertIntoLUT_S( lut_t *lut, char *old_item, void *new_item)
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
InsertIntoLUT_S (lut_t *lut, char *old_item, void *new_item)
{
    DBUG_ENTER ("InsertIntoLUT_S");

    lut = InsertIntoLUT (lut, StringCopy (old_item), new_item,
                         GetHashKey_String (old_item), "\"%s\"", F_PTR);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *UpdateLUT_P( lut_t *lut, void *old_item, void *new_item,
 *                       void **found_item)
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
 ******************************************************************************/

lut_t *
UpdateLUT_P (lut_t *lut, void *old_item, void *new_item, void **found_item)
{
    DBUG_ENTER ("UpdateLUT_P");

    lut = UpdateLUT (lut, old_item, new_item, GetHashKey_Pointer (old_item),
                     IsEqual_Pointer, F_PTR, F_PTR, found_item);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * function:
 *   lut_t *UpdateLUT_S( lut_t *lut, char *old_item, void *new_item,
 *                       void **found_item)
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
UpdateLUT_S (lut_t *lut, char *old_item, void *new_item, void **found_item)
{
    DBUG_ENTER ("UpdateLUT_S");

    lut = UpdateLUT (lut, old_item, new_item, GetHashKey_String (old_item),
                     IsEqual_String, "\"%s\"", F_PTR, found_item);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   lut_t *MapLUT_S( lut_t *lut, void *(*fun)( void *))
 *
 * Description:
 *   Applies 'fun' to all data found in the LUT which is associated to strings.
 *
 ******************************************************************************/

lut_t *
MapLUT_S (lut_t *lut, void *(*fun) (void *))
{
    DBUG_ENTER ("MapLUT_S");

    lut = MapLUT (lut, fun, HASH_KEYS_POINTER, HASH_KEYS);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   lut_t *MapLUT_P( lut_t *lut, void *(*fun)( void *))
 *
 * Description:
 *   Applies 'fun' to all data found in the LUT which is associated to pointers.
 *
 ******************************************************************************/

lut_t *
MapLUT_P (lut_t *lut, void *(*fun) (void *))
{
    DBUG_ENTER ("MapLUT_P");

    lut = MapLUT (lut, fun, 0, HASH_KEYS_POINTER);

    DBUG_RETURN (lut);
}

/******************************************************************************
 *
 * Function:
 *   void *FoldLUT_S( lut_t *lut, void *init, void *(*fun)( void *, void *))
 *
 * Description:
 *   Applies 'fun' to all data found in the LUT which is associated to strings.
 *
 ******************************************************************************/

void *
FoldLUT_S (lut_t *lut, void *init, void *(*fun) (void *, void *))
{
    DBUG_ENTER ("FoldLUT_S");

    init = FoldLUT (lut, init, fun, HASH_KEYS_POINTER, HASH_KEYS);

    DBUG_RETURN (init);
}

/******************************************************************************
 *
 * Function:
 *   void *FoldLUT_P( lut_t *lut, void *init, void *(*fun)( void *, void *))
 *
 * Description:
 *   Applies 'fun' to all data found in the LUT which is associated to pointers.
 *
 ******************************************************************************/

void *
FoldLUT_P (lut_t *lut, void *init, void *(*fun) (void *, void *))
{
    DBUG_ENTER ("FoldLUT_P");

    init = FoldLUT (lut, init, fun, 0, HASH_KEYS_POINTER);

    DBUG_RETURN (init);
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
    void **tmp;
    hash_key_t k;
    lut_size_t i;

    DBUG_ENTER ("PrintLUT");

    DBUG_PRINT ("LUT", ("> lut (" F_PTR ")", lut));

    if (handle == NULL) {
        handle = stderr;
    }

    if (lut != NULL) {
        for (k = 0; k < (HASH_KEYS_POINTER); k++) {
            fprintf (handle, "*** pointers: hash key %i ***\n", k);
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                fprintf (handle, "%i: [ " F_PTR " -> " F_PTR " ]\n", i, tmp[0], tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                }
            }
            fprintf (handle, "number of entries: %i\n", lut[k].size);
        }
        for (k = (HASH_KEYS_POINTER); k < (HASH_KEYS); k++) {
            fprintf (handle, "*** strings: hash key %i ***\n", k);
            DBUG_ASSERT ((lut[k].size >= 0), "illegal LUT size found!");
            tmp = lut[k].first;
            for (i = 0; i < lut[k].size; i++) {
                fprintf (handle, "%i: [ \"%s\" -> " F_PTR " ]\n", i, (char *)(tmp[0]),
                         tmp[1]);
                tmp += 2;
                if ((i + 1) % (LUT_SIZE) == 0) {
                    /* last table entry is reached -> enter next table of the chain */
                    tmp = *tmp;
                }
            }
            fprintf (handle, "number of entries: %i\n", lut[k].size);
        }

        DBUG_PRINT ("LUT", ("< finished"));
    } else {
        DBUG_PRINT ("LUT", ("< FAILED: lut is NULL"));
    }

    DBUG_VOID_RETURN;
}
