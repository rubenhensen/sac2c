/*
 *
 * $Log$
 * Revision 1.15  1997/04/24 14:59:22  sbs
 * HAVE_MALLOC_O inserted
 *
 * Revision 1.14  1996/09/11  06:13:14  cg
 * Function SystemCall2 added that executes a system call and returns the
 * exit code rather than terminating with an error message upon failure.
 *
 * Revision 1.13  1996/05/24  16:18:26  sbs
 *   * if -dnocleanup flag is set print all syscalls !
 *    * This allows for easy C-code patches.
 *    *
 *   if (!cleanup)
 *     NOTE(("%s", syscall));
 *
 * inserted in SystemCall since EVERYBODY who uses -dnocleanup
 * ALLWAYS wants to know how to invoce the compiler!!
 *
 * Revision 1.12  1996/01/16  16:44:42  cg
 * added function TmpVar for generation of variable names
 *
 * Revision 1.11  1996/01/07  16:53:11  cg
 * added DBUG_PRINT
 *
 * Revision 1.10  1996/01/05  12:26:54  cg
 * added functions SystemTest and SystemCall
 *
 * Revision 1.9  1995/12/28  14:24:12  cg
 * bug in StringCopy fixed, StringCopy extremely simplified.
 *
 * Revision 1.8  1995/12/28  10:31:55  cg
 * malloc_verify is used in Malloc with Fred Fish Tag MEMVERIFY
 *
 * Revision 1.7  1995/12/15  13:40:59  cg
 * DBUG_PRINT in Malloc reactivated, ( Malloc no longer used by ItemName
 * and ModName)
 *
 * Revision 1.6  1995/10/20  11:29:04  cg
 * DBUG_PRINT removed
 *
 * Revision 1.5  1995/10/18  12:51:58  cg
 * converted to new error macros
 *
 * Revision 1.4  1995/07/24  15:43:52  asi
 * itoa will now work correctly ;-)
 *
 * Revision 1.3  1995/07/24  09:01:48  asi
 * added function itoa
 *
 * Revision 1.2  1995/05/12  13:14:10  hw
 * changed tag of DBUG_PRINT in function Malloc to MEM
 *
 * Revision 1.1  1995/03/28  12:01:50  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"

#define MAX_SYSCALL 1000

/*
 *
 *  functionname  : Malloc
 *  arguments     :  1) size of memory to allocate
 *  description   : allocates memory, if there is enough
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : malloc, Error
 *  macros        : DBUG...,NULL
 *
 *  remarks       : exit if there is not enough memory
 *
 */

void *
Malloc (int size)
{
    void *tmp;

    DBUG_ENTER ("Malloc");

    DBUG_PRINT ("MEM", ("trying to allocate %d bytes", size));

    tmp = malloc (size);
    if (NULL == tmp)
        SYSABORT (("Out of memory"));

    DBUG_PRINT ("MEM", ("new memory: " P_FORMAT, tmp));

#ifdef HAVE_MALLOC_O
    DBUG_EXECUTE ("MEMVERIFY", malloc_verify (););
#endif /* HAVE_MALLOC_O */

    DBUG_RETURN (tmp);
}

/*
 *
 *  functionname  : StringCopy
 *  arguments     : 1) source string
 *  description   : allocates memory and returns a pointer to the copy of 1)
 *  global vars   : ---
 *  internal funs : Malloc
 *  external funs : strlen, strcpy
 *  macros        : DBUG..., NULL
 *
 *  remarks       :
 *
 */

char *
StringCopy (char *source)
{
    char *ret;

    DBUG_ENTER ("StringCopy");

    if (NULL != source) {
        DBUG_PRINT ("STRINGCOPY", ("copying string \"%s\"", source));

        ret = (char *)Malloc (sizeof (char) * (strlen (source) + 2));
        strcpy (ret, source);
    } else {
        ret = NULL;
    }

    DBUG_RETURN (ret);
}

