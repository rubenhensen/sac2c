/*
 *
 * $Log$
 * Revision 3.6  2001/01/22 13:48:45  dkr
 * bug in WL icms fixed
 *
 * Revision 3.5  2001/01/19 11:54:10  dkr
 * some with-loop ICMs renamed
 *
 * Revision 3.3  2001/01/10 18:33:30  dkr
 * icm WL_ADJUST_OFFSET renamed into WL_SET_OFFSET
 *
 * Revision 3.2  2001/01/09 20:01:25  dkr
 * comment modified
 *
 * Revision 3.1  2000/11/20 18:01:21  sacbase
 * new release made
 *
 * Revision 2.10  2000/07/28 11:43:37  cg
 * Added new ICM WL_ASSIGN_NOOP for efficient handling of dummy
 * iteration space segments introduced through array padding.
 *
 * Revision 2.9  2000/07/06 16:41:38  dkr
 * macro SAC_WL_DEST used
 *
 * Revision 2.8  2000/07/06 13:11:58  dkr
 * minor changes in comments done
 *
 * Revision 2.7  2000/03/20 19:29:26  dkr
 * some comments added
 *
 * Revision 2.6  2000/03/10 10:33:41  dkr
 * output format of ICM WL_INIT_OFFSET changed
 *
 * Revision 2.5  1999/11/16 15:23:05  dkr
 * comments added
 *
 * Revision 2.4  1999/07/21 12:28:17  jhs
 * Improved indenting.
 *
 * Revision 2.3  1999/05/12 16:38:22  cg
 * include main.h removed
 *
 * Revision 2.2  1999/04/12 09:37:48  cg
 * All accesses to C arrays are now performed through the new ICMs
 * ND_WRITE_ARRAY and ND_READ_ARRAY. This allows for an integration
 * of cache simulation as well as boundary checking.
 *
 * Revision 2.1  1999/02/23 12:42:43  sacbase
 * new release made
 *
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
 */

#include <malloc.h>
#include <stdio.h>

#include "icm2c_basic.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

