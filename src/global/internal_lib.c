/*
 *
 * $Log$
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
 * TmpVar(): modification for TAGGED_ARRAYS done
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

/******************************************************************************
 *
 * function:
 *   void *Malloc( int size)
 *   void *Free( void *address)
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
Malloc (int size)
{
    void *tmp;

    DBUG_ENTER ("Malloc");

    DBUG_ASSERT ((size >= 0), "Malloc called with negative size!");

    if (size > 0) {
        tmp = malloc (size + malloc_align_step);

        /*
         * Since some UNIX system (e.g. ALPHA) do return NULL for size 0 as well
         * we do complain for ((NULL == tmp) && (size > 0)) only!!
         */
        if (tmp == NULL) {
            SYSABORT (
              ("Out of memory: %u Bytes already allocated!", current_allocated_mem));
        }

        *(int *)tmp = size;
        tmp = (char *)tmp + malloc_align_step;

        DBUG_ASSERT ((current_allocated_mem + size >= current_allocated_mem),
                     "counter for allocated memory: overflow detected");
        current_allocated_mem += size;
        if (max_allocated_mem < current_allocated_mem) {
            max_allocated_mem = current_allocated_mem;
        }
    } else {
        tmp = NULL;
    }

    DBUG_PRINT ("MEM_ALLOC", ("Alloc memory: %d Bytes at adress: " F_PTR, size, tmp));

    DBUG_PRINT ("MEM_TOTAL", ("Currently allocated memory: %u", current_allocated_mem));

    DBUG_RETURN (tmp);
}

#ifdef NOFREE

void *
Free (void *address)
{
    DBUG_ENTER ("Free");

    address = NULL;

    DBUG_RETURN (address);
}

#else /* NOFREE */

