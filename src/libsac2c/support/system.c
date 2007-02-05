/*
 * $Id$
 */
#include "internal_lib.h"
#include "str.h"
#include "memory.h"

#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "dbug.h"

#include "ctinfo.h"
#include "globals.h"
#include "traverse.h"
#include "filemgr.h"

/*
 * experimental support for garbage collection
 */

#ifdef GC
#include <gc.h>
#define malloc(n) GC_malloc (n)
#endif /* GC */

#define MAX_SYSCALL 1000

/**
 * global file handle for syscall tracking
 */
FILE *syscalltrack = NULL;
bool syscalltrack_active = FALSE;

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

    res = (ptr_buf *)MEMmalloc (sizeof (ptr_buf));
    res->buf = (void **)MEMmalloc (size * sizeof (void *));
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

        new_buf = (void **)MEMmalloc (new_size * sizeof (void *));
        for (i = 0; i < s->pos; i++) {
            new_buf[i] = s->buf[i];
        }
        s->buf = MEMfree (s->buf);
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
    s->buf = MEMfree (s->buf);
    s = MEMfree (s);

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

    res = (str_buf *)MEMmalloc (sizeof (str_buf));
    res->buf = (char *)MEMmalloc (size * sizeof (char));
    res->buf[0] = '\0';
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("STRBUF", ("allocating buffer size %d : %p", size, res));

    DBUG_RETURN (res);
}

