/*
 *
 * $Log$
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

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"

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
 *   Tags may be from the set in | out | in_rc | out_rc | inout.
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

    fprintf (outfile, "unsigned int %s SAC_MT_SPMD_FUN_REALARGS()\n", name);
    fprintf (outfile, "{\n");

    for (i = 0; i < narg; i += 3) {
        fprintf (outfile, " SAC_MT_SPMD_PARAM_%s( %s, %s)\n", vararg[i], vararg[i + 1],
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
 *   MT_SYNC_FOLD( narg [, type, accu_var, tmp_var, foldop]*)
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
    int i;

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
 *   void ICMCompileMT_SYNC_FOLD_NONFOLD(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_SYNC_FOLD_NONFOLD( narg [, type, accu_var, tmp_var, foldop]*)
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
    int i;

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
 *   void ICMCompileMT_CONTINUE(int narg, char **vararg)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   MT_CONTINUE( narg [, type, accu_var]*)
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
    int i;

    DBUG_ENTER ("ICMCompileMT_CONTINUE");

#define MT_CONTINUE
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_CONTINUE

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
 *   Tags may be from the set in | out | in_rc | out_rc | inout<n> | pre.
 *   'pre' specifies an spmd argument that is preset by using the ICM
 *   MT_SPMD_PRESET().
 *
 ******************************************************************************/

void
ICMCompileMT_SPMD_BLOCK (char *name, int narg, char **vararg)
{
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_BLOCK");

#define MT_SPMD_BLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_BLOCK

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
    int i;

    DBUG_ENTER ("ICMCompileMT_SPMD_PRESET");

#define MT_SPMD_PRESET
#include "icm_comment.c"
#include "icm_trace.c"
#undef MT_SPMD_PRESET

    DBUG_VOID_RETURN;
}
