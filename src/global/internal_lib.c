/*
 *
 * $Log$
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
 * Revision 3.6  2001/04/24 09:14:00  dkr
 * P_FORMAT replaced by F_PTR
 *
 * Revision 3.5  2001/04/09 15:58:27  nmw
 * VarName for ssalir/lirmov traversal completed
 *
 * Revision 3.4  2001/03/27 11:29:56  dkr
 * OptCmp() removed
 *
 * Revision 3.3  2001/03/12 10:29:39  nmw
 * TmpVar for undossa_tab added
 *
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
 * [...]
 *
 */

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "Error.h"
#include "DupTree.h"
#include "free.h"
#include "dbug.h"
#include "scnprs.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "globals.h"
#include "traverse.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "convert.h"

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
 * function:
 *   void *Malloc( int size)
 *   void *Free( void *address)
 *
 * description:
 *
 *   These functions for memory allocation and de-allocation are wrappers
 *   for the standard functions malloc() and free().
 *
 *   They allow to implement some additional functionality, e.g. accounting
 *   of currently allocated memory.
 *
 ******************************************************************************/

#ifdef SHOW_MALLOC

void *
Malloc (int size)
{
    void *tmp;

    DBUG_ENTER ("Malloc");

    DBUG_PRINT ("MEMALLOC_TRY", ("trying to allocate %d bytes", size));
    DBUG_ASSERT ((size >= 0), "Malloc called with negative size!");

    tmp = malloc (size + malloc_align_step);

    /*
     * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
     * we do complain for ((NULL == tmp) && (size > 0)) only!!
     */
    if ((NULL == tmp) && (size > 0)) {
        SYSABORT (("Out of memory: %d Bytes allocated!", current_allocated_mem));
    }
    *(int *)tmp = size;
    tmp = (char *)tmp + malloc_align_step;

    total_allocated_mem += size;
    current_allocated_mem += size;
    DBUG_PRINT ("MEM_OBSERVE", ("mem currently allocated: %d", current_allocated_mem));
    if (max_allocated_mem < current_allocated_mem) {
        max_allocated_mem = current_allocated_mem;
    }

    DBUG_PRINT ("MEMALLOC", ("new memory: " F_PTR, tmp));

    DBUG_RETURN (tmp);
}

#ifdef NOFREE

void *
Free (void *address)
{
    DBUG_ENTER ("Free");
    DBUG_PRINT ("FREEMEM", ("Not freeing memory at adress: " F_PTR, address));
    DBUG_RETURN ((void *)NULL);
}

#else /* NOFREE */

