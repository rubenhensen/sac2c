/*
 *
 * $Log$
 * Revision 3.35  2003/09/17 17:48:29  dkr
 * *** empty log message ***
 *
 * Revision 3.34  2003/09/17 17:19:37  dkr
 * some minor changes done.
 * This revision does not work correctly yet :(
 *
 * Revision 3.33  2003/09/17 14:17:25  dkr
 * some function parameters renamed
 *
 * Revision 3.32  2003/09/17 13:03:26  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.31  2003/09/15 17:06:05  dkr
 * bug in ICMCompileMT_MASTER_RECEIVE_FOLDRESULT() fixed
 *
 * Revision 3.30  2003/09/15 16:51:04  dkr
 * typo in ICMCompileMT_SYNC_FOLD corrected
 *
 * Revision 3.29  2003/09/15 16:45:53  dkr
 * Several modifications for TAGGED_ARRAYS done.
 * This revision is incomplete yet.
 *
 * Revision 3.28  2003/08/04 16:57:08  dkr
 * argument of MT_SPMD_FUN_DEC, MT_SPMD_FUN_RET renamed
 *
 * Revision 3.26  2001/11/22 08:36:43  sbs
 * foldop initialized to NULL; just to please gcc 8-)
 *
 * Revision 3.25  2001/07/13 13:23:41  cg
 * Some useless DBUG_PRINTs eliminated.
 *
 * Revision 3.24  2001/05/18 09:58:03  cg
 * #include <malloc.h> removed.
 *
 * Revision 3.23  2001/05/17 12:08:34  dkr
 * FREE, MALLOC eliminated
 *
 * Revision 3.22  2001/05/14 10:21:20  cg
 * Bug in indentation of SPMD_BEGIN/END ICMs fixed.
 *
 * Revision 3.21  2001/05/11 14:36:56  cg
 * Implementations of ICMs concerned with loop scheduling
 * are now moved to new specific file icm2c_sched.c
 *
 * Revision 3.20  2001/05/09 15:13:00  cg
 * All scheduling ICMs get an additional first parameter,
 * i.e. the segment ID. This is required to identify the appropriate
 * set of scheduler internal variables.
 *
 * Revision 3.19  2001/05/04 11:48:42  ben
 *  MT_SCHEDULER_Even_... deleted
 *  MT_SCHEDULER_Cyclic_... renamed to _Static_
 *  MT_SCHEDULER_Afs_... renamed to Affinity
 * in MT_SCHEDULER_... some variables renamed
 * in MT_SCHEDULER_... some parmaters renamed
 * SelectTask renamed to TaskSelector
 * SAC_MT_SCHEDULER_ uses now ..._FIRST_TASK and ..._NEXT_TASK
 *
 * Revision 3.18  2001/04/03 22:29:40  dkr
 * some minor changes done
 *
 * Revision 3.17  2001/04/03 19:45:12  dkr
 * - MT_ADJUST_SCHEDULER renamed into MT_ADJUST_SCHEDULER__OFFSET.
 *   signature for MT_ADJUST_SCHEDULER_... icms modified.
 *   MT_ADJUST_SCHEDULER icm is not a c- but a h-icm now.
 * - InitializeBoundaries() and SelectTask() marked as static.
 * - Defines for task selection strategies added.
 * - Obsolete icm MT_SPMD_BLOCK removed.
 * - Signature of MT_SCHEDULER_..._BEGIN, MT_SCHEDULER_..._END icms
 *   modified: Blocking-vector is now longer given as an argument because
 *   it is completely useless!!
 *
 * Revision 3.16  2001/03/28 12:52:03  ben
 * param added to MT_SCHEDULER_(Cyclic,Self,AFS)_...
 *
 * Revision 3.15  2001/03/28 09:19:53  ben
 * InitializeBoundaries added
 *
 * Revision 3.14  2001/03/27 11:52:03  ben
 * MT_SCHEDULER_Afs_... added
 *
 * Revision 3.13  2001/03/23 13:35:38  ben
 * ICMCompileMT_SCHEDULER_Self_... modified
 *
 * Revision 3.12  2001/03/22 17:39:30  ben
 * ICMs MT_SCHEDULER_Self_... added
 *
 * Revision 3.11  2001/03/22 12:44:12  ben
 * ICMs MT_SCHEDULER_Cyclic_... added
 *
 * Revision 3.10  2001/03/21 11:55:42  ben
 * Bugs fixed in ICMs MT_SCHEDULER_Even_..., SelectTask
 *
 * Revision 3.9  2001/03/20 16:11:46  ben
 * Just implemented Static renamed to Even, because of existing Static
 * scheduling
 *
 * Revision 3.8  2001/03/20 13:19:33  ben
 * ICMs MT_SCHEDULER_Static_... (first version) implemented
 * SelectTask implemented
 *
 * Revision 3.7  2001/03/15 11:15:16  dkr
 * fixed a bug in ICMCompileMT_ADJUST_SCHEDULER:
 * right format string used now ... :-/
 *
 * Revision 3.6  2001/03/15 10:24:38  dkr
 * icm SAC_MT_ADJUST_SCHEDULER_OFFSET renamed into
 * SAC_MT_ADJUST_SCHEDULER__OFFSET
 *
 * Revision 3.5  2001/03/14 16:25:09  dkr
 * signature (parameter types) of icm MT_ADJUST_SCHEDULER modified
 *
 * Revision 3.4  2001/03/14 10:12:43  ben
 *  ICMs MT_SCHEDULER_BlockVar_... implemented
 *
 * Revision 3.3  2001/01/24 23:38:44  dkr
 * type of arguments of ICMs MT_SCHEDULER_..._BEGIN, MT_SCHEDULER_..._END
 * changed from int* to char**
 *
 * Revision 3.2  2001/01/22 13:44:54  dkr
 * bug in ICMCompileMT_ADJUST_SCHEDULER fixed:
 * adjustment of offset only allowed if offset is really used!
 *
 * Revision 3.1  2000/11/20 18:01:15  sacbase
 * new release made
 *
 * Revision 2.16  2000/10/09 19:16:46  dkr
 * GetUnadjustedFoldCode() renamed into GetFoldCode()
 *
 * Revision 2.15  2000/07/06 16:28:42  dkr
 * ICM ND_KD_A_SHAPE renamed into ND_A_SHAPE
 *
 * Revision 2.14  2000/07/06 14:50:24  dkr
 * BEtest support added
 *
 * Revision 2.13  2000/05/26 11:35:07  dkr
 * GetFoldCode() renamed into GetUnadjustedFoldCode()
 *
 * Revision 2.10  2000/01/17 16:25:58  cg
 * Removed static and dynamic versions of the ICMs
 * MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END].
 * General version now is identical with the former dynamic
 * version.
 *
 * Revision 2.9  1999/11/18 12:07:12  jhs
 * Added multi-thread-tracing in MULTIFOLD icm's.
 *
 * Revision 2.8  1999/09/01 17:12:39  jhs
 * Expanded COMPSync to refcounters in barriers.
 *
 * Revision 2.7  1999/07/30 13:52:12  jhs
 * Deleted unused parts.
 *
 * Revision 2.6  1999/07/21 12:13:29  jhs
 * Adjusted indenting.
 *
 * Revision 2.5  1999/07/20 16:55:06  jhs
 * Added comments.
 * Changed behaviour of MT_SPMD_SETUP, so shared[_rc] variables are no
 * longer setuped.
 * Changed signature of MT_SYNC_FOLD, added barrier_id.
 *
 * Revision 2.4  1999/06/30 16:00:11  jhs
 * Expanded backend, so compilation of fold-with-loops is now possible
 * during SPMD-Blocks containing more than one SYNC-Block.
 *
 * Revision 2.3  1999/06/25 15:41:47  jhs
 * Just to provide compilablity.
 *
 * Revision 2.2  1999/06/03 13:09:04  jhs
 * Changed ICMCompileMT_CONTINUE to handle exchanges of new allocated
 * arrays between master and slaves threads.
 *
 * Revision 2.1  1999/02/23 12:42:38  sacbase
 * new release made
 *
 * Revision 1.18  1998/08/27 14:48:45  cg
 * ICM ADJUST_SCHEDULER now gets also upper bound of respective block.
 *
 * Revision 1.17  1998/08/07 18:11:35  cg
 * bug fixed in ICMCompileMT_ADJUST_SCHEDULER
 *
 * Revision 1.16  1998/08/07 16:04:41  dkr
 * MT_SCHEDULER_BEGIN, MT_SCHEDULER_END added
 *
 * Revision 1.15  1998/08/03 10:50:52  cg
 * added implementation of new ICM MT_ADJUST_SCHEDULER
 *
 * Revision 1.14  1998/07/07 13:45:42  cg
 * bug fixed: fold function are now retrieved correctly.
 *
 * Revision 1.13  1998/07/03 10:18:15  cg
 * Super ICM MT_SPMD_BLOCK replaced by combinations of new ICMs
 * MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END]
 * MT_SPMD_SETUP and MT_SPMD_EXECUTE
 *
 * Revision 1.12  1998/06/29 08:57:13  cg
 * added tracing facilities
 *
 * Revision 1.11  1998/06/25 08:05:29  cg
 * syntax bug in produced C code fixed.
 *
 * Revision 1.10  1998/06/23 12:49:48  cg
 * added implementations of scheduling ICMs
 * MT_SCHEDULER_Block_BEGIN and MT_SCHEDULER_Block_END
 *
 * Revision 1.9  1998/06/12 14:06:09  cg
 * fixed bug using function GetFoldCode()
 *
 * Revision 1.8  1998/06/10 15:02:38  cg
 * Synchronisation ICMs now use function GetFoldCode to extract
 * implementation of fold operation.
 *
 * Revision 1.7  1998/06/03 14:52:44  cg
 * ICMs MT_SPMD_FUN_DEC and MT_SPMD_FUN_RET now longer generate preprocessor
 * conditional #if SAC_DO_MULTITHREAD
 *
 * Revision 1.6  1998/05/19 10:06:23  cg
 * minor bugs fixed
 *
 * Revision 1.5  1998/05/19 09:20:26  cg
 * works with BEtest now.
 *
 * Revision 1.4  1998/05/15 15:43:56  cg
 * first bugs removed
 *
 * Revision 1.3  1998/05/15 09:20:20  cg
 * first complete version
 *
 * Revision 1.2  1998/05/13 14:48:03  cg
 * added ICM MT_SYNC_ONEFOLD
 *
 * Revision 1.1  1998/05/13 07:22:57  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   icm2c_mt.c
 *
 * prefix: ICMCompile
 *
 * description:
 *
 *   This file contains the definitions of C implemented ICMs.
 *
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#include "compile.h"  /* for GetFoldCode()                        */
#include "free.h"
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   node *SearchFoldImplementation( char *foldop)
 *
 * description:
 *   This function traverses the fundef chain of the syntax tree in order to
 *   find the implementation of the given fold operation. The function
 *   GetFoldCode() is used afterwards to extract the relevant part of the
 *   function definition.
 *
 ******************************************************************************/

