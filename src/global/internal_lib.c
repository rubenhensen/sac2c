/*
 * $Id$
 */
#include "internal_lib.h"
#undef ILIBmalloc

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "dbug.h"

#include "ctinfo.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "traverse.h"
#include "convert.h"
#include "filemgr.h"
#include "check_mem.h"

/*
 * experimental support for garbage collection
 */

#ifdef GC
#include <gc.h>
#define malloc(n) GC_malloc (n)
#endif /* GC */

#define MAX_SYSCALL 1000

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

int malloc_align_step = 0;

/**
 * global file handle for syscall tracking
 */
FILE *syscalltrack = NULL;

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

void *
ILIBmalloc (int size)
{
    void *tmp;

    DBUG_ENTER ("ILIBmalloc");

    DBUG_ASSERT ((size >= 0), "ILIBmalloc called with negative size!");

    if (size > 0) {

#ifdef SHOW_MALLOC
        tmp = malloc (size + malloc_align_step);
#else  /* SHOW_MALLOC */
        tmp = malloc (size);
#endif /* SHOW_MALLOC */

        /*
         * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
         * we do complain for ((NULL == tmp) && (size > 0)) only!!
         */
        if (tmp == NULL) {

#ifdef SHOW_MALLOC
            CTIabort ("Out of memory: %u Bytes already allocated",
                      global.current_allocated_mem);
#else
            CTIabort ("Out of memory");
#endif
        }
#ifdef SHOW_MALLOC
        tmp = CHKMregisterMem (size, tmp);

        if (global.current_allocated_mem + size < global.current_allocated_mem) {
            DBUG_ASSERT ((0), "counter for allocated memory: overflow detected");
        }
        global.current_allocated_mem += size;
        if (global.max_allocated_mem < global.current_allocated_mem) {
            global.max_allocated_mem = global.current_allocated_mem;
        }

        DBUG_PRINT ("MEM_ALLOC", ("Alloc memory: %d Bytes at adress: " F_PTR, size, tmp));

        DBUG_PRINT ("MEM_TOTAL",
                    ("Currently allocated memory: %u", global.current_allocated_mem));

#endif /* SHOW_MALLOC */

#ifdef CLEANMEM
        /*
         * Initialize memory
         */
        tmp = memset (tmp, 0, size);
#endif

    } else {
        tmp = NULL;
    }

    DBUG_RETURN (tmp);
}

#ifdef SHOW_MALLOC

void *
ILIBmallocAt (int size, char *file, int line)
{
    void *pointer;

    DBUG_ENTER ("ILIBmallocAt");

    /*  CHKMsetLocation(size, file, line); */

    pointer = ILIBmalloc (size);

    DBUG_RETURN (pointer);
}

#endif /* SHOW_MALLOC */

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
#ifdef SHOW_MALLOC
    void *orig_address;
    int size;
