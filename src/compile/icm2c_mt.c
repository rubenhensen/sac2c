/*
 *
 * $Log$
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
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#endif                /* BEtest */

/******************************************************************************
 *
 * global variable:  int barrier_id
 *
 * description:
 *
 *   An unambigious barrier identification is required because of the use of
 *   labels in the barrier implementation and the fact that several
 *   synchronisation blocks may be found within a single spmd block which
 *   means that several synchronisation barriers are situated in one function.
 *
 ******************************************************************************/

static int barrier_id = 0;

/******************************************************************************
 *
 * function:
 *   node *SearchFoldImplementation(char *foldop)
 *
 * description:
 *
 *   This function traverses the fundef chain of the syntax tree in order to
 *   find the implementation of the given fold operation. A pointer to the
 *   respective code block is returned.
 *
 ******************************************************************************/

#ifndef BEtest
static node *
SearchFoldImplementation (char *foldop)
{
    node *fundef;

    DBUG_ENTER ("SearchFoldImplementation");

    fundef = MODUL_FUNS (syntax_tree);

    while ((fundef != NULL) && (0 != strcmp (FUNDEF_NAME (fundef), foldop))) {
        fundef = FUNDEF_NEXT (fundef);
    }

    DBUG_ASSERT ((fundef != NULL),
                 ("Unknown fold operation specified in sunchronisation ICM"));

    DBUG_RETURN (FUNDEF_BODY (fundef));
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

    fprintf (outfile, "#undef SAC_MT_CURRENT_FUN()\n");
    fprintf (outfile, "#define SAC_MT_CURRENT_FUN() %s\n", from);

    fprintf (outfile, "\n");

    fprintf (outfile, "#undef SAC_MT_CURRENT_SPMD()\n");
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

    for (i = 0; i < 3 * narg; i += 3) {
        INDENT;
        fprintf (outfile, "SAC_MT_SPMD_PARAM_%s( %s, %s)\n", vararg[i], vararg[i + 1],
                 vararg[i + 2]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SPMD_FUN_RET(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SPMD_FUN_RET( narg [, tag, param]*)
 *
 *   This ICM implements the return statement of an spmd-function,
 *   i.e. it has to realize the return of several out parameters.
 *
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_FUN_RET (int narg, char **vararg)
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
 *   void ICMCompileMT_START_SYNCBLOCK(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_START_SYNCBLOCK(narg [, tag, type, param]*)
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
ICMCompileMT_START_SYNCBLOCK (int narg, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_START_SYNCBLOCK");

#define MT_START_SYNCBLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_START_SYNCBLOCK

    barrier_id++;

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    for (i = 0; i < 3 * narg; i += 3) {
        if (0 == strcmp (vararg[i], "in_rc")) {
            INDENT;
            fprintf (outfile, "int SAC_ND_A_RC(%s) = &SAC_dummy_refcount;\n",
                     vararg[i + 2]);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_FOLD(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD( narg [, foldtype, accu_var, tmp_var, foldop]*)
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
ICMCompileMT_SYNC_FOLD (int narg, char **vararg)
{
    DBUG_ENTER ("ICMCompileMT_SYNC_FOLD");

#define MT_SYNC_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SYNC_FOLD

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileMT_SYNC_ONEFOLD(char *foldtype, char *accu_var,
 *                                  char *tmp_var, char *foldop)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_ONEFOLD( foldtype, accu_var, tmp_var, foldop)
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
ICMCompileMT_SYNC_ONEFOLD (char *foldtype, char *accu_var, char *tmp_var, char *foldop)
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
    Trav (fold_code, NULL);
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_2( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

#ifdef BEtest
    INDENT;
    fprintf (outfile, "/* fold operation: %s */\n", foldop);
#else  /* BEtest */
    Trav (fold_code, NULL);
#endif /* BEtest */

    INDENT;
    fprintf (outfile, "SAC_MT_SYNC_ONEFOLD_3( %s, %s, %s, %d)\n", foldtype, accu_var,
             tmp_var, barrier_id);

    indent--;

    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, "{\n");

    indent++;

    INDENT;
    fprintf (outfile, "label_master_continue_%d:\n", barrier_id);

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
 *   MT_SYNC_NONFOLD( )
 *
 *   This ICM implements barrier synchronisation for synchronisation blocks
 *   that contain exclusively modarray/genarray with-loops.
 *
 ******************************************************************************/

void
ICMCompileMT_SYNC_NONFOLD ()
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

    INDENT;
    fprintf (outfile, "{\n");

    indent++;

    INDENT;
    fprintf (outfile, "label_master_continue_%d:\n", barrier_id);

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
 *   void ICMCompileMT_CONTINUE(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_CONTINUE( narg [, foldtype, accu_var]*)
 *
 *   This ICM implements the continuation after a barrier synchronisation.
 *   It restarts the synchronized worker threads and updates their current
 *   values of fold results from the preceding synchronisation block.
 *   The variable argument part consists of pairs, that specify the type and
 *   accumulation variable of these fold operations.
 *
 ******************************************************************************/

void
ICMCompileMT_CONTINUE (int narg, char **vararg)
{
    int i, j;

    DBUG_ENTER ("ICMCompileMT_CONTINUE");

#define MT_CONTINUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_CONTINUE

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

    for (i = 0, j = 1; i < 2 * narg; i += 2, j++) {
        INDENT;
        fprintf (outfile, "%s = SAC_MT_GET_BARRIER_RESULT(0, %d, %s);\n", vararg[i + 1],
                 j, vararg[i]);
    }

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, "label_continue_%d:\n", barrier_id);

    DBUG_VOID_RETURN;
}

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

void
ICMCompileMT_SPMD_BLOCK (char *name, int narg, char **vararg)
{
    int i;
    static char basetype[32];

    DBUG_ENTER ("ICMCompileMT_SPMD_BLOCK");

#define MT_SPMD_BLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BLOCK

    fprintf (outfile, "\n");

    fprintf (outfile, "#if SAC_DO_MULTITHREAD\n\n");

    INDENT;
    fprintf (outfile, "if (SAC_MT_not_yet_parallel)\n");

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    INDENT;
    fprintf (outfile, "SAC_MT_not_yet_parallel=0\n");

    for (i = 0; i < 3 * narg; i += 3) {
        if (0 == strncmp (vararg[i], "inout_rc", 8)) {

            strncpy (basetype, vararg[i + 1],
                     strchr (vararg[i + 1], '*') - vararg[i + 1]);

            INDENT;
            fprintf (outfile, "SAC_ND_ALLOC_ARRAY(%s, %s, %s);\n", basetype,
                     vararg[i + 2], vararg[i] + 8);

            INDENT;
            fprintf (outfile, "SAC_MT_SPMD_SETARG_inout_rc(%s, %s);\n", name,
                     vararg[i + 2]);
        } else {
            INDENT;
            fprintf (outfile, "SAC_MT_SPMD_SETARG_%s(%s, %s);\n", vararg[i], name,
                     vararg[i + 2]);
        }
    }

    INDENT;
    fprintf (outfile, "\n");

    INDENT;
    fprintf (outfile, "SAC_MT_START_SPMD(%s);\n", name);

    INDENT;
    fprintf (outfile, "SAC_MT_not_yet_parallel=1\n");

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    INDENT;
    fprintf (outfile, "else\n");

    fprintf (outfile, "\n#endif  /* SAC_DO_MULTITHREAD */\n\n");

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
