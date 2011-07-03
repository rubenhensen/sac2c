/*
 * $Id$
 */

#include <stdio.h>
#include <ctype.h>

#include "icm2c_basic.h"
#include "icm2c_utils.h"
#include "icm2c_std.h"
#include "icm2c_prf.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"
#include "free.h"
#include "str.h"
#include "memory.h"

#ifdef BEtest
#define MEMfree(x)                                                                       \
    x;                                                                                   \
    free (x)
#define MEMmalloc(x) malloc (x)
#endif /* BEtest */

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_SHAPE_A__DATA( char *to_NT, int to_sdim,
 *                                        char *from_NT, int from_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SHAPE_A__DATA( to_NT, to_sdim, from_NT, from_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SHAPE_A__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim)
{
    int i;
#ifndef DBUG_OFF
    hidden_class_t to_hc = ICUGetHiddenClass (to_NT);
#endif
    shape_class_t from_sc = ICUGetShapeClass (from_NT);
    int from_dim = DIM_NO_OFFSET (from_sdim);

    DBUG_ENTER ();

#define ND_PRF_SHAPE_A__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SHAPE_A__DATA

    DBUG_ASSERT (to_hc == C_nhd, "result of F_shape_A must be non-hidden!");

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SHAPE_A__DATA( %s, %d, %s, %d)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    switch (from_sc) {
    case C_scl:
        INDENT;
        fprintf (global.outfile, "SAC_NOOP()\n");
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT (from_dim >= 0, "illegal dimension found!");
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
        DBUG_ASSERT (0, "Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_RESHAPE_VxA__SHAPE_id( char *to_NT, int to_sdim,
 *                                                char *shp_NT)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE_VxA__SHAPE_id( to_NT, to_sdim, shp_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE_VxA__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT)
{
    DBUG_ENTER ();

#define ND_PRF_RESHAPE_VxA__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE_VxA__SHAPE_id

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE_VxA__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", shp_NT);
                     , fprintf (global.outfile, "1st argument of %s is not a vector!",
                                global.prf_name[F_reshape_VxA]););

    ICMCompileND_SET__SHAPE_id (to_NT, to_sdim, shp_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_RESHAPE_VxA__SHAPE_arr( char *to_NT, int to_sdim,
 *                                                 int shp_size, char **shp_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_RESHAPE_VxA__SHAPE_arr( to_NT, to_sdim, shp_size, shp_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_RESHAPE_VxA__SHAPE_arr (char *to_NT, int to_sdim, int shp_size,
                                         char **shp_ANY)
{
    int i;

    DBUG_ENTER ();

#define ND_PRF_RESHAPE_VxA__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_RESHAPE_VxA__SHAPE_arr

    /*
     * CAUTION:
     * 'shp_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_RESHAPE_VxA__SHAPE( %s, %d, ...)\"))\n",
             to_NT, to_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shp_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      shp_ANY[i]);
                             , fprintf (global.outfile,
                                        "1st argument of %s is not a vector!",
                                        global.prf_name[F_reshape_VxA]););
        }
    }

    ICMCompileND_SET__SHAPE_arr (to_NT, shp_size, shp_ANY);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL_VxA__SHAPE_id( char *to_NT, int to_sdim,
 *                                            char *from_NT, int from_sdim,
 *                                            char *idx_NT)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL_VxA__SHAPE_id( to_NT, to_sdim, from_NT, from_sdim, idx_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL_VxA__SHAPE_id (char *to_NT, int to_sdim, char *from_NT,
                                    int from_sdim, char *idx_NT)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);
    char **shp;
    int i;

    DBUG_ENTER ();

#define ND_PRF_SEL_VxA__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL_VxA__SHAPE_id

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL_VxA__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile,
                              "SAC_ND_A_DIM( %s) =="
                              " SAC_ND_A_DIM( %s) + SAC_ND_A_SIZE( %s)",
                              from_NT, to_NT, idx_NT);
                     , fprintf (global.outfile, "Inconsistant call of %s found!",
                                global.prf_name[F_sel_VxA]););

    switch (to_sc) {
    case C_scl:
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        shp = (char **)MEMmalloc (to_dim * sizeof (char *));
        for (i = 0; i < to_dim; i++) {
            shp[i] = (char *)MEMmalloc ((2 * STRlen (from_NT) + 50) * sizeof (char));
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
            shp[i] = MEMfree (shp[i]);
        }
        shp = MEMfree (shp);
        break;

    case C_aud:
        /*
         * The dimension of 'to_NT' must be computed dynamically:
         *   shape( to_NT) := drop( size( idx_NT), shape( from_NT))
         * Unfortunately, such a drop-operation is not covered by the
         * function Set_Shape() for the time being ...
         *
         * Hence, F_sel_VxA is implemented for scalar results only!
         */
        ASSURE_TYPE_ASS (fprintf (global.outfile,
                                  "SAC_ND_A_DIM( %s) == SAC_ND_A_SIZE( %s)", from_NT,
                                  idx_NT);
                         , fprintf (global.outfile, "Result of %s is not a scalar!",
                                    global.prf_name[F_sel_VxA]););
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    default:
        DBUG_ASSERT (0, "Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL_VxA__SHAPE_arr( char *to_NT, int to_sdim,
 *                                             char *from_NT, int from_sdim,
 *                                             int idx_size, char **idxs_ANY)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL_VxA__SHAPE_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                          idx_size, [ idxs_ANY ]* )
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL_VxA__SHAPE_arr (char *to_NT, int to_sdim, char *from_NT,
                                     int from_sdim, int idx_size, char **idxs_ANY)
{
    shape_class_t to_sc = ICUGetShapeClass (to_NT);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int from_dim = DIM_NO_OFFSET (from_sdim);
    char **shp;
    int i;

    DBUG_ENTER ();

#define ND_PRF_SEL_VxA__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL_VxA__SHAPE_arr

    /*
     * CAUTION:
     * 'idxs_ANY[i]' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL_VxA__SHAPE( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile,
                              "SAC_ND_A_DIM( %s) == SAC_ND_A_DIM( %s) + %d", from_NT,
                              to_NT, idx_size);
                     , fprintf (global.outfile, "Inconsistant call of %s found!",
                                global.prf_name[F_sel_VxA]););

    switch (to_sc) {
    case C_scl:
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        shp = (char **)MEMmalloc (to_dim * sizeof (char *));
        for (i = 0; i < to_dim; i++) {
            shp[i] = (char *)MEMmalloc ((2 * STRlen (from_NT) + 50) * sizeof (char));
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
            shp[i] = MEMfree (shp[i]);
        }
        shp = MEMfree (shp);
        break;

    case C_aud:
        /*
         * The dimension of 'to_NT' must be computed dynamically:
         *   shape( to_NT) := drop( size( idx_NT), shape( from_NT))
         * Unfortunately, such a drop-operation is not covered by the
         * function Set_Shape() for the time being ...
         *
         * Hence, F_sel_VxA is implemented for scalar results only!
         */
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == %d", from_NT,
                                  idx_size);
                         , fprintf (global.outfile, "Result of %s is not a scalar!",
                                    global.prf_name[F_sel_VxA]););
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    default:
        DBUG_ASSERT (0, "Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

    DBUG_ASSERT (to_dim == 0, "Primitive selection can only yield scalar results!");

    BLOCK_VARDECS (fprintf (global.outfile, "int SAC_idx;");
                   , Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun,
                                  from_NT, from_dim);
                   INDENT; fprintf (global.outfile,
                                    "SAC_ND_WRITE_READ_COPY( %s, 0, %s, SAC_idx, %s)\n",
                                    to_NT, from_NT, copyfun););

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL_VxA__DATA_id( char *to_NT, int to_sdim,
 *                                           char *from_NT, int from_sdim,
 *                                           char *idx_NT, int idx_size,
 *                                           char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL_VxA__DATA_id( to_NT, to_sdim, from_NT, from_sdim, idx_NT, idx_size,
 *                            copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL_VxA__DATA_id (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                   char *idx_NT, int idx_size, char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_SEL_VxA__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL_VxA__DATA_id

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL_VxA__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (global.outfile, "1st argument of %s is not a vector!",
                                global.prf_name[F_sel_VxA]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == SAC_ND_A_SIZE( %s)",
                              from_NT, idx_NT);
                     , fprintf (global.outfile,
                                "Length of index vector used for %s does not "
                                "match rank of argument array!",
                                global.prf_name[F_sel_VxA]););

    PrfSel_Data (to_NT, to_sdim, from_NT, from_sdim, idx_NT, idx_size, SizeId, ReadId,
                 copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_SEL_VxA__DATA_arr( char *to_NT, int to_sdim,
 *                                            char *from_NT, int from_sdim,
 *                                            int idx_size, char **idxs_ANY,
 *                                            char *copyfun)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_SEL_VxA__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                             idx_size, [ idxs_ANY ]* , copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_SEL_VxA__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                    int from_sdim, int idx_size, char **idxs_ANY,
                                    char *copyfun)
{
    int i;

    DBUG_ENTER ();

#define ND_PRF_SEL_VxA__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_SEL_VxA__DATA_arr

    /*
     * CAUTION:
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_SEL_VxA__DATA( %s, %d, %s, %d, ...)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      idxs_ANY[i]);
                             , fprintf (global.outfile,
                                        "1st argument of %s is not a vector!",
                                        global.prf_name[F_sel_VxA]););
        }
    }
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == %d", from_NT,
                              idx_size);
                     , fprintf (global.outfile,
                                "Length of index vector used for %s does not "
                                "match rank of argument array!",
                                global.prf_name[F_sel_VxA]););

    PrfSel_Data (to_NT, to_sdim, from_NT, from_sdim, idxs_ANY, idx_size, NULL,
                 ReadConstArray_Str, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void PrfModarrayScalarVal_Data( char *to_NT, int to_sdim,
 *                                char *from_NT, int from_sdim,
 *                                bool idx_unrolled, void *idx, int idx_size,
 *                                void (*idx_size_fun)( void *),
 *                                void (*idx_read_fun)( void *, char *, int),
 *                                char *val_scalar,
 *                                char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._MODARRAY__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfModarrayScalarVal_Data (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                           bool idx_unrolled, void *idx, int idx_size,
                           void (*idx_size_fun) (void *),
                           void (*idx_read_fun) (void *, char *, int), char *val_scalar,
                           char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ();

    INDENT;
    BLOCK_VARDECS (fprintf (global.outfile, "int SAC_idx;");
                   ,
                   if (idx_unrolled) {
                       INDENT;
                       fprintf (global.outfile, "SAC_idx = ");
                       idx_read_fun (idx, NULL, 0);
                       fprintf (global.outfile, ";\n");
                   } else {
                       Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun,
                                    to_NT, to_dim);
                   } INDENT;
                   fprintf (global.outfile, "SAC_ND_WRITE_COPY( %s, SAC_idx, ", to_NT);
                   ReadScalar (val_scalar, NULL, 0);
                   fprintf (global.outfile, " , %s)\n", copyfun););

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void PrfModarrayArrayVal_Data( char *to_NT, int to_sdim,
 *                               char *from_NT, int from_sdim,
 *                               bool idx_unrolled, void *idx, int idx_size,
 *                               void (*idx_size_fun)( void *),
 *                               void (*idx_read_fun)( void *, char *, int),
 *                               char *val_array,
 *                               char *copyfun)
 *
 * Description:
 *   implements all the ND_PRF_..._MODARRAY__DATA_... ICMs.
 *
 ******************************************************************************/

static void
PrfModarrayArrayVal_Data (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                          bool idx_unrolled, void *idx, int idx_size,
                          void (*idx_size_fun) (void *),
                          void (*idx_read_fun) (void *, char *, int), char *val_array,
                          char *copyfun)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ();

    BLOCK_VARDECS (fprintf (global.outfile, "int SAC_i, SAC_j, SAC_idx;");
                   ,
                   if (idx_unrolled) {
                       INDENT;
                       fprintf (global.outfile, "SAC_idx = ");
                       idx_read_fun (idx, NULL, 0);
                       fprintf (global.outfile, ";\n");
                   } else {
                       Vect2Offset ("SAC_idx", idx, idx_size, idx_size_fun, idx_read_fun,
                                    to_NT, to_dim);
                   } INDENT;
                   FOR_LOOP (fprintf (global.outfile, "SAC_i = SAC_idx, SAC_j = 0");
                             , fprintf (global.outfile, "SAC_j < SAC_ND_A_SIZE( %s)",
                                        val_array);
                             , fprintf (global.outfile, "SAC_i++, SAC_j++");, INDENT;
                             fprintf (global.outfile,
                                      "SAC_ND_WRITE_READ_COPY("
                                      " %s, SAC_i, %s, SAC_j, %s)\n",
                                      to_NT, val_array, copyfun);););

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_id( char *to_NT, int to_sdim,
 *                                                  char *from_NT, int from_sdim,
 *                                                  char *idx_NT, int idx_size,
 *                                                  char *val_scalar, char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                             idx_NT, idx_size, val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxS__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                          int from_sdim, char *idx_NT, int idx_size,
                                          char *val_scalar, char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_MODARRAY_AxVxS__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxS__DATA_id

    /*
     * CAUTION:
     * 'val_ANY' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY_AxVxS__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_scalar);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_name[F_modarray_AxVxS]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)",
                              from_NT, idx_NT);
                     , fprintf (global.outfile, "2nd argument of %s has illegal size!",
                                global.prf_name[F_modarray_AxVxS]););

    PrfModarrayScalarVal_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idx_NT,
                               idx_size, SizeId, ReadId, val_scalar, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_arr( char *to_NT, int to_sdim,
 *                                                   char *from_NT, int from_sdim,
 *                                                   int idx_size, char **idxs_ANY,
 *                                                   char *val_scalar,
 *                                                   char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                              idx_size, [ idxs_ANY ]* , val_scalar, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxS__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                           int from_sdim, int idx_size, char **idxs_ANY,
                                           char *val_scalar, char *copyfun)
{
    int i;

    DBUG_ENTER ();

#define ND_PRF_MODARRAY_AxVxS__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxS__DATA_arr

    /*
     * CAUTION:
     * 'idxs_ANY[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY_AxVxS__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_scalar);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      idxs_ANY[i]);
                             , fprintf (global.outfile,
                                        "2nd argument of %s is not a vector",
                                        global.prf_name[F_modarray_AxVxS]););
        }
    }
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT,
                              idx_size);
                     , fprintf (global.outfile, "2nd argument of %s has illegal size!",
                                global.prf_name[F_modarray_AxVxS]););

    PrfModarrayScalarVal_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idxs_ANY,
                               idx_size, NULL, ReadConstArray_Str, val_scalar, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_id( char *to_NT, int to_sdim,
 *                                                  char *from_NT, int from_sdim,
 *                                                  char *idx_NT, int idx_size,
 *                                                  char *val_scalar, char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxS__DATA_id( to_NT, to_sdim, from_NT, from_sdim,
 *                             idx_NT, idx_size, val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxA__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                          int from_sdim, char *idx_NT, int idx_size,
                                          char *val_array, char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_MODARRAY_AxVxA__DATA_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxA__DATA_id

    /*
     * CAUTION:
     * 'val_ANY' is either a tagged identifier or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_MODARRAY_AxVxA__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_array);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", idx_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_name[F_modarray_AxVxA]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) >= SAC_ND_A_SIZE( %s)",
                              from_NT, idx_NT);
                     , fprintf (global.outfile, "2nd argument of %s has illegal size!",
                                global.prf_name[F_modarray_AxVxA]););

    PrfModarrayArrayVal_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idx_NT, idx_size,
                              SizeId, ReadId, val_array, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_MODARRAY_AxVxA__DATA_arr( char *to_NT, int to_sdim,
 *                                                   char *from_NT, int from_sdim,
 *                                                   int idx_size, char **idxs_ANY,
 *                                                   char *val_array,
 *                                                   char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_MODARRAY_AxVxA__DATA_arr( to_NT, to_sdim, from_NT, from_sdim,
 *                              idx_size, [ idxs_ANY ]* , val_ANY, copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_MODARRAY_AxVxA__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                           int from_sdim, int idx_size, char **idxs_ANY,
                                           char *val_array, char *copyfun)
{
    int i;

    DBUG_ENTER ();

#define ND_PRF_MODARRAY_AxVxA__DATA_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_MODARRAY_AxVxA__DATA_arr

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
             " (\"ND_PRF_MODARRAY_AxVxA__DATA( %s, %d, %s, %d, ..., %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, val_array);

    for (i = 0; i < idx_size; i++) {
        if (idxs_ANY[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0",
                                      idxs_ANY[i]);
                             , fprintf (global.outfile,
                                        "2nd argument of %s is not a vector",
                                        global.prf_name[F_modarray_AxVxA]););
        }
    }
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) >= %d", from_NT,
                              idx_size);
                     , fprintf (global.outfile, "2nd argument of %s has illegal size!",
                                global.prf_name[F_modarray_AxVxA]););

    PrfModarrayArrayVal_Data (to_NT, to_sdim, from_NT, from_sdim, FALSE, idxs_ANY,
                              idx_size, NULL, ReadConstArray_Str, val_array, copyfun);

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
                                global.prf_name[F_idx_sel]););

    switch (to_sc) {
    case C_scl:
        ICMCompileND_SET__SHAPE_arr (to_NT, 0, NULL);
        break;

    case C_aks:
        /* here is no break missing */
    case C_akd:
        DBUG_ASSERT (to_dim >= 0, "illegal dimension found!");
        shp = (char **)MEMmalloc (to_dim * sizeof (char *));
        for (i = 0; i < to_dim; i++) {
            shp[i] = (char *)MEMmalloc ((2 * STRlen (from_NT) + 50) * sizeof (char));
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
            shp[i] = MEMfree (shp[i]);
        }
        shp = MEMfree (shp);
        break;

    case C_aud:
        /*
         * F_idx_sel works only for arrays with known dimension!
         */
        DBUG_ASSERT (0, "F_idx_sel with unknown dimension found!");
        break;

    default:
        DBUG_ASSERT (0, "Unknown shape class found!");
        break;
    }

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
                                    global.prf_name[F_idx_sel]););
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

    DBUG_RETURN ();
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

    DBUG_ENTER ();

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
                                    global.prf_name[F_idx_shape_sel]););
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

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_IDX_MODARRAY_AxSxS__DATA( char *to_NT, int to_sdim,
 *                                                   char *from_NT, int from_sdim,
 *                                                   char *idx_ANY, char *val_ANY,
 *                                                   char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_MODARRAY_AxSxS__DATA( to_NT, to_sdim, from_NT, from_sdim, idx, val,
 *                                    copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_MODARRAY_AxSxS__DATA (char *to_NT, int to_sdim, char *from_NT,
                                           int from_sdim, char *idx_ANY, char *val_scalar,
                                           char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_IDX_MODARRAY_AxSxS__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_MODARRAY_AxSxS__DATA

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
             " (\"ND_PRF_IDX_MODARRAY_AxSxS__DATA( %s, %d, %s, %d, %s, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY, val_scalar);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (global.outfile, "2nd argument of %s is not a scalar!",
                                    global.prf_name[F_idx_modarray_AxSxS]););
    }

    PrfModarrayScalarVal_Data (to_NT, to_sdim, from_NT, from_sdim, TRUE, idx_ANY, 1, NULL,
                               ReadScalar, val_scalar, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_IDX_MODARRAY_AxSxA__DATA( char *to_NT, int to_sdim,
 *                                                   char *from_NT, int from_sdim,
 *                                                   char *idx_ANY, char *val_array,
 *                                                   char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_IDX_MODARRAY_AxSxA__DATA( to_NT, to_sdim, from_NT, from_sdim, idx, val,
 *                                    copyfun)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_IDX_MODARRAY_AxSxA__DATA (char *to_NT, int to_sdim, char *from_NT,
                                           int from_sdim, char *idx_ANY, char *val_array,
                                           char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_IDX_MODARRAY_AxSxA__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_IDX_MODARRAY_AxSxA__DATA

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
             " (\"ND_PRF_IDX_MODARRAY_AxSxA__DATA( %s, %d, %s, %d, %s, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, idx_ANY, val_array);

    if (idx_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", idx_ANY);
                         , fprintf (global.outfile, "2nd argument of %s is not a scalar!",
                                    global.prf_name[F_idx_modarray_AxSxA]););
    }

    PrfModarrayArrayVal_Data (to_NT, to_sdim, from_NT, from_sdim, TRUE, idx_ANY, 1, NULL,
                              ReadScalar, val_array, copyfun);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_TAKE_SxV__SHAPE( char *to_NT, int to_sdim,
 *                                          char *from_NT, int from_sdim,
 *                                          char *cnt_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_TAKE_SxV__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_TAKE_SxV__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                  char *cnt_ANY)
{
    char **shp;

    DBUG_ENTER ();

#define ND_PRF_TAKE_SxV__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_TAKE_SxV__SHAPE

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_TAKE_SxV__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    if (cnt_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", cnt_ANY);
                         , fprintf (global.outfile, "1st argument of %s is not a scalar!",
                                    global.prf_name[F_take_SxV]););
    }

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_name[F_take_SxV]););

    shp = (char **)MEMmalloc (sizeof (char *));
    shp[0] = (char *)MEMmalloc ((STRlen (cnt_ANY) + 30) * sizeof (char));
    if (cnt_ANY[0] == '(') {
        sprintf (shp[0], "SAC_ABS( SAC_ND_A_FIELD( %s))", cnt_ANY);
    } else {
        sprintf (shp[0], "SAC_ABS( %s)", cnt_ANY);
    }
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = MEMfree (shp[0]);
    shp = MEMfree (shp);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_TAKE_SxV__DATA( char *to_NT, int to_sdim,
 *                                         char *from_NT, int from_sdim,
 *                                         char *cnt_ANY,
 *                                         char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_TAKE_SxV__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_TAKE_SxV__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 char *cnt_ANY, char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_TAKE_SxV__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_TAKE_SxV__DATA

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_TAKE_SxV__DATA( %s, %d, %s, %d, %s)\"))\n",
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
                                               global.prf_name[F_take_SxV]););

                   FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                                        , fprintf (global.outfile, "0");
                                        , fprintf (global.outfile, "SAC_cnt");, INDENT;
                                        fprintf (global.outfile,
                                                 "SAC_ND_WRITE_READ_COPY( %s, SAC_i,"
                                                 " %s, SAC_off + SAC_i, %s);\n",
                                                 to_NT, from_NT, copyfun);););

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_DROP_SxV__SHAPE( char *to_NT, int to_sdim,
 *                                          char *from_NT, int from_sdim,
 *                                          char *cnt_ANY)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_DROP_SxV__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_DROP_SxV__SHAPE (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                  char *cnt_ANY)
{
    char **shp;

    DBUG_ENTER ();

#define ND_PRF_DROP_SxV__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_DROP_SxV__SHAPE

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_DROP_SxV__SHAPE( %s, %d, %s, %d, %s)\"))\n",
             to_NT, to_sdim, from_NT, from_sdim, cnt_ANY);

    if (cnt_ANY[0] == '(') {
        ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 0", cnt_ANY);
                         , fprintf (global.outfile, "1st argument of %s is not a scalar!",
                                    global.prf_name[F_drop_SxV]););
    }

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_name[F_drop_SxV]););

    shp = (char **)MEMmalloc (sizeof (char *));
    shp[0]
      = (char *)MEMmalloc ((STRlen (from_NT) + STRlen (cnt_ANY) + 50) * sizeof (char));
    if (cnt_ANY[0] == '(') {
        sprintf (shp[0], "SAC_ND_A_SIZE( %s) - SAC_ABS( SAC_ND_A_FIELD( %s))", from_NT,
                 cnt_ANY);
    } else {
        sprintf (shp[0], "SAC_ND_A_SIZE( %s) - SAC_ABS( %s)", from_NT, cnt_ANY);
    }
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = MEMfree (shp[0]);
    shp = MEMfree (shp);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_DROP_SxV__DATA( char *to_NT, int to_sdim,
 *                                         char *from_NT, int from_sdim,
 *                                         char *cnt_ANY,
 *                                         char *copyfun)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_DROP_SxV__DATA( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_DROP_SxV__DATA (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                 char *cnt_ANY, char *copyfun)
{
    DBUG_ENTER ();

#define ND_PRF_DROP_SxV__DATA
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_DROP_SxV__DATA

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_DROP_SxV__DATA( %s, %d, %s, %d, %s)\"))\n",
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
                                               global.prf_name[F_drop_SxV]););

                   FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                                        , fprintf (global.outfile, "0");
                                        , fprintf (global.outfile, "SAC_cnt");, INDENT;
                                        fprintf (global.outfile,
                                                 "SAC_ND_WRITE_READ_COPY( %s, SAC_i,"
                                                 " %s, SAC_off + SAC_i, %s);\n",
                                                 to_NT, from_NT, copyfun);););

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_PRF_CAT_VxV__SHAPE( char *to_NT, int to_sdim,
 *                                         char *from1_NT, int from1_sdim,
 *                                         char *from2_NT, int from2_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_CAT_VxV__SHAPE( to_NT, to_sdim, from_NT, from_sdim, cnt_ANY)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_CAT_VxV__SHAPE (char *to_NT, int to_sdim, char *from1_NT, int from1_sdim,
                                 char *from2_NT, int from2_sdim)
{
    char **shp;

    DBUG_ENTER ();

#define ND_PRF_CAT_VxV__SHAPE
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_CAT_VxV__SHAPE

    /*
     * CAUTION:
     * 'cnt_ANY' is either a tagged identifier (representing a scalar)
     * or a constant scalar!
     */

    INDENT;
    fprintf (global.outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_PRF_CAT_VxV__SHAPE( %s, %d, %s, %d, %s, %d)\"))\n",
             to_NT, to_sdim, from1_NT, from1_sdim, from2_NT, from2_sdim);

    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from1_NT);
                     , fprintf (global.outfile, "1st argument of %s is not a vector!",
                                global.prf_name[F_cat_VxV]););
    ASSURE_TYPE_ASS (fprintf (global.outfile, "SAC_ND_A_DIM( %s) == 1", from2_NT);
                     , fprintf (global.outfile, "2nd argument of %s is not a vector!",
                                global.prf_name[F_cat_VxV]););

    shp = (char **)MEMmalloc (sizeof (char *));
    shp[0]
      = (char *)MEMmalloc ((STRlen (from1_NT) + STRlen (from2_NT) + 40) * sizeof (char));
    sprintf (shp[0], "SAC_ND_A_SIZE( %s) + SAC_ND_A_SIZE( %s)", from1_NT, from2_NT);
    ICMCompileND_SET__SHAPE_arr (to_NT, 1, shp);
    shp[0] = MEMfree (shp[0]);
    shp = MEMfree (shp);

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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

    DBUG_RETURN ();
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
    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void ICMCompileND_PRF_TYPE_CONSTRAINT_AKS
 *
 *****************************************************************************/
