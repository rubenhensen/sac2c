/*
 * $Id$
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"
#include "icm2c_prf.h"

#include "dbug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "free.h"
#include "internal_lib.h"

#ifdef BEtest
#define ILIBfree(x)                                                                      \
    x;                                                                                   \
    free (x)
#define ILIBmalloc(x) malloc (x)
#endif /* BEtest */

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

    DBUG_ASSERT ((to_hc == C_nhd), "result of F_shape must be non-hidden!");

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SHAPE__DATA( %s, %d, %s, %d)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (from_sc) {
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT ((from_dim >= 0), "illegal dimension found!");
        for (i = 0; i < from_dim; i++) {
            INDENT; /* is NHD -> no copyfun needed */
            fprintf (global.outfile,
                     "SAC_ND_WRITE_COPY( %s, %d, "
                     "SAC_ND_A_SHAPE( %s, %d), );\n",
                     to_NT, i, from_NT, i);
        }
        break;

    case C_aud:
        FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                             , fprintf (global.outfile, "0");
                             , fprintf (global.outfile, "SAC_ND_A_DIM( %s)", from_NT);
                             , INDENT; /* is NHD -> no copyfun needed */
                             fprintf (global.outfile,
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", shp_NT);
                     , fprintf (global.outfile, "1st argument of %s is not a vector!",
                                global.prf_string[F_reshape]););

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shp_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      shp_ANY[i]);
                             , fprintf (global.outfile,
                                        "1st argument of %s is not a vector!",
                                        global.prf_string[F_reshape]););
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
    int from_dim = DIM_NO_OFFSET (from_sdim);
    char **shp;
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_SEL__SHAPE_id");

#define ND_PRF_SEL__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL__SHAPE_id

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile,
                              "SAC_ND_A_DIM( %s) =="
                              " SAC_ND_A_DIM( %s) + SAC_ND_A_SIZE( %s)",
                              from_NT, to_NT, idx_NT);
                     , fprintf (global.outfile, "Inconsistant call of %s found!",
                                global.prf_string[F_sel]););

    switch (to_sc) {
    case C_scl:
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        shp = (char **)ILIBmalloc (to_dim * sizeof (char *));
        for (i = 0; i < to_dim; i++) {
            shp[i] = (char *)ILIBmalloc ((2 * strlen (from_NT) + 50) * sizeof (char));
            if (from_dim >= 0) {
                sprintf (shp[i], "SAC_ND_A_SHAPE( %s, %d)", from_NT,
                         from_dim - (to_dim - i));
            } else {
                sprintf (shp[i], "SAC_ND_A_SHAPE( %s, SAC_ND_A_DIM( %s) - %d)", from_NT,
                         from_NT, to_dim - i);
            }
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, to_dim, shp);
        for (i = 0; i < to_dim; i++) {
            shp[i] = ILIBfree (shp[i]);
        }
        shp = ILIBfree (shp);
        break;

    case C_aud:
        /*
         * The dimension of 'to_NT' must be computed dynamically:
         *   shape( to_NT) := drop( size( idx_NT), shape( from_NT))
         * Unfortunately, such a drop-operation is not covered by the
         * function Set_Shape() for the time being ...
         *
         * Hence, F_sel is implemented for scalar results only!
         */
        ASSURE_TYPE_ASS (fprintf (global.outfile,
                                  "SAC_ND_A_DIM( %s) == SAC_ND_A_SIZE( %s)", from_NT,
                                  idx_NT);
                         , fprintf (global.outfile, "Result of %s is not a scalar!",
                                    global.prf_string[F_sel]););
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
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
    int from_dim = DIM_NO_OFFSET (from_sdim);
    char **shp;
    int i;

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile,
                              "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s) + %d", from_NT,
                              to_NT, idx_size);
                     , fprintf (global.outfile, "Inconsistant call of %s found!",
                                global.prf_string[F_sel]););

    switch (to_sc) {
    case C_scl:
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        shp = (char **)ILIBmalloc (to_dim * sizeof (char *));
        for (i = 0; i < to_dim; i++) {
            shp[i] = (char *)ILIBmalloc ((2 * strlen (from_NT) + 50) * sizeof (char));
            if (from_dim >= 0) {
                sprintf (shp[i], "SAC_ND_A_SHAPE( %s, %d)", from_NT,
                         from_dim - (to_dim - i));
            } else {
                sprintf (shp[i], "SAC_ND_A_SHAPE( %s, SAC_ND_A_DIM( %s) - %d)", from_NT,
                         from_NT, to_dim - i);
            }
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, to_dim, shp);
        for (i = 0; i < to_dim; i++) {
            shp[i] = ILIBfree (shp[i]);
        }
        shp = ILIBfree (shp);
        break;

    case C_aud:
        /*
         * The dimension of 'to_NT' must be computed dynamically:
         *   shape( to_NT) := drop( size( idx_NT), shape( from_NT))
         * Unfortunately, such a drop-operation is not covered by the
         * function Set_Shape() for the time being ...
         *
         * Hence, F_sel is implemented for scalar results only!
         */
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == %d", from_NT,
                                  idx_size);
                         , fprintf (global.outfile, "Result of %s is not a scalar!",
                                    global.prf_string[F_sel]););
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
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
#ifndef DBUG_OFF
    int to_dim = DIM_NO_OFFSET (to_sdim);
