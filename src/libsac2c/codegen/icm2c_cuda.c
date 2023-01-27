
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
#include "ctinfo.h"
#include "gpukernel_comp_funs.h"

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
                 global.config.cuda_opt_threads, global.config.cuda_opt_blocks);
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

    if (global.gpukernel)
        fprintf (global.outfile,
                 ", unsigned int* SAC_gkco_check_threadmapping_bitmask_dev");
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

    fprintf (global.outfile,
             "if (block.x * block.y * block.z * grid.x * grid.y * grid.z > 0) {\n");

    INDENT;
    INDENT;
    fprintf (global.outfile, "SAC_TR_GPU_PRINT (\"   kernel name \\\"%s\\\"\\n\");\n",
             funname);
    fprintf (global.outfile, "SAC_PF_BEGIN_CUDA_KNL ();\n");

    if (global.gpu_measure_kernel_time) {
        fprintf(global.outfile,
                "SAC_CUDA_MEASURE_KERNEL_TIME_START\n");
    }

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

    if (global.gpukernel)
        fprintf (global.outfile, ", SAC_gkco_check_threadmapping_bitmask_dev");
    fprintf (global.outfile, ");\n");

    if (global.gpu_measure_kernel_time) {
        fprintf(global.outfile,
                "SAC_CUDA_MEASURE_KERNEL_TIME_END\n");
    }

    if (STReq (global.config.cuda_alloc, "cuman")
        || STReq (global.config.cuda_alloc, "cumanp")) {
        fprintf (global.outfile, "cudaDeviceSynchronize ();\n");
    }
    fprintf (global.outfile, "SAC_PF_END_CUDA_KNL ();\n");

    fprintf (global.outfile, "SAC_CUDA_GET_LAST_KERNEL_ERROR();\n");
    fprintf (global.outfile, "} else {\n");
    fprintf (global.outfile,
             "SAC_TR_GPU_PRINT(\"Skipping kernel because it has no elements\");\n");
    fprintf (global.outfile, "SAC_PRAGMA_BITMASK_CHECK_NL\n");
    fprintf (global.outfile, "}\n");
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

    fprintf (global.outfile, "\n\n");
    GKCOcompCheckEnd ();

    INDENT
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_THREAD_SPACE (node *spap, unsigned int bounds_count, char
 ***var_ANY)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_THREAD_SPACE (node *spap, unsigned int bounds_count, char **var_ANY)
{
    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (spap) == N_spap),
                 "N_spap expected in ICMCompileCUDA_THREAD_SPACE");

    INDENT
    fprintf (global.outfile, "\n{\n\n");
    GKCOcompHostKernelPragma (spap, bounds_count, var_ANY);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_INDEX_SPACE (node *spap, unsigned int bounds_count, char
 ***var_ANY)
 *
 *
 ******************************************************************************/
void
ICMCompileCUDA_INDEX_SPACE (node *spap, unsigned int bounds_count, char **var_ANY)
{
    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (spap) == N_spap),
                 "N_spap expected in ICMCompileCUDA_INDEX_SPACE");

    GKCOcompGPUDkernelPragma (spap, bounds_count, var_ANY);

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
    fprintf (global.outfile, "\n}\n\n");

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
            CTIerrorInternal ("CUDA transfer direction is not supported: `%s`!",
                              direction);
        }
        fprintf (global.outfile, "%s);", basetype);
    }

    fprintf (global.outfile,
             "SAC_TR_GPU_PRINT (\"%s size %%d %s -> %s\\n\", SAC_ND_A_SIZE( %s));\n",
             direction, from_NT, to_NT, from_NT);

    if (STReq (direction, "cudaMemcpyHostToDevice")) {
        fprintf (global.outfile, "SAC_PF_BEGIN_CUDA_HtoD()\n");
    } else if (STReq (direction, "cudaMemcpyDeviceToHost")) {
        fprintf (global.outfile, "SAC_PF_BEGIN_CUDA_DtoH()\n");
    } else {
        CTIerrorInternal ("CUDA transfer direction is not supported: `%s`!", direction);
    }

    fprintf (global.outfile, "SAC_CUDA_MEM_TRANSFER(%s, %s, %s, %s)\n", to_NT, from_NT,
             basetype, direction);

    if (STReq (direction, "cudaMemcpyHostToDevice")) {
        fprintf (global.outfile, "SAC_PF_END_CUDA_HtoD()\n");
    } else if (STReq (direction, "cudaMemcpyDeviceToHost")) {
        fprintf (global.outfile, "SAC_PF_END_CUDA_DtoH()\n");
    } else {
        CTIerrorInternal ("CUDA transfer direction is not supported: `%s`!", direction);
    }

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
ICMCompileCUDA_MEM_TRANSFER_START (char *to_NT, char *from_NT, char *basetype,
                                   char *direction)
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
    fprintf (global.outfile, "SAC_CUDA_MEM_PREFETCH(%s, %s, %d)", var_NT, basetype,
             device);

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
        ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == (0)", idx_ANY),
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

    DBUG_PRINT ("create cuda assign of %s to %s", to_NT, from_NT);

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