static str_buf *
EnsureStrBufSpace (str_buf *s, int len)
{
    int new_size;
    char *new_buf;

    DBUG_ENTER ("EnsureStrBufSpace");

    if ((len + 1) > (s->size - s->pos)) {

        new_size = (len >= s->size ? s->size + 2 * len : 2 * s->size);

        DBUG_PRINT ("STRBUF", ("increasing buffer %p from size %d to size %d", s, s->size,
                               new_size));

        new_buf = (char *)MEMmalloc (new_size * sizeof (char));
        memcpy (new_buf, s->buf, s->pos + 1);
        s->buf = MEMfree (s->buf);
        s->buf = new_buf;
        s->size = new_size;
    }

    DBUG_RETURN (s);
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

    DBUG_ENTER ("ILIBstrBufPrint");

    len = strlen (string);

    s = EnsureStrBufSpace (s, len);

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
    int len, rem;
    bool ok;

    DBUG_ENTER ("ILIBstrBufPrintf");

    ok = FALSE;

    while (!ok) {
        rem = s->size - s->pos;

        va_start (arg_p, format);
        len = vsnprintf (&s->buf[s->pos], rem, format, arg_p);
        va_end (arg_p);

        if ((len >= 0) && (len < rem)) {
            ok = TRUE;
        } else {
            if (len < 0) {
                len = 2 * (s->size + 10);
            }
            s = EnsureStrBufSpace (s, len);
        }
    }

    s->pos += len;

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

    DBUG_RETURN (STRcpy (s->buf));
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

    s->buf = MEMfree (s->buf);
    s = MEMfree (s);

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * Function:
 *   char *STRcpy( const char *source)
 *
 * Description:
 *   Allocates memory and returns a pointer to the copy of 'source'.
 *
 ******************************************************************************/

char *
STRcpy (const char *source)
{
    char *ret;

    DBUG_ENTER ("STRcpy");

    if (source != NULL) {
        ret = (char *)MEMmalloc (sizeof (char) * (strlen (source) + 1));
        strcpy (ret, source);
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

char *
STRncpy (const char *source, int maxlen)
{
    char *ret;
    int max;

    DBUG_ENTER ("STRcpy");

    if (source != NULL) {
        max = strlen (source);
        if (max > maxlen) {
            max = maxlen;
        }

        ret = (char *)MEMmalloc (sizeof (char) * (max + 1));
        strncpy (ret, source, max);

        /* make sure string ends with 0 */
        ret[max] = '\0';
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   char *STRcat( char *first, char* second)
 *
 * description
 *   Reserves new memory for the concatinated string first + second,
 *   and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *
STRcat (const char *first, const char *second)
{
    char *result;

    DBUG_ENTER ("STRcat");

    result = (char *)MEMmalloc (strlen (first) + strlen (second) + 1);

    strcpy (result, first);
    strcat (result, second);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *STRcatn( 3, const char *first, const char* second, const char *third)
 *
 * description
 *   Reserves new memory for the concatinated string first + second + third,
 *   and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *STRcatn (3, const char *first, const char *second, const char *third)
{
    char *result;

    DBUG_ENTER ("STRcat");

    result = (char *)MEMmalloc (strlen (first) + strlen (second) + strlen (third) + 1);

    strcpy (result, first);
    strcat (result, second);
    strcat (result, third);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *STRcatn( 4, const char *first, const char* second,
 *                            const char *third, const char* fourth)
 *
 * description
 *   Reserves new memory for the concatinated string first + second + third
 *   + fourth, and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *STRcatn (4, const char *first, const char *second, const char *third,
               const char *fourth)
{
    char *result;

    DBUG_ENTER ("STRcat");

    result = (char *)MEMmalloc (strlen (first) + strlen (second) + strlen (third)
                                + strlen (fourth) + 1);

    strcpy (result, first);
    strcat (result, second);
    strcat (result, third);
    strcat (result, fourth);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn  bool STReq( const char *first, const char *second)
 *
 *   @brief  compares two strings for equality
 *
 *   @param  first
 *   @param  second
 *
 ******************************************************************************/

bool
STReq (const char *first, const char *second)
{
    bool res;

    DBUG_ENTER ("STReq");

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
 *   char *STRtok( char *first, char *sep)
 *
 * description
 *    Implements a version of the c-strtok, which can operate on static
 *    strings, too. It returns the string always till the next occurence off
 *    the string sep in first. If there are no more tokens in first a NULL
 *    pointer will be returned.
 *    On first call the string first will be copied, and on last call the
 *    allocated memory of the copy will be freeed.
 *    To get more than one token from one string, call STRtok with NULL as
 *    first parameter, just like c-strtok.
 *
 ******************************************************************************/

char *
STRtok (char *first, char *sep)
{
    static char *act_string = NULL;
    char *new_string = NULL;
    char *ret;

    DBUG_ENTER ("STRtok");

    if (first != NULL) {
        if (act_string != NULL) {
            act_string = MEMfree (act_string);
        }
        new_string = STRcpy (first);
        act_string = new_string;
    }

    ret = strtok (new_string, sep);

    if (ret == NULL) {
        act_string = MEMfree (act_string);
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

    result = MEMmalloc (sizeof (char) * size);

    result = memcpy (result, mem, size);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * Function:
 *   char *STRitoa( long number)
 *
 * Description:
 *   converts long to string
 *
 ******************************************************************************/

char *
STRitoa (long number)
{
    char *str;
    int tmp;
    int length, i;

    DBUG_ENTER ("STRitoa");

    tmp = number;
    length = 1;
    while (9 < tmp) {
        tmp /= 10;
        length++;
    }

    str = (char *)MEMmalloc (sizeof (char) * length + 1);
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
STRhex2Bytes (unsigned char *array, const char *string)
{
    int pos;

    DBUG_ENTER ("STRhex2Bytes");

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
STRbytes2Hex (int len, unsigned char *array)
{
    int pos;
    char *result;

    DBUG_ENTER ("STRbytes2Hex");

    result = MEMmalloc ((1 + len * 2) * sizeof (char));

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

    if (syscalltrack_active) {
        syscalltrack = FMGRappendOpen ("%s.sac2c", global.outfilename);
    } else {
        CTInote ("Creating cc call shell script `%s.sac2c'", global.outfilename);
        syscalltrack = FMGRwriteOpen ("%s.sac2c", global.outfilename);
        fprintf (syscalltrack, "#! /bin/sh\n\n");
    }

    syscalltrack_active = TRUE;

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
    result = (char *)MEMmalloc ((strlen (prefix) + ILIBnumberOfDigits (counter) + 3)
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

    result = STRcatn (3, prefix, "_", postfix);

    MEMfree (prefix);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *STRreplaceSpecialCharacters( const char *name)
 *
 * description:
 *   Replaces special characters such that they can be used as identifiers
 *   in a C program.
 *
 *****************************************************************************/
char *
STRreplaceSpecialCharacters (const char *name)
{
    char *new_name;
    char *tmp;
    int i, j;

    DBUG_ENTER ("STRreplaceSpecialCharacters");

    new_name = MEMmalloc ((3 * strlen (name)) * sizeof (char));
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

/** <!-- ****************************************************************** -->
 * @brief Converts its argument into a string that can be safely used when
 *        printing C code. It does so by replacing all occurences of '"'
 *        by '\"'.
 *
 * @param string a string.
 *
 * @return a safe string
 ******************************************************************************/
char *
STRstring2SafeCEncoding (const char *string)
{
    char *result, *tmp;
    int i, len;

    DBUG_ENTER ("STRstring2SafeCEncoding");

    len = strlen (string);

    result = MEMmalloc (len * 2 + 1);
    tmp = result;

    for (i = 0; i < len; i++) {
        if (string[i] == '"') {
            tmp += sprintf (tmp, "\\\"");
        } else {
            *tmp = string[i];
            tmp++;
        }
    }

    *tmp = '\0';

    DBUG_RETURN (result);
}
