/*
 *
 * $Log$
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
 * Changed behaviour of MT_SPMD_SETUP, so shared[_rc] variables are no longer setuped.
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
 *
 *
 *
 *****************************************************************************/

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree             */
#include "traverse.h" /* for traversal of fold operation function        */
#include "compile.h"  /* for GetFoldCode()                               */
#include "free.h"     /* for freeing fold-code produced by GetFoldCode() */
#endif                /* BEtest */

/******************************************************************************
 *
 * function:
 *   node *SearchFoldImplementation(char *foldop)
 *
 * description:
 *
 *   This function traverses the fundef chain of the syntax tree in order to
 *   find the implementation of the given fold operation. The function GetFoldCode
 *   is used afterwards to extract the relevant part of the function definition.
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
                 ("Unknown fold operation specified in synchronisation ICM"));

    DBUG_RETURN (GetFoldCode (fundef));
}
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_DEC(char *name, char *from,
 *                                  int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_DEC( name, from, narg [, tag, type, param]*)
 *
 *   This ICM implements the protoype of an spmd function. The first parameter
 *   specifies the name of this function while the seconf one specifies the
 *   function where the respective piece of code originally has been situated.
 *   Tags may be from the set in | out | in_rc | out_rc | inout_rc.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_DEC (char *name, char *from, int narg, char **vararg)
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

    INDENT;
    fprintf (outfile, "int SAC_dummy_refcount=10;\n");

    INDENT;
    fprintf (outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_single_threaded)\n");

    for (i = 0; i < 3 * narg; i += 3) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_PARAM_%s( %s, %s)\n", vararg[i], vararg[i + 1],
                 vararg[i + 2]);
        DBUG_PRINT ("ICMCompile", ("SAC_MT_SPMD_PARAM_%s( %s, %s)", vararg[i],
                                   vararg[i + 1], vararg[i + 2]));
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_RET( int barrier_id, int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_RET( barrier_id, narg [, tag, param]*)
 *
 *   This ICM implements the return statement of an spmd-function,
 *   i.e. it has to realize the return of several out parameters.
 *
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_RET (int barrier_id, int narg, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_FUN_RET");

#define MT_SPMD_FUN_RET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_FUN_RET

    for (i = 0; i < 2 * narg; i += 2) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_RET_%s(%s);\n", vararg[i], vararg[i + 1]);
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
 *   void ICMCompileMT_START_SYNCBLOCK(int barrier_id, int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_START_SYNCBLOCK(int barrier_id, narg [, tag, type, param]*)
 *
 *   This ICM implements the begin of a synchronisation block. Essentially,
 *   the reference counters of the arguments tagged in_rc are shadowed by
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
ICMCompileMT_START_SYNCBLOCK (int barrier_id, int narg, char **vararg)
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

    INDENT;
    fprintf (outfile, "SAC_HM_DEFINE_THREAD_STATUS( SAC_HM_multi_threaded)\n");

    for (i = 0; i < 3 * narg; i += 3) {
        if (0 == strcmp (vararg[i], "in_rc")) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_RC(%s) = &SAC_dummy_refcount;\n",
                     vararg[i + 2]);
        }
    }

    INDENT;
    fprintf (outfile,
             "SAC_TR_MT_PRINT((\"Starting execution of synchronisation block\"));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD(int barrier_id, int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD( barrier_id, narg [, foldtype, accu_var, tmp_var, foldop]*)
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
ICMCompileMT_SYNC_FOLD (int barrier_id, int narg, char **vararg)
{
    node **foldcodes;
    char *foldop;
    int i;

    DBUG_ENTER ("ICMCompileMT_SYNC_FOLD");

#define MT_SYNC_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_FOLD

    /*
     *  fetch code-elements for all fold-operations
     */
    foldcodes = (node **)Malloc (narg * sizeof (node *));
    for (i = 0; i < narg; i++) {
        foldop = vararg[(i * 4) + 3];
        DBUG_PRINT ("COMPi", ("%i %s", i, foldop));
        foldcodes[i] = SearchFoldImplementation (foldop);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1A( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT(%s, %s, \"Pure thread fold result:\");\n",
                 vararg[(i * 4) + 0], vararg[(i * 4) + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1B( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        if (0 != strcmp ("array_rc", vararg[(i * 4)])) {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RESULT(SAC_MT_MYTHREAD(), %i, %s, %s);\n", i + 1,
                     vararg[(i * 4)], vararg[(i * 4) + 1]);
        } else {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RC_RESULT(SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[(i * 4)], vararg[(i * 4) + 1]);
        }
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1C( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT(%s, %s, \"Partial thread fold "
                 "result:\");\n",
                 vararg[(i * 4) + 0], vararg[(i * 4) + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_1D( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        if (0 != strcmp ("array_rc", vararg[(i * 4)])) {
            INDENT;
            fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT(SAC_MT_son_id, %i, %s);\n",
                     vararg[(i * 4) + 2], i + 1, vararg[(i * 4)]);
        } else {
            INDENT;
            fprintf (outfile,
                     "%s = SAC_MT_GET_BARRIER_RC_RESULT_PTR(SAC_MT_son_id, %i, %s);\n",
                     vararg[(i * 4) + 2], i + 1, vararg[(i * 4)]);
            INDENT;
            fprintf (outfile,
                     "%s__rc = SAC_MT_GET_BARRIER_RC_RESULT_RC(SAC_MT_son_id, %i, %s);\n",
                     vararg[(i * 4) + 2], i + 1, vararg[(i * 4)]);
        }
        INDENT;
        fprintf (outfile, "{\n");
        indent++;
        Trav (foldcodes[i], NULL);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_2A( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        if (0 != strcmp ("array_rc", vararg[(i * 4)])) {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RESULT(SAC_MT_MYTHREAD(), %i, %s, %s);\n", i + 1,
                     vararg[(i * 4)], vararg[(i * 4) + 1]);
        } else {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RC_RESULT(SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[(i * 4)], vararg[(i * 4) + 1]);
        }
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_2B( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT(%s, %s, \"Partial thread fold "
                 "result:\");\n",
                 vararg[(i * 4) + 0], vararg[(i * 4) + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_2C( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        if (0 != strcmp ("array_rc", vararg[(i * 4)])) {
            INDENT;
            fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT(SAC_MT_son_id, %i, %s);\n",
                     vararg[(i * 4) + 2], i + 1, vararg[(i * 4)]);
        } else {
            INDENT;
            fprintf (outfile,
                     "%s = SAC_MT_GET_BARRIER_RC_RESULT_PTR(SAC_MT_son_id, %i, %s);\n",
                     vararg[(i * 4) + 2], i + 1, vararg[(i * 4)]);
            INDENT;
            fprintf (outfile,
                     "%s__rc = SAC_MT_GET_BARRIER_RC_RESULT_RC(SAC_MT_son_id, %i, %s);\n",
                     vararg[(i * 4) + 2], i + 1, vararg[(i * 4)]);
        }
        INDENT;
        fprintf (outfile, "{\n");
        indent++;
        Trav (foldcodes[i], NULL);
        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_3A( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        if (0 != strcmp ("array_rc", vararg[(i * 4)])) {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RESULT(SAC_MT_MYTHREAD(), %i, %s, %s);\n", i + 1,
                     vararg[(i * 4)], vararg[(i * 4) + 1]);
        } else {
            fprintf (outfile,
                     "SAC_MT_SET_BARRIER_RC_RESULT(SAC_MT_MYTHREAD(), %i, %s, %s);\n",
                     i + 1, vararg[(i * 4)], vararg[(i * 4) + 1]);
        }
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_3B( %d)\n", barrier_id);

    for (i = 0; i < narg; i++) {
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MT_PRINT_FOLD_RESULT(%s, %s, \"Partial thread fold "
                 "result:\");\n",
                 vararg[(i * 4) + 0], vararg[(i * 4) + 1]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_MULTIFOLD_3C( %d)\n", barrier_id);

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    FREE (foldcodes);
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

#ifdef BEtest
    INDENT;
    fprintf (outfile, "/* fold operation: %s */\n", foldop);
#else  /* BEtest */
    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    Trav (fold_code, NULL);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_2( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

#ifdef BEtest
    INDENT;
    fprintf (outfile, "/* fold operation: %s */\n", foldop);
#else  /* BEtest */
    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    Trav (fold_code, NULL);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_3( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

    indent--;

    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

#ifndef BEtest
    FreeTree (fold_code);
#endif /* BEtest */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_NONFOLD( )
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_NONFOLD( int barrier_id)
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
    fprintf (outfile, "SAC_MT_SYNC_NONFOLD_1(%d)\n", barrier_id);

    indent--;

    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD_NONFOLD(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD_NONFOLD( narg [, foldtype, accu_var, tmp_var, foldop]*)
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
ICMCompileMT_SYNC_FOLD_NONFOLD (int narg, char **vararg)
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
 *   void ICMCompileMT_MASTER_SEND_FOLDRESULTS( int nfoldargs, char **foldargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_SEND_FOLDRESULTS( nfoldargs, [, foldtype, accu_var]*)
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
ICMCompileMT_MASTER_SEND_FOLDRESULTS (int nfoldargs, char **foldargs)
{
    DBUG_ENTER ("ICMCompileMT_NASTER_SEND_FOLDRESULTS");

#define MT_MASTER_SEND_FOLDRESULTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_FOLDRESULTS

    if (nfoldargs > 0) {
        INDENT;
        fprintf (outfile, "/* all needed values are already stored in the barrier */");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS( int nfoldargs, char **foldargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_FOLDRESULTS( nfoldargs, [, foldtype, accu_var]*)
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
ICMCompileMT_MASTER_RECEIVE_FOLDRESULTS (int nfoldargs, char **foldargs)
{
    int i, j;

    DBUG_ENTER ("ICMCompileMT_NASTER_RECEIVE_FOLDRESULTS");

#define MT_MASTER_RECEIVE_FOLDRESULTS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_RECEIVE_FOLDRESULTS

    for (i = 0, j = 1; i < 2 * nfoldargs; i += 2, j++) {
        INDENT;
        fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT(0, %d, %s);\n", foldargs[i + 1],
                 j, foldargs[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_SEND_SYNCARGS( int nsyncargs, char **syncargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_FOLDRESULTS( nfoldargs, [, foldtype, accu_var]*)
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to send values changed in the master part.
 *   The values are put in the SPMD-frame.
 *
 ******************************************************************************/
void
ICMCompileMT_MASTER_SEND_SYNCARGS (int nsyncargs, char **syncargs)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_MASTER_SEND_SYNCARGS");

#define MT_MASTER_SEND_SYNCARGS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_SYNCARGS

    for (i = 0; i < nsyncargs; i++) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_RET_shared_rc( %s);\n", syncargs[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_MASTER_RECEIVE_SYNCARGS( int nsyncargs, char **syncargs)
 *
 * description:
 *   compiles the corresponding ICM:
 *
 *   MT_MASTER_RECEIVE_FOLDRESULTS( nfoldargs, [, foldtype, accu_var]*)
 *
 *   As part of the value exchange between two SYNC-blocks it is responsible
 *   to receive values changed in the master part.
 *   The values are fetched from the SPMD-frame.
 *
 ******************************************************************************/
void
ICMCompileMT_MASTER_RECEIVE_SYNCARGS (int nsyncargs, char **syncargs)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_MASTER_RECEIVE_SYNCARGS");

#define MT_MASTER_SEND_SYNCARGS
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_MASTER_SEND_SYNCARGS

    for (i = 0; i < nsyncargs; i++) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_GET_shared_rc( %s);\n", syncargs[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 * ####jhs no longer needed, but kept for references
 *
 * function:
 *   void ICMCompileMT_CONTINUE(int nfoldargs, int nsyncargs,
 *                              char **vararg, char **syncargs)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_CONTINUE( nfoldargs, nsyncargs
 *                [, foldtype, accu_var]*,
 *                [, ... ]*);
 *
 *   This ICM implements the continuation after a barrier synchronisation.
 *   It restarts the synchronized worker threads and updates their current
 *   values of fold results from the preceding synchronisation block.
 *   The variable argument part consists of pairs, that specify the type and
 *   accumulation variable of these fold operations.
 *
 ******************************************************************************/

void
ICMCompileMT_CONTINUE (int nfoldargs, char **vararg, int nsyncargs, char **syncargs)
{
    int i, j;
    int barrier_id = -88; /* former global variable */

    DBUG_ENTER ("ICMCompileMT_CONTINUE");

#define MT_CONTINUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_CONTINUE

    for (i = 0; i < nsyncargs; i++) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_RET_shared_rc( %s);\n", syncargs[i]);
    }

    INDENT;
    fprintf (outfile, "SAC_MT_START_WORKERS()\n");

    INDENT;
    fprintf (outfile, "goto label_continue_%d;\n", barrier_id);

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    INDENT;
    fprintf (outfile, "{\n");

    indent++;

    INDENT;
    fprintf (outfile, "label_worker_continue_%d:\n", barrier_id);

    INDENT;
    fprintf (outfile, "SAC_MT_WORKER_WAIT()\n");

    for (i = 0, j = 1; i < 2 * nfoldargs; i += 2, j++) {
        INDENT;
        fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT(0, %d, %s);\n", vararg[i + 1],
                 j, vararg[i]);
    }

    for (i = 0; i < nsyncargs; i++) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_GET_shared_rc( %s);\n", syncargs[i]);
    }

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, "label_continue_%d:\n", barrier_id);

    DBUG_VOID_RETURN;
}

#if 0
This ICM is no longer used. It is replaced by the more modular ICMs
MT_SPMD_[STATIC|DYNAMIC]_MODE_[BEGIN|ALTSEQ|END]
MT_SPMD_SETUP
MT_SPMD_EXECUTE
  #### can it be deleted then ??? 

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_BLOCK(char *name, int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_BLOCK( name, narg [, tag, type, arg]*)
 *   
 *   This ICM implements SPMD blocks within the sequential code, i.e.
 *     - it decides whether to execute the SPMD block in parallel or sequentially.
 *     - it establishes the execution environment of the particular spmd function.
 *     - it starts the worker threads.
 *     - it calls the spmd function on behalf of the master thread.
 *     - finally, it resumes and returns to sequential execution order.
 *
 *   Tags may be from the set in | out | in_rc | out_rc | inout_rc<n> | preset.
 *   'preset' specifies an spmd argument that is preset by using the ICM
 *   MT_SPMD_PRESET().
 *
 ******************************************************************************/

void ICMCompileMT_SPMD_BLOCK(char *name, int narg, char **vararg)
{
  int i;
  static char basetype[32];
  
  DBUG_ENTER("ICMCompileMT_SPMD_BLOCK");

#define MT_SPMD_BLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BLOCK

  fprintf(outfile, "\n");
  
  fprintf(outfile, "#if SAC_DO_MULTITHREAD\n\n");

  INDENT;
  fprintf(outfile, "if (SAC_MT_not_yet_parallel)\n");
  
  INDENT;
  fprintf(outfile, "{\n");
  indent++;
  
  
  INDENT;
  fprintf(outfile, "SAC_MT_not_yet_parallel=0;\n");

  for (i=0; i<3*narg; i+=3)      
  {
    if (0==strncmp(vararg[i], "inout_rc", 8)) {
  
      strncpy(basetype, vararg[i+1], strchr(vararg[i+1], '*') - vararg[i+1]);
      
      INDENT;
      fprintf(outfile, "SAC_ND_ALLOC_ARRAY(%s, %s, %s);\n",
              basetype, vararg[i+2], vararg[i]+8);
      
      INDENT;
      fprintf(outfile, "SAC_MT_SPMD_SETARG_inout_rc(%s, %s);\n", name, vararg[i+2]);
    }
    else {
      INDENT;
      fprintf(outfile, "SAC_MT_SPMD_SETARG_%s(%s, %s);\n", vararg[i], name, vararg[i+2]);
    }
  }
  
  INDENT;
  fprintf(outfile, "\n");

  INDENT;
  fprintf(outfile, "SAC_MT_START_SPMD(%s);\n", name);

  INDENT;
  fprintf(outfile, "SAC_MT_not_yet_parallel=1;\n");

  indent--;
  INDENT;
  fprintf(outfile, "}\n");
  
  INDENT;
  fprintf(outfile, "else\n");
  
  fprintf(outfile, "\n#endif  /* SAC_DO_MULTITHREAD */\n\n");
  
  
  DBUG_VOID_RETURN;
}

ICM no longer used !!
#endif /* 0 */

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_SETUP(char *name, int narg, char **vararg)
 *
 * description:
 *
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_SETUP( name, narg [, tag, type, arg]*)
 *
 *   the arguments handed over could have the tags:
 *     in, in_rc, out, out_rc, shared and shared_rc.
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
ICMCompileMT_SPMD_SETUP (char *name, int narg, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_SETUP");

#define MT_SPMD_SETUP
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_SETUP

    for (i = 0; i < 3 * narg; i += 3) {
        if ((strcmp (vararg[i], "in") == 0) || (strcmp (vararg[i], "in_rc") == 0)
            || (strcmp (vararg[i], "out") == 0) || (strcmp (vararg[i], "out_rc") == 0)) {
            INDENT;
            fprintf (outfile, "SAC_MT_SPMD_SETARG_%s(%s, %s);\n", vararg[i], name,
                     vararg[i + 2]);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_BEGIN(char *name)
 *   void ICMCompileMT_SPMD_ALTSEQ(char *name)
 *   void ICMCompileMT_SPMD_END(char *name)
 *
 * description:
 *
 *   These functions implement the control flow ICMs around SPMD-blocks.
 *
 *
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

    fprintf (outfile, "\n#if SAC_DO_MULTITHREAD\n");
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

#define MT_SPMD_ALTSEQ
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_ALTSEQ

    INDENT;
    fprintf (outfile, "}\n");
    indent--;
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

    fprintf (outfile, "\n#if SAC_DO_MULTITHREAD\n");
    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "#endif  /* SAC_DO_MULTITHREAD */\n\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_PRESET(char *name, int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_PRESET( name, narg [, tag, arg]*)
 *
 *   This ICM implements a feature that allows to partially build up an spmd
 *   frame in advance, i.e. without immediately starting concurrent execution.
 *   If an spmd block is situated within the body of a sequential loop,
 *   it usually will be executed repeatedly resulting in the repeated building
 *   of its spmd frame. However, some of the parameters may be loop-invariant
 *   which allows to move them out of the loop body.
 *
 *   Tags may be from the set in | in_rc.
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_PRESET (char *name, int narg, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SPMD_PRESET");

#define MT_SPMD_PRESET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_PRESET

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *
 *
 * description:
 *
 *
 *
 *
 *
 ******************************************************************************/

void
ICMCompileMT_ADJUST_SCHEDULER (int current_dim, int array_dim, int lower, int upper,
                               int unrolling, char *array)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_ADJUST_SCHEDULER");

#define MT_ADJUST_SCHEDULER
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_ADJUST_SCHEDULER

    INDENT;
    fprintf (outfile, "SAC_MT_ADJUST_SCHEDULER(%s, %d, %d, %d, %d, (", array, current_dim,
             lower, upper, unrolling);

    if (current_dim == array_dim - 1) {
        fprintf (outfile, "1");
    } else {
        fprintf (outfile, "SAC_ND_KD_A_SHAPE(%s, %d)", array, current_dim + 1);

        for (i = current_dim + 2; i < array_dim; i++) {
            fprintf (outfile, " * SAC_ND_KD_A_SHAPE(%s, %d)", array, current_dim + i);
        }
    }

    fprintf (outfile, "));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_BEGIN(int dim, int *vararg)
 *   void ICMCompileMT_SCHEDULER_END(int dim, int *vararg)
 *
 * description:
 *
 *   These two ICMs implement the default scheduling.
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_BEGIN (int dim, int *varint)
{
    int *lower_bound = varint;
    int *upper_bound = varint + dim;
    int i;

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_BEGIN");

#define MT_SCHEDULER_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_BEGIN

    for (i = 0; i < dim; i++) {
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_START(%d) = %d;\n", i, lower_bound[i]);
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP(%d) = %d;\n", i, upper_bound[i]);
    }

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_END (int dim, int *varint)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_END");

#define MT_SCHEDULER_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_END

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SCHEDULER_Block_BEGIN(int dim, int *vararg)
 *   void ICMCompileMT_SCHEDULER_Block_END(int dim, int *vararg)
 *
 * description:
 *
 *   These two ICMs implement the scheduling for constant segments
 *   called "Block".
 *
 *   This scheduling is a very simple one that partitions the iteration
 *   space along the outermost dimension upon the available processors.
 *   Blocking is not considered!
 *   Unrolling is not considered!
 *
 ******************************************************************************/

void
ICMCompileMT_SCHEDULER_Block_BEGIN (int dim, int *varint)
{
    int *lower_bound = varint;
    int *upper_bound = varint + dim;
    int *unrolling = varint + 3 * dim;
    /*
     * int *blocking    = varint+2*dim;
     */
    int i;

    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Block_BEGIN");

#define MT_SCHEDULER_Block_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_BEGIN

    INDENT;
    fprintf (outfile, "SAC_MT_SCHEDULER_Block_DIM0(%d, %d, %d);\n", lower_bound[0],
             upper_bound[0], unrolling[0]);

    for (i = 1; i < dim; i++) {
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_START(%d) = %d;\n", i, lower_bound[i]);
        INDENT;
        fprintf (outfile, "SAC_WL_MT_SCHEDULE_STOP(%d) = %d;\n", i, upper_bound[i]);
    }

    DBUG_VOID_RETURN;
}

void
ICMCompileMT_SCHEDULER_Block_END (int dim, int *varint)
{
    DBUG_ENTER ("ICMCompileMT_SCHEDULER_Block_END");

#define MT_SCHEDULER_Block_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SCHEDULER_Block_END

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}
