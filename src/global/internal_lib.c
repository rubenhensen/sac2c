/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:39:23  sacbase
 * new release made
 *
 * Revision 1.35  1999/01/07 14:02:15  sbs
 * *** empty log message ***
 *
 * Revision 1.34  1998/08/20 12:09:00  srs
 * removed upper allocation bound in Malloc (maybe this was inserted
 * for debugging).
 *
 * Revision 1.33  1998/06/23 15:04:35  cg
 * The printing of system calls is now triggered via the global variable
 * show_syscall and the command line option -dshow_syscall.
 *
 * Revision 1.32  1998/06/19 12:51:44  srs
 * compute_malloc_align_step() => ComputeMallocAlignStep()
 *
 * Revision 1.31  1998/06/19 09:10:37  sbs
 * added some DBUG_PRINTs
 *
 * Revision 1.30  1998/06/03 14:24:00  cg
 * added some new fun_tabs for TmpVar.
 * now, sufficient memory is allocated even for unknown fun_tabs in TmpVar
 *
 * Revision 1.29  1998/03/25 19:22:53  srs
 * added new WL phases to TmpVar()
 *
 * Revision 1.28  1998/03/17 11:54:31  dkr
 * added fun lcm()
 *
 * Revision 1.27  1998/03/03 13:50:21  srs
 * removed 'tmp' infix from name generation in TmpVar()
 *
 * Revision 1.26  1998/03/02 13:57:21  cg
 * added function OptCmp() to compare two strings regardless of lower
 * or upper case letters (used for scanning optimization command line options.
 *
 * Revision 1.25  1998/02/24 16:10:32  srs
 * new function TmpVarName()
 *
 * Revision 1.24  1998/02/05 17:08:30  srs
 * TmpVar(): wr_tab and changed fusion_tab into wlf_tab
 *
 * Revision 1.23  1998/01/29 13:17:49  srs
 * modified TmpVar(). Now the compiler phase is inserted
 * in the variable name.
 *
 * Revision 1.22  1997/12/08 19:19:34  dkr
 * no arithmetic on void-pointers anymore (Malloc())
 *
 * Revision 1.21  1997/12/06 17:15:13  srs
 * changed Malloc (SHOW_MALLOC)
 *
 * Revision 1.20  1997/12/05 16:36:40  srs
 * StringCopy: allocate one byte less now
 *
 * Revision 1.19  1997/11/23 15:18:45  dkr
 * CC-flag: show_malloc -> SHOW_MALLOC
 *
 * Revision 1.18  1997/10/29 14:56:07  srs
 * changed Malloc() and removed HAVE_MALLOC_O
 *
 * Revision 1.17  1997/10/09 13:53:49  srs
 * counter for memory allocation
 *
 * Revision 1.16  1997/08/04 15:11:18  dkr
 * changed DBUG_PRINT-strings in Malloc
 *
 * Revision 1.15  1997/04/24  14:59:22  sbs
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
#include <ctype.h>

#include "Error.h"
#include "free.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "globals.h"
#include "traverse.h"
#include "types.h"
#include "tree_compound.h"

#define MAX_SYSCALL 1000

#ifdef SHOW_MALLOC
/* These types are only used to compute malloc_align_step.
   No instances are raised */
typedef union {
    long int l;
    double d;
} malloc_align_type;

typedef struct {
    int size;
    malloc_align_type align;
} malloc_header_type;
#endif

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
    DBUG_PRINT ("MEMALLOC_TRY", ("trying to allocate %d bytes", size));

#ifdef SHOW_MALLOC
    tmp = malloc (size + malloc_align_step);
    if (NULL == tmp)
        SYSABORT (("Out of memory: %d Bytes allocated!", current_allocated_mem));
    *(int *)tmp = size;
    tmp = (char *)tmp + malloc_align_step;

    total_allocated_mem += size;
    current_allocated_mem += size;
    DBUG_PRINT ("MEM_OBSERVE", ("mem currently allocated: %d", current_allocated_mem));
    if (max_allocated_mem < current_allocated_mem)
        max_allocated_mem = current_allocated_mem;

#else /* not SHOW_MALLOC */
    tmp = malloc (size);
    if (NULL == tmp)
        SYSABORT (("Out of memory"));
#endif

    DBUG_PRINT ("MEMALLOC", ("new memory: " P_FORMAT, tmp));
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

    if (source) {
        DBUG_PRINT ("STRINGCOPY", ("copying string \"%s\"", source));

        ret = (char *)Malloc (sizeof (char) * (strlen (source) + 1));
        strcpy (ret, source);
    } else
        ret = NULL;

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

/******************************************************************************
 *
 * function:
 *   int lcm(int x, int y)
 *
 * description:
 *   returns the lowest-common-multiple of x, y.
 *
 ******************************************************************************/

int
lcm (int x, int y)
{
    int u, v;

    DBUG_ENTER ("lcm");

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
    if (show_syscall)
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
    if (show_syscall)
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
 *                  The string has the form "__tmp_" plus compiler phase
 *                  plus consecutive number.
 */

char *
TmpVar ()
{
    static int counter = 0;
    char *result, *s = NULL;

    DBUG_ENTER ("TmpVar");

    result = (char *)Malloc (sizeof (char) * 16);

    if (act_tab == imp_tab) {
        s = "imp";
    }
    if (act_tab == flat_tab) {
        s = "flat";
    }
    if (act_tab == print_tab) {
        s = "prt";
    }
    if (act_tab == type_tab) {
        s = "type";
    }
    if (act_tab == genmask_tab) {
        s = "gnm";
    }
    if (act_tab == dcr_tab) {
        s = "dcr";
    }
    if (act_tab == cf_tab) {
        s = "cf";
    }
    if (act_tab == free_tab) {
        s = "free";
    }
    if (act_tab == refcnt_tab) {
        s = "refcnt";
    }
    if (act_tab == comp_tab) {
        s = "comp";
    }
    if (act_tab == lir_tab) {
        s = "lir";
    }
    if (act_tab == lir_mov_tab) {
        s = "lirm";
    }
    if (act_tab == dup_tab) {
        s = "dup";
    }
    if (act_tab == inline_tab) {
        s = "inl";
    }
    if (act_tab == unroll_tab) {
        s = "unr";
    }
    if (act_tab == unswitch_tab) {
        s = "uns";
    }
    if (act_tab == idx_tab) {
        s = "idx";
    }
    if (act_tab == wlt_tab) {
        s = "wlt";
    }
    if (act_tab == wli_tab) {
        s = "wli";
    }
    if (act_tab == wlf_tab) {
        s = "wlf";
    }
    if (act_tab == ae_tab) {
        s = "ae";
    }
    if (act_tab == writesib_tab) {
        s = "wsib";
    }
    if (act_tab == obj_tab) {
        s = "obj";
    }
    if (act_tab == impltype_tab) {
        s = "impl";
    }
    if (act_tab == objinit_tab) {
        s = "obji";
    }
    if (act_tab == analy_tab) {
        s = "analy";
    }
    if (act_tab == checkdec_tab) {
        s = "cdec";
    }
    if (act_tab == writedec_tab) {
        s = "wdec";
    }
    if (act_tab == unique_tab) {
        s = "uniq";
    }
    if (act_tab == rmvoid_tab) {
        s = "rmvoid";
    }
    if (act_tab == precomp_tab) {
        s = "pcomp";
    }
    if (act_tab == readsib_tab) {
        s = "rsib";
    }
    if (act_tab == cse_tab) {
        s = "cse";
    }
    if (act_tab == dfr_tab) {
        s = "dfr";
    }
    if (act_tab == o2nWith_tab) {
        s = "o2nW";
    }
    if (act_tab == spmdinit_tab) {
        s = "spmdi";
    }
    if (act_tab == spmdopt_tab) {
        s = "spmdo";
    }
    if (act_tab == spmdlift_tab) {
        s = "spmdl";
    }
    if (act_tab == syncinit_tab) {
        s = "synci";
    }
    if (act_tab == syncopt_tab) {
        s = "synco";
    }

    if (!s)
        s = "unknown";

    sprintf (result, "_%s_%d", s, counter);
    counter++;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *TmpVarName(char* postfix)
 *
 * description:
 *   creates a unique variable like TmpVar() and additionally appends
 *   an individual string.
 *
 ******************************************************************************/

char *
TmpVarName (char *postfix)
{
    char *result, *tmp;

    DBUG_ENTER ("TmpVarName");

    tmp = TmpVar ();
    result = (char *)Malloc ((strlen (tmp) + strlen (postfix) + 2) * sizeof (char));
    sprintf (result, "%s_%s", tmp, postfix);
    FREE (tmp);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   int OptCmp(char *first, char *second)
 *
 * description:
 *   compares two strings and returns 1 (true) if the strings are equal.
 *   All characters are converted to lower case before the comparison applies.
 *
 ******************************************************************************/

int
OptCmp (char *first, char *second)
{
    int i = 0;

    while ((first[i] != '\0') && (second[i] != '\0')
           && (tolower (first[i]) == tolower (second[i])))
        i++;

    if (first[i] == second[i])
        return (1);
    else
        return (0);
}

#ifdef SHOW_MALLOC
/* -------------------------------------------------------------------------- *
 * task: calculates the number of bytes for a safe alignment (used in Malloc)
 * initializes global variable malloc_align_step
 *
 * remarks: the c-compiler alignment of structs is exploited.
 * -------------------------------------------------------------------------- */
void
ComputeMallocAlignStep (void)
{
    DBUG_ENTER ("ComputeMallocAlignStep");

    /* calculate memory alignment steps for this machine */
    malloc_align_step = sizeof (malloc_header_type) - sizeof (malloc_align_type);

    DBUG_VOID_RETURN;
}
/* ========================================================================== */
#endif
