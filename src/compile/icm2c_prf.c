/*
 *
 * $Log$
 * Revision 1.10  2003/09/30 19:29:24  dkr
 * code brushed: Set_Shape() used
 *
 * Revision 1.9  2003/09/30 00:03:25  dkr
 * unused variables removed
 *
 * Revision 1.8  2003/09/30 00:02:13  dkr
 * definition of ND_PRF_RESHAPE__SHAPE_id simplified: uses
 * ND_SET__SHAPE_id now
 *
 * Revision 1.7  2003/09/29 22:52:59  dkr
 * PrfReshape_Shape() no longer needed
 *
 * Revision 1.6  2003/09/29 18:17:24  dkr
 * fixed a bug in PrfReshape_Shape()
 *
 * Revision 1.5  2003/09/25 13:45:41  dkr
 * new argument 'copyfun' added to some ICMs.
 * ND_WRITE replaced by ND_WRITE_READ_COPY.
 *
 * Revision 1.4  2003/09/22 12:38:40  dkr
 * minor bugs fixed
 *
 * Revision 1.3  2003/09/22 11:59:21  dkr
 * _take_SxV_, _drop_SxV_ can handle negative 1st arguments as well now
 *
 * Revision 1.2  2003/09/20 14:38:28  dkr
 * C-ICMs for F_take_SxV, F_drop_SxV, F_cat_VxV added
 *
 * Revision 1.1  2003/09/20 14:20:18  dkr
 * Initial revision
 *
 */

#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"
#include "icm2c_prf.h"

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

#ifdef BEtest
#define Free(x)                                                                          \
    x;                                                                                   \
    free (x)
#define Malloc(x) malloc (x)
#endif /* BEtest */

#ifndef TAGGED_ARRAYS

#define NewBlock(init, body)                                                             \
    fprintf (outfile, "{\n");                                                            \
    indent++;                                                                            \
    init;                                                                                \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n")

#define InitPtr(src, dest)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "int SAC_isrc = ");                                                \
    src;                                                                                 \
    fprintf (outfile, ";\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "int SAC_idest = ");                                               \
    dest;                                                                                \
    fprintf (outfile, ";\n")

#define InitVecs(from, to, vn, v_i_str)                                                  \
    {                                                                                    \
        int i;                                                                           \
        for (i = from; i < to; i++) {                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "int %s%d=", vn, i);                                       \
            v_i_str;                                                                     \
            fprintf (outfile, ";\n");                                                    \
        }                                                                                \
    }

#define InitIMaxs(from, to, v_i_str) InitVecs (from, to, "SAC_imax", v_i_str)

#define InitSrcOffs(from, to, v_i_str) InitVecs (from, to, "SAC_srcoff", v_i_str)

#define FillRes(res, body)                                                               \
    INDENT;                                                                              \
    fprintf (outfile, "do {\n");                                                         \
    indent++;                                                                            \
    body;                                                                                \
    indent--;                                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "}\n");                                                            \
    INDENT;                                                                              \
    fprintf (outfile, "while( SAC_idest < SAC_ND_A_SIZE( %s));\n", res)

#define AccessSeg(dim, body)                                                             \
    {                                                                                    \
        int i;                                                                           \
        INDENT;                                                                          \
        fprintf (outfile, "{\n");                                                        \
        indent++;                                                                        \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "int SAC_i%d;\n", i);                                      \
        }                                                                                \
        for (i = 1; i < dim; i++) {                                                      \
            INDENT;                                                                      \
            fprintf (outfile, "for (SAC_i%d = 0; SAC_i%d < SAC_imax%d; SAC_i%d++) {\n",  \
                     i, i, i, i);                                                        \
            indent++;                                                                    \
        }                                                                                \
        body;                                                                            \
        for (i = dim - 1; i > 0; i--) {                                                  \
            indent--;                                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "}\n");                                                    \
            INDENT;                                                                      \
            fprintf (outfile, "SAC_isrc += SAC_srcoff%d;\n", i);                         \
        }                                                                                \
        indent--;                                                                        \
        INDENT;                                                                          \
        fprintf (outfile, "}\n");                                                        \
    }

