/*
 *
 * $Log$
 * Revision 3.49  2005/08/24 10:21:26  ktr
 * added support for explicit with-loop offsets
 *
 * Revision 3.48  2005/06/28 16:34:06  sah
 * removed a warning
 *
 * Revision 3.47  2005/06/22 20:48:15  sah
 * fixed WL_MODARRAY_SUBSHAPE and its
 * usage.
 *
 * Revision 3.46  2005/06/21 23:39:39  sah
 * added setting of modarray wls subvar descriptors
 * using new C-icm WL_MODARRAY_SUBSHAPE
 *
 * Revision 3.45  2005/06/16 22:28:53  sbs
 * bug in ASSERT macro eliinated.
 *
 * Revision 3.44  2005/06/10 15:56:11  sbs
 * changed WL_EMM_ASSIGN compilation. As second arg to the SHAPE_FACTOR h-icm
 * we now pass 0 whenever dims < 0. This should cover the AUD case properly
 * (cross fingers).
 *
 * Revision 3.43  2004/11/25 10:26:46  jhb
 * compile SACdevCamp 2k4
 *
 * Revision 3.42  2004/11/24 15:46:31  jhb
 * removed include my_dbug.h
 *
 * Revision 3.41  2004/10/28 17:50:22  khf
 * splitted WL_OFFSET into WL_OFFSET and WL_OFFSET_SHAPE_FACR
 * to avoid mixed declarations and code
 *
 * Revision 3.40  2004/10/05 17:26:53  khf
 * added INDENT in WL_OFFSET
 *
 * Revision 3.39  2004/08/13 16:38:37  khf
 * splitted WL_BEGIN_OFFSET into WL_SCHEDULE__BEGIN and
 * WL_OFFSET, added WL_SCHEDULE__END, removed WL_BEGIN_OFFSET,
 * WL_END_OFFSET, WL_BEGIN, and WL_END
 *
 * Revision 3.38  2004/08/05 16:12:09  ktr
 * added WL_INC_OFFSET and modified WL_EMM_ASSIGN which now resembles
 * WL_ASSIGN without incrementing the WL_OFFSET.
 *
 * Revision 3.37  2004/08/02 16:17:49  ktr
 * renamed ND_WL_GENARRAY__SHAPE_id into ND_WL_GENARRAY__SHAPE_id_id
 * renamed ND_WL_GENARRAY__SHAPE_arr into ND_WL_GENARRAY__SHAPE_arr_id
 * added ND_WL_GENARRAY__SHAPE_id_arr
 * added ND_WL_SUBALLOC
 * added ND_WL_EMM_ASSIGN
 *
 * Revision 3.36  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.35  2003/09/30 19:29:21  dkr
 * code brushed: Set_Shape() used
 *
 * Revision 3.34  2003/09/29 22:53:49  dkr
 * WLGenarray_Shape() modified
 *
 * Revision 3.33  2003/09/25 19:19:54  dkr
 * bug in WL_ASSIGN for old backend fixed
 *
 * Revision 3.32  2003/09/25 13:43:30  dkr
 * new argument 'copyfun' added to some ICMs.
 * ND_WRITE replaced by ND_WRITE_READ_COPY.
 *
 * Revision 3.31  2003/09/17 14:17:12  dkr
 * some function parameters renamed
 *
 * Revision 3.30  2003/09/17 12:57:46  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.29  2003/03/14 18:27:33  dkr
 * some fixes for old backend done
 *
 * Revision 3.28  2003/03/14 13:22:42  dkr
 * all arguments of WL-ICMs are tagged now
 *
 * Revision 3.27  2002/10/30 14:20:16  dkr
 * some new macros used
 *
 * Revision 3.26  2002/10/29 19:11:00  dkr
 * several bugs removed,
 * new macros for code generation used.
 *
 * Revision 3.25  2002/10/28 13:18:03  dkr
 * bug in WL_BEGIN__OFFSET fixed
 *
 * Revision 3.24  2002/10/28 09:25:04  dkr
 * bugs in WLGenarray_Shape() fixed
 *
 * Revision 3.23  2002/10/24 20:54:18  dkr
 * some ICMs modified in order to support dynamic shapes
 *
 * Revision 3.22  2002/10/07 23:35:45  dkr
 * some bugs with new backend fixed
 *
 * Revision 3.21  2002/08/06 12:18:58  dkr
 * ups, ... error in WL_BEGIN__OFFSET corrected
 *
 * Revision 3.20  2002/08/06 12:08:09  dkr
 * some cosmetical changes done
 *
 * Revision 3.19  2002/08/06 08:58:30  dkr
 * works also with old backend, now
 *
 * Revision 3.18  2002/08/05 20:42:10  dkr
 * ND_WL_GENARRAY__SHAPE... added
 *
 * Revision 3.17  2002/07/30 20:05:02  dkr
 * some comments corrected
 *
 * Revision 3.16  2002/07/15 14:43:59  dkr
 * bug in WL_ASSIGN__COPY fixed
 *
 * Revision 3.15  2002/07/12 23:10:50  dkr
 * bug in WL_ASSIGN__INIT fixed
 *
 * Revision 3.14  2002/07/12 18:52:46  dkr
 * some modifications for new backend done
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
 * [...]
 *
 */