/******************************************************************************
 *
 * Function:
 *   void PrintTraceICM( char *target, int dims, char **idx_scalars,
 *                       char *operation, bool print_offset)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintTraceICM (char *target, int dims, char **idx_scalars, char *operation,
               bool print_offset)
{
    int i;

    DBUG_ENTER ("PrintTraceICM");

    INDENT;
    fprintf (outfile, "SAC_TR_WL_PRINT((\"index vector [%%d");
    for (i = 1; i < dims; i++) {
        fprintf (outfile, ", %%d");
    }
    fprintf (outfile, "]");
    if (print_offset) {
        fprintf (outfile, " -- offset %%d");
    }
    fprintf (outfile, " -- %s", operation);
    fprintf (outfile, "\", %s", idx_scalars[0]);
    for (i = 1; i < dims; i++) {
        fprintf (outfile, ", %s", idx_scalars[i]);
    }
    if (print_offset) {
        fprintf (outfile, ", SAC_WL_OFFSET( %s)", target);
    }
    fprintf (outfile, "));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrintShapeFactor( int current_dim, int target_dim, char *target)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintShapeFactor (int current_dim, int target_dim, char *target)
{
    int j;

    DBUG_ENTER ("PrintShapeFactor");

    for (j = current_dim + 1; j < target_dim; j++) {
        fprintf (outfile, " * SAC_ND_A_SHAPE( %s, %d)", target, j);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_BEGIN__OFFSET( char *target, char *idx_vec, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_BEGIN__OFFSET( target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_BEGIN__OFFSET (char *target, char *idx_vec, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_BEGIN__OFFSET");

#define WL_BEGIN__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_BEGIN__OFFSET

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    INDENT;
    fprintf (outfile, "int SAC_WL_OFFSET( %s);\n", target);

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_START( %d);\n", i);
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_STOP( %d);\n", i);
    }

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_BEGIN( char *target, char *idx_vec, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_BEGIN(target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_BEGIN (char *target, char *idx_vec, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_BEGIN");

#define WL_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_BEGIN

    INDENT;
    fprintf (outfile, "{\n");
    indent++;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_START( %d);\n", i);
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_STOP( %d);\n", i);
    }

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_END__OFFSET( char *target, char *idx_vec, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_END__OFFSET( target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_END__OFFSET (char *target, char *idx_vec, int dims)
{
    DBUG_ENTER ("ICMCompileWL_END__OFFSET");

#define WL_END__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_END__OFFSET

    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_END( char *target, char *idx_vec, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_END(target, idx_vec, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_END (char *target, char *idx_vec, int dims)
{
    DBUG_ENTER ("ICMCompileWL_END");

#define WL_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_END

    indent--;
    INDENT;
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
 *   Implements the compilation of the following ICM:
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

    PrintTraceICM (idx_vec, dims, idx_scalars, "assign", TRUE);

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
                 "SAC_ND_WRITE_ARRAY( %s, SAC_WL_OFFSET( %s)) = "
                 "SAC_ND_READ_ARRAY( %s, SAC_i);\n",
                 target, target, expr);
        INDENT;
        fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);

        indent--;
        INDENT;
        fprintf (outfile, "}\n");

        indent--;
        INDENT;
        fprintf (outfile, "}\n");
    } else {
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, SAC_WL_OFFSET( %s)) = %s;\n", target,
                 target, expr);
        INDENT;
        fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN__INIT( int dims_target, char *target,
 *                                   char *idx_vec,
 *                                   int dims, char **idx_scalars)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN__INIT( dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN__INIT (int dims_target, char *target, char *idx_vec, int dims,
                           char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_ASSIGN__INIT");

#define WL_ASSIGN__INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN__INIT

    PrintTraceICM (idx_vec, dims, idx_scalars, "init", TRUE);

    if (dims_target > dims) {
        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SHAPE( %s, %d)", target,
                 dims);
        for (i = dims + 1; i < dims_target; i++) {
            fprintf (outfile, " * SAC_ND_A_SHAPE( %s, %d)", target, i);
        }
        fprintf (outfile, "; SAC_i++) {\n");
        indent++;
    }

    INDENT;
    fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, SAC_WL_OFFSET( %s)) = 0;\n", target,
             target);

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);

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
 *   void ICMCompileWL_ASSIGN__COPY( char *source,
 *                                   int dims_target, char *target,
 *                                   char *idx_vec,
 *                                   int dims, char **idx_scalars)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN__COPY( source,
 *                    dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN__COPY (char *source, int dims_target, char *target, char *idx_vec,
                           int dims, char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_ASSIGN__COPY");

#define WL_ASSIGN__COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN__COPY

    PrintTraceICM (idx_vec, dims, idx_scalars, "copy", TRUE);

    if (dims_target > dims) {
        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SHAPE( %s, %d)", target,
                 dims);
        for (i = dims + 1; i < dims_target; i++) {
            fprintf (outfile, " * SAC_ND_A_SHAPE( %s, %d)", target, i);
        }
        fprintf (outfile, "; SAC_i++) {\n");
        indent++;
    }

    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_WL_OFFSET( %s)) = "
             "SAC_ND_READ_ARRAY( %s, SAC_WL_OFFSET( %s));\n",
             target, target, source, target);

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);

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
 *   void ICMCompileWL_FOLD( int dims_target, char *target,
 *                           char *idx_vec,
 *                           int dims, char **idx_scalars)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD( source, dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD (int dims_target, char *target, char *idx_vec, int dims,
                   char **idx_scalars)
{
    DBUG_ENTER ("ICMCompileWL_FOLD");

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

    PrintTraceICM (idx_vec, dims, idx_scalars, "fold", FALSE);

    INDENT;
    fprintf (outfile, "/* fold operation */\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD__OFFSET( int dims_target, char *target,
 *                                   char *idx_vec,
 *                                   int dims, char **idx_scalars)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD__OFFSET( source,
 *                    dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD__OFFSET (int dims_target, char *target, char *idx_vec, int dims,
                           char **idx_scalars)
{
    DBUG_ENTER ("ICMCompileWL_FOLD__OFFSET");

#define WL_FOLD__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD__OFFSET

    PrintTraceICM (idx_vec, dims, idx_scalars, "fold", TRUE);

    INDENT;
    fprintf (outfile, "/* fold operation */\n");
    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_NOOP__OFFSET( int dims_target, char *target,
 *                                   char *idx_vec,
 *                                   int dims, char **idx_scalars,
 *                                   int cnt_bounds, char **bounds)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_NOOP__OFFSET( dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_NOOP__OFFSET (int dims_target, char *target, char *idx_vec, int dims,
                           char **idx_scalars)
{
    DBUG_ENTER ("ICMCompileWL_NOOP__OFFSET");

#define WL_NOOP__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_NOOP__OFFSET

    INDENT;
    fprintf (outfile, "/* noop */\n");
    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_NOOP( int dims_target, char *target,
 *                           char *idx_vec,
 *                           int dims, char **idx_scalars,
 *                           int cnt_bounds, char **bounds)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_NOOP( dims_target, target, idx_vec, dims, [ idx_scalars ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_NOOP (int dims_target, char *target, char *idx_vec, int dims,
                   char **idx_scalars)
{
    DBUG_ENTER ("ICMCompileWL_NOOP");

#define WL_NOOP
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_NOOP

    INDENT;
    fprintf (outfile, "/* noop */\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_INIT_OFFSET( int dims_target, char *target,
 *                                  char *idx_vec, int dims_wl)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *     WL_INIT_OFFSET( dims_target, target, idx_vec, dims_wl)
 *
 *   The SAC_WL_OFFSET() of the WL-array is initialized, i.e. set to the index
 *   of the first WL element.
 *
 * remark:
 *   The names of the variables WL_MT_SCHEDULE_START, WL_MT_SCHEDULE_STOP
 *   are a bit misleading. These variables are not only used in MT-mode but
 *   in ST-mode, too!
 *   In ST-mode WL_MT_SCHEDULE_START(i) and WL_MT_SCHEDULE_STOP(i) contain the
 *   smallest and greatest WL-index, respectively, of the i-th dimension.
 *
 ******************************************************************************/

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
    fprintf (outfile, "SAC_WL_OFFSET( %s) = ", target);
    indent++;

    fprintf (outfile, "SAC_WL_MT_SCHEDULE_START( 0)");

    PrintShapeFactor (0, dims_target, target);

    for (i = 1; i < dims_wl; i++) {
        fprintf (outfile, "\n");
        INDENT;
        fprintf (outfile, "+ SAC_WL_MT_SCHEDULE_START( %d)", i);

        PrintShapeFactor (i, dims_target, target);
    }

    fprintf (outfile, ";\n");
    indent--;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SET_OFFSET( int dim, int first_block_dim,
 *                                 int dims_target, char *target,
 *                                 char *idx_vec,
 *                                 int dims, char **idx_scalars)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_SET_OFFSET( dim, first_block_dim, dims_target, target, idx_vec,
 *                  dims, [ idx_scalars ]* )
 *
 * remark:
 *   This ICM is needed (and usefull) in combination with multiple segments,
 *   (unrolling-)blocking or naive compilation only!!
 *   If the C compiler reports an undeclared 'SAC__start...' then there is
 *   an error in compile.c:
 *   Either a WL_(U)BLOCK_LOOP_BEGIN ICM is missing, or the WL_SET_OFFSET
 *   ICM is obsolete (probably a bug in the inference function COMPWLgrid...).
 *
 *   The names of the variables WL_MT_SCHEDULE_START, WL_MT_SCHEDULE_STOP
 *   are a bit misleading. These variables are not only used in MT-mode but
 *   in ST-mode, too!
 *   In ST-mode WL_MT_SCHEDULE_START(i) and WL_MT_SCHEDULE_STOP(i) contain the
 *   smallest and greatest WL-index, respectively, of the i-th dimension.
 *
 ******************************************************************************/

void
ICMCompileWL_SET_OFFSET (int dim, int first_block_dim, int dims_target, char *target,
                         char *idx_vec, int dims, char **idx_scalars)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_SET_OFFSET");

#define WL_SET_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SET_OFFSET

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s) = ", target);

    for (i = dims - 1; i > 0; i--) {
        fprintf (outfile, "( SAC_ND_A_SHAPE( %s, %d) * ", target, i);
    }
    fprintf (outfile, "%s", idx_scalars[0]);
    for (i = 1; i < dims; i++) {
        if (i <= dim) {
            fprintf (outfile, " + %s )", idx_scalars[i]);
        } else {
            if (i <= first_block_dim) {
                /*
                 * no blocking in this dimension
                 *  -> we use the start index of the current MT region
                 */
                fprintf (outfile, " + SAC_WL_MT_SCHEDULE_START( %d) )", i);
            } else {
                /*
                 * blocking in this dimension
                 *  -> we use the first index of the current block
                 */
                fprintf (outfile, " + SAC_WL_VAR( first, %s) )", idx_scalars[i]);
            }
        }
    }

    for (; i < dims_target; i++) {
        fprintf (outfile, " * SAC_ND_A_SHAPE( %s, %d)", target, i);
    }
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}