void
ICMCompileND_PRF_TYPE_CONSTRAINT_AKS (char *to_NT, char *from_NT, int dim, int *shp)
{
    int i;

    DBUG_ENTER ();

    COND1 (
      fprintf (global.outfile, "(SAC_ND_A_DIM(%s) != %d)", from_NT, dim);
      for (i = 0; i < dim; i++) {
          fprintf (global.outfile, "|| (SAC_ND_A_SHAPE(%s,%d) != %d)", from_NT, i,
                   shp[i]);
      },
      fprintf (global.outfile,
               "SAC_RuntimeErrorLine(%d, \"Array '\" TO_STR( NT_NAME( %s)) \"' does not "
               "adhere "
               "to type constraint\");\n",
               global.linenum, from_NT););

    INDENT;
    fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = 1;\n", to_NT);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void ICMCompileND_PRF_SAME_SHAPE
 *
 *****************************************************************************/
void
ICMCompileND_PRF_SAME_SHAPE (char *to_NT, char *from_NT, int from_sdim, char *from2_NT,
                             int from2_sdim)
{
    int i;
    int dim = ARRAY_OR_SCALAR;

    DBUG_ENTER ();

    if (KNOWN_DIMENSION (from_sdim)) {
        dim = DIM_NO_OFFSET (from_sdim);
    } else if (KNOWN_DIMENSION (from2_sdim)) {
        dim = DIM_NO_OFFSET (from2_sdim);
    }

    if (dim != ARRAY_OR_SCALAR) {
        /*
         * At least one array has a known number of axes:
         * access shape vectors using compile-time constants, thereby
         * potentially accessing mirrors
         */
        COND1 (
          fprintf (global.outfile, "(SAC_ND_A_DIM(%s) != SAC_ND_A_DIM(%s))", from_NT,
                   from2_NT);
          for (i = 0; i < dim; i++) {
              fprintf (global.outfile,
                       "|| (SAC_ND_A_SHAPE(%s,%d) != SAC_ND_A_SHAPE(%s,%d))", from_NT, i,
                       from2_NT, i);
          },
          fprintf (global.outfile, "SAC_RuntimeError(\"Arrays do not adhere "
                                   "to same shape constraint\");\n"););
    } else {
        /*
         * Both arrays are AUD:
         * Compare descriptor contents using a run-time for-loop
         */
        COND1 (fprintf (global.outfile, "SAC_ND_A_DIM(%s) != SAC_ND_A_DIM(%s)", from_NT,
                        from2_NT);
               , fprintf (global.outfile, "SAC_RuntimeError(\"Arrays do not adhere "
                                          "to same shape constraint\");\n"););
        FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                             , fprintf (global.outfile, "0");
                             , fprintf (global.outfile, "SAC_ND_A_DIM(%s)", from_NT);
                             ,
                             COND1 (fprintf (global.outfile,
                                             "SAC_ND_A_SHAPE(%s,SAC_i) != "
                                             "SAC_ND_A_SHAPE(%s,SAC_i)",
                                             from_NT, from2_NT);
                                    , fprintf (global.outfile,
                                               "SAC_RuntimeError(\"Arrays do not adhere "
                                               "to same shape constraint\");\n");););
    }

    INDENT;
    fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = 1;\n", to_NT);

    DBUG_RETURN ();
}

