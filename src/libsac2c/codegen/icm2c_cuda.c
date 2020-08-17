
#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_cuda.h"
#include "icm2c_std.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
CompileCUDA_GLOBALFUN_HEADER (char *funname, unsigned int vararg_cnt, char **vararg)
{
    int j, dim;
    unsigned int i;
    char *basetype;

    DBUG_ENTER ();

    INDENT;
    fprintf (global.outfile, "__global__ void");
    if (global.optimize.dolb) {
        fprintf (global.outfile, " __launch_bounds__(%d, %d) ",
                 global.cuda_options.optimal_threads, global.cuda_options.optimal_blocks);
    }
    fprintf (global.outfile, " %s(", funname);

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

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_DECL( char *funname,
 *                                       unsigned int vararg_cnt,
 *                                       char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_DECL (char *funname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define CUDA_GLOBALFUN_DECL
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_DECL

    CompileCUDA_GLOBALFUN_HEADER (funname, vararg_cnt, vararg);

    fprintf (global.outfile, ";\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_DEF_BEGIN( char *funname,
 *                                            unsigned int vararg_cnt,
 *                                            char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_DEF_BEGIN (char *funname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define CUDA_GLOBALFUN_DEF_BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_DEF_BEGIN

    CompileCUDA_GLOBALFUN_HEADER (funname, vararg_cnt, vararg);

    fprintf (global.outfile, "{\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_RET( char *funname,
 *                                      unsigned int vararg_cnt,
 *                                      char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_RET (char *funname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

    INDENT;
    fprintf (global.outfile, "SAC_NOOP()\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_DEF_END( char *funname,
 *                                          unsigned int vararg_cnt,
 *                                          char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_DEF_END (char *funname, unsigned int vararg_cnt, char **vararg)
{
    DBUG_ENTER ();

#define CUDA_GLOBALFUN_DEF_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_DEF_END

    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GLOBALFUN_AP( char *funname,
 *                                     unsigned int vararg_cnt,
 *                                     char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_AP (char *funname, unsigned int vararg_cnt, char **vararg)
{
    int dim, j;
    unsigned int i; 
    char *basetype;

    DBUG_ENTER ();

#define CUDA_GLOBALFUN_AP
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GLOBALFUN_AP

    INDENT;
    INDENT;
    fprintf (global.outfile,
             "SAC_TR_GPU_PRINT (\"   kernel name \\\"%s\\\"\\n\");\n",
             funname);
    fprintf (global.outfile, "SAC_PF_BEGIN_CUDA_KNL ();\n");
    if (global.backend == BE_cudahybrid) {
        // on cudahybrid, we make use of streams, which have a fixed name
        fprintf (global.outfile, "%s<<<grid, block, 0, *stream>>>(", funname);
    } else {
        fprintf (global.outfile, "%s<<<grid, block>>>(", funname);
    }

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

    if (STReq (global.config.cuda_alloc, "cuman")
        || STReq (global.config.cuda_alloc, "cumanp")) {
        fprintf (global.outfile, "cudaDeviceSynchronize ();\n");
    }
    fprintf (global.outfile, "SAC_PF_END_CUDA_KNL ();\n");

    fprintf (global.outfile, "SAC_CUDA_GET_LAST_KERNEL_ERROR();\n");
    /*
      fprintf( global.outfile, "cudaThreadSynchronize();\n");
      fprintf( global.outfile, "cutStopTimer(timer);\n");
      fprintf( global.outfile, "fprintf(stderr,\"%s: %%f\\n\",
      cutGetTimerValue(timer));\n", funname); fprintf( global.outfile,
      "cutResetTimer(timer);\n");
    */

    /*
      INDENT;
      fprintf( global.outfile, "cutilCheckMsg(\"%s failed!\\n\");\n", funname);
    */

    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GRID_BLOCK( unsigned int bounds_count, char **var_ANY)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_GRID_BLOCK (unsigned int bounds_count, char **var_ANY)
{
    DBUG_ENTER ();

#define CUDA_GRID_BLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GRID_BLOCK

#define CUDA_SET_GRID(fmt, ...)                                                          \
    fprintf (global.outfile, "dim3 grid(" fmt ");\n", __VA_ARGS__);                      \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_TR_GPU_PRINT (\"   CUDA XYZ grid dimension of "        \
                             "%%u x %%u x %%u\", grid.x , grid.y , grid.z );\n");        \
    INDENT;                                                                              \
    fprintf (global.outfile, "if (grid.x <= 0 ) {\n"                                     \
                             "SAC_RuntimeError(\"CUDA X grid dimension must be bigger "  \
                             "than zero. Current"                                        \
                             " value is %%u\", grid.x);");                               \
    fprintf (global.outfile, "}\n");                                                     \
    fprintf (global.outfile, "if (grid.y <= 0 ) {\n"                                     \
                             "SAC_RuntimeError(\"CUDA Y grid dimension must be bigger "  \
                             "than zero. Current"                                        \
                             " value is %%u\", grid.y);");                               \
    fprintf (global.outfile, "}\n");                                                     \
    fprintf (global.outfile, "if (grid.z <= 0 ) {\n"                                     \
                             "SAC_RuntimeError(\"CUDA Z grid dimension must be bigger "  \
                             "than zero. Current"                                        \
                             " value is %%u\", grid.z);");                               \
    fprintf (global.outfile, "}\n");                                                     \
    fprintf (global.outfile, "if (grid.x > %u || grid.y > %u || grid.z > %u) {\n",       \
             global.cuda_options.cuda_max_x_grid, global.cuda_options.cuda_max_yz_grid,  \
             global.cuda_options.cuda_max_yz_grid);                                      \
    INDENT;                                                                              \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile,                                                             \
             "SAC_RuntimeError(\"CUDA XYZ grid dimension of %%u x %%u x %%u exceeds "    \
             "the compute capability's max value: %u x %u x %u\","                       \
             " grid.x, grid.y, grid.z );\n",                                             \
             global.cuda_options.cuda_max_x_grid, global.cuda_options.cuda_max_yz_grid,  \
             global.cuda_options.cuda_max_yz_grid);                                      \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile, "}\n");

#define CUDA_SET_BLOCK(fmt, ...)                                                         \
    fprintf (global.outfile, "dim3 block(" fmt ");", __VA_ARGS__);                       \
    INDENT;                                                                              \
    fprintf (global.outfile, "SAC_TR_GPU_PRINT (\"   CUDA XYZ block dimension of "       \
                             "%%u x %%u x %%u\", block.x , block.y , block.z );\n");     \
    INDENT;                                                                              \
    fprintf (global.outfile,                                                             \
             "if (block.x <= 0 ) {\n"                                                    \
             "SAC_RuntimeError(\"CUDA X block dimension must be bigger than zero. "      \
             "Current value is %%u\", block.x);");                                       \
    fprintf (global.outfile, "}\n");                                                     \
    fprintf (global.outfile,                                                             \
             "if (block.y <= 0 ) {\n"                                                    \
             "SAC_RuntimeError(\"CUDA Y block dimension must be bigger than zero. "      \
             "Current value is %%u\", block.y);");                                       \
    fprintf (global.outfile, "}\n");                                                     \
    fprintf (global.outfile,                                                             \
             "if (block.z <= 0 ) {\n"                                                    \
             "SAC_RuntimeError(\"CUDA Z block dimension must be bigger than zero. "      \
             "Current value is %%u\", block.z);");                                       \
    fprintf (global.outfile, "}\n");                                                     \
    fprintf (global.outfile, "if (block.x > %u || block.y > %u || block.z > %u) {\n",    \
             global.cuda_options.cuda_max_xy_block,                                      \
             global.cuda_options.cuda_max_xy_block,                                      \
             global.cuda_options.cuda_max_z_block);                                      \
    INDENT;                                                                              \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile,                                                             \
             "SAC_RuntimeError(\"CUDA XYZ block dimension of %%u x %%u x %%u exceeds "   \
             "the compute capability's max value: %u x %u x %u\", "                      \
             "block.x, block.y, block.z);\n",                                            \
             global.cuda_options.cuda_max_xy_block,                                      \
             global.cuda_options.cuda_max_xy_block,                                      \
             global.cuda_options.cuda_max_z_block);                                      \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile, "}\n");                                                     \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile, "if (block.x * block.y *block.z > %u ) {\n",                \
             global.cuda_options.cuda_max_threads_block);                                \
    INDENT;                                                                              \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile,                                                             \
             "SAC_RuntimeError(\"CUDA XYZ block dimension of %%u x %%u x %%u = %%u "     \
             "exceeds compute capability's max number of threads per block: %u\", "      \
             "block.x, block.y, block.z, block.x * block.y * block.z);\n",               \
             global.cuda_options.cuda_max_threads_block);                                \
    INDENT;                                                                              \
    INDENT;                                                                              \
    fprintf (global.outfile, "}\n");

#define CUDA_DIM_GRID_1D(arr)                                                            \
    CUDA_SET_GRID ("(%s-%s)/%s+1", arr[0], arr[1], arr[2])                               \
    INDENT;                                                                              \
    INDENT;                                                                              \
    CUDA_SET_BLOCK ("%s", arr[2])

#define CUDA_DIM_GRID_2D(arr)                                                            \
    CUDA_SET_GRID ("(%s-%s)/%s+1, (%s-%s)/%s+1", arr[0], arr[2], arr[5], arr[1], arr[3], \
                   arr[4])                                                               \
    INDENT;                                                                              \
    INDENT;                                                                              \
    CUDA_SET_BLOCK ("%s, %s", arr[5], arr[4])

#define CUDA_DIM_GRID_3D(arr)                                                            \
    CUDA_SET_GRID ("(%s-%s), (%s-%s)", arr[1], arr[4], arr[2], arr[5])                   \
    INDENT;                                                                              \
    INDENT;                                                                              \
    CUDA_SET_BLOCK ("(%s-%s)", arr[0], arr[3])

#define CUDA_DIM_GRID_4D(arr)                                                            \
    CUDA_SET_GRID ("(%s-%s), (%s-%s)", arr[2], arr[6], arr[3], arr[7])                   \
    INDENT;                                                                              \
    INDENT;                                                                              \
    CUDA_SET_BLOCK ("(%s-%s), (%s-%s)", arr[0], arr[4], arr[1], arr[5])

#define CUDA_DIM_GRID_5D(arr)                                                            \
    CUDA_SET_GRID ("(%s-%s), (%s-%s)", arr[3], arr[8], arr[4], arr[9])                   \
    INDENT;                                                                              \
    INDENT;                                                                              \
    CUDA_SET_BLOCK ("(%s-%s), (%s-%s), (%s-%s)", arr[0], arr[5], arr[1], arr[6], arr[2], \
                    arr[7])

    INDENT;
    fprintf (global.outfile, "{\n");

    fprintf (global.outfile,
             "SAC_TR_GPU_PRINT (\"launching kernel for %dD With-Loop\");",
             bounds_count/3); 
    INDENT;
    if (bounds_count == 3) { /* 1D CUDA withloop */
        INDENT;
        INDENT;
        CUDA_DIM_GRID_1D (var_ANY)
    } else if (bounds_count == 6) { /* 2D CUDA withloop */
        INDENT;
        INDENT;
        CUDA_DIM_GRID_2D (var_ANY)
    } else {
        if (bounds_count == 9) { /* 3D CUDA withloop */
            INDENT;
            INDENT;
            CUDA_DIM_GRID_3D (var_ANY)
        } else if (bounds_count == 12) { /* 4D CUDA withloop */
            INDENT;
            INDENT;
            CUDA_DIM_GRID_4D (var_ANY)
        } else {
            INDENT;
            INDENT;
            CUDA_DIM_GRID_5D (var_ANY)
        }
    }

    INDENT;
    INDENT;
    /*
      fprintf( global.outfile, "unsigned int timer;\n");
      fprintf( global.outfile, "cutCreateTimer(&timer);\n");
      fprintf( global.outfile, "cutStartTimer(timer);\n");
    */
    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_ST_GLOBALFUN_AP( char *funname,
 *                                     unsigned int vararg_cnt,
 *                                     char **vararg)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_ST_GLOBALFUN_AP (char *funname, unsigned int vararg_cnt, char **vararg)
{
    int dim, j;
    unsigned int i;
    char *basetype;

    DBUG_ENTER ();

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
    fprintf (global.outfile,
             "SAC_TR_GPU_PRINT (\"   kernel name \\\"%s\\\"\\n\");\n",
             funname); 
    fprintf (global.outfile, "SAC_PF_BEGIN_CUDA_KNL ();\n");
    fprintf (global.outfile, "%s<<<1, 1>>>(", funname);
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
    if (STReq (global.config.cuda_alloc, "cuman")
        || STReq (global.config.cuda_alloc, "cumanp")) {
        fprintf (global.outfile, "cudaDeviceSynchronize ();\n");
    }
    fprintf (global.outfile, "SAC_PF_END_CUDA_KNL ();\n");

    /*
      fprintf( global.outfile, "cutStopTimer(timer);\n");
      fprintf( global.outfile, "fprintf(stderr,\"%s: %%f\\n\",
      cutGetTimerValue(timer));\n", funname); fprintf( global.outfile,
      "cutResetTimer(timer);\n");
    */
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
            DBUG_UNREACHABLE ("Invalid index found!");
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
            DBUG_UNREACHABLE (
              "Invalid combination of array dimension and dimension index!");
        }
    } else {
        DBUG_UNREACHABLE ("Invalid array dimension found!");
    }

    fprintf (global.outfile, "SAC_ND_WRITE( %s, %d) = SAC_ND_READ( %s, 0);\n", iv_NT,
             wlids_dim_pos, wlids_NT);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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
            DBUG_UNREACHABLE ("Illegal dimension position found!");
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
            DBUG_UNREACHABLE ("Illegal dimension position found!");
        }
    } else {
        DBUG_UNREACHABLE ("Illegal dimension found!");
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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
            DBUG_UNREACHABLE ("Illegal dimension position found!");
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
            DBUG_UNREACHABLE ("Illegal dimension position found!");
        }
    } else {
        DBUG_UNREACHABLE ("Illegal dimension found!");
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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

    DBUG_RETURN ();
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

    DBUG_ENTER ();

#define CUDA_MEM_TRANSFER
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_MEM_TRANSFER

    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_SIZE( %s) == SAC_ND_A_SIZE( %s)", to_NT, from_NT),
                 ASSURE_TEXT ("cudaMemcpy: Destionation and source arrays "
                              "should have equal sizes!"));

    INDENT;
    if (STReq (global.config.cuda_alloc, "cureg")) {
        fprintf (global.outfile, "SAC_ND_CUDA_PIN(");
        if (STReq (direction, "cudaMemcpyHostToDevice")) {
            fprintf (global.outfile, "%s, ", from_NT);
        } else if (STReq (direction, "cudaMemcpyDeviceToHost")) {
            fprintf (global.outfile, "%s, ", to_NT);
        } else {
            CTIerrorInternal ("CUDA transfer direction is not supported: `%s`!", direction);
        }
        fprintf (global.outfile, "%s);", basetype);
    }

    fprintf (global.outfile,
             "SAC_TR_GPU_PRINT (\"%s size %%d %s -> %s\\n\", SAC_ND_A_SIZE( %s));",
             direction, from_NT, to_NT, from_NT);

    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER(%s, %s, %s, %s)", to_NT, from_NT,
             basetype, direction);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_MEM_TRANSFER_START( char *to_NT, char *from_NT,
 *                                           char *basetype, char *direction)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_MEM_TRANSFER_START (char *to_NT, char *from_NT, char *basetype, char *direction)
{

    DBUG_ENTER ();

#define CUDA_MEM_TRANSFER_START
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_MEM_TRANSFER_START

    // we use existing MEM_TRANSFER ICM, we just append a new ICM.
    ICMCompileCUDA_MEM_TRANSFER (to_NT, from_NT, basetype, direction);

    fprintf (global.outfile, "\n");
    INDENT;
    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER_SYNC_START()");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_MEM_TRANSFER_END( char *to_NT, char *from_NT,
 *                                           char *basetype, char *direction)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_MEM_TRANSFER_END (char *var_NT)
{

    DBUG_ENTER ();

#define CUDA_MEM_TRANSFER_END
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_MEM_TRANSFER_END

    INDENT;
    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER_SYNC_END(%s)", var_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_MEM_PREFETCH( char *var_NT, char *basetype, int device)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_MEM_PREFETCH (char *var_NT, char *basetype, int device)
{
    DBUG_ENTER ();

#define CUDA_MEM_PREFETCH
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_MEM_PREFETCH

    INDENT;
    fprintf (global.outfile, "SAC_CUDA_MEM_PREFETCH(%s, %s, %d)", var_NT,
             basetype, device);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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

    DBUG_RETURN ();
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
    DBUG_ENTER ();

#define CUDA_PRF_IDX_MODARRAY_AxSxA__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_PRF_IDX_MODARRAY_AxSxA__DATA

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == 0", idx_ANY),
                     ASSURE_TEXT ("2nd argument of %s is not a scalar!",
                                  global.prf_name[F_idx_modarray_AxSxS]));
    }

    INDENT;
    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER_D2D( %s, ", to_NT);
    ReadScalar (idx_ANY, NULL, 0);
    fprintf (global.outfile, ", %s, %s)", val_array, basetype);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

#define CUDA_DECL_KERNEL_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_DECL_KERNEL_ARRAY

    sc = ICUGetShapeClass (var_NT);
    dim = DIM_NO_OFFSET (sdim);

    switch (sc) {
    case C_aks:
        INDENT;
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");
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
        DBUG_UNREACHABLE ("Non-AKS array found in CUDA kernel!");
        break;
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

#define CUDA_DECL_SHMEM_ARRAY
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_DECL_SHMEM_ARRAY

    sc = ICUGetShapeClass (var_NT);
    dim = DIM_NO_OFFSET (sdim);

    switch (sc) {
    case C_aks:
        INDENT;
        DBUG_ASSERT (dim >= 0, "illegal dimension found!");

        for (i = 0; i < dim; i++) {
            size *= shp[i];
        }
        fprintf (global.outfile, "__shared__ %s SAC_ND_A_FIELD( %s)[%d];\n", basetype,
                 var_NT, size);
        break;
    default:
        DBUG_UNREACHABLE ("Non-AKS shared memory array found in CUDA kernel!");
        break;
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

#define CUDA_SHMEM_BOUNDARY_CHECK
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_SHMEM_BOUNDARY_CHECK

    INDENT;
    fprintf (global.outfile,
             "SAC_ND_A_FIELD( %s) = ( ( SACp_ub_%d-%d) == SAC_ND_A_FIELD( %s))\n", to_NT,
             dim_pos, offset, idx_NT);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_COND_WL_ASSIGN( char *cond_NT, char *shmemidx_NT,
 *                                       char *shmem_NT, char *devidx_NT,
 *                                       char *devmem_NT)
 *
 *
 ******************************************************************************/

void
ICMCompileCUDA_COND_WL_ASSIGN (char *cond_NT, char *shmemidx_NT, char *shmem_NT,
                               char *devidx_NT, char *devmem_NT)
{
    DBUG_ENTER ();

#define CUDA_COND_WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_COND_WL_ASSIGN

    INDENT;
    fprintf (global.outfile, "if( NT_NAME( %s)) {\n", cond_NT);

    INDENT;
    INDENT;
    fprintf (global.outfile, "NT_NAME( %s)[NT_NAME( %s)] = NT_NAME( %s)[NT_NAME( %s)];\n",
             devmem_NT, devidx_NT, shmem_NT, shmemidx_NT);

    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