#endif
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ("PrfSel_Data");

    DBUG_ASSERT ((to_dim == 0), "Primitive selection can only yield scalar results!");

    BLOCK_VARDECS (fprintf (global.outfile, "int SAC_idx;");
                   , Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun,
                                  from_NT, from_dim);
                   INDENT; fprintf (global.outfile,
                                    "SAC_ND_WRITE_READ_COPY( %s, 0, %s, SAC_idx, %s)\n",
                                    to_NT, from_NT, copyfun););

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (global.outfile, "1st argument of %s is not a vector!",
                                global.prf_string[F_sel]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_SIZE( %s)",
                              from_NT, idx_NT);
                     , fprintf (global.outfile,
                                "Length of index vector used for %s does not "
                                "match rank of argument array!",
                                global.prf_string[F_sel]););

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      idxs_ANY[i]);
                             , fprintf (global.outfile,
                                        "1st argument of %s is not a vector!",
                                        global.prf_string[F_sel]););
        }
    }
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == %d", from_NT,
                              idx_size);
                     , fprintf (global.outfile,
                                "Length of index vector used for %s does not "
                                "match rank of argument array!",
                                global.prf_string[F_sel]););

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
        BLOCK_VARDECS (fprintf (global.outfile, "int SAC_idx;");
                       ,
                       if (idx_unrolled) {
                           INDENT;
                           fprintf (global.outfile, "SAC_idx = ");
                           idx_read_fun (idx, NULL, 0);
                           fprintf (global.outfile, ";\n");
                       } else {
                           Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                        idx_read_fun, to_NT, to_dim);
                       } INDENT;
                       fprintf (global.outfile, "SAC_ND_WRITE_COPY( %s, SAC_idx, ",
                                to_NT);
                       ReadScalar (val_ANY, NULL, 0);
                       fprintf (global.outfile, " , %s)\n", copyfun););
    } else {
        /* 'val_ANY' is a tagged identifier representing a non-scalar array */
        BLOCK_VARDECS (fprintf (global.outfile, "int SAC_i, SAC_j, SAC_idx;");
                       ,
                       if (idx_unrolled) {
                           INDENT;
                           fprintf (global.outfile, "SAC_idx = ");
                           idx_read_fun (idx, NULL, 0);
                           fprintf (global.outfile, ";\n");
                       } else {
                           Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun,
                                        idx_read_fun, to_NT, to_dim);
                       } INDENT;
                       FOR_LOOP (fprintf (global.outfile, "SAC_i = SAC_idx, SAC_j = 0");
                                 , fprintf (global.outfile, "SAC_j < SAC_ND_A_SIZE( %s)",
                                            val_ANY);
                                 , fprintf (global.outfile, "SAC_i++, SAC_j++");, INDENT;
                                 fprintf (global.outfile,
                                          "SAC_ND_WRITE_READ_COPY("
                                          " %s, SAC_i, %s, SAC_j, %s)\n",
                                          to_NT, val_ANY, copyfun);););
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_ANY);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_string[F_modarray]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)",
                              from_NT, idx_NT);
                     , fprintf (global.outfile, "2nd argument of %s has illegal size!",
                                global.prf_string[F_modarray]););

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_ANY);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      idxs_ANY[i]);
                             , fprintf (global.outfile,
                                        "2nd argument of %s is not a vector",
                                        global.prf_string[F_modarray]););
        }
    }
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT,
                              idx_size);
                     , fprintf (global.outfile, "2nd argument of %s has illegal size!",
                                global.prf_string[F_modarray]););

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
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);
    char **shp;
    int i;

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) <= SAC_ND_A_DIM( %s)",
                              to_NT, from_NT);
                     , fprintf (global.outfile, "result of %s has illegal dimension!",
                                global.prf_string[F_idx_sel]););

    switch (to_sc) {
    case C_scl:
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        shp = (char **)ILIBmalloc (to_dim * sizeof (char *));
        for (i = 0; i < to_dim; i++) {
            shp[i] = (char *)ILIBmalloc ((2 * strlen (from_NT) + 50) * sizeof (char));
            if (from_dim >= 0) {
                sprintf (shp[i], "SAC_ND_A_SHAPE( %s, %d)", from_NT,
                         from_dim - (to_dim - i));
            } else {
                sprintf (shp[i], "SAC_ND_A_SHAPE( %s, SAC_ND_A_DIM( %s) - %d)", from_NT,
                         from_NT, to_dim - i);
            }
        }
        ICMCompileND_SET__SHAPE_arr (to_NT, to_dim, shp);
        for (i = 0; i < to_dim; i++) {
            shp[i] = ILIBfree (shp[i]);
        }
        shp = ILIBfree (shp);
        break;

    case C_aud:
        /*
         * F_idx_sel works only for arrays with known dimension!
         */
        DBUG_ASSERT ((0), "F_idx_sel with unknown dimension found!");
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SEL__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (global.outfile, "1st argument of %s is not a scalar!",
                                    global.prf_string[F_idx_sel]););
    }

    if (to_dim == 0) {
        /*
         * 'to_NT' is scalar
         */
        INDENT;
        fprintf (global.outfile, "SAC_ND_WRITE_READ_COPY( %s, 0, %s, ", to_NT, from_NT);
        ReadScalar (idx_ANY, NULL, 0);
        fprintf (global.outfile, ", %s)\n", copyfun);
    } else {
        /*
         * 'to_NT' is array
         */
        FOR_LOOP_VARDECS (fprintf (global.outfile, "int SAC_i, SAC_j;");
                          , fprintf (global.outfile, "SAC_i = 0, SAC_j = ");
                          ReadScalar (idx_ANY, NULL, 0);
                          , fprintf (global.outfile, "SAC_i < SAC_ND_A_SIZE( %s)", to_NT);
                          , fprintf (global.outfile, "SAC_i++, SAC_j++");, INDENT;
                          fprintf (global.outfile,
                                   "SAC_ND_WRITE_READ_COPY( %s, SAC_i, %s, SAC_j, %s)\n",
                                   to_NT, from_NT, copyfun););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_IDX_SHAPE_SEL__DATA( char *to_NT, int to_sdim,
 *                                              char *from_NT, int from_sdim,
 *                                              char *idx_ANY)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_SHAPE_SEL__DATA( to_NT, to_sdim, from_NT, from_sdim, idx_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_SHAPE_SEL__DATA (char *to_NT, int to_sdim, char *from_NT,
                                      int from_sdim, char *idx_ANY)
{
    int i;

    DBUG_ENTER ("ICMCompileND_PRF_IDX_SHAPE_SEL__DATA");

#define ND_PRF_IDX_SHAPE_SEL__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_SHAPE_SEL__DATA

    /*
     * CAUTION:
     * 'idx_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_SHAPE_SEL__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (global.outfile, "1st argument of %s is not a scalar!",
                                    global.prf_string[F_idx_shape_sel]););
    }

    /*
     * if the index is given by an id, we cannot use SAC_ND_A_SHAPE
     */
    if (idx_ANY[0] == '(') {
        if (from_sdim >= 0) {
            for (i = 0; i < from_sdim; i++) {
                INDENT;
                fprintf (global.outfile, "if ( %d == ", i);
                ReadScalar (idx_ANY, NULL, 0);
                fprintf (global.outfile,
                         ") { SAC_ND_CREATE__SCALAR__DATA( %s, SAC_ND_A_SHAPE( %s, %d)) "
                         "} else\n",
                         to_NT, from_NT, i);
            }
            INDENT;
            fprintf (global.outfile, "SAC_RuntimeError(\"Illegal shape access!\");\n");
        } else {
            INDENT;
            fprintf (global.outfile,
                     "SAC_ND_CREATE__SCALAR__DATA( %s, SAC_ND_A_DESC_SHAPE( %s, ", to_NT,
                     from_NT);
            ReadScalar (idx_ANY, NULL, 0);
            fprintf (global.outfile, "))\n");
        }
    } else {
        INDENT;
        fprintf (global.outfile, "SAC_ND_CREATE__SCALAR__DATA( %s, SAC_ND_A_SHAPE( %s, ",
                 to_NT, from_NT);
        ReadScalar (idx_ANY, NULL, 0);
        fprintf (global.outfile, "))\n");
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_IDX_MODARRAY__DATA( %s, %d, %s, %d, %s, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY, val_ANY);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (global.outfile, "2nd argument of %s is not a scalar!",
                                    global.prf_string[F_modarray]););
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_TAKE__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    if (cnt_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", cnt_ANY);
                         , fprintf (global.outfile, "1st argument of %s is not a scalar!",
                                    global.prf_string[F_take_SxV]););
    }

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_string[F_take_SxV]););

    shp = (char **)ILIBmalloc (sizeof (char *));
    shp[0] = (char *)ILIBmalloc ((strlen (cnt_ANY) + 30) * sizeof (char));
    if (cnt_ANY[0] == '(') {
        sprintf (shp[0], "SAC_ABS( SAC_ND_A_FIELD( %s))", cnt_ANY);
    } else {
        sprintf (shp[0], "SAC_ABS( %s)", cnt_ANY);
    }
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = ILIBfree (shp[0]);
    shp = ILIBfree (shp);

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_TAKE__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    BLOCK_VARDECS (fprintf (global.outfile, "int SAC_cnt, SAC_off;");, INDENT;
                   fprintf (global.outfile, "SAC_cnt = "); ReadScalar (cnt_ANY, NULL, 0);
                   fprintf (global.outfile, ";\n");

                   COND2 (fprintf (global.outfile, "("); ReadScalar (cnt_ANY, NULL, 0);
                          fprintf (global.outfile, " < 0)");, INDENT;
                          fprintf (global.outfile, "SAC_cnt = - SAC_cnt;\n"); INDENT;
                          fprintf (global.outfile,
                                   "SAC_off = SAC_ND_A_SIZE( %s) - SAC_cnt;\n", from_NT);
                          , INDENT; fprintf (global.outfile, "SAC_off = 0;\n"););

                   ASSURE_TYPE_ASS (fprintf (global.outfile,
                                             "SAC_cnt <= SAC_ND_A_SIZE( %s)", from_NT);
                                    , fprintf (global.outfile,
                                               "1st argument of %s is out of range!",
                                               global.prf_string[F_take_SxV]););

                   FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                                        , fprintf (global.outfile, "0");
                                        , fprintf (global.outfile, "SAC_cnt");, INDENT;
                                        fprintf (global.outfile,
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_DROP__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    if (cnt_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", cnt_ANY);
                         , fprintf (global.outfile, "1st argument of %s is not a scalar!",
                                    global.prf_string[F_drop_SxV]););
    }

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_string[F_drop_SxV]););

    shp = (char **)ILIBmalloc (sizeof (char *));
    shp[0]
      = (char *)ILIBmalloc ((strlen (from_NT) + strlen (cnt_ANY) + 50) * sizeof (char));
    if (cnt_ANY[0] == '(') {
        sprintf (shp[0], "SAC_ND_A_SIZE( %s) - SAC_ABS( SAC_ND_A_FIELD( %s))", from_NT,
                 cnt_ANY);
    } else {
        sprintf (shp[0], "SAC_ND_A_SIZE( %s) - SAC_ABS( %s)", from_NT, cnt_ANY);
    }
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = ILIBfree (shp[0]);
    shp = ILIBfree (shp);

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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_DROP__DATA( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    BLOCK_VARDECS (fprintf (global.outfile, "int SAC_cnt, SAC_off;");, INDENT;
                   fprintf (global.outfile, "SAC_off = "); ReadScalar (cnt_ANY, NULL, 0);
                   fprintf (global.outfile, ";\n");

                   COND2 (fprintf (global.outfile, "("); ReadScalar (cnt_ANY, NULL, 0);
                          fprintf (global.outfile, " < 0)");, INDENT;
                          fprintf (global.outfile,
                                   "SAC_cnt = SAC_ND_A_SIZE( %s) + SAC_off;\n", from_NT);
                          INDENT; fprintf (global.outfile, "SAC_off = 0;\n");, INDENT;
                          fprintf (global.outfile,
                                   "SAC_cnt = SAC_ND_A_SIZE( %s) - SAC_off;\n",
                                   from_NT););

                   ASSURE_TYPE_ASS (fprintf (global.outfile,
                                             "SAC_cnt <= SAC_ND_A_SIZE( %s)", from_NT);
                                    , fprintf (global.outfile,
                                               "1st argument of %s is out of range!",
                                               global.prf_string[F_drop_SxV]););

                   FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                                        , fprintf (global.outfile, "0");
                                        , fprintf (global.outfile, "SAC_cnt");, INDENT;
                                        fprintf (global.outfile,
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
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_CAT__SHAPE( %s, %d, %s, %d, %s, %d)\"))\n",
             to_NT, to_sdim, from1_NT, from1_sdim, from2_NT, from2_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from1_NT);
                     , fprintf (global.outfile, "1st argument of %s is not a vector!",
                                global.prf_string[F_cat_VxV]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from2_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_string[F_cat_VxV]););

    shp = (char **)ILIBmalloc (sizeof (char *));
    shp[0]
      = (char *)ILIBmalloc ((strlen (from1_NT) + strlen (from2_NT) + 40) * sizeof (char));
    sprintf (shp[0], "SAC_ND_A_SIZE( %s) + SAC_ND_A_SIZE( %s)", from1_NT, from2_NT);
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = ILIBfree (shp[0]);
    shp = ILIBfree (shp);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_PROP_OBJ_IN( char *to_NT, int to_sdim,
 *                                      char *from1_NT, int from1_sdim,
 *                                      char *from2_NT, int from2_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_PROP_OBJ_IN( vararg_cnt, vararg)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_PROP_OBJ_IN (int vararg_cnt, char **vararg)
{
    int i;
    DBUG_ENTER ("ICMCompileND_PRF_PROP_OBJ_IN");

#define ND_PRF_PROP_OBJ_IN
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_PROP_OBJ_IN

    INDENT;
    fprintf (global.outfile, "SAC_ND_PROP_OBJ_IN( )\n");

    for (i = 0; i < 2 * vararg_cnt; i += 2) {
        INDENT;
        fprintf (global.outfile, "SAC_ND_PROP_OBJ_UNBOX( %s, %s );\n", vararg[i],
                 vararg[i + 1]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_PROP_OBJ_OUT( char *to_NT, int to_sdim,
 *                                       char *from1_NT, int from1_sdim,
 *                                       char *from2_NT, int from2_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_PROP_OBJ_OUT( vararg_cnt, vararg)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_PROP_OBJ_OUT (int vararg_cnt, char **vararg)
{
    int i;
    DBUG_ENTER ("ICMCompileND_PRF_PROP_OBJ_OUT");

#define ND_PRF_PROP_OBJ_OUT
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_PROP_OBJ_OUT

    for (i = 0; i < 2 * vararg_cnt; i += 2) {
        INDENT;
        fprintf (global.outfile, "SAC_ND_PROP_OBJ_BOX( %s, %s );\n", vararg[i],
                 vararg[i + 1]);
    }

    INDENT;
    fprintf (global.outfile, "SAC_ND_PROP_OBJ_OUT( )\n");

    DBUG_VOID_RETURN;
}
