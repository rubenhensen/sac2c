/*
 *
 * $Log$
 * Revision 1.26  1998/10/29 20:38:26  dkr
 * - signature of ICM WL_FOLD_NOOP changed
 * - WL_FOLD_NOOP is empty now
 *
 * Revision 1.25  1998/08/07 19:47:30  dkr
 * WL_ADJUST_OFFSET changed
 *
 * Revision 1.24  1998/08/07 16:01:58  dkr
 * WL_MT_...FOLD_BEGIN, WL_MT_...FOLD_END removed
 * WL_MT_SCHEDULER_SET_OFFSET renamed to WL_INIT_OFFSET
 * WL_ADJUST_OFFSET changed
 *
 * Revision 1.23  1998/08/07 12:38:08  dkr
 * WL_ADJUST_OFFSET changed
 *
 * Revision 1.22  1998/08/06 01:15:40  dkr
 * Fixed a bug in WL_ADJUST_OFFSET
 *
 * Revision 1.21  1998/07/02 09:26:16  cg
 * tracing capabilities improved
 *
 * Revision 1.20  1998/06/29 08:55:51  cg
 * added new multi-threaded versions of new with-loop begin/end ICMs
 * added compilation of ICM WL_MT_SCHEDULER_SET_OFFSET
 *
 * Revision 1.19  1998/06/24 10:37:07  dkr
 * WL_(NON)FOLD_BEGIN/END are now h-icms
 *
 * Revision 1.18  1998/06/19 18:29:46  dkr
 * added WL_NONFOLD_END, WL_FOLD_END
 *
 * Revision 1.17  1998/06/19 10:26:12  dkr
 * removed unused vars
 *
 * Revision 1.16  1998/06/09 16:46:36  dkr
 * changed signature of WL_NONFOLD_BEGIN, WL_FOLD_BEGIN
 *
 * Revision 1.15  1998/06/06 15:59:22  dkr
 * renamed some local vars in ICMs
 *
 * Revision 1.14  1998/05/24 12:17:14  dkr
 * optimized macros for non-MT (if SAC_DO_MULTITHREAD not defined)
 *
 * Revision 1.12  1998/05/19 15:42:18  dkr
 * ICM for fold changed
 *
 * Revision 1.11  1998/05/16 16:38:26  dkr
 * WL_END is now a h-icm
 *
 * Revision 1.10  1998/05/16 00:09:24  dkr
 * changed some macros
 * added PRINT_INDEX_CODE, PRINT_INDEX macros, to log the values of the
 * index-vector
 *
 * Revision 1.9  1998/05/14 21:37:55  dkr
 * changed some ICMs
 *
 * Revision 1.8  1998/05/12 22:29:57  dkr
 * added new macros for fold
 *
 * Revision 1.7  1998/05/12 18:49:56  dkr
 * changed some ICMs
 *
 * Revision 1.6  1998/05/07 16:20:43  dkr
 * changed signature of ICMs
 *
 * Revision 1.5  1998/05/07 08:10:02  cg
 * C implemented ICMs converted to new naming conventions.
 *
 * Revision 1.4  1998/05/06 14:49:08  dkr
 * changed WL_ASSIGN
 *
 * Revision 1.3  1998/05/04 15:35:29  dkr
 * added WL_ASSIGN
 *
 * Revision 1.2  1998/05/04 09:36:19  dkr
 * changed WL_BEGIN
 *
 * Revision 1.1  1998/05/03 14:06:09  dkr
 * Initial revision
 *
 *
 *
 */

#include <malloc.h>
#include <stdio.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "main.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

#define PRINT_INDEX 0

