
#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_cuda.h"
#include "icm2c_std.h"
#include "dbug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "tree_basic.h"
#include "str.h"
#include "memory.h"

#ifndef BEtest
#include "scnprs.h"   /* for big magic access to syntax tree      */
#include "traverse.h" /* for traversal of fold operation function */
#include "compile.h"  /* for GetFoldCode()                        */
#include "free.h"
#endif /* BEtest */

static void
CompileCUDA_GLOBALFUN_HEADER (char *funname, int vararg_cnt, char **vararg)
{
    int i, j, cnt, dim;
    char *basetype;

    DBUG_ENTER ("ICMCompileCUDA_GLOBALFUN_DECL");

    INDENT;
    fprintf (global.outfile, "__global__ void __launch_bounds__(%d, %d) %s(",
             global.optimal_threads, global.optimal_blocks, funname);

    cnt = 0;
    for (i = 0; i < 4 * vararg_cnt; i += 4) {
        if (STReq (vararg[i + 1], "float_dev")) {
            basetype = "float";
        } else if (STReq (vararg[i + 1], "int_dev")) {
            basetype = "int";
        } else {
            basetype = vararg[i + 1];
        }

        INDENT;
        fprintf (global.outfile, "SAC_CUDA_PARAM_%s( %s, %s)", vararg[i], vararg[i + 2],
                 basetype);

        dim = DIM_NO_OFFSET (atoi (vararg[i + 3]));
        if (dim > 0) {
            fprintf (global.outfile, ", ");
            for (j = 0; j < dim; j++) {
                fprintf (global.outfile, "int SAC_ND_A_MIRROR_SHAPE(%s, %d), ",
                         vararg[i + 2], j);
            }
            fprintf (global.outfile, "int SAC_ND_A_MIRROR_SIZE(%s), ", vararg[i + 2]);
            fprintf (global.outfile, "int SAC_ND_A_MIRROR_DIM(%s)", vararg[i + 2]);
        }
        if (i != 4 * (vararg_cnt - 1)) {
            fprintf (global.outfile, ", ");
        }
    }
    fprintf (global.outfile, ")");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_DECL( char *funname,
 *                                       int vararg_cnt,
 *                                       char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_DECL (char *funname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileCUDA_GLOBALFUN_DECL");

#define CUDA_GLOBALFUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_DECL

    CompileCUDA_GLOBALFUN_HEADER (funname, vararg_cnt, vararg);

    fprintf (global.outfile, ";\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_DEF_BEGIN( char *funname,
 *                                            int vararg_cnt,
 *                                            char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_DEF_BEGIN (char *funname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileCUDA_GLOBALFUN_DEF_BEGIN");

#define CUDA_GLOBALFUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_DEF_BEGIN

    CompileCUDA_GLOBALFUN_HEADER (funname, vararg_cnt, vararg);

    fprintf (global.outfile, "{\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_RET( char *funname,
 *                                      int vararg_cnt,
 *                                      char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_RET (char *funname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileCUDA_GLOBALFUN_RET");

    INDENT;
    fprintf (global.outfile, "SAC_NOOP()\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_DEF_END( char *funname,
 *                                          int vararg_cnt,
 *                                          char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_DEF_END (char *funname, int vararg_cnt, char **vararg)
{
    DBUG_ENTER ("ICMCompileCUDA_GLOBALFUN_DEF_END");

#define CUDA_GLOBALFUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_DEF_END

    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_AP( char *funname,
 *                                     int vararg_cnt,
 *                                     char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_AP (char *funname, int vararg_cnt, char **vararg)
{
    int dim, cnt, i, j;
    char *basetype;

    DBUG_ENTER ("ICMCompileCUDA_GLOBALFUN_AP");

#define CUDA_GLOBALFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_AP

    INDENT;
    INDENT;
    fprintf (global.outfile, "%s<<<grid, block>>>(", funname);
    cnt = 0;
    for (i = 0; i < 4 * vararg_cnt; i += 4) {
        if (STReq (vararg[i + 1], "float_dev")) {
            basetype = "float";
        } else if (STReq (vararg[i + 1], "int_dev")) {
            basetype = "int";
        } else {
            basetype = vararg[i + 1];
        }

        INDENT;
        fprintf (global.outfile, "SAC_CUDA_ARG_%s( %s, %s)", vararg[i], vararg[i + 3],
                 basetype);

        dim = DIM_NO_OFFSET (atoi (vararg[i + 2]));
        if (dim > 0) {
            fprintf (global.outfile, ", ");
            for (j = 0; j < dim; j++) {
                fprintf (global.outfile, "SAC_ND_A_MIRROR_SHAPE(%s, %d), ", vararg[i + 3],
                         j);
            }
            fprintf (global.outfile, "SAC_ND_A_MIRROR_SIZE(%s), ", vararg[i + 3]);
            fprintf (global.outfile, "SAC_ND_A_MIRROR_DIM(%s)", vararg[i + 3]);
        }
        if (i != 4 * (vararg_cnt - 1)) {
            fprintf (global.outfile, ", ");
        }
    }
    fprintf (global.outfile, ");\n");
    /*
      fprintf( global.outfile, "cudaThreadSynchronize();\n");
      fprintf( global.outfile, "cutStopTimer(timer);\n");
      fprintf( global.outfile, "fprintf(stderr,\"%s: %%f\\n\",
      cutGetTimerValue(timer));\n", funname); fprintf( global.outfile,
      "cutResetTimer(timer);\n");
    */
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GRID_BLOCK( int bounds_count, char **var_ANY)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GRID_BLOCK (int bounds_count, char **var_ANY)
{
    /*
      int array_dim;
    */

    DBUG_ENTER ("ICMCompileCUDA_GRID_BLOCK");

#define CUDA_GRID_BLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GRID_BLOCK
    /*
      array_dim = bounds_count/2;
    */
    INDENT;
    fprintf (global.outfile, "{\n");

    if (bounds_count == 3) { /* 1D CUDA withloop */
        INDENT;
        INDENT;
        /*
            fprintf( global.outfile, "dim3 grid((%s-%s)/%d+1);\n",
                     var_ANY[0], var_ANY[1], global.cuda_1d_block_large);
            INDENT;INDENT;
            fprintf( global.outfile, "dim3 block(%d);", global.cuda_1d_block_large);
        */
        fprintf (global.outfile, "dim3 grid((%s-%s)/%s+1);\n", var_ANY[0], var_ANY[1],
                 var_ANY[2]);
        INDENT;
        INDENT;
        fprintf (global.outfile, "dim3 block(%s);", var_ANY[2]);

    } else if (bounds_count == 6) { /* 2D CUDA withloop */
        INDENT;
        INDENT;
        /*
            fprintf( global.outfile,
                     "dim3 grid((%s-%s)/%d+1, (%s-%s)/%d+1);\n",
                     var_ANY[0], var_ANY[2], global.cuda_2d_block_x,
                     var_ANY[1], var_ANY[3], global.cuda_2d_block_y);
            INDENT;INDENT;
            fprintf( global.outfile, "dim3 block(%d, %d);",
                     global.cuda_2d_block_x, global.cuda_2d_block_y);
        */
        fprintf (global.outfile, "dim3 grid((%s-%s)/%s+1, (%s-%s)/%s+1);\n", var_ANY[0],
                 var_ANY[2], var_ANY[5], var_ANY[1], var_ANY[3], var_ANY[4]);
        INDENT;
        INDENT;
        fprintf (global.outfile, "dim3 block(%s, %s);", var_ANY[5], var_ANY[4]);
    } else {
        if (bounds_count == 9) { /* 3D CUDA withloop */
            INDENT;
            INDENT;
            /*
            fprintf( global.outfile, "dim3 grid((%s-%s), (%s-%s));\n",
                     var_ANY[2], var_ANY[5], var_ANY[1], var_ANY[4]);
            */
            fprintf (global.outfile, "dim3 grid((%s-%s), (%s-%s));\n", var_ANY[1],
                     var_ANY[4], var_ANY[2], var_ANY[5]);
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 block((%s-%s));", var_ANY[0], var_ANY[3]);
        } else if (bounds_count == 12) { /* 4D CUDA withloop */
            INDENT;
            INDENT;
            /*
            fprintf( global.outfile, "dim3 grid((%s-%s), (%s-%s));\n",
                     var_ANY[3], var_ANY[7], var_ANY[2], var_ANY[6]);
            INDENT;INDENT;
            fprintf( global.outfile, "dim3 block((%s-%s), (%s-%s));",
                     var_ANY[1], var_ANY[5], var_ANY[0], var_ANY[4]);
            */
            fprintf (global.outfile, "dim3 grid((%s-%s), (%s-%s));\n", var_ANY[2],
                     var_ANY[6], var_ANY[3], var_ANY[7]);
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 block((%s-%s), (%s-%s));", var_ANY[0],
                     var_ANY[4], var_ANY[1], var_ANY[5]);
        } else {
            INDENT;
            INDENT;
            /*
            fprintf( global.outfile, "dim3 grid((%s-%s), (%s-%s));\n",
                     var_ANY[4], var_ANY[9], var_ANY[3], var_ANY[8]);
            INDENT;INDENT;
            fprintf( global.outfile, "dim3 block((%s-%s), (%s-%s), (%s-%s));",
                     var_ANY[2], var_ANY[7], var_ANY[1], var_ANY[6],
                     var_ANY[0], var_ANY[5]);
            */
            fprintf (global.outfile, "dim3 grid((%s-%s), (%s-%s));\n", var_ANY[3],
                     var_ANY[8], var_ANY[4], var_ANY[9]);
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 block((%s-%s), (%s-%s), (%s-%s));", var_ANY[0],
                     var_ANY[5], var_ANY[1], var_ANY[6], var_ANY[2], var_ANY[7]);
        }
    }

    INDENT;
    INDENT;
    /*
      fprintf( global.outfile, "unsigned int timer;\n");
      fprintf( global.outfile, "cutCreateTimer(&timer);\n");
      fprintf( global.outfile, "cutStartTimer(timer);\n");
    */
    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_ST_GLOBALFUN_AP( char *funname,
 *                                     int vararg_cnt,
 *                                     char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_ST_GLOBALFUN_AP (char *funname, int vararg_cnt, char **vararg)
{
    int dim, cnt, i, j;
    char *basetype;

    DBUG_ENTER ("ICMCompileCUDA_ST_GLOBALFUN_AP");

#define CUDA_ST_GLOBALFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_ST_GLOBALFUN_AP

    INDENT;
    fprintf (global.outfile, "{\n");

    INDENT;
    INDENT;
    /*
    fprintf( global.outfile, "unsigned int timer;\n");
    fprintf( global.outfile, "cutCreateTimer(&timer);\n");
    fprintf( global.outfile, "cutStartTimer(timer);\n");
    */
    INDENT;
    INDENT;
    fprintf (global.outfile, "%s<<<1, 1>>>(", funname);
    cnt = 0;
    for (i = 0; i < 4 * vararg_cnt; i += 4) {
        if (STReq (vararg[i + 1], "float_dev")) {
            basetype = "float";
        } else if (STReq (vararg[i + 1], "int_dev")) {
            basetype = "int";
        } else {
            basetype = vararg[i + 1];
        }

        INDENT;
        fprintf (global.outfile, "SAC_CUDA_ARG_%s( %s, %s)", vararg[i], vararg[i + 3],
                 basetype);

        dim = DIM_NO_OFFSET (atoi (vararg[i + 2]));
        if (dim > 0) {
            fprintf (global.outfile, ", ");
            for (j = 0; j < dim; j++) {
                fprintf (global.outfile, "SAC_ND_A_MIRROR_SHAPE(%s, %d), ", vararg[i + 3],
                         j);
            }
            fprintf (global.outfile, "SAC_ND_A_MIRROR_SIZE(%s), ", vararg[i + 3]);
            fprintf (global.outfile, "SAC_ND_A_MIRROR_DIM(%s)", vararg[i + 3]);
        }
        if (i != 4 * (vararg_cnt - 1)) {
            fprintf (global.outfile, ", ");
        }
    }
    fprintf (global.outfile, ");\n");

    /*
      fprintf( global.outfile, "cutStopTimer(timer);\n");
      fprintf( global.outfile, "fprintf(stderr,\"%s: %%f\\n\",
      cutGetTimerValue(timer));\n", funname); fprintf( global.outfile,
      "cutResetTimer(timer);\n");
    */
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_WLIDS( char *wlids_NT, int wlids_NT_dim,
 *                              int array_dim, int wlids_dim,
 *                              char *iv_NT, char *hasstepwidth)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_WLIDS (char *wlids_NT, int wlids_NT_dim, int array_dim, int wlids_dim_pos,
                      char *iv_NT, char *hasstepwidth)
{
    bool has_postfix;

    DBUG_ENTER ("ICMCompileCUDA_WLIDS");

#define CUDA_WLIDS
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_WLIDS

    /*
      if( array_dim <= 2) {
        INDENT;
        if( STReq( hasstepwidth, "true")) {
          fprintf( global.outfile,
                   "SAC_CUDA_WLIDS_%dD_SW_%d( %s, %d, SACp_step_%d, SACp_width_%d,
      SACp_lb_%d, SACp_ub_%d)\n", array_dim, wlids_dim_pos, wlids_NT, wlids_NT_dim,
                   wlids_dim_pos, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos);
        }
        else {
          fprintf( global.outfile,
                   "SAC_CUDA_WLIDS_%dD_%d( %s, %d, SACp_lb_%d, SACp_ub_%d)\n",
                   array_dim, wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos,
      wlids_dim_pos);
        }
      }
      else {
        INDENT;
        if( STReq( hasstepwidth, "true")) {
          fprintf( global.outfile,
                   "SAC_CUDA_WLIDS_ND_SW_%d( %s, %d, SACp_step_%d, SACp_width_%d,
      SACp_lb_%d, SACp_ub_%d)\n", wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos,
      wlids_dim_pos, wlids_dim_pos, wlids_dim_pos);
        }
        else {
          fprintf( global.outfile,
                   "SAC_CUDA_WLIDS_ND_%d( %s, %d, SACp_lb_%d, SACp_ub_%d)\n",
                   wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos);
        }
      }
    */

    has_postfix = STReq (hasstepwidth, "true");

    if (array_dim == 1) {
        INDENT;
        fprintf (global.outfile, "SAC_CUDA_WLIDS");
        if (has_postfix) {
            fprintf (global.outfile, "_SW");
        }
        fprintf (global.outfile,
                 "( %s, %d, BLOCKIDX_X, BLOCKDIM_X, THREADIDX_X, SACp_step_%d, "
                 "SACp_width_%d, SACp_lb_%d, SACp_ub_%d)\n",
                 wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                 wlids_dim_pos);
    } else if (array_dim == 2) {
        INDENT;
        fprintf (global.outfile, "SAC_CUDA_WLIDS");
        if (has_postfix) {
            fprintf (global.outfile, "_SW");
        }
        if (wlids_dim_pos == 0) {
            fprintf (global.outfile,
                     "( %s, %d, BLOCKIDX_Y, BLOCKDIM_Y, THREADIDX_Y, SACp_step_%d, "
                     "SACp_width_%d, SACp_lb_%d, SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else if (wlids_dim_pos == 1) {
            fprintf (global.outfile,
                     "( %s, %d, BLOCKIDX_X, BLOCKDIM_X, THREADIDX_X, SACp_step_%d, "
                     "SACp_width_%d, SACp_lb_%d, SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else {
            DBUG_ASSERT ((0), "Invalid index found!");
        }
    } else if (array_dim >= 3) {
        INDENT;
        fprintf (global.outfile, "SAC_CUDA_WLIDS_HD");
        if (has_postfix) {
            fprintf (global.outfile, "_SW");
        }
        if (wlids_dim_pos == 0) {
            fprintf (global.outfile,
                     "( %s, %d, BLOCKIDX_Y, SACp_step_%d, SACp_width_%d, SACp_lb_%d, "
                     "SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else if (wlids_dim_pos == 1) {
            fprintf (global.outfile,
                     "( %s, %d, BLOCKIDX_X, SACp_step_%d, SACp_width_%d, SACp_lb_%d, "
                     "SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else if ((array_dim - wlids_dim_pos) == 1) {
            fprintf (global.outfile,
                     "( %s, %d, THREADIDX_X, SACp_step_%d, SACp_width_%d, SACp_lb_%d, "
                     "SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else if ((array_dim - wlids_dim_pos) == 2) {
            fprintf (global.outfile,
                     "( %s, %d, THREADIDX_Y, SACp_step_%d, SACp_width_%d, SACp_lb_%d, "
                     "SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else if ((array_dim - wlids_dim_pos) == 3) {
            fprintf (global.outfile,
                     "( %s, %d, THREADIDX_Z, SACp_step_%d, SACp_width_%d, SACp_lb_%d, "
                     "SACp_ub_%d)\n",
                     wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos);
        } else {
            DBUG_ASSERT ((0),
                         "Invalid combination of array dimension and dimension index!");
        }
    } else {
        DBUG_ASSERT ((0), "Invalid array dimension found!");
    }

    fprintf (global.outfile, "SAC_ND_WRITE( %s, %d) = SAC_ND_READ( %s, 0);\n", iv_NT,
             wlids_dim_pos, wlids_NT);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_WLIDXS( char *wlidxs_NT, char *array_NT,
 *                               int array_dim, char **var_ANY)
 *
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_WLIDXS (char *wlidxs_NT, int wlidxs_NT_dim, char *array_NT, int array_dim,
                       char **var_ANY)
{
    DBUG_ENTER ("ICMCompileCUDA_WLIDXS");

#define CUDA_WLIDXS
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_WLIDXS

    if (array_dim == 1) {
        INDENT;
        fprintf (global.outfile, "SAC_ND_WRITE( %s, %d) = %s;\n", wlidxs_NT,
                 wlidxs_NT_dim, var_ANY[0]);
    } else if (array_dim == 2) {
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_WRITE( %s, %d) = %s * SAC_ND_A_MIRROR_SHAPE(%s, 1) + %s;\n",
                 wlidxs_NT, wlidxs_NT_dim, var_ANY[0], array_NT, var_ANY[1]);
    } else {
        int i, j;
        INDENT;
        fprintf (global.outfile, "SAC_ND_WRITE( %s, %d) = ", wlidxs_NT, wlidxs_NT_dim);
        for (i = 0; i < array_dim; i++) {
            fprintf (global.outfile, "%s", var_ANY[i]);
            for (j = i + 1; j < array_dim; j++) {
                fprintf (global.outfile, "*SAC_ND_A_MIRROR_SHAPE(%s, %d)", array_NT, j);
            }
            if (i != array_dim - 1) {
                fprintf (global.outfile, "+");
            }
        }
        fprintf (global.outfile, ";\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_THREADIDX( char *to_NT, int dim, int dim_pos)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_THREADIDX (char *to_NT, int dim, int dim_pos)
{
    DBUG_ENTER ("ICMCompileCUDA_THREADIDX");

#define CUDA_THREADIDX
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_THREADIDX

    if (dim == 1) {
        INDENT;
        fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = THREADIDX_X;\n", to_NT);
    } else if (dim == 2) {
        INDENT;
        if (dim_pos == 0) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = THREADIDX_Y;\n", to_NT);
        } else if (dim_pos == 1) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = THREADIDX_X;\n", to_NT);
        } else {
            DBUG_ASSERT ((0), "Illegal dimension position found!");
        }
    } else if (dim == 3) {
        INDENT;
        if (dim_pos == 0) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = THREADIDX_Z;\n", to_NT);
        } else if (dim_pos == 1) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = THREADIDX_Y;\n", to_NT);
        } else if (dim_pos == 2) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = THREADIDX_X;\n", to_NT);
        } else {
            DBUG_ASSERT ((0), "Illegal dimension position found!");
        }
    } else {
        DBUG_ASSERT ((0), "Illegal dimension found!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_BLOCKDIM( char *to_NT, int dim, int dim_pos)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_BLOCKDIM (char *to_NT, int dim, int dim_pos)
{
    DBUG_ENTER ("ICMCompileCUDA_BLOCKDIM");

#define CUDA_BLOCKDIM
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_BLOCKDIM

    if (dim == 1) {
        INDENT;
        fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = BLOCKDIM_X;\n", to_NT);
    } else if (dim == 2) {
        INDENT;
        if (dim_pos == 0) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = BLOCKDIM_Y;\n", to_NT);
        } else if (dim_pos == 1) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = BLOCKDIM_X;\n", to_NT);
        } else {
            DBUG_ASSERT ((0), "Illegal dimension position found!");
        }
    } else if (dim == 3) {
        INDENT;
        if (dim_pos == 0) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = BLOCKDIM_Z;\n", to_NT);
        } else if (dim_pos == 1) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = BLOCKDIM_Y;\n", to_NT);
        } else if (dim_pos == 2) {
            fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = BLOCKDIM_X;\n", to_NT);
        } else {
            DBUG_ASSERT ((0), "Illegal dimension position found!");
        }
    } else {
        DBUG_ASSERT ((0), "Illegal dimension found!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_WL_ASSIGN( char *val_NT, int val_sdim,
 *                                  char *to_NT, int to_sdim,
 *                                  char *off_NT)
 *
 ******************************************************************************/
void
ICMCompileCUDA_WL_ASSIGN (char *val_NT, int val_sdim, char *to_NT, int to_sdim,
                          char *off_NT)
{
    int val_dim;

    DBUG_ENTER ("ICMCompileCUDA_WL_ASSIGN");

#define CUDA_WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_WL_ASSIGN

    val_dim = DIM_NO_OFFSET (val_sdim);

    if (val_dim == 0) {
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_WRITE_READ_COPY( %s, SAC_ND_READ( %s, 0),"
                 " %s, 0, %s);\n",
                 to_NT, off_NT, val_NT, "");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_MEM_TRANSFER( char *to_NT, char *from_NT,
 *                                     char *basetype, char *direction)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_MEM_TRANSFER (char *to_NT, char *from_NT, char *basetype, char *direction)
{

    DBUG_ENTER ("ICMCompileCUDA_MEM_TRANSFER");

#define CUDA_MEM_TRANSFER
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_MEM_TRANSFER

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)",
                              to_NT, from_NT);
                     , fprintf (global.outfile, "cudaMemcpy: Destionation and source "
                                                "arrays should have equal sizes!"););

    INDENT;
    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER(%s, %s, %s, %s)", to_NT, from_NT,
             basetype, direction);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_WL_SUBALLOC( char *sub_NT, int sub_dim, char *to_NT,
 *                                    int to_dim, char *off_NT)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_WL_SUBALLOC (char *sub_NT, int sub_dim, char *to_NT, int to_dim,
                            char *off_NT)
{
    int sdim, tdim, i;

    DBUG_ENTER ("ICMCompileCUDA_WL_SUBALLOC");

#define CUDA_WL_SUBALLOC
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_WL_SUBALLOC

    sdim = DIM_NO_OFFSET (sub_dim);
    tdim = DIM_NO_OFFSET (to_dim);

    INDENT;
    fprintf (global.outfile,
             "SAC_ND_GETVAR(%s, SAC_ND_A_FIELD( %s)) = SAC_ND_GETVAR( %s, "
             "SAC_ND_A_FIELD( %s))+SAC_ND_READ( %s, 0)",
             sub_NT, sub_NT, to_NT, to_NT, off_NT);

    for (i = sdim; i < tdim; i++) {
        fprintf (global.outfile, " * SAC_ND_A_MIRROR_SHAPE(%s, %d)", to_NT, i);
    }

    fprintf (global.outfile, ";\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxA__DATA( char *to_NT,
 *                                                     int to_sdim,
 *                                                     char *from_NT,
 *                                                     int from_sdim,
 *                                                     char *idx_ANY,
 *                                                     char *val_array,
 *                                                     char *basetype)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxA__DATA (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, char *idx_ANY,
                                             char *val_array, char *basetype)

{
    DBUG_ENTER ("ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxA__DATA");

#define CUDA_PRF_IDX_MODARRAY_AxSxA__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_PRF_IDX_MODARRAY_AxSxA__DATA

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (global.outfile, "2nd argument of %s is not a scalar!",
                                    global.prf_name[F_idx_modarray_AxSxS]););
    }

    INDENT;
    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER_D2D( %s, ", to_NT);
    ReadScalar (idx_ANY, NULL, 0);
    fprintf (global.outfile, ", %s, %s)", val_array, basetype);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_DECL_KERNEL_ARRAY( char *var_NT, char *basetype,
 *                                          int sdim, int *shp)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_DECL_KERNEL_ARRAY (char *var_NT, char *basetype, int sdim, int *shp)
{
    int i, dim;
    shape_class_t sc;

    DBUG_ENTER ("ICMCompileCUDA_DECL_KERNEL_ARRAY");

#define CUDA_DECL_KERNEL_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_DECL_KERNEL_ARRAY

    sc = ICUGetShapeClass (var_NT);
    dim = DIM_NO_OFFSET (sdim);

    switch (sc) {
    case C_aks:
        INDENT;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");
        fprintf (global.outfile, "%s SAC_ND_A_FIELD( %s)[", basetype, var_NT);
        for (i = 0; i < dim; i++) {
            fprintf (global.outfile, "%d", shp[i]);
            if (i != dim - 1) {
                fprintf (global.outfile, ", ");
            }
        }
        fprintf (global.outfile, "];\n");

        INDENT;
        fprintf (global.outfile, "SAC_ND_DECL__DESC( %s, )\n", var_NT);

        ICMCompileND_DECL__MIRROR (var_NT, sdim, shp);
        break;
    default:
        DBUG_ASSERT ((0), "Non-AKS array found in CUDA kernel!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_DECL_SHMEM_ARRAY( char *var_NT, char *basetype,
 *                                          int sdim, int *shp)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_DECL_SHMEM_ARRAY (char *var_NT, char *basetype, int sdim, int *shp)
{
    int i, dim, size = 1;
    shape_class_t sc;

    DBUG_ENTER ("ICMCompileCUDA_DECL_SHMEM_ARRAY");

#define CUDA_DECL_SHMEM_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_DECL_SHMEM_ARRAY

    sc = ICUGetShapeClass (var_NT);
    dim = DIM_NO_OFFSET (sdim);

    switch (sc) {
    case C_aks:
        INDENT;
        DBUG_ASSERT ((dim >= 0), "illegal dimension found!");

        for (i = 0; i < dim; i++) {
            size *= shp[i];
        }
        fprintf (global.outfile, "__shared__ %s SAC_ND_A_FIELD( %s)[%d];\n", basetype,
                 var_NT, size);
        break;
    default:
        DBUG_ASSERT ((0), "Non-AKS shared memory array found in CUDA kernel!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_SHMEM_BOUNDARY_CHECK( char *to_NT, int dim_pos,
 *                                             char *idx_NT, int offset)
 *
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_SHMEM_BOUNDARY_CHECK (char *to_NT, int dim_pos, char *idx_NT, int offset)
{
    DBUG_ENTER ("ICMCompileCUDA_SHMEM_BOUNDARY_CHECK");

#define CUDA_SHMEM_BOUNDARY_CHECK
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_SHMEM_BOUNDARY_CHECK

    INDENT;
    fprintf (global.outfile,
             "SAC_ND_A_FIELD( %s) = ( ( SACp_ub_%d-%d) == SAC_ND_A_FIELD( %s))\n", to_NT,
             dim_pos, offset, idx_NT);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_ASSIGN( char *to_NT, int to_sdim,
 *                               char *from_NT, int from_sdim,
 *                               char *copyfun)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_ASSIGN (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                       char *copyfun)
{
    int from_dim;

    DBUG_ENTER ("ICMCompileCUDA_ASSIGN");

#define CUDA_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_ASSIGN

    from_dim = DIM_NO_OFFSET (from_sdim);

    Check_Mirror (to_NT, to_sdim, from_NT, from_dim, DimId, ShapeId, NULL, 0, NULL, NULL);

    ICMCompileND_UPDATE__MIRROR (to_NT, to_sdim, from_NT, from_sdim);

    INDENT;
    fprintf (global.outfile, "SAC_ND_ASSIGN__DATA( %s, %s, %s)\n", to_NT, from_NT,
             copyfun);

    DBUG_VOID_RETURN;
}
