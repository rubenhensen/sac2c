/*
 *
 * $Log$
 * Revision 3.74  2004/11/30 14:50:42  sah
 * fixed a segfault in ILIBcreateCppCallString
 *
 * Revision 3.73  2004/11/27 00:47:56  cg
 * function name typos fixed
 *
 * Revision 3.72  2004/11/27 00:36:39  cg
 * functions adapted to new naming conventions.
 *
 * Revision 3.71  2004/11/26 23:55:16  sbs
 * further renamings
 *
 * Revision 3.70  2004/11/26 23:36:01  sbs
 * *** empty log message ***
 *
 * Revision 3.69  2004/11/26 23:16:36  sbs
 * *** empty log message ***
 *
 * Revision 3.68  2004/11/26 23:01:36  ktr
 * From denmark with love.
 *
 * Revision 3.67  2004/11/24 22:43:37  cg
 * Moved NumberOfDigits() from Error.c.
 *
 * Revision 3.66  2004/11/24 22:30:02  ktr
 * replaceSpecialCharacters moved from precompile.
 *
 * Revision 3.65  2004/11/24 22:19:44  cg
 * SacDevCamp approved.
 *
 * Revision 3.64  2004/11/14 13:45:43  ktr
 * added support for reuse branching (emrb_tab)
 *
 * Revision 3.63  2004/11/09 22:18:07  ktr
 * Added Explicit Copy (emec_tab)
 *
 * Revision 3.62  2004/11/07 16:10:51  ktr
 * added -o parameter to CPP syscall.
 *
 * Revision 3.61  2004/11/07 14:31:34  ktr
 * CreateCppCallString now needs a third parameter for the temporary file
 * created in /tmp that is used for CPP's output.
 *
 * Revision 3.60  2004/11/02 14:58:36  sah
 * added MemCopy
 *
 * Revision 3.59  2004/11/02 14:22:22  ktr
 * Added emlr_tab, emdr_tab.
 *
 * Revision 3.58  2004/10/28 22:08:39  sah
 * stringbuffers are now initialised properly!
 *
 * Revision 3.57  2004/10/07 12:38:37  ktr
 * Replaced the old With-Loop Scalarization with a new implementation.
 *
 * Revision 3.56  2004/10/05 13:48:50  sah
 * added some more funtabs to TmpVar in
 * NEW_AST mode
 *
 * Revision 3.55  2004/09/29 16:42:33  sbs
 * fixed a memory leak in StrBufprint
 *
 * Revision 3.54  2004/09/28 16:32:19  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.53  2004/09/28 14:07:30  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.52  2004/09/23 21:13:47  sah
 * TmpVar(): added set traversal
 *
 * Revision 3.51  2004/09/22 13:19:11  sah
 * changed argument to StringCopy to const char*
 * as the functions does not modify it
 *
 * Revision 3.50  2004/09/02 12:23:49  skt
 * added repfun_tab into PrefixForTmpVar
 *
 * Revision 3.49  2004/08/26 14:17:00  skt
 * added crwiw_tab into PrefixForTmpVar
 *
 * Revision 3.48  2004/08/13 18:02:34  skt
 * removed blkli_tab
 *
 * Revision 3.47  2004/08/06 14:41:32  sah
 * adding support for new ast
 *
 * Revision 3.46  2004/07/21 12:40:38  khf
 * TmpVar(): ea_tab added
 *
 * Revision 3.45  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.44  2004/07/14 15:29:54  ktr
 * Nothing really changed.
 *
 * Revision 3.43  2004/04/08 08:17:45  khf
 * TmpVar(): wlfs_tab added
 *
 * Revision 3.42  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.41  2004/02/26 13:07:46  khf
 * TmpVar(): wlpg_tab added
 *
 * Revision 3.40  2003/09/30 22:44:44  dkrHH
 * Free(): DBUG_ASSERT put into if-clause to ease debugging
 *
 * Revision 3.39  2003/09/30 22:39:52  dkrHH
 * Malloc(): DBUG_ASSERT put into if-clause to ease debugging
 *
 * Revision 3.38  2003/09/09 14:57:23  sbs
 * PtrBuf support added.
 *
 * Revision 3.37  2003/08/16 08:38:03  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.36  2003/05/18 13:36:43  ktr
 * removed printing of new variable name in TmpVarName.
 *
 * Revision 3.35  2003/05/14 20:00:39  ktr
 * TmpVarName now avoids to prepend the same prefix twice.
 *
 * Revision 3.34  2003/04/26 20:48:47  mwe
 * esd_tab added
 *
 * Revision 3.33  2003/03/24 16:36:16  sbs
 * CreateCppCallString added.
 *
 * Revision 3.32  2003/03/20 20:48:18  sah
 * DbugMemoryLeakCheck only works
 * if SHOW_MALLOC is set, as it
 * depends on current_allocated_mem.
 *
 * Revision 3.31  2003/02/08 15:58:00  mwe
 * (TmpVar) dl_tab added
 *
 * Revision 3.30  2002/10/08 00:59:22  dkr
 * TmpVar(): nt2ot_tab added
 *
 * Revision 3.29  2002/09/11 23:19:24  dkr
 * NumberOfDigits() used
 *
 * Revision 3.28  2002/09/05 21:17:40  dkr
 * TmpVar() modified
 *
 * Revision 3.27  2002/09/05 20:29:23  dkr
 * PrefixForTmpVar() added
 *
 * Revision 3.26  2002/09/03 14:40:28  sbs
 * ntc prefix for tmp_var added
 *
 * Revision 3.25  2002/09/03 13:18:06  sbs
 * StrBuf support added
 *
 * Revision 3.24  2002/08/25 14:21:46  mwe
 * AssociativeLaw.h added
 *
 * Revision 3.23  2002/08/14 09:53:22  dkr
 * - DBUG_ASSERT for overflow in allocation counter corrected
 * - Malloc(0) returns NULL on all architectures now
 *
 * Revision 3.22  2002/08/13 17:19:46  dkr
 * Free(): DBUG_ASSERT for overflow in allocation counter added
 *
 * Revision 3.21  2002/08/12 20:58:24  dkr
 * TmpVar(): cwc_tab added
 *
 * Revision 3.20  2002/07/23 12:08:58  sah
 * hd_tab added to TmpVar known tables.
 *
 * Revision 3.19  2002/07/12 16:57:27  dkr
 * TmpVar(): modification for new backend done
 *
 * Revision 3.18  2002/04/09 08:22:17  dkr
 * TmpVar(): some trav-tables added, DBUG_ASSERT added.
 *
 * Revision 3.17  2002/04/09 08:03:43  ktr
 * Support for WithloopScalarization added at TmpVar()
 *
 * Revision 3.16  2002/04/05 10:27:18  dkr
 * TmpVar(): precomp3_tab added
 *
 * Revision 3.15  2001/07/13 13:23:41  cg
 * Some useless DBUG_PRINTs eliminated, memory management DBUG_PRINTs
 * converted to some standardized form.
 *
 * Revision 3.14  2001/06/21 11:04:40  ben
 * one bug fixed in StrTok
 *
 * Revision 3.13  2001/06/20 11:33:59  ben
 * StrTok implemented
 *
 * Revision 3.12  2001/05/17 13:29:29  cg
 * Moved de-allocation function Free() from free.c to internal_lib.c
 *
 * Revision 3.11  2001/05/17 11:15:59  sbs
 * return value of Free used now 8-()
 *
 * Revision 3.10  2001/05/17 10:04:16  nmw
 * missing include of convert.h added
 *
 * Revision 3.9  2001/05/17 08:36:25  sbs
 * DbugMemoryLeakCheck added.
 *
 * Revision 3.8  2001/05/15 15:53:08  nmw
 * new ssawlX phases added to TmpVar
 *
 * Revision 3.7  2001/04/26 15:51:44  dkr
 * rmvoidfun removed from TmpVar()
 *
 * [...]
 *
 */
