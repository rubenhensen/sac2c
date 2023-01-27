#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_wl.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "system.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"

/******************************************************************************
 *
 * Function:
 *   void PrintTraceICM( char *to_NT, char *idx_vec_NT,
 *                       int dims, char **idxs_scl_NT,
 *                       char *operation)
 *
 * Description:
 *
 *
 *****************************************************************************/

static void
PrintTraceICM (char *to_NT, char *idx_vec_NT, int dims, char **idxs_scl_NT,
               char *operation)
{
    int i;

    DBUG_ENTER ();

    indout ("SAC_TR_WL_PRINT( (\"index vector [%%d");
    for (i = 1; i < dims; i++) {
        out (", %%d");
    }
    out ("]");
    out (" -- %s", operation);
    out ("\", SAC_ND_READ( %s, 0)", idxs_scl_NT[0]);

    for (i = 1; i < dims; i++) {
        out (", SAC_ND_READ( %s, 0)", idxs_scl_NT[i]);
    }
    out ("));\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void DefineShapeFactor( char *to_NT, int to_sdim, int current_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
DefineShapeFactor (char *to_NT, int to_sdim, int current_dim)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int i;

    DBUG_ENTER ();

    indout ("SAC_WL_SHAPE_FACTOR( %s, %d) = 1", to_NT, current_dim);
    if (to_dim >= 0) {
        for (i = current_dim + 1; i < to_dim; i++) {
            out (" * SAC_ND_A_SHAPE( %s, %d)", to_NT, i);
        }

        out (";\n");
    } else {
        out (";\n");
        FOR_LOOP_BEGIN ("SAC_i = %d; SAC_i < SAC_ND_A_DIM( %s); SAC_i++", current_dim + 1,
                        to_NT)
            ;
            indout ("SAC_WL_SHAPE_FACTOR( %s, %d) *= SAC_ND_A_SHAPE( %s, SAC_i);\n",
                    to_NT, current_dim, to_NT);
        FOR_LOOP_END ();
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_WL_GENARRAY__SHAPE_id_arr( char *to_NT, int to_sdim,
 *                                                char *shp_NT,
 *                                                int val_size, char **val_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_WL_GENARRAY__SHAPE_id_arr( to_NT, to_sdim, shp_NT, val_size, val_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_WL_GENARRAY__SHAPE_id_arr (char *to_NT, int to_sdim, char *shp_NT,
                                        int val_size, char **vals_ANY)
{
    int i;

    DBUG_ENTER ();

#define ND_WL_GENARRAY__SHAPE_id_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_id_arr

    indout ("SAC_TR_PRF_PRINT( (\"ND_WL_GENARRAY__SHAPE( %s, %d, %s, ...)\"))\n", to_NT,
            to_sdim, shp_NT);

    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == (1)", shp_NT),
                 ASSURE_TEXT ("Shape of genarray with-loop has (dim != 1)!"));

    /*
     * CAUTION:
     * 'val_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!!
     */
    for (i = 0; i < val_size; i++) {
        if (vals_ANY[i][0] == '(') {
            ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == (0)", vals_ANY[i]),
                         ASSURE_TEXT ("Shape of genarray with-loop has (dim != 1)!"));
        }
    }

    Set_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, NULL, ReadId, vals_ANY, val_size, NULL,
               NULL, ReadConstArray_Str);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_WL_GENARRAY__SHAPE_id_id( char *to_NT, int to_sdim,
 *                                               char *shp_NT,
 *                                               char *val_NT, int val_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_WL_GENARRAY__SHAPE_id_id( to_NT, to_sdim, shp_NT, val_NT, val_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_WL_GENARRAY__SHAPE_id_id (char *to_NT, int to_sdim, char *shp_NT,
                                       char *val_NT, int val_sdim)
{
    int val_dim = DIM_NO_OFFSET (val_sdim);

    DBUG_ENTER ();

#define ND_WL_GENARRAY__SHAPE_id_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_id_id

    indout ("SAC_TR_PRF_PRINT("
            " (\"ND_WL_GENARRAY__SHAPE( %s, %d, ..., %s, %d)\"))\n",
            to_NT, to_sdim, val_NT, val_sdim);

    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == (1)", shp_NT),
                 ASSURE_TEXT ("Shape of genarray with-loop has (dim != 1)!"));

    Set_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, NULL, ReadId, val_NT, val_dim, DimId,
               SizeId, ShapeId);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_WL_GENARRAY__SHAPE_arr_id( char *to_NT, int to_sdim,
 *                                                int shp_size, char **shp_ANY,
 *                                                char *val_NT, int val_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_WL_GENARRAY__SHAPE_arr_id( to_NT, to_sdim, shp_size, shp_ANY,
 *                                 val_NT, val_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_WL_GENARRAY__SHAPE_arr_id (char *to_NT, int to_sdim, int shp_size,
                                        char **shp_ANY, char *val_NT, int val_sdim)
{
    int val_dim = DIM_NO_OFFSET (val_sdim);
    int i;

    DBUG_ENTER ();

#define ND_WL_GENARRAY__SHAPE_arr_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_arr_id

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!!
     */

    indout ("SAC_TR_PRF_PRINT( (\"ND_WL_GENARRAY__SHAPE( %s, %d, ..., %s, %d)\"))\n",
            to_NT, to_sdim, val_NT, val_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shp_ANY[i][0] == '(') {
            ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) == (0)", shp_ANY[i]),
                         ASSURE_TEXT ("Shape of genarray with-loop has (dim != 1)!"));
        }
    }

    Set_Shape (to_NT, to_sdim, shp_ANY, shp_size, NULL, NULL, ReadConstArray_Str, val_NT,
               val_dim, DimId, SizeId, ShapeId);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SCHEDULE__BEGIN( int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_SCHEDULE__BEGIN( dims)
 *
 ******************************************************************************/

void
ICMCompileWL_SCHEDULE__BEGIN (int dims)
{
    int i;

    DBUG_ENTER ();

#define WL_SCHEDULE__BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SCHEDULE__BEGIN

    indout ("{\n");
    global.indent++;

    for (i = 0; i < dims; i++) {
        indout ("int SAC_WL_MT_SCHEDULE_START( %d);\n", i);
        indout ("int SAC_WL_MT_SCHEDULE_STOP( %d);\n", i);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_DIST_SCHEDULE__BEGIN( int dims, bool is_distributable, char *to_NT)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_DIST_SCHEDULE__BEGIN( dims, is_distributable, to_NT)
 *
 ******************************************************************************/

void
ICMCompileWL_DIST_SCHEDULE__BEGIN (int dims, bool is_distributable, char *to_NT,
                                   char *to_basetype)
{
    int i;

    DBUG_ENTER ();

#define WL_DIST_SCHEDULE__BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_DIST_SCHEDULE__BEGIN

    indout ("{\n");
    global.indent++;

    if (is_distributable) {
        /* The with-loop is distributed if the array is distributed (i.e. not only
         * allocated in DSM memory but actually distributed). */
        IF_BEGIN ("SAC_ND_A_IS_DIST( %s) && !SAC_ND_A_IS_DSM( %s) && "
                  "SAC_DISTMEM_exec_mode != SAC_DISTMEM_exec_mode_sync",
                  to_NT, to_NT)
            ;
            indout ("SAC_RuntimeError( \"Tried to execute distributed with-loop in "
                    "non-synchronous execution mode (%%s is distributed).\", NT_STR( "
                    "%s));\n",
                    to_NT);
        IF_END ();

        indout ("const bool SAC_WL_IS_DISTRIBUTED = SAC_ND_A_IS_DIST( %s) && "
                "!SAC_ND_A_IS_DSM( %s) && SAC_DISTMEM_exec_mode == "
                "SAC_DISTMEM_exec_mode_sync;\n",
                to_NT, to_NT);
        indout ("const int SAC_WL_DIST_DIM0_SIZE = SAC_ND_A_SHAPE( %s, 0);\n", to_NT);
        indout ("const uintptr_t SAC_WL_DIST_OFFS = SAC_ND_A_OFFS( %s);\n", to_NT);
        indout (
          "const size_t SAC_WL_DIST_BYTES = SAC_ND_A_FIRST_ELEMS( %s) * sizeof( %s);\n",
          to_NT, to_basetype);

        IF_BEGIN ("SAC_WL_IS_DISTRIBUTED")
            ;
            indout ("SAC_TR_DISTMEM_PRINT( \"Executing distributed with-loop (arr: "
                    "%%s).\", NT_STR( %s));\n",
                    to_NT);
            /* Perform the switch before the barrier so that the profiling of the
             * execution modes is correct. */
            indout ("SAC_DISTMEM_SWITCH_TO_DIST_EXEC();\n");
            /* TODO: Maybe we can get rid of this barrier in some cases? It is required
             * because of a possible anti-dependency only. */
            indout ("SAC_DISTMEM_BARRIER();\n");
            indout ("SAC_DISTMEM_ALLOW_DIST_WRITES();\n");
        IF_END ();
        ELSE_BEGIN ()
            ;
            indout ("SAC_TR_DISTMEM_PRINT( \"Executing non-distributed with-loop (arr: "
                    "%%s, arr distributed: %%d, in replicated exec mode? %%d).\", "
                    "NT_STR( %s), SAC_ND_A_IS_DIST( %s), SAC_DISTMEM_exec_mode == "
                    "SAC_DISTMEM_exec_mode_sync);\n",
                    to_NT, to_NT);
        ELSE_END ();
    } else {
        indout ("const bool SAC_WL_IS_DISTRIBUTED = FALSE;\n");
        indout ("const int SAC_WL_DIST_DIM0_SIZE = 0;\n");
        indout ("SAC_TR_DISTMEM_PRINT( \"Executing non-distributable with-loop (arr: "
                "%%s).\", NT_STR( %s));\n",
                to_NT);
        indout ("const uintptr_t SAC_WL_DIST_OFFS = 0;\n");
        indout ("const size_t SAC_WL_DIST_BYTES = 0;\n");
    }

    for (i = 0; i < dims; i++) {
        indout ("int SAC_WL_MT_SCHEDULE_START( %d);\n", i);
        indout ("int SAC_WL_MT_SCHEDULE_STOP( %d);\n", i);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL3_SCHEDULE__BEGIN( int lb, char *idx_nt, int ub, int chunksz, bool
 *need_unroll)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL3_SCHEDULE__BEGIN( dims)
 *
 ******************************************************************************/

void
ICMCompileWL3_SCHEDULE__BEGIN (int lb, char *idx_nt, int ub, int chunksz,
                               bool need_unroll)
{
    DBUG_ENTER ();

#define WL3_SCHEDULE__BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL3_SCHEDULE__BEGIN

    if (need_unroll) {
        indout ("#pragma unroll\n");
    }

    indout ("for( SAC_ND_A_FIELD( %s) = %d; "
            "SAC_ND_A_FIELD( %s) < %d; "
            "SAC_ND_A_FIELD( %s) += %d)\n",
            idx_nt, lb, idx_nt, ub, idx_nt, chunksz);
    indout ("{\n");
    global.indent++;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_DECLARE_SHAPE_FACTOR( char *to_NT, int to_sdim,
 *                                            char *idx_vec_NT, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_DECLARE_SHAPE_FACTOR( to_NT, to_sdim, idx_vec_NT, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_DECLARE_SHAPE_FACTOR (char *to_NT, int to_sdim, char *idx_vec_NT, int dims)
{
    int i;

    DBUG_ENTER ();

#define WL_DECLARE_SHAPE_FACTOR
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_DECLARE_SHAPE_FACTOR

    for (i = 0; i < dims; i++) {
        indout ("int SAC_WL_SHAPE_FACTOR( %s, %d);\n", to_NT, i);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_DEFINE_SHAPE_FACTOR( char *to_NT, int to_sdim,
 *                                          char *idx_vec_NT, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_DEFINE_SHAPE_FACTOR( to_NT, to_sdim, idx_vec_NT, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_DEFINE_SHAPE_FACTOR (char *to_NT, int to_sdim, char *idx_vec_NT, int dims)
{
    int i;

    DBUG_ENTER ();

#define WL_DEFINE_SHAPE_FACTOR
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_DEFINE_SHAPE_FACTOR

    indout ("{\n");
    global.indent++;

    indout ("int SAC_i;\n"); /* for DefineShapeFactor() !!! */
    for (i = 0; i < dims; i++) {
        DefineShapeFactor (to_NT, to_sdim, i);
    }

    global.indent--;
    indout ("}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SCHEDULE__END( int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_SCHEDULE__END( dims)
 *
 ******************************************************************************/

void
ICMCompileWL_SCHEDULE__END (int dims)
{
    DBUG_ENTER ();

#define WL_SCHEDULE__END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SCHEDULE__END

    if (global.backend == BE_distmem) {
        IF_BEGIN ("SAC_WL_IS_DISTRIBUTED")
            ;
            indout (
              "SAC_TR_DISTMEM_PRINT( \"Done executing distributed with-loop.\");\n");
            indout ("SAC_DISTMEM_INVAL_CACHE( SAC_WL_DIST_OFFS, SAC_WL_DIST_BYTES);\n");
            indout ("SAC_DISTMEM_BARRIER();\n");
            /* Perform the switch after the barrier so that the profiling of the execution
             * modes is correct. */
            indout ("SAC_DISTMEM_SWITCH_TO_SYNC_EXEC();\n");
            indout ("SAC_DISTMEM_FORBID_DIST_WRITES();\n");
        IF_END ();
        ELSE_BEGIN ()
            ;
            indout (
              "SAC_TR_DISTMEM_PRINT( \"Done executing non-distributed with-loop.\");\n");
        ELSE_END ();
    }

    global.indent--;
    indout ("}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL3_SCHEDULE__END( char *idx_nt)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL3_SCHEDULE__END( dims)
 *
 ******************************************************************************/

void
ICMCompileWL3_SCHEDULE__END (char *idx_nt)
{
    DBUG_ENTER ();

#define WL3_SCHEDULE__END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL3_SCHEDULE__END

    global.indent--;
    indout ("}\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SUBALLOC( char *sub_NT, char *to_NT, char *off_NT)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_SUBALLOC( sub_NT, to_NT, off_NT)
 *
 ******************************************************************************/

void
ICMCompileWL_SUBALLOC (char *sub_NT, char *to_NT, char *off_NT)
{
    DBUG_ENTER ();

#define WL_SUBALLOC
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SUBALLOC

    indout ("SAC_ND_GETVAR(%s, SAC_ND_A_FIELD( %s)) "
            "= SAC_ND_GETVAR( %s, SAC_ND_A_FIELD( %s))+SAC_ND_READ( %s, 0);\n",
            sub_NT, sub_NT, to_NT, to_NT, off_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_DISTMEM_SUBALLOC( char *sub_NT, char *to_NT, char *off_NT)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_DISTMEM_SUBALLOC( sub_NT, to_NT, off_NT)
 *
 *   Creates code for a suballoc where the outer and/or the inner array
 *   is distributable.
 *
 ******************************************************************************/

void
ICMCompileWL_DISTMEM_SUBALLOC (char *sub_NT, char *to_NT, char *off_NT)
{
    DBUG_ENTER ();

#define WL_DISTMEM_SUBALLOC
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_DISTMEM_SUBALLOC

    /* Check whether the outer array is distributable at compile time. */
    if (ICUGetDistributedClass (to_NT) == C_distr) {
        /* The inner array has to be DSM (not distributed but possibly allocated in DSM
         * memory). */
        if (ICUGetDistributedClass (sub_NT) != C_distmem) {
            indout ("SAC_RuntimeError( \"Suballoc with DIS outer array ( %%s) but "
                    "non-DSM inner array ( %%s).\", NT_STR( %s), NT_STR( %s));\n",
                    to_NT, sub_NT);
        }

        /* Check whether the outer array is actually distributed at runtime.
         * We can't check whether a possible outer with-loop is distributed because
         * it may have been unrolled. */
        IF_BEGIN ("SAC_ND_A_IS_DIST( %s)", to_NT)
            ;
            /* The outer array (and possibly with-loop) are distributed. */
            /* The inner array is sub-allocated in DSM memory (but not distributed). */
            indout ("SAC_ND_A_DESC_IS_DIST( %s) = SAC_ND_A_MIRROR_IS_DIST( %s) = TRUE;\n",
                    sub_NT, sub_NT);
            indout ("SAC_TR_DISTMEM_PRINT( \"%%s is allocated in DSM memory because it "
                    "is sub-allocated from the distributed variable %%s.\", NT_STR( %s), "
                    "NT_STR( %s));\n",
                    sub_NT, to_NT);

            /* Get the pointer to the start of the sub-allocated array. */
            indout ("SAC_ND_GETVAR(%s, SAC_ND_A_FIELD( %s)) = "
                    "SAC_DISTMEM_ELEM_POINTER(SAC_ND_A_OFFS( %s), SAC_NT_CBASETYPE( %s),"
                    "                         SAC_ND_A_FIRST_ELEMS( %s), SAC_ND_READ( "
                    "%s, 0));\n",
                    sub_NT, sub_NT, to_NT, to_NT, to_NT, off_NT);
        IF_END ();
        ELSE_BEGIN ()
            ;
            /* The outer array (and possibly with-loop) are not distributed. Do nothing
             * special. */
            ICMCompileWL_SUBALLOC (sub_NT, to_NT, off_NT);
        ELSE_END ();
    } else {
        /* The outer array (and possibly with-loop) are not distributed. Do nothing
         * special. */
        ICMCompileWL_SUBALLOC (sub_NT, to_NT, off_NT);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_MODARRAY_SUBSHAPE( char *sub_NT, char *idx_NT,
 *                                        int dims, char *to_NT)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_MODARRAY_SUBSHAPE( sub_NT, idx_NT, dims, to_NT)
 *
 ******************************************************************************/

void
ICMCompileWL_MODARRAY_SUBSHAPE (char *sub_NT, char *idx_NT, int dims, char *to_NT)
{
    shape_class_t sub_sc = ICUGetShapeClass (sub_NT);
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int cnt;

    DBUG_ENTER ();

#define WL_MODARRAY_SUBSHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_MODARRAY_SUBSHAPE

    switch (sub_sc) {
    case C_scl:
    case C_aks:
        out ("SAC_NOOP()\n");
        break;
    case C_akd:
        DBUG_ASSERT (to_sc == C_akd || to_sc == C_aud,
                     "applied WL_MODARRAY_SUBSHAPE to AKD subvar but "
                     "non AKD/AUD resultvar. cannot handle this!");
        /* Start a fresh block.  */
        BLOCK_BEGIN ("int SAC_size = 1;")
            ;
            /*
             * update shape elements and calculate size.
             *
             * NOTE: we have to use DESC_SHAPE here as the
             *       selection index is not constant. But we
             *       do know that it is a AKD/AUD so this
             *       should exist;)
             */
            for (cnt = 0; cnt < dims; cnt++) {
                SET_SHP_AKD (sub_NT, cnt,
                             out ("SAC_ND_A_DESC_SHAPE( %s, SAC_ND_A_DIM( %s) - %d)",
                                  to_NT, to_NT, dims - cnt));
                out ("SAC_size *= SAC_ND_A_SHAPE( %s, %d);", sub_NT, cnt);
            }

            /* Update size.  */
            SET_SIZE (sub_NT, out ("SAC_size"););
        BLOCK_END ();
        break;

    case C_aud:
        DBUG_ASSERT (to_sc == C_aud || to_sc == C_akd,
                     "applied WL_MODARRAY_SUBSHAPE to AUD subvar but "
                     "non AUD/AKD resultvar. cannot handle this!");
        /*
         * set shape and calculate size
         *
         * NOTE: we use SAC_ND_A_DESC_SHAPE here, as the index
         *       is not constant. but we know that the to_sc
         *       is an AKD/AUD, so the descriptor exists
         */
        BLOCK_BEGIN ("int SAC_size = 1;")
            ;
            FOR_LOOP_BEGIN ("int SAC_i = 0; SAC_i < SAC_ND_A_DIM( %s); SAC_i++", sub_NT)
                ;
                SET_SHP_AUD (sub_NT, out ("SAC_i"),
                             out ("SAC_ND_A_DESC_SHAPE( %s, SAC_ND_A_DIM( %s)"
                                  " - SAC_ND_A_DIM( %s) + SAC_i)",
                                  to_NT, to_NT, sub_NT));
                indout ("SAC_size *= SAC_ND_A_SHAPE( %s, SAC_i);\n", sub_NT);
            FOR_LOOP_END ();

            SET_SIZE (sub_NT, out ("SAC_size"););
        BLOCK_END ();
        break;

    default:
        DBUG_UNREACHABLE ("unknown shape class found!");
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN( char *val_NT, int val_sdim,
 *                             char *to_NT, int to_sdim
 *                             char *idx_vec_NT, int dims,
 *                             char *off_NT,
 *                             char *copyfun)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( val_NT, val_sdim, to_NT, to_sdim,
 *              idx_vec_NT, dims, offset, copyfun )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (char *val_NT, int val_sdim, char *to_NT, int to_sdim,
                     char *idx_vec_NT, int dims, char *off_NT, char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int val_dim = DIM_NO_OFFSET (val_sdim);

    DBUG_ENTER ();

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_DIM( %s) "
                              "== (SAC_ND_A_DIM( %s) - SAC_ND_A_SIZE( %s))",
                              val_NT, to_NT, idx_vec_NT),
                 ASSURE_TEXT ("WL expression with illegal dimension found!"));

    ASSURE_TYPE (ASSURE_COND ("SAC_ND_A_SIZE( %s) "
                              "== SAC_WL_SHAPE_FACTOR( %s, %d)",
                              val_NT, to_NT, (dims < 0 ? 0 : dims - 1)),
                 ASSURE_TEXT ("WL expression with illegal size found!"));

    if (val_dim == 0 || to_dim == dims) {
        indout ("SAC_ND_WRITE_READ_COPY( %s, SAC_ND_READ( %s, 0), %s, 0, %s);\n", to_NT,
                off_NT, val_NT, copyfun);
    } else if (global.backend == BE_distmem) { // TODO: optimize
        FOR_LOOP_BEGIN ("int SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++", val_NT)
            ;
            indout ("SAC_ND_WRITE_READ_COPY( %s, SAC_ND_READ( %s, 0) + SAC_i, "
                    "%s, SAC_i, %s);\n",
                    to_NT, off_NT, val_NT, copyfun);
        FOR_LOOP_END ();
    } else {
        FOR_LOOP_BEGIN ("int SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++", val_NT)
            ;
            indout ("SAC_ND_WRITE_READ_COPY( %s, SAC_ND_READ( %s, 0) + SAC_i, "
                    "%s, SAC_i, %s);\n",
                    to_NT, off_NT, val_NT, copyfun);
        FOR_LOOP_END ();
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD( char *to_NT, int to_sdim,
 *                           char *idx_vec_NT, int dims, char **idxs_scl_NT)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD( to_NT, to_sdim, idx_vec_NT, dims, [ idxs_scl_NT ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD (char *to_NT, int to_sdim, char *idx_vec_NT, int dims,
                   char **idxs_scl_NT)
{
    DBUG_ENTER ();

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

    PrintTraceICM (to_NT, idx_vec_NT, dims, idxs_scl_NT, "fold");
    indout ("/* fold operation */\n");

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_INIT_OFFSET( char *off_NT,
 *                                  char *to_NT, int to_sdim,
 *                                  char *idx_vec_NT, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *     WL_INIT_OFFSET( off_NT, to_NT, to_sdim, idx_vec_NT, dims)
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
ICMCompileWL_INIT_OFFSET (char *off_NT, char *to_NT, int to_sdim, char *idx_vec_NT,
                          int dims)
{
    int i;

    DBUG_ENTER ();

#define WL_INIT_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_INIT_OFFSET

    indout ("SAC_ND_WRITE( %s, 0)\n", off_NT);
    global.indent++;

    indout ("= SAC_WL_MT_SCHEDULE_START( 0)"
            " * SAC_WL_SHAPE_FACTOR( %s, %d)",
            to_NT, 0);

    for (i = 1; i < dims; i++) {
        out ("\n");
        indout ("+ SAC_WL_MT_SCHEDULE_START( %d)", i);
        out (" * SAC_WL_SHAPE_FACTOR( %s, %d)", to_NT, i);
    }

    out (";\n");
    global.indent--;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ADJUST_OFFSET( char *off_NT, int dim,
 *                                    char *to_NT, int to_sdim,
 *                                    char *idx_vec_NT,
 *                                    int dims, char **idxs_scl_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ADJUST_OFFSET( off_NT, dim, to_NT, to_sdim,
 *                     idx_vec_NT, dims, [ idxs_scl_NT ]* )
 *
 * remark:
 *   This ICM is needed (and usefull) in combination with NOOP N_WL..-nodes
 *   only! It uses the variable 'SAC_diff...' defined by the ICMs
 *   WL_..._NOOP_BEGIN.
 *
 ******************************************************************************/

void
ICMCompileWL_ADJUST_OFFSET (char *off_NT, int dim, char *to_NT, int to_sdim,
                            char *idx_vec_NT, int dims, char **idxs_scl_NT)
{
    DBUG_ENTER ();

#define WL_ADJUST_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ADJUST_OFFSET

    indout ("SAC_ND_WRITE( %s, 0) += SAC_WL_VAR( diff, %s)"
            " * SAC_WL_SHAPE_FACTOR( %s, %d);\n",
            off_NT, idxs_scl_NT[dim], to_NT, dim);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SET_OFFSET( char *off_NT,
 *                                 int dim, int first_block_dim,
 *                                 char *to_NT, int to_sdim,
 *                                 char *idx_vec_NT,
 *                                 int dims, char **idxs_scl_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_SET_OFFSET( off_NT, dim, first_block_dim, to_NT, to_sdim, idx_vec_NT,
 *                  dims, [ idxs_scl_NT ]* )
 *
 * remark:
 *   This ICM is needed (and usefull) in combination with multiple segments,
 *   (unrolling-)blocking or naive compilation only!!
 *   If the C compiler reports an undeclared 'SAC_start...' then there is
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
ICMCompileWL_SET_OFFSET (char *off_NT, int dim, int first_block_dim, char *to_NT,
                         int to_sdim, char *idx_vec_NT, int dims, char **idxs_scl_NT)
{
    int i;

    DBUG_ENTER ();

#define WL_SET_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SET_OFFSET

    indout ("SAC_ND_WRITE( %s, 0) \n", off_NT);
    global.indent++;

    indout ("= ");
    for (i = dims - 1; i > 0; i--) {
        out ("( SAC_ND_A_SHAPE( %s, %d) * ", to_NT, i);
    }
    out ("SAC_ND_READ( %s, 0)\n", idxs_scl_NT[0]);

    INDENT;
    for (i = 1; i < dims; i++) {
        if (i <= dim) {
            out ("+ SAC_ND_READ( %s, 0) )", idxs_scl_NT[i]);
        } else {
            if (i <= first_block_dim) {
                /*
                 * no blocking in this dimension
                 *  -> we use the start index of the current MT region
                 */
                out (" + SAC_WL_MT_SCHEDULE_START( %d) )", i);
            } else {
                /*
                 * blocking in this dimension
                 *  -> we use the first index of the current block
                 */
                out (" + SAC_WL_VAR( first, %s) )", idxs_scl_NT[i]);
            }
        }
    }
    out (" * SAC_WL_SHAPE_FACTOR( %s, %d);\n", to_NT, dims - 1);
    global.indent--;

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
