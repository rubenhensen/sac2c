/*
 *
 * $Log$
 * Revision 3.2  2000/12/06 18:26:04  cg
 * Added new traversal tccp for typecheck constant propagation.
 *
 * Revision 3.1  2000/11/20 17:59:30  sacbase
 * new release made
 *
 * Revision 2.24  2000/11/15 17:18:24  sbs
 * Malloc for size==0 and ndef SHOW_MALLOC now returns NULL rather than calling
 * SYSABORT (iff the built-in malloc returns NULL in that case).
 *
 * Revision 2.23  2000/11/15 14:02:38  sbs
 * args of tolower casted from char to int.
 *
 * Revision 2.22  2000/07/17 11:15:13  jhs
 * Added blkli at TmpVar().
 *
 * Revision 2.21  2000/07/14 15:36:08  nmw
 * oops, big crash ... MALLOC reverted to Malloc
 *
 * Revision 2.20  2000/07/14 14:42:31  nmw
 * malloc in StringConcat changed to MALLOC
 *
 * Revision 2.19  2000/06/13 13:40:24  dkr
 * O2NWith_tab renamed into patchwith_tab
 *
 * Revision 2.18  2000/05/31 14:39:33  mab
 * renamed tables for array padding
 *
 * Revision 2.17  2000/05/31 11:23:17  mab
 * added traversal tables for array padding
 *
 * Revision 2.16  2000/05/29 14:31:10  dkr
 * second traversal table for precompile added
 *
 * Revision 2.15  2000/03/15 17:28:05  dkr
 * DBUG_ASSERT for lcm() added
 *
 * Revision 2.14  2000/02/18 14:38:22  cg
 * Added TmpVar names for ai_tab and fun2lac_tab.
 *
 * Revision 2.13  2000/02/11 16:26:29  dkr
 * function StringConcat added
 *
 * Revision 2.12  2000/01/26 17:29:22  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.11  2000/01/25 13:39:03  dkr
 * all the Constvec stuff moved to tree_compound.c
 *
 * Revision 2.10  1999/07/20 11:48:29  cg
 * Definition (!) of global variable malloc_align_step removed;
 * malloc_align_step is now defined in globals.c.
 *
 * Revision 2.9  1999/07/05 14:28:00  sbs
 * warnings concerning uninitialized usages of res in AnnotateIdWithConstVec and
 * CopyConstVec eliminated .
 *
 * Revision 2.8  1999/06/03 14:29:21  sbs
 * missing code for T_bool const-vecs added in ModConstVec.
 *
 * Revision 2.7  1999/05/17 11:24:26  jhs
 * CopyConstVec will be called only if ID/ARRAY_ISCONST.
 *
 * Revision 2.6  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various compilation stages.
 *
 * Revision 2.5  1999/05/12 08:41:11  sbs
 * CopyIntVector and friends eliminated ; instead,
 * CopyConstVec, AllocConstVec, and ModConstVec have been added.
 * /.
 *
 * Revision 2.4  1999/04/14 16:29:49  jhs
 * Adjustment for empty arrays.
 *
 * Revision 2.3  1999/03/15 13:51:15  bs
 * CopyIntArray renamed into CopyIntVector, CopyFloatVector and CopyDoubleVector added.
 *
 * Revision 2.2  1999/02/24 20:21:59  bs
 * New function added: CopyIntArray
 *
 * Revision 2.1  1999/02/23 12:39:23  sacbase
 * new release made
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
#include "tree_basic.h"
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

/******************************************************************************
 *
 *  functionname  : Malloc
 *  arguments     :  1) size of memory to allocate
 *  description   : allocates memory, if there is enough
 *                  iff size=0 NULL is returned!
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : malloc, Error
 *  macros        : DBUG...,NULL
 *
 *  remarks       : exit if there is not enough memory
 *
 ******************************************************************************/

void *
Malloc (int size)
{
    void *tmp;

    DBUG_ENTER ("Malloc");
    DBUG_PRINT ("MEMALLOC_TRY", ("trying to allocate %d bytes", size));
    DBUG_ASSERT ((size >= 0), "Malloc called with negative size!");

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
    /*
     * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
     * we do complain for ((NULL == tmp) && (size > 0)) only!!
     */
    if ((NULL == tmp) && (size > 0))
        SYSABORT (("Out of memory"));
#endif

    DBUG_PRINT ("MEMALLOC", ("new memory: " P_FORMAT, tmp));
    DBUG_RETURN (tmp);
}