#include "internal_lib.h"

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "dbug.h"

#include "Error.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "traverse.h"
#include "convert.h"

/*
 * experimental support for garbage collection
 */

#ifdef GC
#include <gc.h>
#define malloc(n) GC_malloc (n)
#endif /* GC */

#define MAX_SYSCALL 1000

#ifdef SHOW_MALLOC
/*
 * These types are only used to compute malloc_align_step.
 * No instances are raised.
 */
typedef union {
    long int l;
    double d;
} malloc_align_type;

typedef struct {
    int size;
    malloc_align_type align;
} malloc_header_type;

#endif /* SHOW_MALLOC */

static int malloc_align_step = 0;

/******************************************************************************
 *
 * function:
 *   void *ILIBmalloc( int size)
 *   void *ILIBfree( void *address)
 *
 * description:
 *   These functions for memory allocation and de-allocation are wrappers
 *   for the standard functions malloc() and free().
 *
 *   They allow to implement some additional functionality, e.g. accounting
 *   of currently allocated memory.
 *
 ******************************************************************************/

#ifdef SHOW_MALLOC

void *
ILIBmalloc (int size)
{
    void *tmp;

    DBUG_ENTER ("ILIBmalloc");

    DBUG_ASSERT ((size >= 0), "ILIBmalloc called with negative size!");

    if (size > 0) {
        tmp = malloc (size + malloc_align_step);

        /*
         * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
         * we do complain for ((NULL == tmp) && (size > 0)) only!!
         */
        if (tmp == NULL) {
            SYSABORT (("Out of memory: %u Bytes already allocated!",
                       global.current_allocated_mem));
        }

        *(int *)tmp = size;
        tmp = (char *)tmp + malloc_align_step;

        if (global.current_allocated_mem + size < global.current_allocated_mem) {
            DBUG_ASSERT ((0), "counter for allocated memory: overflow detected");
        }
        global.current_allocated_mem += size;
        if (global.max_allocated_mem < global.current_allocated_mem) {
            global.max_allocated_mem = global.current_allocated_mem;
        }
    } else {
        tmp = NULL;
    }

    DBUG_PRINT ("MEM_ALLOC", ("Alloc memory: %d Bytes at adress: " F_PTR, size, tmp));

    DBUG_PRINT ("MEM_TOTAL",
                ("Currently allocated memory: %u", global.current_allocated_mem));

    DBUG_RETURN (tmp);
}