#ifndef BEtest
static node *
SearchFoldImplementation (char *foldop)
{
    node *fundef;

    DBUG_ENTER ("SearchFoldImplementation");

    fundef = MODUL_FOLDFUNS (syntax_tree);

    while ((fundef != NULL) && (0 != strcmp (FUNDEF_NAME (fundef), foldop))) {
        fundef = FUNDEF_NEXT (fundef);
    }

    DBUG_ASSERT ((fundef != NULL),
                 "Unknown fold operation specified in synchronisation ICM");

    DBUG_RETURN (GetFoldCode (fundef));
}
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_DEC( char *name, char *from,
 *                                   int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_DEC( name, from, vararg_cnt, [ TAG, type, param_NT ]* )
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function while the seconf one specifies the
 *   function where the respective piece of code originally has been situated.
 *   TAG may be from the set { in, out, inout }.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_DEC (char *name, char *from, int vararg_cnt, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_FUN_DEC");

#define MT_SPMD_FUN_DEC
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FUN_DEC

    fprintf (outfile, "#undef SAC_MT_CURRENT_FUN\n");
    fprintf (outfile, "#define SAC_MT_CURRENT_FUN() %s\n", from);

    fprintf (outfile, "\n");

    fprintf (outfile, "#undef SAC_MT_CURRENT_SPMD\n");
    fprintf (outfile, "#define SAC_MT_CURRENT_SPMD() %s\n", name);

    fprintf (outfile, "\n");

    fprintf (outfile,
             "SAC_MT_SPMD_FUN_REAL_RETTYPE()"
             " %s( SAC_MT_SPMD_FUN_REAL_PARAM_LIST())\n",
             name);
    fprintf (outfile, "{\n");

    indent++;

#ifndef TAGGED_ARRAYS
    INDENT;
    fprintf (outfile, "int SAC_dummy_refcount = 10;\n");
#endif /* TAGGED_ARRAYS */

    INDENT;
    fprintf (outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");

    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_PARAM_%s( %s, %s)\n", vararg[i], vararg[i + 1],
                 vararg[i + 2]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_RET( int barrier_id,
 *                                   int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_RET( barrier_id, vararg_cnt, [ tag, param_NT ]* )
 *
 *   This ICM implements the return statement of an spmd-function,
 *   i.e. it has to realize the return of several out parameters.
 *
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_RET (int barrier_id, int vararg_cnt, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_FUN_RET");

#define MT_SPMD_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FUN_RET

    for (i = 0; i < 2 * vararg_cnt; i += 2) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_RET_%s( %s);\n", vararg[i], vararg[i + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SPMD_FUN_REAL_RETURN();\n"),

      indent--;
    INDENT;
    fprintf (outfile, "}\n");

    INDENT;
    fprintf (outfile, "{\n");

    indent++;

    INDENT;
    fprintf (outfile, "label_worker_continue_%d:\n", barrier_id);

    INDENT;
    fprintf (outfile, "SAC_MT_SPMD_FUN_REAL_RETURN();\n");

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_START_SYNCBLOCK( int barrier_id,
 *                                      int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_START_SYNCBLOCK( barrier_id, vararg_cnt, [ tag, type, param_NT ]* )
 *
 *   This ICM implements the begin of a synchronisation block.
 *   Essentially, the reference counters of the arguments are shadowed by
 *   thread-specific dummy reference counters. Dummies can be used safely
 *   since memory may exclusively been released in between synchronisation
 *   blocks. Nevertheless, they must be provided because arbitrary code in
 *   the with-loop body may want to increment/decrement reference counters.
 *
 *   However, this ICM also complements the various ICMs that implement
 *   different kinds of barrier synchronisations.
 *
 ******************************************************************************/

void
ICMCompileMT_START_SYNCBLOCK (int barrier_id, int vararg_cnt, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_START_SYNCBLOCK");

#define MT_START_SYNCBLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_START_SYNCBLOCK

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

#ifdef TAGGED_ARRAYS
    for (i = 0; i < 3 * vararg_cnt; i += 3) {
        INDENT;
        fprintf (outfile, "SAC_MT_DECL_LOCAL_DESC( %s)\n", vararg[i + 2]);
    }
    fprintf (outfile, "\n");
#endif /* TAGGED_ARRAYS */

    INDENT;
    fprintf (outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");

    for (i = 0; i < 3 * vararg_cnt; i += 3) {
#ifdef TAGGED_ARRAYS
        INDENT;
        fprintf (outfile, "SAC_MT_INIT_DESC( %s)\n", vararg[i + 2]);
#else  /* TAGGED_ARRAYS */
        if (0 == strcmp (vararg[i], "in_rc")) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_RC( %s) = &SAC_dummy_refcount;\n",
                     vararg[i + 2]);
        }
#endif /* TAGGED_ARRAYS */
    }

    INDENT;
    fprintf (outfile, "SAC_TR_MT_PRINT("
                      " (\"Starting execution of synchronisation block\"));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD( int barrier_id, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD( barrier_id, vararg_cnt, [ foldtype, accu_NT, tmp_NT, foldop ]* )
 *
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain several fold with-loops but no genarray/modarray
 *   with-loops.
 *
 *   Each fold with-loop corresponds with a quadrupel of ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void
ICMCompileMT_SYNC_FOLD (int barrier_id, int vararg_cnt, char **vararg)
{
#ifndef BEtest
    node **foldcodes;
#endif /*BEtest*/
    char *foldop = NULL;
    int i;

    DBUG_ENTER ("ICMCompileMT_SYNC_FOLD");

#define MT_SYNC_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_FOLD

    /*
     * fetch code-elements for all fold-operations
     */
#ifndef BEtest
    foldcodes = (node **)Malloc (vararg_cnt * sizeof (node *));
    for (i = 0; i < vararg_cnt; i++) {
        foldop = vararg[4 * i + 3];
        foldcodes[i] = SearchFoldImplementation (foldop);
    }
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1A( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s, \"Pure thread fold result:\");\n",
                 vararg[4 * i], vararg[4 * i + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1B( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_SET_BARRIER_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s)\n",
                 i + 1, vararg[4 * i], vararg[4 * i + 1]);
#else  /* TAGGED_ARRAYS */
        if (0 != strcmp ("array_rc", vararg[4 * i])) {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[4 * i], vararg[4 * i + 1]);
        } else {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RC_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[4 * i], vararg[4 * i + 1]);
        }
#endif /* TAGGED_ARRAYS */
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1C( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s, \"Partial thread fold "
                 "result:\");\n",
                 vararg[4 * i], vararg[4 * i + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1D( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_GET_BARRIER_RESULT( SAC_MT_son_id, %i, %s, %s)\n",
                 i + 1, vararg[4 * i], vararg[4 * i + 2]);
#else  /* TAGGED_ARRAYS */
        if (0 != strcmp ("array_rc", vararg[4 * i])) {
            fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT( SAC_MT_son_id, %i, %s);\n",
                     vararg[4 * i + 2], i + 1, vararg[4 * i]);
        } else {
            fprintf (outfile,
                     "%s = SAC_MT_GET_BARRIER_RC_RESULT_PTR( SAC_MT_son_id, %i, %s);\n",
                     vararg[4 * i + 2], i + 1, vararg[4 * i]);
            INDENT;
            fprintf (outfile,
                     "%s__rc = SAC_MT_GET_BARRIER_RC_RESULT_RC( SAC_MT_son_id, %i, "
                     "%s);\n",
                     vararg[4 * i + 2], i + 1, vararg[4 * i]);
        }
#endif /* TAGGED_ARRAYS */
#ifndef BEtest
        INDENT;
        fprintf (outfile, "{\n");
        indent++;
        Trav (foldcodes[i], NULL);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
#else  /* BEtest */
        INDENT;
        fprintf (outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_2A( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_SET_BARRIER_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s)\n",
                 i + 1, vararg[4 * i], vararg[4 * i + 1]);
#else  /* TAGGED_ARRAYS */
        if (0 != strcmp ("array_rc", vararg[4 * i])) {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[4 * i], vararg[4 * i + 1]);
        } else {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RC_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[4 * i], vararg[4 * i + 1]);
        }
#endif /* TAGGED_ARRAYS */
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_2B( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s, \"Partial thread fold "
                 "result:\");\n",
                 vararg[4 * i], vararg[4 * i + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_2C( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_GET_BARRIER_RESULT( SAC_MT_son_id, %i, %s, %s)\n",
                 i + 1, vararg[4 * i], vararg[4 * i + 2]);
#else  /* TAGGED_ARRAYS */
        if (0 != strcmp ("array_rc", vararg[4 * i])) {
            fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT( SAC_MT_son_id, %i, %s);\n",
                     vararg[4 * i + 2], i + 1, vararg[4 * i]);
        } else {
            fprintf (outfile,
                     "%s = SAC_MT_GET_BARRIER_RC_RESULT_PTR( SAC_MT_son_id, %i, %s);\n",
                     vararg[4 * i + 2], i + 1, vararg[4 * i]);
            INDENT;
            fprintf (outfile,
                     "%s__rc = SAC_MT_GET_BARRIER_RC_RESULT_RC( SAC_MT_son_id, %i, "
                     "%s);\n",
                     vararg[4 * i + 2], i + 1, vararg[4 * i]);
        }
#endif /* TAGGED_ARRAYS */

#ifndef BEtest
        INDENT;
        fprintf (outfile, "{\n");
        indent++;
        Trav (foldcodes[i], NULL);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
#else  /* BEtest */
        INDENT;
        fprintf (outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_3A( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_SET_BARRIER_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s)\n",
                 i + 1, vararg[4 * i], vararg[4 * i + 1]);
#else  /* TAGGED_ARRAYS */
        if (0 != strcmp ("array_rc", vararg[4 * i])) {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[4 * i], vararg[4 * i + 1]);
        } else {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RC_RESULT( SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[4 * i], vararg[4 * i + 1]);
        }
#endif /* TAGGED_ARRAYS */
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_3B( %d)\n", barrier_id);

    for (i = 0; i < vararg_cnt; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT( %s, %s, \"Partial thread fold "
                 "result:\");\n",
                 vararg[4 * i], vararg[4 * i + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_3C( %d)\n", barrier_id);

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

#ifndef BEtest
    for (i = 0; i < vararg_cnt; i++) {
        foldcodes[i] = FreeTree (foldcodes[i]);
    }
    foldcodes = Free (foldcodes);
#endif /* BEtest */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_ONEFOLD(int barrier_id,
 *                                  char *foldtype, char *accu_var,
 *                                  char *tmp_var, char *foldop)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_ONEFOLD( barrier_id, foldtype, accu_var, tmp_var, foldop)
 *
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exactly one fold with-loop.
 *
 *   The fold with-loop is described by 4 ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void
ICMCompileMT_SYNC_ONEFOLD (int barrier_id, char *foldtype, char *accu_var, char *tmp_var,
                           char *foldop)
{
#ifndef BEtest
    node *fold_code;
#endif /* BEtest */

    DBUG_ENTER ("ICMCompileMT_SYNC_ONEFOLD");

#define MT_SYNC_ONEFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_ONEFOLD

#ifndef BEtest
    fold_code = SearchFoldImplementation (foldop);
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_1( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

#ifndef BEtest
    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    Trav (fold_code, NULL);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
#else  /* BEtest */
    INDENT;
    fprintf (outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_2( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

#ifndef BEtest
    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    Trav (fold_code, NULL);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
#else  /* BEtest */
    INDENT;
    fprintf (outfile, "/* fold operation: %s */\n", foldop);
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_3( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

    indent--;

    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

#ifndef BEtest
    fold_code = FreeTree (fold_code);
#endif /* BEtest */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_NONFOLD( int barrier_id)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_NONFOLD( barrier_id)
 *
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exclusively modarray/genarray with-loops.
 *
 ******************************************************************************/

void
ICMCompileMT_SYNC_NONFOLD (int barrier_id)
{
    DBUG_ENTER ("ICMCompileMT_SYNC_NONFOLD");

#define MT_SYNC_NONFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_NONFOLD

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_NONFOLD_1( %d)\n", barrier_id);

    indent--;

    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD_NONFOLD( int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD_NONFOLD( vararg_cnt, [ foldtype, accu_NT, tmp_NT, foldop ]* )
 *
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain several fold with-loops as well as additional genarray/modarray
 *   with-loops.
 *
 *   Each fold with-loop corresponds with a quadrupel of ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void
ICMCompileMT_SYNC_FOLD_NONFOLD (int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SYNC_FOLD_NONFOLD");

#define MT_SYNC_FOLD_NONFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_FOLD_NONFOLD

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_ONEFOLD_NONFOLD(char *foldtype, char *accu_var,
 *                                          char *tmp_var, char *foldop)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_ONEFOLD_NONFOLD( foldtype, accu_var, tmp_var, foldop)
 *
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exactly one fold with-loop and additionally an arbitrary
 *   number of modarray/genarray with-loops.
 *
 *   The fold with-loop is described by 4 ICM arguments,
 *   specifying the type of the fold result, the name of the accumulator
 *   variable, the name of the accumulated variable, and the fold operation
 *   itself. The type may be either one of the scalar data types provided
 *   by SAC or one of the specifiers 'array' or 'hidden'.
 *
 ******************************************************************************/

void
ICMCompileMT_SYNC_ONEFOLD_NONFOLD (char *foldtype, char *accu_var, char *tmp_var,
                                   char *foldop)
{
    DBUG_ENTER ("ICMCompileMT_SYNC_ONEFOLD_NONFOLD");

#define MT_SYNC_ONEFOLD_NONFOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_ONEFOLD_NONFOLD

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_SEND_FOLDRESULTS( int foldargs_cnt, char **foldargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_SEND_FOLDRESULTS( foldargs_cnt, [ foldtype, accu_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to send results of fold with-loops to the SPMD-frame ... BUT ...
 *
 *   ... All results from fold-with-loops will be stored automatically in the
 *   barrier, so one does not need to put them in the frame too (they will
 *   be fetched from the barrier instead). The ICM is needed only to have a
 *   complete set of macros, so the code is readable.
 *
 ******************************************************************************/

void
ICMCompileMT_MASTER_SEND_FOLDRESULTS (int foldargs_cnt, char **foldargs)
{
    DBUG_ENTER ("ICMCompileMT_NASTER_SEND_FOLDRESULTS");

#define MT_MASTER_SEND_FOLDRESULTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_FOLDRESULTS

    if (foldargs_cnt > 0) {
        INDENT;
        fprintf (outfile, "/* all needed values are already stored in the barrier */");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS( int foldargs_cnt, char **foldargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_FOLDRESULTS( foldargs_cnt, [ foldtype, accu_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to receive results of fold with-loops from the SPMD-frame ... BUT ...
 *
 *   ... All results from fold-with-loops will be stored automatically in the
 *   barrier, so one does not put them in the frame. Here they will be fetched
 *   from the barrier instead.
 *
 ******************************************************************************/

void
ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS (int foldargs_cnt, char **foldargs)
{
    int i, j;

    DBUG_ENTER ("ICMCompileMT_NASTER_RECEIVE_FOLDRESULTS");

#define MT_MASTER_RECEIVE_FOLDRESULTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_RECEIVE_FOLDRESULTS

    for (i = 0, j = 1; i < 2 * foldargs_cnt; i += 2, j++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_GET_BARRIER_RESULT( 0, %d, %s, %s)\n", j, foldargs[i],
                 foldargs[i + 1]);
#else  /* TAGGED_ARRAYS */
        fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT( 0, %d, %s);\n",
                 foldargs[i + 1], j, foldargs[i]);
#endif /* TAGGED_ARRAYS */
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_SEND_SYNCARGS( int syncargs_cnt, char **syncargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_SEND_SYNCARGS( syncargs_cnt, [ arg_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to send values changed in the master part.
 *   The values are put in the SPMD-frame.
 *
 ******************************************************************************/

void
ICMCompileMT_MASTER_SEND_SYNCARGS (int syncargs_cnt, char **syncargs)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_MASTER_SEND_SYNCARGS");

#define MT_MASTER_SEND_SYNCARGS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_SYNCARGS

    for (i = 0; i < syncargs_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_SPMD_RET_shared( %s);\n", syncargs[i]);
#else  /* TAGGED_ARRAYS */
        fprintf (outfile, "SAC_MT_SPMD_RET_shared_rc( %s);\n", syncargs[i]);
#endif /* TAGGED_ARRAYS */
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_RECEIVE_SYNCARGS( int syncargs_cnt, char **syncargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_SYNCARGS( syncargs_cnt, [ arg_NT ]* )
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to receive values changed in the master part.
 *   The values are fetched from the SPMD-frame.
 *
 ******************************************************************************/

void
ICMCompileMT_MASTER_RECEIVE_SYNCARGS (int syncargs_cnt, char **syncargs)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_MASTER_RECEIVE_SYNCARGS");

#define MT_MASTER_RECEIVE_SYNCARGS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_RECEIVE_SYNCARGS

    for (i = 0; i < syncargs_cnt; i++) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile, "SAC_MT_SPMD_GET_shared( %s);\n", syncargs[i]);
#else  /* TAGGED_ARRAYS */
        fprintf (outfile, "SAC_MT_SPMD_GET_shared_rc( %s);\n", syncargs[i]);
#endif /* TAGGED_ARRAYS */
    }

    DBUG_VOID_RETURN;
}

#if 0 /* jhs: no longer needed, but kept for references */

/******************************************************************************
 * 
 *
 * function:
 *   void ICMCompileMT_CONTINUE( int foldargs_cnt, char **vararg,
 *                               int syncargs_cnt, char **syncargs)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_CONTINUE( foldargs_cnt, [ foldtype, accu_NT ]*,
 *                syncargs_cnt, [ arg_NT ]* );
 *   
 *   This ICM implements the continuation after a barrier synchronisation.
 *   It restarts the synchronized worker threads and updates their current
 *   values of fold results from the preceding synchronisation block.
 *   The variable argument part consists of pairs, that specify the type and
 *   accumulation variable of these fold operations.
 *
 ******************************************************************************/

void ICMCompileMT_CONTINUE( int foldargs_cnt, char **vararg,
                            int syncargs_cnt, char **syncargs)
{
  int i, j;
  int barrier_id = -88;   /* former global variable */ 
  
  DBUG_ENTER("ICMCompileMT_CONTINUE");

#define MT_CONTINUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_CONTINUE

  for (i = 0; i < syncargs_cnt; i++) {
    INDENT;
#ifdef TAGGED_ARRAYS
    fprintf( outfile, "SAC_MT_SPMD_RET_shared( %s);\n", syncargs[i]);
#else  /* TAGGED_ARRAYS */
    fprintf( outfile, "SAC_MT_SPMD_RET_shared_rc( %s);\n", syncargs[i]);
#endif /* TAGGED_ARRAYS */
  }

  INDENT;
  fprintf( outfile, "SAC_MT_START_WORKERS()\n");

  INDENT;
  fprintf( outfile, "goto label_continue_%d;\n", barrier_id);
  
  indent--;
  INDENT;
  fprintf( outfile, "}\n");
  
  INDENT;
  fprintf( outfile, "{\n");
  
  indent++;

  INDENT;
  fprintf( outfile, "label_worker_continue_%d:\n", barrier_id);

  INDENT;
  fprintf( outfile, "SAC_MT_WORKER_WAIT()\n");
  
  for (i = 0, j = 1; i < 2 * foldargs_cnt; i += 2, j++) 
  {
    INDENT;
#ifdef TAGGED_ARRAYS
    fprintf( outfile, "SAC_MT_GET_BARRIER_RESULT( 0, %d, %s, %s)\n",
                      j, vararg[i], vararg[i+1]);
#else  /* TAGGED_ARRAYS */
    fprintf( outfile, "%s = SAC_MT_GET_BARRIER_RESULT( 0, %d, %s);\n",
                      vararg[i+1], j, vararg[i]);
#endif /* TAGGED_ARRAYS */
  }

  for (i = 0; i < syncargs_cnt; i++) {
    INDENT;
#ifdef TAGGED_ARRAYS
    fprintf( outfile, "SAC_MT_SPMD_GET_shared( %s);\n", syncargs[i]);
#else  /* TAGGED_ARRAYS */
    fprintf( outfile, "SAC_MT_SPMD_GET_shared_rc( %s);\n", syncargs[i]);
#endif /* TAGGED_ARRAYS */
  }
  
  indent--;
  INDENT;
  fprintf( outfile, "}\n");
  
  fprintf( outfile, "\n");
  
  INDENT;
  fprintf( outfile, "label_continue_%d:\n", barrier_id);
    
  DBUG_VOID_RETURN;
}

#endif /* 0 */

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_SETUP( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_SETUP( name, vararg_cnt, [ tag, type, arg_NT ]* )
 *
 *   the arguments handed over could have the tags:
 *     in, out, shared.
 *   One needs to set the in variables to the spmd-region, that is normal.
 *   One also needs to set the adresses ot the out-variables, so the
 *   spmd-region can put the values there automatically.
 *   One does *not* need to set the shared variables here (they are only used
 *   within the spmd-region), so they ar ignored here!
 *   Because this icm is also used to determine the spmd-frame, one needs
 *   to have the shared-variabels here (until this ic changed in someway).
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_SETUP (char *name, int vararg_cnt, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_SETUP");

#define MT_SPMD_SETUP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_SETUP

    for (i = 0; i < 3 * vararg_cnt; i += 3) {
#ifdef TAGGED_ARRAYS
        if ((strcmp (vararg[i], "in") == 0) || (strcmp (vararg[i], "out") == 0)) {
            INDENT;
            fprintf (outfile, "SAC_MT_SPMD_SETARG_%s( %s, %s);\n", vararg[i], name,
                     vararg[i + 2]);
        }
#else  /* TAGGED_ARRAYS */
        if ((strcmp (vararg[i], "in") == 0) || (strcmp (vararg[i], "in_rc") == 0)
            || (strcmp (vararg[i], "out") == 0) || (strcmp (vararg[i], "out_rc") == 0)) {
            INDENT;
            fprintf (outfile, "SAC_MT_SPMD_SETARG_%s( %s, %s);\n", vararg[i], name,
                     vararg[i + 2]);
        }
#endif /* TAGGED_ARRAYS */
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_BEGIN( char *name)
 *   void ICMCompileMT_SPMD_ALTSEQ( char *name)
 *   void ICMCompileMT_SPMD_END( char *name)
 *
 * description:
 *   These functions implement the control flow ICMs around SPMD-blocks.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_BEGIN (char *name)
{
    DBUG_ENTER ("ICMCompileMT_SPMD_BEGIN");

#define MT_SPMD_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BEGIN

    fprintf (outfile, "\n"
                      "#if SAC_DO_MULTITHREAD\n");
    INDENT;
    fprintf (outfile, "if (SAC_MT_not_yet_parallel)\n");
    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    INDENT;
    fprintf (outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SPMD_ALTSEQ (char *name)
{
    DBUG_ENTER ("ICMCompileMT_SPMD_ALTSEQ");

    indent--;

#define MT_SPMD_ALTSEQ
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_ALTSEQ

    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "else\n");
    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    INDENT;
    fprintf (outfile, "SAC_MT_DETERMINE_THREAD_ID()\n");
    INDENT;
    fprintf (outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");
    fprintf (outfile, "#endif  /* SAC_DO_MULTITHREAD */\n\n");

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SPMD_END (char *name)
{
    DBUG_ENTER ("ICMCompileMT_SPMD_END");

#define MT_SPMD_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_END

    fprintf (outfile, "\n"
                      "#if SAC_DO_MULTITHREAD\n");
    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "#endif  /* SAC_DO_MULTITHREAD */\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_PRESET( char *name, int vararg_cnt, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_PRESET( name, vararg_cnt, [ tag, arg ]* )
 *
 *   This ICM implements a feature that allows to partially build up an spmd
 *   frame in advance, i.e. without immediately starting concurrent execution.
 *   If an spmd block is situated within the body of a sequential loop,
 *   it usually will be executed repeatedly resulting in the repeated building
 *   of its spmd frame. However, some of the parameters may be loop-invariant
 *   which allows to move them out of the loop body.
 *
 *   Tags should be "in".
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_PRESET (char *name, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SPMD_PRESET");

#define MT_SPMD_PRESET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_PRESET

    DBUG_VOID_RETURN;
}
