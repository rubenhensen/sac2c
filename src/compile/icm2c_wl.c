/*
 *
 * $Log$
 * Revision 3.13  2002/07/10 19:24:43  dkr
 * no changes done
 *
 * Revision 3.12  2001/05/18 09:58:03  cg
 * #include <malloc.h> removed.
 *
 * Revision 3.11  2001/02/06 01:43:55  dkr
 * WL_NOOP_... replaced by WL_ADJUST_OFFSET
 *
 * Revision 3.10  2001/01/30 12:21:58  dkr
 * PrintTraceICM() modified
 * implementation of ICMs WL_NOOP, WL_NOOP__OFFSET modified
 *
 * Revision 3.9  2001/01/25 12:08:16  dkr
 * layout of ICMs WL_SET_OFFSET and WL_INIT_OFFSET modified.
 *
 * Revision 3.8  2001/01/25 09:42:54  dkr
 * PrintShapeFactor() used wherever possible
 *
 * Revision 3.7  2001/01/22 15:55:24  dkr
 * PrintTraceICM modified
 *
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
 * [ eliminated ]
 *
 * Revision 1.1  1998/05/03 14:06:09  dkr
 * Initial revision
 *
 */

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
 *   void PrintTraceICM( char *target, char *idx_vec,
 *                       int dims, char **idxs_nt,
 *                       char *operation, bool print_offset)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintTraceICM (char *target, char *idx_vec, int dims, char **idxs_nt, char *operation,
               bool print_offset)
{
    int i;

    DBUG_ENTER ("PrintTraceICM");

    INDENT;
    fprintf (outfile, "SAC_TR_WL_PRINT( (\"index vector [%%d");
    for (i = 1; i < dims; i++) {
        fprintf (outfile, ", %%d");
    }
    fprintf (outfile, "]");
    if (print_offset) {
        fprintf (outfile, " (== offset %%d) -- offset %%d");
    }
    fprintf (outfile, " -- %s", operation);
    fprintf (outfile, "\", %s", idxs_nt[0]);
    for (i = 1; i < dims; i++) {
        fprintf (outfile, ", %s", idxs_nt[i]);
    }
    if (print_offset) {
        fprintf (outfile, ", ");
        for (i = dims - 1; i > 0; i--) {
            fprintf (outfile, "( SAC_ND_A_SHAPE( %s, %d) * ", target, i);
        }
        fprintf (outfile, "%s ", idxs_nt[0]);
        for (i = 1; i < dims; i++) {
            fprintf (outfile, "+ %s )", idxs_nt[i]);
        }
        fprintf (outfile, ", SAC_WL_OFFSET( %s)", target);
    }
    fprintf (outfile, "));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrintShapeFactor( int current_dim, int dims_target, char *target)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintShapeFactor (int current_dim, int dims_target, char *target)
{
    int j;

    DBUG_ENTER ("PrintShapeFactor");

    for (j = current_dim + 1; j < dims_target; j++) {
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
 *                             int dims, char **idxs_nt)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( target, dims_expr, expr, idx_vec, dims, [ idxs_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (int dims_expr, char *expr, int dims_target, char *target,
                     char *idx_vec, int dims, char **idxs_nt)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

    PrintTraceICM (target, idx_vec, dims, idxs_nt, "assign", TRUE);

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
 *                                   int dims, char **idxs_nt)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN__INIT( dims_target, target, idx_vec, dims, [ idxs_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN__INIT (int dims_target, char *target, char *idx_vec, int dims,
                           char **idxs_nt)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN__INIT");

#define WL_ASSIGN__INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN__INIT

    PrintTraceICM (target, idx_vec, dims, idxs_nt, "init", TRUE);

    if (dims_target > dims) {
        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SHAPE( %s, %d)", target,
                 dims);
        PrintShapeFactor (dims, dims_target, target);
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
 *                                   int dims, char **idxs_nt)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN__COPY( source,
 *                    dims_target, target, idx_vec, dims, [ idxs_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN__COPY (char *source, int dims_target, char *target, char *idx_vec,
                           int dims, char **idxs_nt)
{
    DBUG_ENTER ("ICMCompileWL_ASSIGN__COPY");

#define WL_ASSIGN__COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN__COPY

    PrintTraceICM (target, idx_vec, dims, idxs_nt, "copy", TRUE);

    if (dims_target > dims) {
        INDENT;
        fprintf (outfile, "{\n");
        indent++;

        INDENT;
        fprintf (outfile, "int SAC_i;\n");

        INDENT;
        fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SHAPE( %s, %d)", target,
                 dims);
        PrintShapeFactor (dims, dims_target, target);
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
 *                           int dims, char **idxs_nt)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD( dims_target, target, idx_vec, dims, [ idxs_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD (int dims_target, char *target, char *idx_vec, int dims, char **idxs_nt)
{
    DBUG_ENTER ("ICMCompileWL_FOLD");

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

    PrintTraceICM (target, idx_vec, dims, idxs_nt, "fold", FALSE);

    INDENT;
    fprintf (outfile, "/* fold operation */\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD__OFFSET( int dims_target, char *target,
 *                                   char *idx_vec,
 *                                   int dims, char **idxs_nt)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD__OFFSET( dims_target, target, idx_vec, dims, [ idxs_nt ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD__OFFSET (int dims_target, char *target, char *idx_vec, int dims,
                           char **idxs_nt)
{
    DBUG_ENTER ("ICMCompileWL_FOLD__OFFSET");

#define WL_FOLD__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD__OFFSET

    PrintTraceICM (target, idx_vec, dims, idxs_nt, "fold", TRUE);

    INDENT;
    fprintf (outfile, "/* fold operation */\n");
    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", target);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_INIT_OFFSET( int dims_target, char *target,
 *                                  char *idx_vec, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *     WL_INIT_OFFSET( dims_target, target, idx_vec, dims)
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
ICMCompileWL_INIT_OFFSET (int dims_target, char *target, char *idx_vec, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_INIT_OFFSET");

#define WL_INIT_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_INIT_OFFSET

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)\n", target);
    indent++;

    INDENT;
    fprintf (outfile, "= SAC_WL_MT_SCHEDULE_START( 0)");
    PrintShapeFactor (0, dims_target, target);

    for (i = 1; i < dims; i++) {
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
 *   void ICMCompileWL_ADJUST_OFFSET( int dim,
 *                                    int dims_target, char *target,
 *                                    char *idx_vec,
 *                                    int dims, char **idxs_nt)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ADJUST_OFFSET( dim, dims_target, target, idx_vec,
 *                     dims, [ idxs_nt ]* )
 *
 * remark:
 *   This ICM is needed (and usefull) in combination with NOOP N_WL..-nodes
 *   only! It uses the variable 'SAC__diff...' defined by the ICMs
 *   WL_..._NOOP_BEGIN.
 *
 ******************************************************************************/

void
ICMCompileWL_ADJUST_OFFSET (int dim, int dims_target, char *target, char *idx_vec,
                            int dims, char **idxs_nt)
{
    DBUG_ENTER ("ICMCompileWL_ADJUST_OFFSET");

#define WL_ADJUST_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ADJUST_OFFSET

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s) += SAC_WL_VAR( diff, %s)", target,
             idxs_nt[dim]);
    PrintShapeFactor (dim, dims_target, target);
    fprintf (outfile, ";\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SET_OFFSET( int dim, int first_block_dim,
 *                                 int dims_target, char *target,
 *                                 char *idx_vec,
 *                                 int dims, char **idxs_nt)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_SET_OFFSET( dim, first_block_dim, dims_target, target, idx_vec,
 *                  dims, [ idxs_nt ]* )
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
                         char *idx_vec, int dims, char **idxs_nt)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_SET_OFFSET");

#define WL_SET_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SET_OFFSET

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s) \n", target);
    indent++;

    INDENT;
    fprintf (outfile, "= ");
    for (i = dims - 1; i > 0; i--) {
        fprintf (outfile, "( SAC_ND_A_SHAPE( %s, %d) * ", target, i);
    }
    fprintf (outfile, "%s \n", idxs_nt[0]);

    INDENT;
    for (i = 1; i < dims; i++) {
        if (i <= dim) {
            fprintf (outfile, "+ %s )", idxs_nt[i]);
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
                fprintf (outfile, " + SAC_WL_VAR( first, %s) )", idxs_nt[i]);
            }
        }
    }
    PrintShapeFactor (dims - 1, dims_target, target);

    fprintf (outfile, ";\n");
    indent--;

    DBUG_VOID_RETURN;
}