#ifdef NOFREE

void *
ILIBfree (void *address)
{
    DBUG_ENTER ("ILIBfree");

    address = NULL;

    DBUG_RETURN (address);
}

#else /* NOFREE */

void *
ILIBfree (void *address)
{
    void *orig_address;
    int size;

    DBUG_ENTER ("ILIBfree");

    if (address != NULL) {
        orig_address = (void *)((char *)address - malloc_align_step);
        size = *(int *)orig_address;
        DBUG_ASSERT ((size >= 0), "illegal size found!");
        DBUG_PRINT ("MEM_ALLOC",
                    ("Free memory: %d Bytes at adress: " F_PTR, size, address));

        if (global.current_allocated_mem < global.current_allocated_mem - size) {
            DBUG_ASSERT ((0), "counter for allocated memory: overflow detected");
        }
        global.current_allocated_mem -= size;

        free (orig_address);

        DBUG_PRINT ("MEM_TOTAL",
                    ("Currently allocated memory: %u", global.current_allocated_mem));

        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* NOFREE */

#else /* SHOW_MALLOC */

void *
ILIBmalloc (int size)
{
    void *tmp;

    DBUG_ENTER ("ILIBmalloc");

    DBUG_ASSERT ((size >= 0), "ILIBmalloc called with negative size!");

    if (size > 0) {
        tmp = malloc (size);

        /*
         * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
         * we do complain for ((NULL == tmp) && (size > 0)) only!!
         */
        if (tmp == NULL) {
            SYSABORT (("Out of memory"));
        }
    } else {
        tmp = NULL;
    }

    DBUG_PRINT ("MEM_ALLOC", ("Alloc memory: %d Bytes at adress: " F_PTR, size, tmp));

    DBUG_RETURN (tmp);
}

#ifdef NOFREE

void *
ILIBfree (void *address)
{
    DBUG_ENTER ("ILIBfree");

    address = NULL;

    DBUG_RETURN (address);
}

#else /* NOFREE */

void *
ILIBfree (void *address)
{
    DBUG_ENTER ("ILIBfree");

    if (address != NULL) {
        free (address);

        DBUG_PRINT ("MEM_ALLOC", ("Free memory: ??? Bytes at adress: " F_PTR, address));

        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* NOFREE */

#endif /* SHOW_MALLOC */

struct PTR_BUF {
    void **buf;
    int pos;
    int size;
};

/** <!--********************************************************************-->
 *
 * @fn  ptr_buf *ILIBptrBufCreate( int size)
 *
 *   @brief  creates an (unbound) pointer buffer
 *
 *           Similar mechanism as used for StrBuf's.
 *           Here, instead of characters, void pointers are stored.
 *   @param  size
 *   @return the pointer to the freshly allocated buffer.
 *
 ******************************************************************************/

ptr_buf *
ILIBptrBufCreate (int size)
{
    ptr_buf *res;

    DBUG_ENTER ("ILIBptrBufCreate");

    res = (ptr_buf *)ILIBmalloc (sizeof (ptr_buf));
    res->buf = (void **)ILIBmalloc (size * sizeof (void *));
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("PTRBUF", ("allocating buffer size %d : %p", size, res));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  ptr_buf *ILIBptrBufAdd(  ptr_buf *s, void *ptr)
 *
 *   @brief  adds ptr to buffer s (new last element)
 *
 *   @param  s
 *   @param  ptr
 *   @return the modified buffer
 *
 ******************************************************************************/

ptr_buf *
ILIBptrBufAdd (ptr_buf *s, void *ptr)
{
    int new_size;
    void **new_buf;
    int i;

    DBUG_ENTER ("ILIBptrBufAdd");

    if (s->pos == s->size) {
        new_size = 2 * s->size;
        DBUG_PRINT ("PTRBUF", ("increasing buffer %p from size %d to size %d", s, s->size,
                               new_size));

        new_buf = (void **)ILIBmalloc (new_size * sizeof (void *));
        for (i = 0; i < s->pos; i++) {
            new_buf[i] = s->buf[i];
        }
        s->buf = ILIBfree (s->buf);
        s->buf = new_buf;
        s->size = new_size;
    }
    s->buf[s->pos] = ptr;
    s->pos++;
    DBUG_PRINT ("PTRBUF", ("%p added to buffer %p", ptr, s));
    DBUG_PRINT ("PTRBUF", ("pos of buffer %p now is %d", s, s->pos));

    DBUG_RETURN (s);
}

/** <!--********************************************************************-->
 *
 * @fn  int ILIBptrBufGetSize(  ptr_buf *s)
 *
 *   @brief  retrieve size of given pointer buffer
 *
 *   @param  s
 *   @return size of the buffer
 *
 ******************************************************************************/

int
ILIBptrBufGetSize (ptr_buf *s)
{
    DBUG_ENTER ("ILIBptrBufGetSize");
    DBUG_RETURN (s->size);
}

/** <!--********************************************************************-->
 *
 * @fn  void *ILIBptrBufGetPtr(  ptr_buf *s, int pos)
 *
 *   @brief  get pointer entry at specified position
 *
 *   @param  s
 *   @param  pos
 *   @return entry
 *
 ******************************************************************************/

void *
ILIBptrBufGetPtr (ptr_buf *s, int pos)
{
    void *res;

    DBUG_ENTER ("ILIBptrBufGetPtr");
    if (pos < s->pos) {
        res = s->buf[pos];
    } else {
        res = NULL;
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn  void ILIBptrBufFlush(  ptr_buf *s)
 *
 *   @brief  flushes the given pointer buffer (no deallocation!)
 *
 *   @param  s
 *
 ******************************************************************************/

void
ILIBptrBufFlush (ptr_buf *s)
{
    DBUG_ENTER ("ILIBptrBufFlush");

    s->pos = 0;
    DBUG_PRINT ("PTRBUF", ("pos of buffer %p reset to %d", s, s->pos));

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn  void *ILIBptrBufFree(  ptr_buf *s)
 *
 *   @brief  deallocates the given pointer buffer!
 *
 *   @param  s
 *
 ******************************************************************************/

void *
ILIBptrBufFree (ptr_buf *s)
{
    DBUG_ENTER ("ILIBptrBufFree");

    DBUG_PRINT ("PTRBUF", ("freeing buffer %p", s));
    s->buf = ILIBfree (s->buf);
    s = ILIBfree (s);

    DBUG_RETURN (s);
}

struct STR_BUF {
    char *buf;
    int pos;
    int size;
};

/******************************************************************************
 *
 * Function:
 *   str_buf *ILIBstrBufCreate( int size);
 *
 * Description:
 *
 *
 ******************************************************************************/

str_buf *
ILIBstrBufCreate (int size)
{
    str_buf *res;

    DBUG_ENTER ("ILIBstrBufCreate");

    res = (str_buf *)ILIBmalloc (sizeof (str_buf));
    res->buf = (char *)ILIBmalloc (size * sizeof (char));
    res->buf[0] = '\0';
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("STRBUF", ("allocating buffer size %d : %p", size, res));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   str_buf *ILIBstrBufPrint(  str_buf *s, const char *string);
 *
 * Description:
 *
 *
 ******************************************************************************/

str_buf *
ILIBstrBufPrint (str_buf *s, const char *string)
{
    int len;
    int new_size;
    char *new_buf;

    DBUG_ENTER ("ILIBstrBufPrint");

    len = strlen (string);

    if ((len + 1) > (s->size - s->pos)) {

        new_size = (len >= s->size ? s->size + 2 * len : 2 * s->size);

        DBUG_PRINT ("STRBUF", ("increasing buffer %p from size %d to size %d", s, s->size,
                               new_size));

        new_buf = (char *)ILIBmalloc (new_size * sizeof (char));
        memcpy (new_buf, s->buf, s->pos + 1);
        s->buf = ILIBfree (s->buf);
        s->buf = new_buf;
        s->size = new_size;
    }

    s->pos += sprintf (&s->buf[s->pos], "%s", string);
    DBUG_PRINT ("STRBUF", ("pos of buffer %p now is %d", s, s->pos));

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * Function:
 *   str_buf *ILIBstrBufPrintf(  str_buf *s, const char *format, ...);
 *
 * Description:
 *
 *
 ******************************************************************************/

str_buf *
ILIBstrBufPrintf (str_buf *s, const char *format, ...)
{
    va_list arg_p;
    static char string[512];

    DBUG_ENTER ("ILIBstrBufPrintf");

    va_start (arg_p, format);
    vsprintf (string, format, arg_p);
    va_end (arg_p);

    DBUG_ASSERT (strlen (string) < 512, "string buffer in ILIBstrBufprintf too small!");

    s = ILIBstrBufPrint (s, string);

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * Function:
 *   char *ILIBstrBuf2String(  str_buf *s);
 *
 * Description:
 *
 *
 ******************************************************************************/

char *
ILIBstrBuf2String (str_buf *s)
{
    DBUG_ENTER ("ILIBstrBuf2String");

    DBUG_RETURN (ILIBstringCopy (s->buf));
}

/******************************************************************************
 *
 * Function:
 *   void ILIBstrBufFlush(  str_buf *s)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ILIBstrBufFlush (str_buf *s)
{
    DBUG_ENTER ("ILIBstrBufFlush");

    s->pos = 0;
    DBUG_PRINT ("STRBUF", ("pos of buffer %p reset to %d", s, s->pos));

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   bool ILIBstrBufIsEmpty( str_buf *s)
 *
 * Description:
 *
 *
 ******************************************************************************/

bool
ILIBstrBufIsEmpty (str_buf *s)
{
    DBUG_ENTER ("ILIBstrBufIsEmpty");

    DBUG_RETURN (s->pos == 0);
}

/******************************************************************************
 *
 * Function:
 *   void *ILIBstrBufFree( str_buf *s);
 *
 * Description:
 *
 ******************************************************************************/

void *
ILIBstrBufFree (str_buf *s)
{
    DBUG_ENTER ("ILIBstrBufFree");

    s->buf = ILIBfree (s->buf);
    s = ILIBfree (s);

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * Function:
 *   char *ILIBstringCopy( const char *source)
 *
 * Description:
 *   Allocates memory and returns a pointer to the copy of 'source'.
 *
 ******************************************************************************/

char *
ILIBstringCopy (const char *source)
{
    char *ret;

    DBUG_ENTER ("ILIBstringCopy");

    if (source != NULL) {
        ret = (char *)ILIBmalloc (sizeof (char) * (strlen (source) + 1));
        strcpy (ret, source);
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   char *ILIBstringConcat( char *first, char* second)
 *
 * description
 *   Reserves new memory for the concatinated string first + second,
 *   and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *
ILIBstringConcat (const char *first, const char *second)
{
    char *result;

    DBUG_ENTER ("ILIBstringConcat");

    result = (char *)ILIBmalloc (strlen (first) + strlen (second) + 1);

    strcpy (result, first);
    strcat (result, second);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *ILIBstringConcat3( const char *first, const char* second, const char *third)
 *
 * description
 *   Reserves new memory for the concatinated string first + second + third,
 *   and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *
ILIBstringConcat3 (const char *first, const char *second, const char *third)
{
    char *result;

    DBUG_ENTER ("ILIBstringConcat");

    result = (char *)ILIBmalloc (strlen (first) + strlen (second) + strlen (third) + 1);

    strcpy (result, first);
    strcat (result, second);
    strcat (result, third);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn  bool ILIBstringCompare( const char *first, const char *second)
 *
 *   @brief  compares two strings for equality
 *
 *   @param  first
 *   @param  second
 *
 ******************************************************************************/

bool
ILIBstringCompare (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ("ILIBstringCompare");

    res = (0 == strcmp (first, second));

    DBUG_RETURN (res);
}

/*
 *
 *  functionname  : ILIBnumberOfDigits
 *  arguments     : 1) integer
 *  description   : returns the number of digits, e.g 4 for 1000
 *
 */

int
ILIBnumberOfDigits (int number)
{
    int i = 1;

    DBUG_ENTER ("ILIBnumberOfDigits");

    while (number / 10 >= 1) {
        number = number / 10;
        i += 1;
    }

    DBUG_RETURN (i);
}

/******************************************************************************
 *
 * function:
 *   char *ILIBstrTok( char *first, char *sep)
 *
 * description
 *    Implements a version of the c-strtok, which can operate on static
 *    strings, too. It returns the string always till the next occurence off
 *    the string sep in first. If there are no more tokens in first a NULL
 *    pointer will be returned.
 *    On first call the string first will be copied, and on last call the
 *    allocated memory of the copy will be freeed.
 *    To get more than one token from one string, call ILIBstrTok with NULL as
 *    first parameter, just like c-strtok.
 *
 ******************************************************************************/

char *
ILIBstrTok (char *first, char *sep)
{
    static char *act_string = NULL;
    char *new_string = NULL;
    char *ret;

    DBUG_ENTER ("ILIBstrTok");

    if (first != NULL) {
        if (act_string != NULL) {
            act_string = ILIBfree (act_string);
        }
        new_string = ILIBstringCopy (first);
        act_string = new_string;
    }

    ret = strtok (new_string, sep);

    if (ret == NULL) {
        act_string = ILIBfree (act_string);
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * Function:
 *   void *ILIBmemCopy( int size, void *mem)
 *
 * Description:
 *   Allocates memory and returns a pointer to the copy of 'mem'.
 *
 ******************************************************************************/

void *
ILIBmemCopy (int size, void *mem)
{
    void *result;

    DBUG_ENTER ("ILIBmemCopy");

    result = ILIBmalloc (sizeof (char) * size);

    result = memcpy (result, mem, size);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   char *ILIBitoa( long number)
 *
 * Description:
 *   converts long to string
 *
 ******************************************************************************/

char *
ILIBitoa (long number)
{
    char *str;
    int tmp;
    int length, i;

    DBUG_ENTER ("ILIBitoa");

    tmp = number;
    length = 1;
    while (9 < tmp) {
        tmp /= 10;
        length++;
    }

    str = (char *)ILIBmalloc (sizeof (char) * length + 1);
    str[length] = atoi ("\0");

    for (i = 0; i < length; i++) {
        str[i] = ((int)'0') + (number / pow (10, (length - 1)));
        number = number % ((int)pow (10, (length - 1)));
    }

    DBUG_RETURN (str);
}

/******************************************************************************
 *
 * function:
 *   int ILIBlcm( int x, int y)
 *
 * description:
 *   returns the lowest-common-multiple of x, y.
 *
 ******************************************************************************/

int
ILIBlcm (int x, int y)
{
    int u, v;

    DBUG_ENTER ("ILIBlcm");

    DBUG_ASSERT (((x > 0) && (y > 0)), "arguments of lcm() must be >0");

    u = x;
    v = y;
    while (u != v) {
        if (u < v) {
            u += x;
        } else {
            v += y;
        }
    }

    DBUG_RETURN (u);
}

/******************************************************************************
 *
 * Function:
 *   void ILIBsystemCall( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
 *
 ******************************************************************************/

void
ILIBsystemCall (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];
    int exit_code;

    DBUG_ENTER ("ILIBsystemCall");

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

    /* if -dnocleanup flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        NOTE (("%s", syscall));
    }

    exit_code = system (syscall);

    if (exit_code > 0) {
        SYSABORT (("System failed to execute shell command\n%s\n"
                   "with exit code %d",
                   syscall, exit_code / 256));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   int ILIBsystemCall2( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   In contrast to SystemCall() no error message is printed upon failure but
 *   the exit code is returned.
 *
 ******************************************************************************/

int
ILIBsystemCall2 (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];

    DBUG_ENTER ("ILIBsystemCall2");

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

    /* if -dnocleanup flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        NOTE (("%s", syscall));
    }

    DBUG_RETURN (system (syscall));
}

/******************************************************************************
 *
 * Function:
 *   int ILIBsystemTest( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
 *
 ******************************************************************************/

int
ILIBsystemTest (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];
    int exit_code;

    DBUG_ENTER ("ILIBsystemTest");

    strcpy (syscall, "test ");

    va_start (arg_p, format);
    vsprintf (syscall + 5 * sizeof (char), format, arg_p);
    va_end (arg_p);

    exit_code = system (syscall);

    if (exit_code == 0) {
        exit_code = 1;
    } else {
        exit_code = 0;
    }

    DBUG_PRINT ("SYSCALL", ("test returns %d", exit_code));

    DBUG_RETURN (exit_code);
}

/******************************************************************************
 *
 * Function:
 *   void ILIBcreateCppCallString( char *file, char *cccallstr)
 *
 * Description:
 *   Checks whether the given filename is empty, i.e., we are reading from
 *   stdin, and generates a system call string in the buffer provided by
 *   cccallstr.
 *
 ******************************************************************************/

void
ILIBcreateCppCallString (char *file, char *cccallstr, char *cppfile)
{
    int i;

    DBUG_ENTER ("ILIBcreateCppCallString");

    if (file == NULL) { /*we are reading from stdin! */
        strcpy (cccallstr, global.config.cpp_stdin);
    } else {
        strcpy (cccallstr, global.config.cpp_file);
    }
    for (i = 0; i < global.num_cpp_vars; i++) {
        strcat (cccallstr, " ");
        strcat (cccallstr, global.config.opt_D);
        strcat (cccallstr, global.cpp_vars[i]);
    }
    for (i = 0; i < global.num_cpp_incs; i++) {
        strcat (cccallstr, " ");
        strcat (cccallstr, global.config.opt_I);
        strcat (cccallstr, global.cpp_incs[i]);
    }
    if (file != NULL) {
        strcat (cccallstr, " ");
        strcat (cccallstr, file);
    }

    strcat (cccallstr, " -o ");
    strcat (cccallstr, cppfile);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   char *ILIBtmpVar( void)
 *
 * Description:
 *   Generates string to be used as artificial variable.
 *   The variable name is different in each call of ILIBtmpVar().
 *   The string has the form "__tmp_" ++ compiler phase ++ consecutive number.
 *
 ******************************************************************************/

char *
ILIBtmpVar (void)
{
    static int counter = 0;
    const char *prefix;
    char *result;

    DBUG_ENTER ("ILIBtmpVar");

    prefix = TRAVgetName ();
    result = (char *)ILIBmalloc ((strlen (prefix) + ILIBnumberOfDigits (counter) + 3)
                                 * sizeof (char));
    sprintf (result, "_%s_%d", prefix, counter);
    counter++;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *ILIBtmpVarName( char* postfix)
 *
 * description:
 *   creates a unique variable like ILIBtmpVar() and additionally appends
 *   an individual string.
 *
 ******************************************************************************/

char *
ILIBtmpVarName (char *postfix)
{
    const char *tmp;
    char *result, *prefix;

    DBUG_ENTER ("ILIBtmpVarName");

    /* avoid chains of same prefixes */
    tmp = TRAVgetName ();

    if ((strlen (postfix) > (strlen (tmp) + 1)) && (postfix[0] == '_')
        && (strncmp ((postfix + 1), tmp, strlen (tmp)) == 0)) {
        postfix = postfix + strlen (tmp) + 2;
        while (postfix[0] != '_') {
            postfix++;
        }
    }

    prefix = ILIBtmpVar ();

    result = ILIBstringConcat3 (prefix, "_", postfix);

    ILIBfree (prefix);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *ILIBreplaceSpecialCharacters( char *name)
 *
 * description:
 *   Replaces special characters such that they can be used as identifiers
 *   in a C program.
 *
 *****************************************************************************/
char *
ILIBreplaceSpecialCharacters (char *name)
{
    char *new_name;
    char *tmp;
    int i, j;

    DBUG_ENTER ("ILIBreplaceSpecialCharacters");

    new_name = ILIBmalloc ((3 * strlen (name)) * sizeof (char));
    new_name[0] = '\0';

    for (i = 0, j = 0; (size_t)i < strlen (name); i++, j++) {
        switch (name[i]) {
        case '.':
            tmp = "_DO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '+':
            tmp = "_PL";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '-':
            tmp = "_MI";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '*':
            tmp = "_ST";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '/':
            tmp = "_DI";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '%':
            tmp = "_PR";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '<':
            tmp = "_LT";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '>':
            tmp = "_GT";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '=':
            tmp = "_EQ";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '&':
            tmp = "_AM";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '|':
            tmp = "_VE";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '!':
            tmp = "_EX";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '[':
            tmp = "_BL";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case ']':
            tmp = "_BR";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '~':
            tmp = "_TI";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '@':
            tmp = "_AT";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '#':
            tmp = "_HA";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '$':
            tmp = "_DO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '^':
            tmp = "_PO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '\\':
            tmp = "_BS";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case ':':
            tmp = "_CL";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        default:
            new_name[j] = name[i];
            new_name[j + 1] = '\0';
            break;
        }
    }

    DBUG_RETURN (new_name);
}

#ifdef SHOW_MALLOC

/* -------------------------------------------------------------------------- *
 * task: calculates the number of bytes for a safe alignment (used in ILIBmalloc)
 * initializes global variable malloc_align_step
 *
 * remarks: the c-compiler alignment of structs is exploited.
 * -------------------------------------------------------------------------- */

void
ILIBcomputeMallocAlignStep (void)
{
    DBUG_ENTER ("ComputeMallocAlignStep");

    /* calculate memory alignment steps for this machine */
    malloc_align_step = sizeof (malloc_header_type) - sizeof (malloc_align_type);

    DBUG_VOID_RETURN;
}

#endif /* SHOW_MALLOC */

#ifndef DBUG_OFF
#ifdef SHOW_MALLOC

/******************************************************************************
 *
 * function:
 *   void ILIBdbugMemoryLeakCheck( void)
 *
 * description:
 *   computes and prints memory usage w/o memory used for the actual
 *   syntax tree.
 *
 ******************************************************************************/

void
ILIBdbugMemoryLeakCheck (void)
{
    node *ast_dup;
    int mem_before;

    DBUG_ENTER ("ILIBdbugMemoryLeakCheck");

    mem_before = global.current_allocated_mem;
    NOTE2 (("*** Currently allocated memory (Bytes):   %s",
            CVintBytes2String (global.current_allocated_mem)));
    ast_dup = DUPdoDupTree (global.syntax_tree);
    NOTE2 (("*** Size of the syntax tree (Bytes):      %s",
            CVintBytes2String (global.current_allocated_mem - mem_before)));
    NOTE2 (("*** Other memory allocated/ Leak (Bytes): %s",
            CVintBytes2String (2 * mem_before - global.current_allocated_mem)));
    FREEdoFreeTree (ast_dup);
    NOTE2 (("*** FreeTree / DupTree leak (Bytes):      %s",
            CVintBytes2String (global.current_allocated_mem - mem_before)));

    DBUG_VOID_RETURN;
}

#endif /* SHOW_MALLOC */
#endif /* DBUG_OFF */
