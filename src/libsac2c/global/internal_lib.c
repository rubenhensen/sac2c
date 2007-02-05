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

static int
NumDigits (int number)
{
    int i = 1;

    DBUG_ENTER ("NumDigits");

    while (number / 10 >= 1) {
        number = number / 10;
        i += 1;
    }

    DBUG_RETURN (i);
}

char *
ILIBtmpVar (void)
{
    static int counter = 0;
    const char *prefix;
    char *result;

    DBUG_ENTER ("ILIBtmpVar");

    prefix = TRAVgetName ();
    result
      = (char *)MEMmalloc ((strlen (prefix) + NumDigits (counter) + 3) * sizeof (char));
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
