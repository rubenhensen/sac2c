/*
 *
 * $Log$
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
 *   void ICMCompileWL_NONFOLD_BEGIN( char *target, char *idx_vec,
 *                                    int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_NONFOLD_BEGIN( target, idx_vec,
 *                     dims, [ idx_scalars, idx_min, idx_max ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_NONFOLD_BEGIN (char *target, char *idx_vec, int dims, char **args)
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
    fprintf (outfile, "int %s__destptr = 0;\n", target);

    fprintf (outfile, "#if SAC_DO_MULTITHREAD\n");
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_VAR( start0, %s) = %s;\n", args[3 * i],
                 args[3 * i + 1]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_VAR( stop0, %s) = %s;\n", args[3 * i],
                 args[3 * i + 2]);
    }
    fprintf (outfile, "#endif  /* SAC_DO_MULTITHREAD */\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD_BEGIN( char *target, char *idx_vec,
 *                                 int dims, char **args)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD_BEGIN( target, idx_vec,
 *                  dims, [ idx_scalars, idx_min, idx_max ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD_BEGIN (char *target, char *idx_vec, int dims, char **args)
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

    fprintf (outfile, "#if SAC_DO_MULTITHREAD\n");
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_VAR( start0, %s) = %s;\n", args[3 * i],
                 args[3 * i + 1]);
    }
    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_VAR( stop0, %s) = %s;\n", args[3 * i],
                 args[3 * i + 2]);
    }
    fprintf (outfile, "#endif  /* SAC_DO_MULTITHREAD */\n");

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
 *   void ICMCompileWL_FOLD_NOOP( int dim,
 *                                int dims_target, char *target,
 *                                char *idx_vec,
 *                                int dims, char **idx_scalars,
 *                                int cnt_bounds, char **bounds)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_FOLD_NOOP( dim, dims_target, target, idx_vec, dims, [ idx_scalars ]*,
 *                 2, [ bounds ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD_NOOP (int dim, int dims_target, char *target, char *idx_vec, int dims,
                        char **idx_scalars, int cnt_bounds, char **bounds)
{
    DBUG_ENTER ("ICMCompileWL_FOLD_NOOP");

#define WL_FOLD_NOOP
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD_NOOP

    DBUG_ASSERT ((cnt_bounds == 2), "wrong parameter found");

    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD( %s)[ %d] += %s;\n", idx_vec, dim, bounds[1]);
    INDENT;
    fprintf (outfile, "SAC_ND_A_FIELD( %s)[ %d] -= %s;\n", idx_vec, dim, bounds[0]);

#if 0
  INDENT;
  fprintf( outfile, "%s += %d;\n",
                    idx_scalars[ dim], bounds[ 1]);
  INDENT;
  fprintf( outfile, "%s -= %d;\n",
                    idx_scalars[ dim], bounds[ 0]);
#endif

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ADJUST_OFFSET( int dim, int dims_target, char *target,
 *                                    char *idx_vec,
 *                                    int dims, char **idx_scalars)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ADJUST_OFFSET( dim, dims_target, target, idx_vec,
 *                     dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ADJUST_OFFSET (int dim, int dims_target, char *target, char *idx_vec,
                            int dims, char **idx_scalars)
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
        if (i < dim) {
            fprintf (outfile, " + %s )", idx_scalars[i]);
        } else {
            fprintf (outfile, " + SAC_VAR( start, %s) )", idx_scalars[i]);
        }
    }

    for (; i < dims_target; i++) {
        fprintf (outfile, " * SAC_ND_KD_A_SHAPE( %s, %d)", target, i);
    }
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}