void *
Free (void *address)
{
    void *orig_address;
    int size;

    DBUG_ENTER ("Free");

    if (address != NULL) {
        orig_address = (void *)((char *)address - malloc_align_step);
        size = *(int *)orig_address;
        DBUG_ASSERT ((size >= 0), "illegal size found!");
        DBUG_PRINT ("MEM_ALLOC",
                    ("Free memory: %d Bytes at adress: " F_PTR, size, address));

        DBUG_ASSERT ((current_allocated_mem >= current_allocated_mem - size),
                     "counter for allocated memory: overflow detected");
        current_allocated_mem -= size;

        free (orig_address);

        DBUG_PRINT ("MEM_TOTAL",
                    ("Currently allocated memory: %u", current_allocated_mem));

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

    DBUG_ASSERT ((size >= 0), "Malloc called with negative size!");

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
Free (void *address)
{
    DBUG_ENTER ("Free");

    address = NULL;

    DBUG_RETURN (address);
}

#else /* NOFREE */

void *
Free (void *address)
{
    DBUG_ENTER ("Free");

    if (address != NULL) {
        free (address);

        DBUG_PRINT ("MEM_ALLOC", ("Free memory: ??? Bytes at adress: " F_PTR, address));

        address = NULL;
    }

    DBUG_RETURN (address);
}

#endif /* NOFREE */

#endif /* SHOW_MALLOC */

struct STR_BUF {
    char *buf;
    int pos;
    int size;
};

/******************************************************************************
 *
 * Function:
 *   str_buf *StrBufCreate(  int size);
 *
 * Description:
 *
 ******************************************************************************/

str_buf *
StrBufCreate (int size)
{
    str_buf *res;

    DBUG_ENTER ("StrBufCreate");

    res = (str_buf *)Malloc (sizeof (str_buf));
    res->buf = (char *)Malloc (size * sizeof (char));
    res->pos = 0;
    res->size = size;

    DBUG_PRINT ("STRBUF", ("allocating buffer size %d : %p", size, res));

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   str_buf *StrBufprint(  str_buf *s, const char *string);
 *
 * Description:
 *
 ******************************************************************************/

str_buf *
StrBufprint (str_buf *s, const char *string)
{
    int len;
    int new_size;
    char *new_buf;

    DBUG_ENTER ("StrBufprint");

    len = strlen (string);

    if ((len + 1) > (s->size - s->pos)) {

        new_size = (len >= s->size ? s->size + 2 * len : 2 * s->size);

        DBUG_PRINT ("STRBUF", ("increasing buffer %p from size %d to size %d", s, s->size,
                               new_size));

        new_buf = (char *)Malloc (new_size * sizeof (char));
        s->pos = sprintf (new_buf, "%s", s->buf);
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
 *   str_buf *StrBufprintf(  str_buf *s, const char *format, ...);
 *
 * Description:
 *
 ******************************************************************************/

str_buf *
StrBufprintf (str_buf *s, const char *format, ...)
{
    va_list arg_p;
    static char string[512];

    DBUG_ENTER ("StrBufprintf");

    va_start (arg_p, format);
    vsprintf (string, format, arg_p);
    va_end (arg_p);

    DBUG_ASSERT (strlen (string) < 512, "string buffer in StrBufprintf too small!");

    s = StrBufprint (s, string);

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * Function:
 *   char *StrBuf2String(  str_buf *s);
 *
 * Description:
 *
 ******************************************************************************/

char *
StrBuf2String (str_buf *s)
{
    DBUG_ENTER ("StrBuf2String");
    DBUG_RETURN (StringCopy (s->buf));
}

/******************************************************************************
 *
 * Function:
 *   void StrBufFlush(  str_buf *s)
 *
 * Description:
 *
 ******************************************************************************/

void
StrBufFlush (str_buf *s)
{
    DBUG_ENTER ("StrBufFlush");

    s->pos = 0;
    DBUG_PRINT ("STRBUF", ("pos of buffer %p reset to %d", s, s->pos));

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   bool     StrBufIsEmpty( str_buf *s)
 *
 * Description:
 *
 ******************************************************************************/

bool
StrBufIsEmpty (str_buf *s)
{
    DBUG_ENTER ("StrBufIsEmpty");

    DBUG_RETURN (s->pos == 0);
}

/******************************************************************************
 *
 * Function:
 *   void *StrBufFree(  str_buf *s);
 *
 * Description:
 *
 ******************************************************************************/

void *
StrBufFree (str_buf *s)
{
    DBUG_ENTER ("StrBufFree");

    s->buf = Free (s->buf);
    s = Free (s);

    DBUG_RETURN (s);
}

/******************************************************************************
 *
 * Function:
 *   char *StringCopy( char *source)
 *
 * Description:
 *   Allocates memory and returns a pointer to the copy of 'source'.
 *
 ******************************************************************************/

char *
StringCopy (char *source)
{
    char *ret;

    DBUG_ENTER ("StringCopy");

    if (source) {
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
 *   char *StringConcat( char *first, char* second)
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
 *   char *StrTok( char *first, char *sep)
 *
 * description
 *    Implements a version of the c-strtok, which can operate on static
 *    strings, too. It returns the string always till the next occurence off
 *    the string sep in first. If there are no more tokens in first a NULL
 *    pointer will be returned.
 *    On first call the string first will be copied, and on last call the
 *    allocated memory of the copy will be freeed.
 *    To get more than one token from one string, call StrTok with NULL as
 *    first parameter, just like c-strtok.
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
        if (act_string != NULL) {
            act_string = Free (act_string);
        }
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
 * Function:
 *   char *itoa( long number)
 *
 * Description:
 *   converts long to string
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

    for (i = 0; i < length; i++) {
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
 * Function:
 *   void SystemCall( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
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
 * Function:
 *   int SystemCall2( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   In contrast to SystemCall() no error message is printed upon failure but
 *   the exit code is returned.
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
 * Function:
 *   int SystemTest( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
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
 *   char *TmpVar()
 *
 * Description:
 *   Generates string to be used as artificial variable.
 *   The variable name is different in each call of TmpVar().
 *   The string has the form "__tmp_" ++ compiler phase ++ consecutive number.
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
    } else
#ifdef TAGGED_ARRAYS
      if (act_tab == comp2_tab) {
        s = "comp";
    } else
#else  /* TAGGED_ARRAYS */
      if (act_tab == comp_tab) {
        s = "comp";
    } else
#endif /* TAGGED_ARRAYS */
      if (act_tab == lir_tab) {
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
        s = "pcomp1";
    } else if (act_tab == precomp2_tab) {
        s = "pcomp2";
    } else if (act_tab == precomp3_tab) {
        s = "pcomp3";
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
    } else if (act_tab == ssafrm_tab) {
        s = "ssa";
    } else if (act_tab == undossa_tab) {
        s = "ussa";
    } else if (act_tab == ssacf_tab) {
        s = "cf";
    } else if (act_tab == ssalir_tab) {
        s = "lir";
    } else if (act_tab == lirmov_tab) {
        s = "lir";
    } else if (act_tab == ssawlt_tab) {
        s = "wlt";
    } else if (act_tab == ssawli_tab) {
        s = "wli";
    } else if (act_tab == ssawlf_tab) {
        s = "wlf";
    } else if (act_tab == wls_tab) {
        s = "wls";
    } else if (act_tab == hd_tab) {
        s = "hd";
    } else if (act_tab == cwc_tab) {
        s = "cwc";
    } else if (act_tab == al_tab) {
        s = "al";
    } else {
        s = "unknown";
        DBUG_ASSERT ((0), "TmpVar(): unknown trav-tab found!");
    }

    sprintf (result, "_%s_%d", s, counter);
    counter++;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *TmpVarName( char* postfix)
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

#endif /* SHOW_MALLOC */

#ifndef DBUG_OFF

/******************************************************************************
 *
 * function:
 *   void DbugMemoryLeakCheck( void)
 *
 * description:
 *   computes and prints memory usage w/o memory used for the actual
 *   syntax tree.
 *
 ******************************************************************************/

void
DbugMemoryLeakCheck (void)
{
    node *ast_dup;
    int mem_before;

    DBUG_ENTER ("DbugMemoryLeakCheck");

    mem_before = current_allocated_mem;
    NOTE2 (("*** Currently allocated memory (Bytes):   %s",
            IntBytes2String (current_allocated_mem)));
    ast_dup = DupTree (syntax_tree);
    NOTE2 (("*** Size of the syntax tree (Bytes):      %s",
            IntBytes2String (current_allocated_mem - mem_before)));
    NOTE2 (("*** Other memory allocated/ Leak (Bytes): %s",
            IntBytes2String (2 * mem_before - current_allocated_mem)));
    FreeTree (ast_dup);
    NOTE2 (("*** FreeTree / DupTree leak (Bytes):      %s",
            IntBytes2String (current_allocated_mem - mem_before)));

    DBUG_VOID_RETURN;
}

#endif /* DBUG_OFF */