#define PRINT_INDEX_CODE                                                                 \
    {                                                                                    \
        int i;                                                                           \
        fprintf (outfile, "fprintf( SACstderr, \"");                                     \
        for (i = 0; i < dims; i++) {                                                     \
            fprintf (outfile, "idx%d=%%d ", i);                                          \
        }                                                                                \
        fprintf (outfile, "\\n\"");                                                      \
        for (i = 0; i < dims; i++) {                                                     \
            fprintf (outfile, ", %s", idx_scalars[i]);                                   \
        }                                                                                \
        fprintf (outfile, ");\n");                                                       \
    }

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_NONFOLD_BEGIN( char *target, char *idx_vec, int dims)
 *
 * description:
 *
 *   implements the compilation of the following ICM:
 *
 *   WL_NONFOLD_BEGIN(target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_NONFOLD_BEGIN (char *target, char *idx_vec, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_NONFOLD_BEGIN");

#define WL_NONFOLD_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_NONFOLD_BEGIN

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    INDENT;
    fprintf (outfile, "int %s__destptr;\n", target);

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_START(%d);\n", i);
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_STOP(%d);\n", i);
    }

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD_BEGIN( char *target, char *idx_vec, int dims)
 *
 * description:
 *
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD_BEGIN(target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD_BEGIN (char *target, char *idx_vec, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_FOLD_BEGIN");

#define WL_FOLD_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD_BEGIN

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_START(%d);\n", i);
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_STOP(%d);\n", i);
    }

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_NONFOLD_END( char *target, char *idx_vec, int dims)
 *
 * description:
 *
 *   implements the compilation of the following ICM:
 *
 *   WL_NONFOLD_END(target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_NONFOLD_END (char *target, char *idx_vec, int dims)
{
    DBUG_ENTER ("ICMCompileWL_NONFOLD_END");

#define WL_NONFOLD_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_NONFOLD_END

    indent--;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD_END( char *target, char *idx_vec, int dims)
 *
 * description:
 *
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD_END(target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD_END (char *target, char *idx_vec, int dims)
{
    DBUG_ENTER ("ICMCompileWL_FOLD_END");

#define WL_FOLD_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD_END

    indent--;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN( int dims_expr, char *expr,
 *                             int dims_target, char *target,
 *                             char *idx_vec,
 *                             int dims, char **idx_scalars)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( target, dims_expr, expr, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (int dims_expr, char *expr, int dims_target, char *target,
                     char *idx_vec, int dims, char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    if (dims_expr > 0) {

        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++) {\n",
                 expr);
        indent++;

        INDENT;
        fprintf (outfile,
                 "SAC_ND_A_FIELD( %s)[ %s__destptr++] = "
                 "SAC_ND_A_FIELD( %s)[ SAC_i];\n",
                 target, target, expr);

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

    } else {

        INDENT;
        fprintf (outfile, "SAC_TR_WL_PRINT((\"Array element %s[[%%d", target);
        for (i = 1; i < dims; i++) {
            fprintf (outfile, ", %%d");
        }
        fprintf (outfile, "]] at offset %%d set.\", %s", idx_scalars[0]);
        for (i = 1; i < dims; i++) {
            fprintf (outfile, ", %s", idx_scalars[i]);
        }
        fprintf (outfile, ", %s__destptr));\n", target);

        INDENT;
        fprintf (outfile, "SAC_ND_A_FIELD( %s)[ %s__destptr++] = %s;\n", target, target,
                 expr);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN_INIT( int dims_target, char *target,
 *                                  char *idx_vec,
 *                                  int dims, char **idx_scalars)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN_INIT( dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN_INIT (int dims_target, char *target, char *idx_vec, int dims,
                          char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_ASSIGN_INIT");

#define WL_ASSIGN_INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN_INIT

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    if (dims_target > dims) {

        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_KD_A_SHAPE( %s, %d)", target,
                 dims);
        for (i = dims + 1; i < dims_target; i++) {
            fprintf (outfile, " * SAC_ND_KD_A_SHAPE( %s, %d)", target, i);
        }
        fprintf (outfile, "; SAC_i++) {\n");
        indent++;
    }

    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD( %s)[ %s__destptr++] = 0;\n", target, target);

    if (dims_target > dims) {

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN_COPY( char *source,
 *                                  int dims_target, char *target,
 *                                  char *idx_vec,
 *                                  int dims, char **idx_scalars)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ASSIGN_COPY( source,
 *                   dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN_COPY (char *source, int dims_target, char *target, char *idx_vec,
                          int dims, char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_ASSIGN_COPY");

#define WL_ASSIGN_COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN_COPY

#if PRINT_INDEX
    PRINT_INDEX_CODE
#endif

    if (dims_target > dims) {

        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_KD_A_SHAPE( %s, %d)", target,
                 dims);
        for (i = dims + 1; i < dims_target; i++) {
            fprintf (outfile, " * SAC_ND_KD_A_SHAPE( %s, %d)", target, i);
        }
        fprintf (outfile, "; SAC_i++) {\n");
        indent++;
    }

    INDENT;
    fprintf (outfile,
             "SAC_ND_A_FIELD( %s)[ %s__destptr] = "
             "SAC_ND_A_FIELD( %s)[ %s__destptr];\n",
             target, target, source, target);

    INDENT;
    fprintf (outfile, "%s__destptr++;\n", target);

    if (dims_target > dims) {

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD_NOOP( int dims_target, char *target,
 *                                char *idx_vec,
 *                                int dims, char **idx_scalars,
 *                                int cnt_bounds, char **bounds)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD_NOOP( dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD_NOOP (int dims_target, char *target, char *idx_vec, int dims,
                        char **idx_scalars)
{
    DBUG_ENTER ("ICMCompileWL_FOLD_NOOP");

#define WL_FOLD_NOOP
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD_NOOP

    INDENT;
    fprintf (outfile, "/* empty */\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_INIT_OFFSET( int dims_target, char *target,
 *                                  char *idx_vec, int dims_wl)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_INIT_OFFSET( dims_target, target, idx_vec, dims_wl)
 *
 ******************************************************************************/

static void
PrintShapeFactor (int current_dim, int target_dim, char *target)
{
    int j;

    DBUG_ENTER ("PrintShapeFactor");

    indent += 2;

    switch (target_dim - current_dim) {
    case 0:
    case 1:
        fprintf (outfile, "\n");
        break;
    case 2:
        fprintf (outfile, " * SAC_ND_KD_A_SHAPE(%s, %d)\n", target, target_dim - 1);
        break;
    default:
        fprintf (outfile, "\n");
        INDENT;
        fprintf (outfile, "* ( SAC_ND_KD_A_SHAPE(%s, %d)\n", target, current_dim + 1);
        indent += 2;

        for (j = current_dim + 2; j < target_dim; j++) {
            INDENT;
            fprintf (outfile, "* SAC_ND_KD_A_SHAPE(%s, %d)\n", target, j);
        }

        indent--;
        INDENT;
        fprintf (outfile, ")\n");

        indent--;
    }

    indent -= 2;

    DBUG_VOID_RETURN;
}

void
ICMCompileWL_INIT_OFFSET (int dims_target, char *target, char *idx_vec, int dims_wl)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_INIT_OFFSET");

#define WL_INIT_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_INIT_OFFSET

    INDENT;
    fprintf (outfile, "%s__destptr = \n", target);
    indent++;

    INDENT;
    fprintf (outfile, "SAC_WL_MT_SCHEDULE_START(0)");

    PrintShapeFactor (0, dims_target, target);

    for (i = 1; i < dims_wl; i++) {
        INDENT;
        fprintf (outfile, "+ SAC_WL_MT_SCHEDULE_START(%d)", i);

        PrintShapeFactor (i, dims_target, target);
    }

    INDENT;
    fprintf (outfile, ";\n");
    indent--;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ADJUST_OFFSET( int dim, int first_block_dim,
 *                                    int dims_target, char *target,
 *                                    char *idx_vec,
 *                                    int dims, char **idx_scalars)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ADJUST_OFFSET( dim, first_block_dim, dims_target, target, idx_vec,
 *                     dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ADJUST_OFFSET (int dim, int first_block_dim, int dims_target, char *target,
                            char *idx_vec, int dims, char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_ADJUST_OFFSET");

#define WL_ADJUST_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ADJUST_OFFSET

    INDENT;
    fprintf (outfile, "%s__destptr = ", target);

    for (i = dims - 1; i > 0; i--) {
        fprintf (outfile, "( SAC_ND_KD_A_SHAPE( %s, %d) * ", target, i);
    }
    fprintf (outfile, "%s", idx_scalars[0]);
    for (i = 1; i < dims; i++) {
        if (i <= dim) {
            fprintf (outfile, " + %s )", idx_scalars[i]);
        } else {
            if (i <= first_block_dim) {
                fprintf (outfile, " + SAC_WL_MT_SCHEDULE_START( %d) )", i);
            } else {
                fprintf (outfile, " + SAC_WL_VAR( start, %s) )", idx_scalars[i]);
            }
        }
    }

    for (; i < dims_target; i++) {
        fprintf (outfile, " * SAC_ND_KD_A_SHAPE( %s, %d)", target, i);
    }
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}