#if 0
/** <!--********************************************************************-->
 *
 * @fn void ICMCompileND_PRF_SHAPE_MATCHES_DIM
 *
 *****************************************************************************/
void ICMCompileND_PRF_SHAPE_MATCHES_DIM( char *to_NT, char *from_NT, 
                                         char *from2_NT)
{
  DBUG_ENTER ();

  COND1(
    fprintf( global.outfile,
             "(SAC_ND_A_DIM(%s) != 1) || "
             "(SAC_ND_A_SHAPE(%s,0) != SAC_ND_A_DIM(%s))",
             from_NT, from_NT, from2_NT);
  ,
    fprintf( global.outfile, 
             "SAC_RuntimeError(\"Arrays do not adhere "
             "to shape matches dim constraint\");\n");
  );

  INDENT;
  fprintf( global.outfile, "SAC_ND_A_FIELD( %s) = 1;\n", to_NT);

  DBUG_RETURN ();
}
#endif

/** <!--********************************************************************-->
 *
 * @fn void ICMCompileND_PRF_VAL_LT_SHAPE_VxA
 *
 *****************************************************************************/
void
ICMCompileND_PRF_VAL_LT_SHAPE_VxA (char *to_NT, char *from_NT, char *from2_NT,
                                   int from2_sdim)
{
    int i;

    DBUG_ENTER ();

    COND1 (fprintf (global.outfile,
                    "(SAC_ND_A_DIM(%s) != 1) &&"
                    "(SAC_ND_A_SHAPE(%s,0) != SAC_ND_A_DIM(%s))",
                    from_NT, from_NT, from2_NT);
           , fprintf (global.outfile, "SAC_RuntimeError(\"Arrays do not adhere "
                                      "to val less than shape constraint\");\n"););

    if (KNOWN_DIMENSION (from2_sdim)) {
        COND1 (
          fprintf (global.outfile, "(0)");
          for (i = 0; i < DIM_NO_OFFSET (from2_sdim); i++) {
              fprintf (global.outfile, "|| (SAC_ND_READ(%s,%d) >= SAC_ND_A_SHAPE(%s,%d))",
                       from_NT, i, from2_NT, i);
          },
          fprintf (global.outfile, "SAC_RuntimeError(\"Arrays do not adhere "
                                   "to val less than shape constraint\");\n"););
    } else {
        FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                             , fprintf (global.outfile, "0");
                             , fprintf (global.outfile, "SAC_ND_A_DIM(%s)", from2_NT);
                             ,
                             COND1 (fprintf (global.outfile,
                                             "SAC_ND_READ(%s,SAC_i) >= "
                                             "SAC_ND_A_SHAPE(%s,SAC_i)",
                                             from_NT, from2_NT);
                                    , fprintf (global.outfile,
                                               "SAC_RuntimeError(\"Arrays do not adhere "
                                               "to val less than shape "
                                               "constraint\");\n");););
    }

    INDENT;
    fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = 1;\n", to_NT);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn void ICMCompileND_PRF_PROD_MATCHES_PROD_SHAPE
 *
 *****************************************************************************/