void *
Free (void *address)
{
    DBUG_ENTER ("Free");

    if (address != NULL) {
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " F_PTR, address));
        address = (void *)((char *)address - malloc_align_step);
        current_allocated_mem -= *(int *)address;

        free (address);
        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* NOFREE */

#else /* SHOW_MALLOC */

void *
Malloc (int size)
{
    void *tmp;

    DBUG_ENTER ("Malloc");

    DBUG_PRINT ("MEMALLOC_TRY", ("trying to allocate %d bytes", size));
    DBUG_ASSERT ((size >= 0), "Malloc called with negative size!");

    tmp = malloc (size);

    /*
     * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
     * we do complain for ((NULL == tmp) && (size > 0)) only!!
     */
    if ((NULL == tmp) && (size > 0)) {
        SYSABORT (("Out of memory"));
    }

    DBUG_PRINT ("MEMALLOC", ("new memory: " F_PTR, tmp));

    DBUG_RETURN (tmp);
}

#ifdef NOFREE

void *
Free (void *address)
{
    DBUG_ENTER ("Free");
    DBUG_PRINT ("FREEMEM", ("Not freeing memory at adress: " F_PTR, address));
    DBUG_RETURN ((void *)NULL);
}

#else /* NOFREE */

void *
Free (void *address)
{
    DBUG_ENTER ("Free");

    if (address != NULL) {
        DBUG_PRINT ("FREEMEM", ("Free memory at adress: " F_PTR, address));

        free (address);
        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* NOFREE */

#endif /* SHOW_MALLOC */

/******************************************************************************
 *
 *  functionname  : StringCopy
 *  arguments     : 1) source string
 *  description   : allocates memory and returns a pointer to the copy of 1)
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
    } else {
        ret = NULL;
    }

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
 * function:
 *   char *StrTok(char *first, char *sep)
 *
 * description
 *    Implements a version of the c-strtok, which can operate on static strings,
 *    too. It returns the string always till the next occurence off the string sep
 *    in first. If there are no more tokens in first a null pointer will be
 *    returned.
 *    On first call the string first will be copied, and on last call the
 *    allocated memory of the copy will be freeed.
 *    To get more than one token from one string, call StrTok with NULL
 *    as first parameter, just like c-strtok
 *
 ******************************************************************************/

char *
StrTok (char *first, char *sep)
{
    static char *act_string = NULL;
    char *new_string = NULL;
    char *ret;

    DBUG_ENTER ("StrTok");

    if (first != NULL) {
        if (act_string != NULL)
            act_string = Free (act_string);
        new_string = StringCopy (first);
        act_string = new_string;
    }

    ret = strtok (new_string, sep);

    if (ret == NULL) {
        act_string = Free (act_string);
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 *  functionname  : itoa
 *  arguments     : 1) number
 *		    R) result string
 *  description   : converts long to string
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
    if (show_syscall) {
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
 *  functionname  : SystemCall2
 *  arguments     : 1) format string like that of printf
 *                  2) variable argument list for 1)
 *  description   : evaluates the given string and executes the respective
 *                  system call. In contrast to SystemCall no error message
 *                  is printed upon failure but the exit code is returned.
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
    if (show_syscall) {
        NOTE (("%s", syscall));
    }

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
 *  remarks       : The variable name is different in each call of TmpVar.
 *                  The string has the form "__tmp_" plus compiler phase
 *                  plus consecutive number.
 *
 ******************************************************************************/

char *
TmpVar ()
{
    static int counter = 0;
    char *result;
    char *s = NULL;

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
        s = "rc";
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
        s = "f2l";
    } else if (act_tab == ai_tab) {
        s = "ai";
    } else if (act_tab == apc_tab) {
        s = "apc";
    } else if (act_tab == apt_tab) {
        s = "apt";
    } else if (act_tab == blkli_tab) {
        s = "blkli";
    } else if (act_tab == undossa_tab) {
        s = "ussa";
    } else if (act_tab == ssalir_tab) {
        s = "lir";
    } else if (act_tab == lirmov_tab) {
        s = "lir";
    } else if (act_tab == ssawlt_tab) {
        s = "swlt";
    } else if (act_tab == ssawli_tab) {
        s = "swli";
    } else if (act_tab == ssawlf_tab) {
        s = "swlf";
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
    tmp = Free (tmp);

    DBUG_RETURN (result);
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
#endif

#ifndef DBUG_OFF
/******************************************************************************
 *
 * function:
 *   void DbugMemoryLeakCheck()
 *
 * description:
 *   computes and prints memory usage w/o memory used for the actual
 *   syntax tree.
 *
 ******************************************************************************/

void
DbugMemoryLeakCheck ()
{
    node *ast_dup;
    int mem_before;

    DBUG_ENTER ("DbugMemoryLeakCheck");

    mem_before = current_allocated_mem;
    NOTE2 (("*** Currently allocated memory (bytes):   %s",
            IntBytes2String (current_allocated_mem)));
    ast_dup = DupTree (syntax_tree);
    NOTE2 (("*** Size of the syntax tree (bytes):      %s",
            IntBytes2String (current_allocated_mem - mem_before)));
    NOTE2 (("*** Other memory allocated/ Leak (bytes): %s",
            IntBytes2String (2 * mem_before - current_allocated_mem)));
    FreeTree (ast_dup);
    NOTE2 (("*** FreeTree / DupTree leak (bytes):      %s",
            IntBytes2String (current_allocated_mem - mem_before)));

    DBUG_VOID_RETURN;
}
#endif /* DBUG_OFF */