#include <stdio.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_wl.h"

#include "dbug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

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

    DBUG_ENTER ("PrintTraceICM");

    INDENT;
    fprintf (global.outfile, "SAC_TR_WL_PRINT( (\"index vector [%%d");
    for (i = 1; i < dims; i++) {
        fprintf (global.outfile, ", %%d");
    }
    fprintf (global.outfile, "]");
    fprintf (global.outfile, " -- %s", operation);
    fprintf (global.outfile, "\", SAC_ND_READ( %s, 0)", idxs_scl_NT[0]);
    for (i = 1; i < dims; i++) {
        fprintf (global.outfile, ", SAC_ND_READ( %s, 0)", idxs_scl_NT[i]);
    }
    fprintf (global.outfile, "));\n");

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("DefineShapeFactor");

    INDENT;
    fprintf (global.outfile, "SAC_WL_SHAPE_FACTOR( %s, %d) = 1", to_NT, current_dim);
    if (to_dim >= 0) {
        for (i = current_dim + 1; i < to_dim; i++) {
            fprintf (global.outfile, " * SAC_ND_A_SHAPE( %s, %d)", to_NT, i);
        }
        fprintf (global.outfile, ";\n");
    } else {
        fprintf (global.outfile, ";\n");
        FOR_LOOP_INC (fprintf (global.outfile, "SAC_i");
                      , fprintf (global.outfile, "%d", current_dim + 1);
                      , fprintf (global.outfile, "SAC_ND_A_DIM( %s)", to_NT);, INDENT;
                      fprintf (global.outfile,
                               "SAC_WL_SHAPE_FACTOR( %s, %d)"
                               " *= SAC_ND_A_SHAPE( %s, SAC_i);\n",
                               to_NT, current_dim, to_NT););
    }

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileND_WL_GENARRAY__SHAPE_id_arr");

#define ND_WL_GENARRAY__SHAPE_id_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_id_arr

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_WL_GENARRAY__SHAPE( %s, %d, %s, ...)\"))\n",
             to_NT, to_sdim, shp_NT);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", shp_NT);
                     , fprintf (global.outfile,
                                "Shape of genarray with-loop has (dim != 1)!"););

    /*
     * CAUTION:
     * 'val_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!!
     */
    for (i = 0; i < val_size; i++) {
        if (vals_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      vals_ANY[i]);
                             , fprintf (global.outfile,
                                        "Shape of genarray with-loop has (dim != 1)!"););
        }
    }

    Set_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, NULL, ReadId, vals_ANY, val_size, NULL,
               NULL, ReadConstArray_Str);

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileND_WL_GENARRAY__SHAPE_id_id");

#define ND_WL_GENARRAY__SHAPE_id_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_id_id

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_WL_GENARRAY__SHAPE( %s, %d, ..., %s, %d)\"))\n",
             to_NT, to_sdim, val_NT, val_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", shp_NT);
                     , fprintf (global.outfile,
                                "Shape of genarray with-loop has (dim != 1)!"););

    Set_Shape (to_NT, to_sdim, shp_NT, -1, SizeId, NULL, ReadId, val_NT, val_dim, DimId,
               SizeId, ShapeId);

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileND_WL_GENARRAY__SHAPE_arr_id");

#define ND_WL_GENARRAY__SHAPE_arr_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_arr_id

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_WL_GENARRAY__SHAPE( %s, %d, ..., %s, %d)\"))\n",
             to_NT, to_sdim, val_NT, val_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shp_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      shp_ANY[i]);
                             , fprintf (global.outfile,
                                        "Shape of genarray with-loop has (dim != 1)!"););
        }
    }

    Set_Shape (to_NT, to_sdim, shp_ANY, shp_size, NULL, NULL, ReadConstArray_Str, val_NT,
               val_dim, DimId, SizeId, ShapeId);

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileWL_SCHEDULE__BEGIN");