void
ICMCompileND_PRF_PROD_MATCHES_PROD_SHAPE (char *to_NT, char *from_NT, char *from2_NT,
                                          int from2_sdim)
{
    DBUG_ENTER ();

    BLOCK_VARDECS (
      fprintf (global.outfile, "int SAC_p1 = 1; int SAC_p2 = 1;");
      , FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                             , fprintf (global.outfile, "0");
                             , fprintf (global.outfile, "SAC_ND_A_SHAPE(%s,0)", from_NT);
                             , fprintf (global.outfile,
                                        "SAC_p1 *= SAC_ND_READ(%s,SAC_i);", from_NT););

      if (KNOWN_DIMENSION (from2_sdim)) {
          int i;
          for (i = 0; i < DIM_NO_OFFSET (from2_sdim); i++) {
              fprintf (global.outfile, "SAC_p2 *= SAC_ND_A_SHAPE(%s,%d);\n", from2_NT, i);
              INDENT;
          }
      } else {
          FOR_LOOP_INC_VARDEC (fprintf (global.outfile, "SAC_i");
                               , fprintf (global.outfile, "0");
                               , fprintf (global.outfile, "SAC_ND_A_DIM(%s)", from2_NT);
                               , fprintf (global.outfile,
                                          "SAC_p2 *= SAC_ND_A_SHAPE(%s,SAC_i);",
                                          from2_NT););
      }

      COND1 (fprintf (global.outfile, "SAC_p1 != SAC_p2");
             , fprintf (global.outfile, "SAC_RuntimeError(\"Arrays do not adhere "
                                        "to prod matches prod shape constraint\");\n");

      ););
    INDENT;
    fprintf (global.outfile, "SAC_ND_A_FIELD( %s) = 1;\n", to_NT);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void ICMCompileND_PRF_COND( char *cond_NT, char *then_NT, char *else_NT)
 *
 * Description:
 *   implements the compilation of the following ICM:
 *
 *   ND_PRF_COND( cond_NT, then_NT, else_NT)
 *
 ******************************************************************************/

void
ICMCompileND_PRF_COND (char *to_NT, char *cond_NT, char *then_NT, char *else_NT)
{
    DBUG_ENTER ();

#define ND_PRF_COND
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_PRF_COND

    INDENT;
    fprintf (global.outfile, "if( NT_NAME( %s)) {\n", cond_NT);

    INDENT;
    INDENT;
    fprintf (global.outfile, "NT_NAME( %s) = NT_NAME( %s);\n", to_NT, then_NT);

    INDENT;
    fprintf (global.outfile, "} else {\n");

    INDENT;
    INDENT;
    fprintf (global.outfile, "NT_NAME( %s) = NT_NAME( %s);\n", to_NT, else_NT);

    INDENT;
    fprintf (global.outfile, "}\n");

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