/*
 *
 *  functionname  : itoa
 *  arguments     : 1) number
 *		    R) result string
 *  description   : converts long to string
 *  global vars   :
 *  internal funs : Malloc
 *  external funs : sizeof
 *  macros        : DBUG..., NULL
 *
 *  remarks       :
 *
 */
char *
itoa (long number)
{
    char *str;
    int tmp;
    int length, i;

    DBUG_ENTER ("itoa");
    tmp = number;
    length = 1;
    while (9 < tmp) {
        tmp /= 10;
        length++;
    }
    str = (char *)Malloc (sizeof (char) * length + 1);
    str[length] = atoi ("\0");
    for (i = 0; (i < length); i++) {
        str[i] = ((int)'0') + (number / pow (10, (length - 1)));
        number = number % ((int)pow (10, (length - 1)));
    }
    DBUG_RETURN (str);
}

/*
 *
 *  functionname  : SystemCall
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : evaluates the given string and executes the respective
 *                  system call. If the system call fails, an error message
 *                  occurs and compilation is aborted.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : system, vsprintf, va_start, va_end
 *  macros        : vararg macros
 *
 *  remarks       :
 *
 */

void
SystemCall (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];
    int exit_code;

    DBUG_ENTER ("SystemCall");

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

    DBUG_PRINT ("SYSCALL", ("%s", syscall));

    /* if -dnocleanup flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (!cleanup)
        NOTE (("%s", syscall));

    exit_code = system (syscall);

    if (exit_code > 0) {
        SYSABORT (("System failed to execute shell command\n%s\n"
                   "with exit code %d",
                   syscall, exit_code / 256));
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : SystemCall2
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : evaluates the given string and executes the respective
 *                  system call. In contrast to SystemCall no error message
 *                  is printed upon failure but the exit code is returned.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : system, vsprintf, va_start, va_end
 *  macros        : vararg macros
 *
 *  remarks       :
 *
 */

int
SystemCall2 (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];

    DBUG_ENTER ("SystemCall2");

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

    DBUG_PRINT ("SYSCALL", ("%s", syscall));

    /* if -dnocleanup flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (!cleanup)
        NOTE (("%s", syscall));

    DBUG_RETURN (system (syscall));
}

/*
 *
 *  functionname  : SystemTest
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : evaluates the given string and executes the respective
 *                  system call. If the system call fails, an error message
 *                  occurs and compilation is aborted.
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : system, vsprintf, va_start, va_end
 *  macros        : vararg macros
 *
 *  remarks       :
 *
 */

int
SystemTest (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];
    int exit_code;

    DBUG_ENTER ("SystemTest");

    strcpy (syscall, "test ");

    va_start (arg_p, format);
    vsprintf (syscall + 5 * sizeof (char), format, arg_p);
    va_end (arg_p);

    DBUG_PRINT ("SYSCALL", ("%s", syscall));

    exit_code = system (syscall);

    if (exit_code == 0) {
        exit_code = 1;
    } else {
        exit_code = 0;
    }

    DBUG_PRINT ("SYSCALL", ("test returns %d", exit_code));

    DBUG_RETURN (exit_code);
}

/*
 *
 *  functionname  : TmpVar
 *  arguments     : ---
 *  description   : generates string to be used as artificial variable
 *  global vars   : ---
 *  internal funs : Malloc
 *  external funs : sprintf
 *  macros        : DBUG...
 *
 *  remarks       : The variable name is different in each call of TmpVar.
 *                  The actual name "tmp__cg_??" was chosen to avoid
 *                  conflicts with so far used artificial variables.
 *
 */

char *
TmpVar ()
{
    static int counter = 0;
    char *result;

    DBUG_ENTER ("TmpVar");

    result = (char *)Malloc (sizeof (char) * 20);
    sprintf (result, "__tmp_cg_%d", counter);
    counter++;

    DBUG_RETURN (result);
}
