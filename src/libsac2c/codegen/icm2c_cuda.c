
#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_cuda.h"

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
    fprintf (global.outfile, "__global__ void %s(", funname);

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
        fprintf (global.outfile, "SAC_CUDA_PARAM( %s, %s)", vararg[i + 2], basetype);

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
 * description:
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
 * description:
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
 * description:
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
 * description:
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
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_GLOBALFUN_AP (char *funname, int vararg_cnt, char **vararg)
{
    int dim, cnt, i, j;
    char *basetype;

    DBUG_ENTER ("ICMCompileCUDA_FUN_AP");

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
        fprintf (global.outfile, "SAC_CUDA_ARG( %s, %s)", vararg[i + 3], basetype);

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
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_GRID_BLOCK( int bounds_count, char **var_ANY)
 *
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_GRID_BLOCK (int bounds_count, char **var_ANY)
{
    int array_dim;

    DBUG_ENTER ("ICMCompileCUDA_GRID_BLOCK");

#define CUDA_GRID_BLOCK
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_GRID_BLOCK

    array_dim = bounds_count / 2;

    INDENT;
    fprintf (global.outfile, "{\n");

    if (array_dim == 1) {
        INDENT;
        INDENT;
        fprintf (global.outfile, "dim3 grid((%s-%s)/LBLOCKSZ+1);\n", var_ANY[0],
                 var_ANY[1]);
        INDENT;
        INDENT;
        fprintf (global.outfile, "dim3 block(LBLOCKSZ);");
    } else if (array_dim == 2) {
        INDENT;
        INDENT;
        fprintf (global.outfile, "dim3 grid((%s-%s)/SBLOCKSZ+1, (%s-%s)/SBLOCKSZ+1);\n",
                 var_ANY[0], var_ANY[2], var_ANY[1], var_ANY[3]);
        INDENT;
        INDENT;
        fprintf (global.outfile, "dim3 block(SBLOCKSZ,SBLOCKSZ);");
    } else {
        if (array_dim == 3) {
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 grid((%s-%s), (%s-%s));\n", var_ANY[2],
                     var_ANY[5], var_ANY[1], var_ANY[4]);
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 block((%s-%s));", var_ANY[0], var_ANY[3]);
        } else if (array_dim == 4) {
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 grid((%s-%s), (%s-%s));\n", var_ANY[3],
                     var_ANY[7], var_ANY[2], var_ANY[6]);
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 block((%s-%s), (%s-%s));", var_ANY[1],
                     var_ANY[5], var_ANY[0], var_ANY[4]);
        } else {
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 grid((%s-%s), (%s-%s));\n", var_ANY[4],
                     var_ANY[9], var_ANY[3], var_ANY[8]);
            INDENT;
            INDENT;
            fprintf (global.outfile, "dim3 block((%s-%s), (%s-%s), (%s-%s));", var_ANY[2],
                     var_ANY[7], var_ANY[1], var_ANY[6], var_ANY[0], var_ANY[5]);
        }
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_WLIDS( char *wlids_NT, int wlids_NT_dim,
 *                              int array_dim, int wlids_dim,
 *                              char *hasstepwidth)
 *
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_WLIDS (char *wlids_NT, int wlids_NT_dim, int array_dim, int wlids_dim_pos,
                      char *hasstepwidth)
{
    DBUG_ENTER ("ICMCompileCUDA_WLIDS");

#define CUDA_WLIDS
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_WLIDS

    if (array_dim <= 2) {
        INDENT;
        if (STReq (hasstepwidth, "true")) {
            fprintf (global.outfile,
                     "SAC_CUDA_WLIDS_%dD_SW_%d( %s, %d, SACp_step_%d, SACp_width_%d, "
                     "SACp_lb_%d, SACp_ub_%d)\n",
                     array_dim, wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos,
                     wlids_dim_pos, wlids_dim_pos, wlids_dim_pos);
        } else {
            fprintf (global.outfile,
                     "SAC_CUDA_WLIDS_%dD_%d( %s, %d, SACp_lb_%d, SACp_ub_%d)\n",
                     array_dim, wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos,
                     wlids_dim_pos);
        }
    } else {
        INDENT;
        if (STReq (hasstepwidth, "true")) {
            fprintf (global.outfile,
                     "SAC_CUDA_WLIDS_ND_SW_%d( %s, %d, SACp_step_%d, SACp_width_%d, "
                     "SACp_lb_%d, SACp_ub_%d)\n",
                     wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos,
                     wlids_dim_pos, wlids_dim_pos);
        } else {
            fprintf (global.outfile,
                     "SAC_CUDA_WLIDS_ND_%d( %s, %d, SACp_lb_%d, SACp_ub_%d)\n",
                     wlids_dim_pos, wlids_NT, wlids_NT_dim, wlids_dim_pos, wlids_dim_pos);
        }
    }
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
 *   void ICMCompileCUDA_WL_ASSIGN( char *val_NT, int val_sdim,
 *                                  char *to_NT, int to_sdim,
 *                                  char *off_NT)
 *
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_WL_ASSIGN (char *val_NT, int val_sdim, char *to_NT, int to_sdim,
                          char *off_NT)
{
    int val_dim = DIM_NO_OFFSET (val_sdim);

    DBUG_ENTER ("ICMCompileCUDA_WL_ASSIGN");

#define CUDA_WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_WL_ASSIGN

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
 * description:
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
 * description:
 *
 ******************************************************************************/
void
ICMCompileCUDA_WL_SUBALLOC (char *sub_NT, int sub_dim, char *to_NT, int to_dim,
                            char *off_NT)
{
    int sdim;
    int tdim;

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

    int i;
    for (i = sdim; i < tdim; i++) {
        fprintf (global.outfile, " * SAC_ND_A_MIRROR_SHAPE(%s, %d)", to_NT, i);
    }

    fprintf (global.outfile, ";\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_PRF_IDX_SEL__DATA( char *to_NT, int to_sdim,
 *                                          char *from_NT, int from_sdim,
 *                                          char *idx_ANY, char *basetype)
 *
 * description:
 *
 ******************************************************************************/
/*
void ICMCompileCUDA_PRF_IDX_SEL__DATA( char *to_NT, int to_sdim,
                                       char *from_NT, int from_sdim,
                                       char *idx_ANY, char *basetype)
{
  int to_dim = DIM_NO_OFFSET( to_sdim);

  DBUG_ENTER( "ICMCompileCUDA_PRF_IDX_SEL__DATA");

#define CUDA_PRF_IDX_SEL__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_PRF_IDX_SEL__DATA

  if (idx_ANY[0] == '(') {
    ASSURE_TYPE_ASS(
      fprintf( global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
    ,
      fprintf( global.outfile, "1st argument of %s is not a scalar!",
                        global.prf_name[F_idx_sel]);
    );
  }

  if (to_dim == 0) {
    INDENT;
    fprintf( global.outfile, "SAC_CUDA_MEM_TRANSFER_AxS(%s, ", to_NT);
    ReadScalar( idx_ANY, NULL, 0);
    fprintf( global.outfile, ", %s, %s)", from_NT, basetype);

  }
  DBUG_VOID_RETURN;
}
*/

/******************************************************************************
 *
 * function:
 *   void ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxS__DATA( char *to_NT,
 *                                                     int to_sdim,
 *                                                     char *from_NT,
 *                                                     int from_sdim,
 *                                                     char *idx_ANY,
 *                                                     char *val_scalar,
 *                                                     char *basetype)
 *
 * description:
 *
 ******************************************************************************/
/*
void ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxS__DATA( char *to_NT,
                                                  int to_sdim,
                                                  char *from_NT,
                                                  int from_sdim,
                                                  char *idx_ANY,
                                                  char *val_scalar,
                                                  char *basetype)
{
  DBUG_ENTER( "ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxS__DATA");

#define CUDA_PRF_IDX_MODARRAY_AxSxS__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef CUDA_PRF_IDX_MODARRAY_AxSxS__DATA

  if (idx_ANY[0] == '(') {
    ASSURE_TYPE_ASS(
      fprintf( global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
    ,
      fprintf( global.outfile, "2nd argument of %s is not a scalar!",
               global.prf_name[F_idx_modarray_AxSxS]);
    );
  }

  INDENT;
  fprintf( global.outfile, "SAC_CUDA_MEM_TRANSFER_SxA(%s, ", to_NT);
  ReadScalar( idx_ANY, NULL, 0);
  fprintf( global.outfile, ",");
  ReadScalar( val_scalar, NULL, 0);
  fprintf( global.outfile, ", %s)", basetype);


  DBUG_VOID_RETURN;
}
*/

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
 * description:
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