#define WL_SCHEDULE__BEGIN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SCHEDULE__BEGIN

    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (global.outfile, "int SAC_WL_MT_SCHEDULE_START( %d);\n", i);
        INDENT;
        fprintf (global.outfile, "int SAC_WL_MT_SCHEDULE_STOP( %d);\n", i);
    }

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileWL_DECLARE_SHAPE_FACTOR");

#define WL_DECLARE_SHAPE_FACTOR
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_DECLARE_SHAPE_FACTOR

    INDENT;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (global.outfile, "int SAC_WL_SHAPE_FACTOR( %s, %d);\n", to_NT, i);
    }

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileWL_DEFINE_SHAPE_FACTOR");

#define WL_DEFINE_SHAPE_FACTOR
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_DEFINE_SHAPE_FACTOR

    INDENT;
    fprintf (global.outfile, "{\n");
    global.indent++;

    INDENT;
    fprintf (global.outfile, "int SAC_i;\n"); /* for DefineShapeFactor() !!! */
    for (i = 0; i < dims; i++) {
        DefineShapeFactor (to_NT, to_sdim, i);
    }

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("ICMCompileWL_SCHEDULE__END");

#define WL_SCHEDULE__END
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SCHEDULE__END

    global.indent--;
    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("ICMCompileWL_SUBALLOC");

#define WL_SUBALLOC
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SUBALLOC

    INDENT;
    fprintf (global.outfile,
             "SAC_ND_A_FIELD( %s) = SAC_ND_A_FIELD( %s)+SAC_ND_READ( %s, 0);\n", sub_NT,
             to_NT, off_NT);

    DBUG_VOID_RETURN;
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
#ifndef DBUG_OFF
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
#endif
    int cnt;

    DBUG_ENTER ("ICMCompileWL_MODARRAY_SUBSHAPE");

#define WL_MODARRAY_SUBSHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_MODARRAY_SUBSHAPE

    switch (sub_sc) {
    case C_scl:
    case C_aks:
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;
    case C_akd:
        DBUG_ASSERT (((to_sc == C_akd) || (to_sc == C_aud)),
                     "applied WL_MODARRAY_SUBSHAPE to AKD subvar but "
                     "non AKD/AUD resultvar. cannot handle this!");
        /*
         * start a fresh block
         */
        BLOCK_VARDECS (fprintf (global.outfile, "int SAC_size = 1;");
                       ,
                       /*
                        * update shape elements and calculate size.
                        *
                        * NOTE: we have to use DESC_SHAPE here as the
                        *       selection index is not constant. But we
                        *       do know that it is a AKD/AUD so this
                        *       should exist;)
                        */
                       for (cnt = 0; cnt < dims; cnt++) {
                           SET_SHAPE_AKD (sub_NT, cnt,
                                          fprintf (global.outfile,
                                                   "SAC_ND_A_DESC_SHAPE( %s, "
                                                   "SAC_ND_A_DIM( %s) - %d)",
                                                   to_NT, to_NT, dims - cnt););
                           fprintf (global.outfile,
                                    "SAC_size *= SAC_ND_A_SHAPE( %s, %d);", sub_NT, cnt);
                       }
                       /*
                        * update size
                        */
                       SET_SIZE (sub_NT, fprintf (global.outfile, "SAC_size");););

        break;
    case C_aud:
        DBUG_ASSERT ((to_sc == C_aud), "applied WL_MODARRAY_SUBSHAPE to AUD subvar but "
                                       "non AUD resultvar. cannot handle this!");
        /*
         * set shape and calculate size
         */
        BLOCK_VARDECS (fprintf (global.outfile, "int SAC_size = 1;\n");
                       ,
                       FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                                            , fprintf (global.outfile, "0");
                                            , fprintf (global.outfile,
                                                       "SAC_ND_A_DIM( %s)", sub_NT);
                                            ,
                                            SET_SHAPE_AUD (sub_NT,
                                                           fprintf (global.outfile,
                                                                    "SAC_i");
                                                           ,
                                                           fprintf (global.outfile,
                                                                    "SAC_ND_A_SHAPE( %s, "
                                                                    "SAC_ND_A_DIM( %s) - "
                                                                    "SAC_ND_A_DIM( %s) + "
                                                                    "SAC_i)",
                                                                    to_NT, to_NT,
                                                                    sub_NT););
                                            fprintf (global.outfile,
                                                     "SAC_size *= "
                                                     "SAC_ND_A_SHAPE( %s, SAC_i);\n",
                                                     sub_NT););
                       SET_SIZE (sub_NT, fprintf (global.outfile, "SAC_size");););

        break;
    default:
        DBUG_ASSERT (0, "unknown shape class found!");
    }

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

    ASSURE_TYPE_ASS (fprintf (global.outfile,
                              "SAC_ND_A_DIM( %s) == (SAC_ND_A_DIM( %s) - SAC_ND_A_SIZE( "
                              "%s))",
                              val_NT, to_NT, idx_vec_NT);
                     , fprintf (global.outfile,
                                "WL expression with illegal dimension found!"););

    ASSURE_TYPE_ASS (fprintf (global.outfile,
                              "SAC_ND_A_SIZE( %s) == SAC_WL_SHAPE_FACTOR( %s, %d)",
                              val_NT, to_NT, (dims < 0 ? 0 : dims - 1));
                     ,
                     fprintf (global.outfile, "WL expression with illegal size found!"););

    if ((val_dim == 0) || (to_dim == dims)) {
        INDENT;
        fprintf (global.outfile,
                 "SAC_ND_WRITE_READ_COPY( %s, SAC_ND_READ( %s, 0),"
                 " %s, 0, %s);\n",
                 to_NT, off_NT, val_NT, copyfun);
    } else {
        FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                             , fprintf (global.outfile, "0");
                             , fprintf (global.outfile, "SAC_ND_A_SIZE( %s)", val_NT);
                             , INDENT; fprintf (global.outfile,
                                                "SAC_ND_WRITE_READ_COPY( %s, "
                                                "SAC_ND_READ( %s, 0) + SAC_i,"
                                                " %s, SAC_i, %s);\n",
                                                to_NT, off_NT, val_NT, copyfun););
    }

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("ICMCompileWL_FOLD");

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

    PrintTraceICM (to_NT, idx_vec_NT, dims, idxs_scl_NT, "fold");

    INDENT;
    fprintf (global.outfile, "/* fold operation */\n");

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileWL_INIT_OFFSET");