#define CopyBlock(a, offset, res)                                                        \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")),                                  \
              FillRes (res, INDENT; fprintf (outfile,                                    \
                                             "SAC_ND_WRITE_ARRAY( %s, SAC_idest)"        \
                                             " = SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",   \
                                             res, a);                                    \
                       INDENT; fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");))

/*
 * TakeSeg( a, dima, offset, dimi, sz_i_str, off_i_str, res)
 *   a        : src-array
 *   dima     : dimension of "a"
 *   offset   : beginning of the segment in the unrolling of "a"
 *   dimi     : length of "sz_i_str" and "off_i_str"
 *   sz_i_str : number of elements to take from the i'th axis
 *   off_i_str: number of elements to skip in the i'th axis
 *   res      : resulting array
 *
 */

#define TakeSeg(a, dima, offset, dimi, sz_i_str, off_i_str, res)                         \
    NewBlock (InitPtr (offset, fprintf (outfile, "0")); InitIMaxs (0, dimi, sz_i_str);   \
              InitIMaxs (dimi, dima,                                                     \
                         fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", a, i));            \
              InitSrcOffs (                                                              \
                0, dimi, off_i_str; {                                                    \
                    int j;                                                               \
                    for (j = i + 1; j < dima; j++)                                       \
                        fprintf (outfile, "* SAC_ND_A_SHAPE( %s, %d)", a, j);            \
                }) InitSrcOffs (dimi, dima, fprintf (outfile, "0")),                     \
              FillRes (res,                                                              \
                       AccessSeg (dima, INDENT;                                          \
                                  fprintf (outfile,                                      \
                                           "SAC_ND_WRITE_ARRAY( %s, SAC_idest) "         \
                                           "= SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",      \
                                           res, a);                                      \
                                  INDENT;                                                \
                                  fprintf (outfile, "SAC_idest++; SAC_isrc++;\n");)))

#endif /* TAGGED_ARRAYS */

#ifdef TAGGED_ARRAYS

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_SHAPE__DATA( char *to_NT, int to_sdim,
 *                                      char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SHAPE__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SHAPE__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
#ifndef DBUG_OFF
    hidden_class_t to_hc = ICUGetHiddenClass (to_NT);
#endif
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SHAPE__DATA");

#define ND_PRF_SHAPE__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SHAPE__DATA

    DBUG_ASSERT ((to_hc == C_nhd), "result of shape() must be non-hidden!");

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SHAPE__DATA( %s, %d, %s, %d)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (from_sc) {
    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
        for (i = 0; i < from_dim; i++) {
            INDENT; /* is NHD -> no copyfun needed */
            fprintf (outfile,
                     "SAC_ND_WRITE_COPY( %s, %d, "
                     "SAC_ND_A_SHAPE( %s, %d), );\n",
                     to_NT, i, from_NT, i);
        }
        break;

    case C_aud:
        FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                             , fprintf (outfile, "SAC_ND_A_DIM( %s)", from_NT);
                             , INDENT; /* is NHD -> no copyfun needed */
                             fprintf (outfile,
                                      "SAC_ND_WRITE_COPY( %s, SAC_i,"
                                      " SAC_ND_A_SHAPE( %s, SAC_i), );\n",
                                      to_NT, from_NT););
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_RESHAPE__SHAPE_id( char *to_NT, int to_sdim,
 *                                            char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE__SHAPE_id( to_NT, to_sdim, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT)
{
    DBUG_ENTER ("ICMCompileND_PRF_RESHAPE__SHAPE_id");

#define ND_PRF_RESHAPE__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE__SHAPE_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", shp_NT);
                     , fprintf (outfile, "1st argument of F_reshape is not a vector!"););

    ICMCompileND_SET__SHAPE_id (to_NT, to_sdim, shp_NT);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_RESHAPE__SHAPE_arr( char *to_NT, int to_sdim,
 *                                             int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE__SHAPE_arr( to_NT, to_sdim, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE__SHAPE_arr (char *to_NT, int to_sdim, int shp_size,
                                     char **shp_ANY)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_RESHAPE__SHAPE_arr");

#define ND_PRF_RESHAPE__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE__SHAPE_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shp_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", shp_ANY[i]);
                             , fprintf (outfile,
                                        "1st argument of F_reshape is not a vector!"););
        }
    }

    ICMCompileND_SET__SHAPE_arr (to_NT, shp_size, shp_ANY);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__SHAPE_id( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim,
 *                                        char *idx_NT)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__SHAPE_id( to_NT, to_sdim, from_NT, from_sdim, idx_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__SHAPE_id (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                char *idx_NT)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_id");

#define ND_PRF_SEL__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (to_sc) {
    case C_aud:
        /*
         * for the time being implemented for scalar results only!
         */
        if (to_dim != 0) {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_SIZE( %s)",
                                      from_NT, idx_NT);
                             , fprintf (outfile, "Result of F_sel is not a scalar!"););
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "sel() with non-scalar result not yet implemented");
        break;

    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__SHAPE_arr( char *to_NT, int to_sdim,
 *                                         char *from_NT, int from_sdim,
 *                                         int idx_size, char **idxs_ANY)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__SHAPE_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                          idx_size, [ idxs_ANY ]* )
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__SHAPE_arr (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 int idx_size, char **idxs_ANY)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_arr");

#define ND_PRF_SEL__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_arr

    /*
     * CAUTION:
     * 'idxs_ANY[i]' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (to_sc) {
    case C_aud:
        /*
         * for the time being implemented for scalar results only!
         */
        if (to_dim != 0) {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == %d", from_NT,
                                      idx_size);
                             , fprintf (outfile, "Result of F_sel is not a scalar!"););
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_akd:
        /* here is no break missing */
    case C_aks:
        DBUG_ASSERT ((0), "sel() with non-scalar result not yet implemented");
        break;

    case C_scl:
        INDENT;
        fprintf (outfile, "SAC_NOOP()\n");
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrfSel_Data( char *to_NT, int to_sdim,
 *                     char *from_NT, int from_sdim,
 *                     void *idx, int idx_size,
 *                     void (*idx_size_fun)( void *),
 *                     void (*idx_read_fun)( void *, char *, int),
 *                     char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._SEL__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfSel_Data (char *to_NT, int to_sdim, char *from_NT, int from_sdim, void *idx,
             int idx_size, void (*idx_size_fun) (void *),
             void (*idx_read_fun) (void *, char *, int), char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("PrfSel_Data");

    if (to_dim == 0) {
        BLOCK_VARDECS (fprintf (outfile, "int SAC_idx;");
                       , Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                      idx_read_fun, from_NT, from_dim);
                       INDENT;
                       fprintf (outfile,
                                "SAC_ND_WRITE_READ_COPY( %s, 0, %s, SAC_idx, %s)\n",
                                to_NT, from_NT, copyfun););
    } else {
        BLOCK_VARDECS (fprintf (outfile, "int SAC_idx, SAC_i;");
                       , Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                      idx_read_fun, from_NT, from_dim);
                       FOR_LOOP_INC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                                     , fprintf (outfile, "SAC_ND_A_SIZE( %s)", to_NT);
                                     , INDENT;
                                     fprintf (outfile,
                                              "SAC_ND_WRITE_READ_COPY( %s, SAC_i,"
                                              " %s, SAC_idx, %s)\n",
                                              to_NT, from_NT, copyfun);
                                     INDENT; fprintf (outfile, "SAC_idx++;\n");););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__DATA_id( char *to_NT, int to_sdim,
 *                                       char *from_NT, int from_sdim,
 *                                       char *idx_NT, int idx_size,
 *                                       char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__DATA_id( to_NT, to_sdim, from_NT, from_sdim, idx_NT, idx_size,
 *                        copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__DATA_id (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                               char *idx_NT, int idx_size, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_SEL__DATA_id");

#define ND_PRF_SEL__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__DATA_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (outfile, "1st argument of F_sel is not a vector!"););
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)", from_NT,
                              idx_NT);
                     , fprintf (outfile, "1st argument of F_sel has illegal size!"););

    PrfSel_Data (to_NT, to_sdim, from_NT, from_sdim, idx_NT, idx_size, SizeId, ReadId,
                 copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL__DATA_arr( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim,
 *                                        int idx_size, char **idxs_ANY,
 *                                        char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                         idx_size, [ idxs_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL__DATA_arr (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                int idx_size, char **idxs_ANY, char *copyfun)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_SEL__DATA_arr");

#define ND_PRF_SEL__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__DATA_arr

    /*
     * CAUTION:
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idxs_ANY[i]);
                             , fprintf (outfile,
                                        "1st argument of F_sel is not a vector!"););
        }
    }
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT, idx_size);
                     , fprintf (outfile, "1st argument of F_sel has illegal size!"););

    PrfSel_Data (to_NT, to_sdim, from_NT, from_sdim, idxs_ANY, idx_size, NULL,
                 ReadConstArray_Str, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void PrfModarray_Data( char *to_NT, int to_sdim,
 *                          char *from_NT, int from_sdim,
 *                          bool idx_unrolled, void *idx, int idx_size,
 *                          void (*idx_size_fun)( void *),
 *                          void (*idx_read_fun)( void *, char *, int),
 *                          char *val_ANY,
 *                          char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._MODARRAY__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfModarray_Data (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                  bool idx_unrolled, void *idx, int idx_size,
                  void (*idx_size_fun) (void *),
                  void (*idx_read_fun) (void *, char *, int), char *val_ANY,
                  char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("PrfModarray_Data");

    /*
     * CAUTION:
     * 'val_ANY' is either a tagged identifier or a constant scalar!
     */

    if ((val_ANY[0] != '(') || /* not a tagged id -> is a const scalar! */
        (ICUGetShapeClass (val_ANY) == C_scl)) {
        /* 'val_ANY' is scalar */
        INDENT;
        fprintf (outfile, "SAC_IS_REUSED__BLOCK_BEGIN( %s, %s)\n", to_NT, from_NT);
        indent++;
        INDENT;
        fprintf (outfile,
                 "SAC_TR_MEM_PRINT("
                 " (\"reuse memory of %s at %%p for %s\","
                 " SAC_ND_A_FIELD( %s)))\n",
                 from_NT, to_NT, from_NT);
        indent--;
        INDENT;
        fprintf (outfile, "SAC_IS_REUSED__BLOCK_ELSE( %s, %s)\n", to_NT, from_NT);
        indent++;
        INDENT;
        fprintf (outfile, "int SAC_i;\n");
        FOR_LOOP_INC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                      , fprintf (outfile, "SAC_ND_A_SIZE( %s)", to_NT);, INDENT;
                      fprintf (outfile,
                               "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_i, %s)\n",
                               to_NT, from_NT, copyfun););
        indent--;
        INDENT;
        fprintf (outfile, "SAC_IS_REUSED__BLOCK_END( %s, %s)\n", to_NT, from_NT);

        BLOCK_VARDECS (fprintf (outfile, "int SAC_idx;");
                       ,
                       if (idx_unrolled) {
                           INDENT;
                           fprintf (outfile, "SAC_idx = ");
                           idx_read_fun (idx, NULL, 0);
                           fprintf (outfile, ";\n");
                       } else {
                           Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                        idx_read_fun, to_NT, to_dim);
                       } INDENT;
                       fprintf (outfile, "SAC_ND_WRITE_COPY( %s, SAC_idx, ", to_NT);
                       ReadScalar (val_ANY, NULL, 0);
                       fprintf (outfile, " , %s)\n", copyfun););
    } else {
        /* 'val_ANY' is a tagged identifier representing a non-scalar array */
        BLOCK_VARDECS (fprintf (outfile, "int SAC_i, SAC_j, SAC_idx;");
                       ,
                       if (idx_unrolled) {
                           INDENT;
                           fprintf (outfile, "SAC_idx = ");
                           idx_read_fun (idx, NULL, 0);
                           fprintf (outfile, ";\n");
                       } else {
                           Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                        idx_read_fun, to_NT, to_dim);
                       } INDENT;
                       fprintf (outfile, "SAC_IS_REUSED__BLOCK_BEGIN( %s, %s)\n", to_NT,
                                from_NT);
                       indent++; INDENT;
                       fprintf (outfile,
                                "SAC_TR_MEM_PRINT("
                                " (\"reuse memory of %s at %%p for %s\","
                                " SAC_ND_A_FIELD( %s)))\n",
                                from_NT, to_NT, from_NT);
                       FOR_LOOP (fprintf (outfile, "SAC_i = SAC_idx, SAC_j = 0");
                                 ,
                                 fprintf (outfile, "SAC_j < SAC_ND_A_SIZE( %s)", val_ANY);
                                 , fprintf (outfile, "SAC_i++, SAC_j++");, INDENT;
                                 fprintf (outfile,
                                          "SAC_ND_WRITE_READ_COPY("
                                          " %s, SAC_i, %s, SAC_j, %s)\n",
                                          to_NT, val_ANY, copyfun););
                       indent--; INDENT;
                       fprintf (outfile, "SAC_IS_REUSED__BLOCK_ELSE( %s, %s)\n", to_NT,
                                from_NT);
                       indent++;
                       FOR_LOOP_INC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                                     , fprintf (outfile, "SAC_idx");, INDENT;
                                     fprintf (outfile,
                                              "SAC_ND_WRITE_READ_COPY("
                                              " %s, SAC_i, %s, SAC_i, %s)\n",
                                              to_NT, from_NT, copyfun););
                       FOR_LOOP_INC (fprintf (outfile, "SAC_j");, fprintf (outfile, "0");
                                     , fprintf (outfile, "SAC_ND_A_SIZE( %s)", val_ANY);
                                     , INDENT; fprintf (outfile,
                                                        "SAC_ND_WRITE_READ_COPY("
                                                        " %s, SAC_i, %s, SAC_j, %s)\n",
                                                        to_NT, val_ANY, copyfun);
                                     INDENT; fprintf (outfile, "SAC_i++;\n"););
                       FOR_LOOP (fprintf (outfile, " ");
                                 , fprintf (outfile, "SAC_i < SAC_ND_A_SIZE( %s)", to_NT);
                                 , fprintf (outfile, "SAC_i++");, INDENT;
                                 fprintf (outfile,
                                          "SAC_ND_WRITE_READ_COPY("
                                          " %s, SAC_i, %s, SAC_i, %s)\n",
                                          to_NT, from_NT, copyfun););
                       indent--; INDENT;
                       fprintf (outfile, "SAC_IS_REUSED__BLOCK_END( %s, %s)\n", to_NT,
                                from_NT););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_id( char *to_NT, int to_sdim,
 *                                            char *from_NT, int from_sdim,
 *                                            char *idx_NT, int idx_size,
 *                                            char *val_ANY, char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                             idx_NT, idx_size, val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                    int from_sdim, char *idx_NT, int idx_size,
                                    char *val_ANY, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_id");

#define ND_PRF_MODARRAY__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_id

    /*
     * CAUTION:
     * 'val_ANY' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_ANY);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (outfile, "2nd argument of F_modarray is not a vector!"););
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)", from_NT,
                              idx_NT);
                     ,
                     fprintf (outfile, "2nd argument of F_modarray has illegal size!"););

    PrfModarray_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idx_NT, idx_size, SizeId,
                      ReadId, val_ANY, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY__DATA_arr( char *to_NT, int to_sdim,
 *                                             char *from_NT, int from_sdim,
 *                                             int idx_size, char **idxs_ANY,
 *                                             char *val_ANY,
 *                                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                              idx_size, [ idxs_ANY ]* , val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                     int from_sdim, int idx_size, char **idxs_ANY,
                                     char *val_ANY, char *copyfun)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY__DATA_arr");

#define ND_PRF_MODARRAY__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY__DATA_arr

    /*
     * CAUTION:
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     * 'val_ANY' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_ANY);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idxs_ANY[i]);
                             , fprintf (outfile,
                                        "2nd argument of F_modarray is not a vector"););
        }
    }
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT, idx_size);
                     ,
                     fprintf (outfile, "2nd argument of F_modarray has illegal size!"););

    PrfModarray_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idxs_ANY, idx_size, NULL,
                      ReadConstArray_Str, val_ANY, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SEL__SHAPE( char *to_NT, int to_sdim,
 *                                         char *from_NT, int from_sdim,
 *                                         char *idx_ANY)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SEL__SHAPE( to_NT, to_sdim, from_NT, from_sdim, idx_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SEL__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 char *idx_ANY)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SEL__SHAPE");

#define ND_PRF_IDX_SEL__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SEL__SHAPE

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    if (to_dim == 0) {
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
    } else {
        DBUG_ASSERT ((0), "idx_sel() with non-scalar result not yet implemented");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SEL__DATA( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim,
 *                                        char *idx_ANY,
 *                                        char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, idx_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SEL__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                char *idx_ANY, char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SEL__DATA");

#define ND_PRF_IDX_SEL__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SEL__DATA

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (outfile,
                                    "1st argument of F_idx_sel is not a scalar!"););
    }

    /*
     * idx_sel() works only for arrays with known dimension!
     */
    DBUG_ASSERT ((to_dim >= 0), "idx_sel() with unknown dimension found!");

    if (to_dim == 0) {
        /*
         * 'to_NT' is scalar
         */
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE_READ_COPY( %s, 0, %s, ", to_NT, from_NT);
        ReadScalar (idx_ANY, NULL, 0);
        fprintf (outfile, ", %s)\n", copyfun);
    } else {
        /*
         * 'to_NT' is array
         */
        FOR_LOOP_VARDECS (fprintf (outfile, "int SAC_i, SAC_j;");
                          , fprintf (outfile, "SAC_i = 0, SAC_j = ");
                          ReadScalar (idx_ANY, NULL, 0);
                          , fprintf (outfile, "SAC_i < SAC_ND_A_SIZE( %s)", to_NT);
                          , fprintf (outfile, "SAC_i++, SAC_j++");, INDENT;
                          fprintf (outfile,
                                   "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n",
                                   to_NT, from_NT, copyfun););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_IDX_MODARRAY__DATA( char *to_NT, int to_sdim,
 *                                             char *from_NT, int from_sdim,
 *                                             char *idx_ANY, char *val_ANY,
 *                                             char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_MODARRAY__DATA( to_NT, to_sdim, from_NT, from_sdim, idx, val,
 *                              copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_MODARRAY__DATA (char *to_NT, int to_sdim, char *from_NT,
                                     int from_sdim, char *idx_ANY, char *val_ANY,
                                     char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_IDX_MODARRAY__DATA");

#define ND_PRF_IDX_MODARRAY__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_MODARRAY__DATA

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     * 'val_ANY' is either a tagged identifier (scalar or non-scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_MODARRAY__DATA( %s, %d, %s, %d, %s, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY, val_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (outfile,
                                    "2nd argument of F_modarray is not a scalar!"););
    }

    PrfModarray_Data (to_NT, to_sdim, from_NT, from_sdim, TRUE, idx_ANY, 1, NULL,
                      ReadScalar, val_ANY, copyfun);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_TAKE__SHAPE( char *to_NT, int to_sdim,
 *                                      char *from_NT, int from_sdim,
 *                                      char *cnt_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_TAKE__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_TAKE__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                              char *cnt_ANY)
{
    char **shp;

    DBUG_ENTER ("ICMCompileND_PRF_TAKE__SHAPE");

#define ND_PRF_TAKE__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_TAKE__SHAPE

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_TAKE__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    if (cnt_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", cnt_ANY);
                         , fprintf (outfile,
                                    "1st argument of F_take_SxV is not a scalar!"););
    }

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", from_NT);
                     , fprintf (outfile, "2nd argument of F_take_SxV is not a vector!"););

    shp = (char **)Malloc (sizeof (char *));
    shp[0] = (char *)Malloc ((strlen (cnt_ANY) + 30) * sizeof (char));
    if (cnt_ANY[0] == '(') {
        sprintf (shp[0], "SAC_ABS( SAC_ND_A_FIELD( %s))", cnt_ANY);
    } else {
        sprintf (shp[0], "SAC_ABS( %s)", cnt_ANY);
    }
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = Free (shp[0]);
    shp = Free (shp);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_TAKE__DATA( char *to_NT, int to_sdim,
 *                                     char *from_NT, int from_sdim,
 *                                     char *cnt_ANY,
 *                                     char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_TAKE__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_TAKE__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                             char *cnt_ANY, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_TAKE__DATA");

#define ND_PRF_TAKE__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_TAKE__DATA

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_TAKE__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    BLOCK_VARDECS (fprintf (outfile, "int SAC_cnt, SAC_off;");, INDENT;
                   fprintf (outfile, "SAC_cnt = "); ReadScalar (cnt_ANY, NULL, 0);
                   fprintf (outfile, ";\n");

                   COND2 (fprintf (outfile, "("); ReadScalar (cnt_ANY, NULL, 0);
                          fprintf (outfile, " < 0)");, INDENT;
                          fprintf (outfile, "SAC_cnt = - SAC_cnt;\n"); INDENT;
                          fprintf (outfile, "SAC_off = SAC_ND_A_SIZE( %s) - SAC_cnt;\n",
                                   from_NT);
                          , INDENT; fprintf (outfile, "SAC_off = 0;\n"););

                   ASSURE_TYPE_ASS (fprintf (outfile, "SAC_cnt <= SAC_ND_A_SIZE( %s)",
                                             from_NT);
                                    , fprintf (outfile, "1st argument of F_take_SxV is "
                                                        "out of range!"););

                   FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");
                                        , fprintf (outfile, "0");
                                        , fprintf (outfile, "SAC_cnt");, INDENT;
                                        fprintf (outfile,
                                                 "SAC_ND_WRITE_READ_COPY( %s, SAC_i,"
                                                 " %s, SAC_off + SAC_i, %s);\n",
                                                 to_NT, from_NT, copyfun);););

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_DROP__SHAPE( char *to_NT, int to_sdim,
 *                                      char *from_NT, int from_sdim,
 *                                      char *cnt_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_DROP__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_DROP__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                              char *cnt_ANY)
{
    char **shp;

    DBUG_ENTER ("ICMCompileND_PRF_DROP__SHAPE");

#define ND_PRF_DROP__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_DROP__SHAPE

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_DROP__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    if (cnt_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", cnt_ANY);
                         , fprintf (outfile,
                                    "1st argument of F_drop_SxV is not a scalar!"););
    }

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", from_NT);
                     , fprintf (outfile, "2nd argument of F_drop_SxV is not a vector!"););

    shp = (char **)Malloc (sizeof (char *));
    shp[0] = (char *)Malloc ((strlen (from_NT) + strlen (cnt_ANY) + 50) * sizeof (char));
    if (cnt_ANY[0] == '(') {
        sprintf (shp[0], "SAC_ND_A_SIZE( %s) - SAC_ABS( SAC_ND_A_FIELD( %s))", from_NT,
                 cnt_ANY);
    } else {
        sprintf (shp[0], "SAC_ND_A_SIZE( %s) - SAC_ABS( %s)", from_NT, cnt_ANY);
    }
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = Free (shp[0]);
    shp = Free (shp);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_DROP__DATA( char *to_NT, int to_sdim,
 *                                     char *from_NT, int from_sdim,
 *                                     char *cnt_ANY,
 *                                     char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_DROP__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_DROP__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                             char *cnt_ANY, char *copyfun)
{
    DBUG_ENTER ("ICMCompileND_PRF_DROP__DATA");

#define ND_PRF_DROP__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_DROP__DATA

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_DROP__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    BLOCK_VARDECS (fprintf (outfile, "int SAC_cnt, SAC_off;");, INDENT;
                   fprintf (outfile, "SAC_off = "); ReadScalar (cnt_ANY, NULL, 0);
                   fprintf (outfile, ";\n");

                   COND2 (fprintf (outfile, "("); ReadScalar (cnt_ANY, NULL, 0);
                          fprintf (outfile, " < 0)");, INDENT;
                          fprintf (outfile, "SAC_cnt = SAC_ND_A_SIZE( %s) + SAC_off;\n",
                                   from_NT);
                          INDENT; fprintf (outfile, "SAC_off = 0;\n");, INDENT;
                          fprintf (outfile, "SAC_cnt = SAC_ND_A_SIZE( %s) - SAC_off;\n",
                                   from_NT););

                   ASSURE_TYPE_ASS (fprintf (outfile, "SAC_cnt <= SAC_ND_A_SIZE( %s)",
                                             from_NT);
                                    , fprintf (outfile, "1st argument of F_drop_SxV is "
                                                        "out of range!"););

                   FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");
                                        , fprintf (outfile, "0");
                                        , fprintf (outfile, "SAC_cnt");, INDENT;
                                        fprintf (outfile,
                                                 "SAC_ND_WRITE_READ_COPY( %s, SAC_i,"
                                                 " %s, SAC_off + SAC_i, %s);\n",
                                                 to_NT, from_NT, copyfun);););

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_CAT__SHAPE( char *to_NT, int to_sdim,
 *                                     char *from1_NT, int from1_sdim,
 *                                     char *from2_NT, int from2_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_CAT__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_CAT__SHAPE (char *to_NT, int to_sdim, char *from1_NT, int from1_sdim,
                             char *from2_NT, int from2_sdim)
{
    char **shp;

    DBUG_ENTER ("ICMCompileND_PRF_CAT__SHAPE");

#define ND_PRF_CAT__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_CAT__SHAPE

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_CAT__SHAPE( %s, %d, %s, %d, %s, %d)\"))\n",
             to_NT, to_sdim, from1_NT, from1_sdim, from2_NT, from2_sdim);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", from1_NT);
                     , fprintf (outfile, "1st argument of F_cat_VxV is not a vector!"););
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", from2_NT);
                     , fprintf (outfile, "2nd argument of F_cat_VxV is not a vector!"););

    shp = (char **)Malloc (sizeof (char *));
    shp[0]
      = (char *)Malloc ((strlen (from1_NT) + strlen (from2_NT) + 40) * sizeof (char));
    sprintf (shp[0], "SAC_ND_A_SIZE( %s) + SAC_ND_A_SIZE( %s)", from1_NT, from2_NT);
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = Free (shp[0]);
    shp = Free (shp);

    DBUG_VOID_RETURN;
}

#else /* TAGGED_ARRAYS */

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_CxA_S( char *a, char *res, int dim, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_CxA_S( a, res, dim, [ v ]* )
 *   selects a single element of the array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_CxA_S (char *a, char *res, int dim, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_CxA_S");

#define ND_KD_SEL_CxA_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_CxA_S

    INDENT;
    fprintf (outfile, "%s = SAC_ND_READ_ARRAY( %s, ", res, a);
    Vect2Offset (dim, AccessConst (vi, i), dim, a);
    fprintf (outfile, ");\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_VxA_S( char *a, char *res, int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_VxA_S( a, res, dim, v )
 *   selects a single element of the array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_VxA_S (char *a, char *res, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_VxA_S");

#define ND_KD_SEL_VxA_S
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_VxA_S

    INDENT;
    fprintf (outfile, "%s = SAC_ND_READ_ARRAY( %s, ", res, a);
    Vect2Offset (dim, AccessVect (v, i), dim, a);
    fprintf (outfile, ");\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_CxA_A( int dima, char *a,
 *                                   char *res, int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_CxA_A( dima, a, res, dimv, [ v ]* )
 *   selects a sub-array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_CxA_A");

#define ND_KD_SEL_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_CxA_A

    INDENT;
    CopyBlock (a, Vect2Offset (dimv, AccessConst (vi, i), dima, a), res);
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_SEL_VxA_A( int dima, char *a,
 *                                   char *res, int dimv, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_SEL_VxA_A( dima, a, res, dimv, v )
 *   selects a sub-array
 *
 ******************************************************************************/

void
ICMCompileND_KD_SEL_VxA_A (int dima, char *a, char *res, int dimv, char *v)
{
    DBUG_ENTER ("ICMCompileND_KD_SEL_VxA_A");

#define ND_KD_SEL_VxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_SEL_VxA_A

    INDENT;
    CopyBlock (a, Vect2Offset (dimv, AccessVect (v, i), dima, a), res);
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_TAKE_CxA_A( int dima, char *a, char *res,
 *                                    int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_TAKE_CxA_A( dima, a, res, dimv, [ v ]* )
 *
 ******************************************************************************/

void
ICMCompileND_KD_TAKE_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_TAKE_CxA_A");

#define ND_KD_TAKE_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_TAKE_CxA_A

    INDENT;
    TakeSeg (a, dima, fprintf (outfile, "0"), /* offset */
             dimv,                            /* dim of sizes & offsets */
             AccessConst (vi, i),             /* sizes */
             fprintf (outfile, "(SAC_ND_A_SHAPE( %s, %d) - ", a, i);
             AccessConst (vi, i); fprintf (outfile, ")"), /* offsets */
                                  res);

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_DROP_CxA_A( int dima, char *a, char *res,
 *                                    int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_DROP_CxA_A( dima, a, res, dimv, [ v ]* )
 *
 ******************************************************************************/

void
ICMCompileND_KD_DROP_CxA_A (int dima, char *a, char *res, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_KD_DROP_CxA_A");

#define ND_KD_DROP_CxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_DROP_CxA_A

    INDENT;
    TakeSeg (a, dima, Vect2Offset (dimv, AccessConst (vi, i), dima, a), /* offset */
             dimv, /* dim of sizes & offsets */
             fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) - ", a, i);
             AccessConst (vi, i), /* sizes */
             AccessConst (vi, i), /* offsets */
             res);

    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_CAT_SxAxA_A( int dima, char **ar, char *res,
 *                                     int catdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_CAT_SxAxA_A( dima, ar0, ar1, res, catdim)
 *
 ******************************************************************************/

void
ICMCompileND_KD_CAT_SxAxA_A (int dima, char **ar, char *res, int catdim)
{
    DBUG_ENTER ("ICMCompileND_KD_CAT_SxAxA_A");

#define ND_KD_CAT_SxAxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_CAT_SxAxA_A

    INDENT;
    NewBlock (InitPtr (fprintf (outfile, "0"), fprintf (outfile, "0"));
              InitVecs (1, 2, "SAC_isrc", fprintf (outfile, "0"));
              InitVecs (0, 2, "SAC_i", fprintf (outfile, "0"));
              InitVecs (0, 2, "SAC_bl",
                        {
                            int j;
                            for (j = catdim; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) *", ar[i], j);
                            }
                            fprintf (outfile, "1");
                        }),
              FillRes (res, INDENT;
                       fprintf (outfile, "for (SAC_i0 = 0; SAC_i0 < SAC_bl0;"
                                         " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",
                                                  res, ar[0]);
                       indent--; INDENT;
                       fprintf (outfile, "for (SAC_i1 = 0; SAC_i1 < SAC_bl1;"
                                         " SAC_i1++, SAC_idest++, SAC_isrc1++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc1);\n",
                                                  res, ar[1]);
                       indent--; INDENT;));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_KD_ROT_CxSxA_A( int rotdim, char **numstr, int dima,
 *                                     char *a, char *res)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_KD_ROT_CxSxA_A( rotdim, numstr, dima, a, res)
 *
 ******************************************************************************/

void
ICMCompileND_KD_ROT_CxSxA_A (int rotdim, char **numstr, int dima, char *a, char *res)
{
    DBUG_ENTER ("ICMCompileND_KD_ROT_CxSxA_A");

#define ND_KD_ROT_CxSxA_A
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_KD_ROT_CxSxA_A

    INDENT;
    NewBlock (InitVecs (0, 1, "SAC_shift",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) * ", a, j);
                            }
                            fprintf (outfile, "(%s<0 ? ", numstr[0]);
                            fprintf (outfile,
                                     "( SAC_ND_A_SHAPE( %s, %d) == 0 ? 0 :"
                                     " SAC_ND_A_SHAPE( %s, %d) + "
                                     " (%s %% SAC_ND_A_SHAPE( %s, %d))) : ",
                                     a, rotdim, a, rotdim, numstr[0], a, rotdim);
                            fprintf (outfile,
                                     "( SAC_ND_A_SHAPE( %s, %d) == 0 ? 0 :"
                                     " %s %% SAC_ND_A_SHAPE( %s, %d)))",
                                     a, rotdim, numstr[0], a, rotdim);
                        });
              InitVecs (0, 1, "SAC_bl",
                        {
                            int j;
                            for (j = rotdim + 1; j < dima; j++) {
                                fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) * ", a, j);
                            }
                            fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", a, rotdim);
                        });
              InitVecs (0, 1, "SAC_i", fprintf (outfile, "0"));
              InitPtr (fprintf (outfile, "- SAC_shift0"), fprintf (outfile, "0")),
              FillRes (res, INDENT; fprintf (outfile, "SAC_isrc += SAC_bl0;\n"); INDENT;
                       fprintf (outfile, "for (SAC_i0 = 0; SAC_i0 < SAC_shift0;"
                                         " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",
                                                  res, a);
                       indent--; INDENT; fprintf (outfile, "SAC_isrc -= SAC_bl0;\n");
                       INDENT; fprintf (outfile, "for (; SAC_i0 < SAC_bl0;"
                                                 " SAC_i0++, SAC_idest++, SAC_isrc++)\n");
                       indent++; INDENT; fprintf (outfile,
                                                  "SAC_ND_WRITE_ARRAY( %s, SAC_idest) ="
                                                  " SAC_ND_READ_ARRAY( %s, SAC_isrc);\n",
                                                  res, a);
                       indent--;));
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxS( char *res_btype, int dimres,
 *                                         char *res, char *old, char **value,
 *                                         int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxS( res_btype, dimres, res, old, value, dimv, [ v ]* )
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxS (char *res_btype, int dimres, char *res, char *old,
                                 char **value, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxS");

#define ND_PRF_MODARRAY_AxCxS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxS

    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) != SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i;\n");
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent--;
    INDENT;
    fprintf (outfile, "}\n");
    indent--;
    INDENT;
    fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, ", res);
    Vect2Offset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ") = %s;\n", value[0]);
    INDENT;
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS( char *res_btype, int dimres,
 *                                         char *res, char *old, char **value,
 *                                         int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS( res_btype, dimres, res, old, value, dimv, v)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxS (char *res_btype, int dimres, char *res, char *old,
                                 char **value, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxVxS");

#define ND_PRF_MODARRAY_AxVxS
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxS

    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) != SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i;\n");
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    INDENT;
    fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, ", res);
    Vect2Offset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, ") = %s;\n", value[0]);
    INDENT;
    fprintf (outfile, "\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxCxA( char *res_btype, int dimres,
 *                                         char *res, char *old, char *val,
 *                                         int dimv, char **vi)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxCxA( res_btype, dimres, res, old, val, dimv, [ v ]* )
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxCxA (char *res_btype, int dimres, char *res, char *old,
                                 char *val, int dimv, char **vi)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxCxA");

#define ND_PRF_MODARRAY_AxCxA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxCxA

    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i, SAC_j;\n");
    INDENT;
    fprintf (outfile, "int SAC_idx = ");
    Vect2Offset (dimv, AccessConst (vi, i), dimres, res);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) == SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent -= 2;
    INDENT;
    fprintf (outfile, "} else {\n");
    indent++;
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx-1; SAC_i++)\n");
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent--;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent--;
    INDENT;
    fprintf (outfile, "for (; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxA( char *res_btype, int dimres,
 *                                         char *res, char *old, char *val,
 *                                         int dim, char *v)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxA( res_btype, dimres, res, old, val, dim, v)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxA (char *res_btype, int dimres, char *res, char *old,
                                 char *val, int dim, char *v)
{
    DBUG_ENTER ("ICMCompileND_PRF_MODARRAY_AxVxA");

#define ND_PRF_MODARRAY_AxVxA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxA

    INDENT;
    fprintf (outfile, "{\n");
    indent++;
    INDENT;
    fprintf (outfile, "int SAC_i, SAC_j;\n");
    INDENT;
    fprintf (outfile, "int SAC_idx = ");
    Vect2Offset (dim, AccessVect (v, i), dimres, res);
    fprintf (outfile, ";\n");
    INDENT;
    fprintf (outfile, "if (SAC_ND_A_FIELD( %s) == SAC_ND_A_FIELD( %s)) {\n", old, res);
    indent++;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent -= 2;
    INDENT;
    fprintf (outfile, "} else {\n");
    indent++;
    INDENT;
    fprintf (outfile, "for (SAC_i = 0; SAC_i < SAC_idx-1; SAC_i++)\n");
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent--;
    INDENT;
    fprintf (outfile,
             "for (SAC_i = SAC_idx, SAC_j = 0;"
             " SAC_j < SAC_ND_A_SIZE( %s); SAC_i++, SAC_j++)\n",
             val);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_j);\n",
             res, val);
    indent--;
    INDENT;
    fprintf (outfile, "for (; SAC_i < SAC_ND_A_SIZE( %s); SAC_i++)\n", res);
    indent++;
    INDENT;
    fprintf (outfile,
             "SAC_ND_WRITE_ARRAY( %s, SAC_i) ="
             " SAC_ND_READ_ARRAY( %s, SAC_i);\n",
             res, old);
    indent -= 2;
    INDENT;
    fprintf (outfile, "}\n");
    indent--;
    INDENT;
    fprintf (outfile, "}\n");

    DBUG_VOID_RETURN;
}

#endif /* TAGGED_ARRAYS */