#endif /* SHOW_MALLOC */

    DBUG_ENTER ("ILIBfree");

    if (address != NULL) {
#ifdef SHOW_MALLOC
        orig_address = CHKMunregisterMem (address);

        size = CHKMgetSize (address);

        DBUG_ASSERT ((size >= 0), "illegal size found!");
        DBUG_PRINT ("MEM_ALLOC",
                    ("Free memory: %d Bytes at adress: " F_PTR, size, address));

#ifdef CLEANMEM
        /*
         * this code overwrites the memory prior to freeing it. This
         * is very useful when watching a memory address in gdb, as
         * one gets notified as soon as it is freed. Needs SHOW_MALLOC
         * to get the size of the freed memory chunk.
         */
        orig_address = memset (orig_address, 0, size);
#endif /* CLEANMEM */

        if (global.current_allocated_mem < global.current_allocated_mem - size) {
            DBUG_ASSERT ((0), "counter for allocated memory: overflow detected");
        }
        global.current_allocated_mem -= size;

        free (orig_address);

        DBUG_PRINT ("MEM_TOTAL",
                    ("Currently allocated memory: %u", global.current_allocated_mem));
#else  /* SHOW_MALLOC */
        free (address);
#endif /* SHOW_MALLOC */

        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* NOFREE */

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

/******************************************************************************
 *
 * function:
 *   char *ILIBstringConcat4( const char *first, const char* second,
 *                            const char *third, const char* fourth)
 *
 * description
 *   Reserves new memory for the concatinated string first + second + third
 *   + fourth, and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *
ILIBstringConcat4 (const char *first, const char *second, const char *third,
                   const char *fourth)
{
    char *result;

    DBUG_ENTER ("ILIBstringConcat");

    result = (char *)ILIBmalloc (strlen (first) + strlen (second) + strlen (third)
                                 + strlen (fourth) + 1);

    strcpy (result, first);
    strcat (result, second);
    strcat (result, third);
    strcat (result, fourth);

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

#define HEX2DIG(x) (((x >= '0') && (x <= '9')) ? (x - '0') : (10 + x - 'A'))

unsigned char *
ILIBhexStringToByteArray (unsigned char *array, const char *string)
{
    int pos;

    DBUG_ENTER ("ILIBhexStringToByteArray");

    pos = 0;

    while (string[pos * 2] != 0) {
        unsigned char low = HEX2DIG (string[pos * 2 + 1]);
        unsigned char high = HEX2DIG (string[pos * 2]);

        array[pos] = high * 16 + low;
        pos++;
    }

    DBUG_RETURN (array);
}

#define DIG2HEX(x) ((x < 10) ? ('0' + x) : ('A' + x - 10))

char *
ILIBbyteArrayToHexString (int len, unsigned char *array)
{
    int pos;
    char *result;

    DBUG_ENTER ("ILIBbyteArrayToHexString");

    result = ILIBmalloc ((1 + len * 2) * sizeof (char));

    for (pos = 0; pos < len; pos++) {
        unsigned char low = array[pos] % 16;
        unsigned char high = array[pos] / 16;

        result[2 * pos] = (char)DIG2HEX (high);
        result[2 * pos + 1] = (char)DIG2HEX (low);
    }

    result[2 * len] = '\0';

    DBUG_RETURN (result);
}

void
ILIBsystemCallStartTracking ()
{
    DBUG_ENTER ("ILIBsystemCallStartTracking");

    DBUG_ASSERT ((syscalltrack == NULL), "tracking has already been enabled!");

    syscalltrack = FMGRwriteOpen ("syscall.sh");

    fprintf (syscalltrack, "#! /bin/sh\n\n");

    DBUG_VOID_RETURN;
}

void
ILIBsystemCallStopTracking ()
{
    DBUG_ENTER ("ILIBsystemCallStopTracking");

    DBUG_ASSERT ((syscalltrack != NULL), "no tracking log open!");

    fclose (syscalltrack);

    syscalltrack = NULL;

    DBUG_VOID_RETURN;
}

static void
trackSystemCall (const char *call)
{
    DBUG_ENTER ("trackSystemCall");

    if (syscalltrack != NULL) {
        fprintf (syscalltrack, "%s\n\n", call);
    }

    DBUG_VOID_RETURN;
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

    /* if -d syscall flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        CTInote ("System call:\n %s", syscall);
    }

    trackSystemCall (syscall);
    exit_code = system (syscall);

    if (exit_code == -1) {
        CTIabort ("System failure while trying to execute shell command.\n"
                  "(e.g. out of memory).");
    } else if (WEXITSTATUS (exit_code) > 0) {
        CTIabort ("System failed to execute shell command\n%s\n"
                  "with exit code %d",
                  syscall, WEXITSTATUS (exit_code));
    } else if (WIFSIGNALED (exit_code)) {
        if (WTERMSIG (exit_code) == SIGINT) {
            CTIabort ("Child recieved SIGINT when executing shell command \n%s\n",
                      syscall);
        } else if (WTERMSIG (exit_code) == SIGQUIT) {
            CTIabort ("Child recieved SIGQUIT when executing shell command \n%s\n",
                      syscall);
        }
    } else if (exit_code != 0) {
        CTIabort ("Unknown failure while executing shell command \n%s\n"
                  "Return value was %d",
                  syscall, exit_code);
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
        CTInote ("System call:\n%s", syscall);
    }

    trackSystemCall (syscall);

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
 *   void ILIBcreateCppCallString( const char *file, char *cccallstr
 *                                 const char *cppfile)
 *
 * Description:
 *   Checks whether the given filename is empty, i.e., we are reading from
 *   stdin, and generates a system call string in the buffer provided by
 *   cccallstr.
 *
 ******************************************************************************/

void
ILIBcreateCppCallString (const char *file, char *cccallstr, const char *cppfile)
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
 *   char *ILIBreplaceSpecialCharacters( const char *name)
 *
 * description:
 *   Replaces special characters such that they can be used as identifiers
 *   in a C program.
 *
 *****************************************************************************/
char *
ILIBreplaceSpecialCharacters (const char *name)
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
        case ',':
            tmp = "_CM";
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
        case ' ':
            tmp = "_SP";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '{':
            tmp = "_CO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '}':
            tmp = "_CC";
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
    CTInote ("*** Currently allocated memory (Bytes):   %s",
             CVintBytes2String (global.current_allocated_mem));
    ast_dup = DUPdoDupTree (global.syntax_tree);
    CTInote ("*** Size of the syntax tree (Bytes):      %s",
             CVintBytes2String (global.current_allocated_mem - mem_before));
    CTInote ("*** Other memory allocated/ Leak (Bytes): %s",
             CVintBytes2String (2 * mem_before - global.current_allocated_mem));
    FREEdoFreeTree (ast_dup);
    CTInote ("*** FreeTree / DupTree leak (Bytes):      %s",
             CVintBytes2String (global.current_allocated_mem - mem_before));

    DBUG_VOID_RETURN;
}

#endif /* DBUG_OFF */