/******************************************************************************
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
 ******************************************************************************/

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

/******************************************************************************
 *
 * function:
 *   char *StringConcat(char *first, char* second)
 *
 * description
 *   Reserves new memory for the concatinated string first + second,
 *   and returns the concatination. Does not free any memory used by
 *   first or second.
 *
 ******************************************************************************/

char *
StringConcat (char *first, char *second)
{
    char *result;

    DBUG_ENTER ("StringConcat");

    result = (char *)Malloc (strlen (first) + strlen (second) + 1);

    strcpy (result, first);
    strcat (result, second);

    DBUG_RETURN (result);
}

/******************************************************************************
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
 ******************************************************************************/
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
 *   int lcm( int x, int y)
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
 ******************************************************************************/

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

/******************************************************************************
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
 ******************************************************************************/

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

/******************************************************************************
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
 ******************************************************************************/

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

/******************************************************************************
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
 *
 ******************************************************************************/

char *
TmpVar ()
{
    static int counter = 0;
    char *result, *s = NULL;

    DBUG_ENTER ("TmpVar");

    result = (char *)Malloc (sizeof (char) * 16);

    if (act_tab == imp_tab) {
        s = "imp";
    } else if (act_tab == flat_tab) {
        s = "flat";
    } else if (act_tab == print_tab) {
        s = "prt";
    } else if (act_tab == type_tab) {
        s = "type";
    } else if (act_tab == tccp_tab) {
        s = "type";
    } else if (act_tab == genmask_tab) {
        s = "gnm";
    } else if (act_tab == dcr_tab) {
        s = "dcr";
    } else if (act_tab == cf_tab) {
        s = "cf";
    } else if (act_tab == free_tab) {
        s = "free";
    } else if (act_tab == refcnt_tab) {
        s = "refcnt";
    } else if (act_tab == comp_tab) {
        s = "comp";
    } else if (act_tab == lir_tab) {
        s = "lir";
    } else if (act_tab == lir_mov_tab) {
        s = "lirm";
    } else if (act_tab == dup_tab) {
        s = "dup";
    } else if (act_tab == inline_tab) {
        s = "inl";
    } else if (act_tab == unroll_tab) {
        s = "unr";
    } else if (act_tab == unswitch_tab) {
        s = "uns";
    } else if (act_tab == idx_tab) {
        s = "idx";
    } else if (act_tab == wlt_tab) {
        s = "wlt";
    } else if (act_tab == wli_tab) {
        s = "wli";
    } else if (act_tab == wlf_tab) {
        s = "wlf";
    } else if (act_tab == ae_tab) {
        s = "ae";
    } else if (act_tab == writesib_tab) {
        s = "wsib";
    } else if (act_tab == obj_tab) {
        s = "obj";
    } else if (act_tab == impltype_tab) {
        s = "impl";
    } else if (act_tab == objinit_tab) {
        s = "obji";
    } else if (act_tab == analy_tab) {
        s = "analy";
    } else if (act_tab == checkdec_tab) {
        s = "cdec";
    } else if (act_tab == writedec_tab) {
        s = "wdec";
    } else if (act_tab == unique_tab) {
        s = "uniq";
    } else if (act_tab == rmvoid_tab) {
        s = "rmvoid";
    } else if (act_tab == precomp1_tab) {
        s = "pcomp";
    } else if (act_tab == precomp2_tab) {
        s = "pcomp";
    } else if (act_tab == readsib_tab) {
        s = "rsib";
    } else if (act_tab == cse_tab) {
        s = "cse";
    } else if (act_tab == dfr_tab) {
        s = "dfr";
    } else if (act_tab == patchwith_tab) {
        s = "pw";
    } else if (act_tab == spmdinit_tab) {
        s = "spmdi";
    } else if (act_tab == spmdopt_tab) {
        s = "spmdo";
    } else if (act_tab == spmdlift_tab) {
        s = "spmdl";
    } else if (act_tab == syncinit_tab) {
        s = "synci";
    } else if (act_tab == syncopt_tab) {
        s = "synco";
    } else if (act_tab == fun2lac_tab) {
        s = "fun2lac";
    } else if (act_tab == ai_tab) {
        s = "ai";
    } else if (act_tab == apc_tab) {
        s = "apc";
    } else if (act_tab == apt_tab) {
        s = "apt";
    } else if (act_tab == blkli_tab) {
        s = "blkli";
    } else {
        s = "unknown";
    }

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
           && (tolower ((int)first[i]) == tolower ((int)second[i])))
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