#define WL_INIT_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_INIT_OFFSET

    INDENT;
    fprintf (global.outfile, "SAC_ND_WRITE( %s, 0)\n", off_NT);
    global.indent++;

    INDENT;
    fprintf (global.outfile,
             "= SAC_WL_MT_SCHEDULE_START( 0)"
             " * SAC_WL_SHAPE_FACTOR( %s, %d)",
             to_NT, 0);

    for (i = 1; i < dims; i++) {
        fprintf (global.outfile, "\n");
        INDENT;
        fprintf (global.outfile, "+ SAC_WL_MT_SCHEDULE_START( %d)", i);
        fprintf (global.outfile, " * SAC_WL_SHAPE_FACTOR( %s, %d)", to_NT, i);
    }

    fprintf (global.outfile, ";\n");
    global.indent--;

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("ICMCompileWL_ADJUST_OFFSET");

#define WL_ADJUST_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ADJUST_OFFSET

    INDENT;
    fprintf (global.outfile,
             "ND_WRITE( %s, 0) += SAC_WL_VAR( diff, %s)"
             " * SAC_WL_SHAPE_FACTOR( %s, %d);\n",
             off_NT, idxs_scl_NT[dim], to_NT, dim);

    DBUG_VOID_RETURN;
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

    DBUG_ENTER ("ICMCompileWL_SET_OFFSET");

#define WL_SET_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SET_OFFSET

    INDENT;
    fprintf (global.outfile, "SAC_ND_WRITE( %s, 0) \n", off_NT);
    global.indent++;

    INDENT;
    fprintf (global.outfile, "= ");
    for (i = dims - 1; i > 0; i--) {
        fprintf (global.outfile, "( SAC_ND_A_SHAPE( %s, %d) * ", to_NT, i);
    }
    fprintf (global.outfile, "SAC_ND_READ( %s, 0)\n", idxs_scl_NT[0]);

    INDENT;
    for (i = 1; i < dims; i++) {
        if (i <= dim) {
            fprintf (global.outfile, "+ SAC_ND_READ( %s, 0) )", idxs_scl_NT[i]);
        } else {
            if (i <= first_block_dim) {
                /*
                 * no blocking in this dimension
                 *  -> we use the start index of the current MT region
                 */
                fprintf (global.outfile, " + SAC_WL_MT_SCHEDULE_START( %d) )", i);
            } else {
                /*
                 * blocking in this dimension
                 *  -> we use the first index of the current block
                 */
                fprintf (global.outfile, " + SAC_WL_VAR( first, %s) )", idxs_scl_NT[i]);
            }
        }
    }
    fprintf (global.outfile, " * SAC_WL_SHAPE_FACTOR( %s, %d);\n", to_NT, dims - 1);
    global.indent--;

    DBUG_VOID_RETURN;
}
