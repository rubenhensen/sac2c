/*
 *
 * $Log$
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
 * some bugs with TAGGED_ARRAYS fixed
 *
 * Revision 3.21  2002/08/06 12:18:58  dkr
 * ups, ... error in WL_BEGIN__OFFSET corrected
 *
 * Revision 3.20  2002/08/06 12:08:09  dkr
 * some cosmetical changes done
 *
 * Revision 3.19  2002/08/06 08:58:30  dkr
 * works also without TAGGED_ARRAYS, now
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
 * some modifications for TAGGED_ARRAYS done
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

#include "dbug.h"
#include "my_debug.h"
#include "convert.h"
#include "globals.h"
#include "print.h"
#include "gen_startup_code.h"

/******************************************************************************
 *
 * Function:
 *   void PrintTraceICM( char *to_nt, char *idx_vec_nt,
 *                       int dims, char **idxa_scl,
 *                       char *operation, bool print_offset)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
PrintTraceICM (char *to_nt, char *idx_vec_nt, int dims, char **idxa_scl, char *operation,
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
    fprintf (outfile, "\", %s", idxa_scl[0]);
    for (i = 1; i < dims; i++) {
        fprintf (outfile, ", %s", idxa_scl[i]);
    }
    if (print_offset) {
        fprintf (outfile, ", ");
        for (i = dims - 1; i > 0; i--) {
            fprintf (outfile, "( SAC_ND_A_SHAPE( %s, %d) * ", to_nt, i);
        }
        fprintf (outfile, "%s ", idxa_scl[0]);
        for (i = 1; i < dims; i++) {
            fprintf (outfile, "+ %s )", idxa_scl[i]);
        }
        fprintf (outfile, ", SAC_WL_OFFSET( %s)", to_nt);
    }
    fprintf (outfile, "));\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void DefineShapeFactor( char *to_nt, int to_sdim, int current_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
DefineShapeFactor (char *to_nt, int to_sdim, int current_dim)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int i;

    DBUG_ENTER ("DefineShapeFactor");

    INDENT;
    fprintf (outfile, "int SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d) = 1", to_nt,
             current_dim);
    if (to_dim >= 0) {
        for (i = current_dim + 1; i < to_dim; i++) {
            fprintf (outfile, " * SAC_ND_A_SHAPE( %s, %d)", to_nt, i);
        }
        fprintf (outfile, ";\n");
    } else {
        fprintf (outfile, ";\n");
        FOR_LOOP_INC (fprintf (outfile, "SAC_i");
                      , fprintf (outfile, "%d", current_dim + 1);
                      , fprintf (outfile, "SAC_ND_A_DIM( %s)", to_nt);, INDENT;
                      fprintf (outfile,
                               "SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d)"
                               " *= SAC_ND_A_SHAPE( %s, SAC_i);\n",
                               to_nt, current_dim, to_nt););
    }

    DBUG_VOID_RETURN;
}

#ifdef TAGGED_ARRAYS

/******************************************************************************
 *
 * Function:
 *   void WLGenarray_Shape( char *to_nt, int to_sdim,
 *                          void *shp, int shp_size,
 *                          void (*shp_size_fun)( void *),
 *                          void (*shp_read_fun)( void *, char *, int),
 *                          char *val_nt)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
WLGenarray_Shape (char *to_nt, int to_sdim, void *shp, int shp_size,
                  void (*shp_size_fun) (void *),
                  void (*shp_read_fun) (void *, char *, int), char *val_nt, int val_sdim)
{
    int i;
    shape_class_t to_sc = ICUGetShapeClass (to_nt);
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int val_dim = DIM_NO_OFFSET (val_sdim);

    DBUG_ENTER ("WLGenarray_Shape");

    /*
     * ND_A_DESC_DIM, ND_A_MIRROR_DIM have already been set by ND_ALLOC__DESC!
     */
    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == ", to_nt);
                     GetAttr (shp, shp_size, shp_size_fun);
                     fprintf (outfile, " + SAC_ND_A_DIM( %s)", val_nt);
                     , fprintf (outfile, "Assignment with incompatible types found!"););

    /*
     * set descriptor and non-constant part of mirror
     */
    switch (to_sc) {
    case C_aud:
        BLOCK_VARDECS (fprintf (outfile, "int SAC_i, "); if (
                         val_dim
                         < 0) { fprintf (outfile, "SAC_j, "); } fprintf (outfile,
                                                                         "SAC_size = 1;");
                       ,
                       if (shp_size >= 0) {
                           SET_SHAPES_AUD__NUM (to_nt, i, 0, shp_size, INDENT;
                                                fprintf (outfile, "SAC_size *= \n");
                                                , shp_read_fun (shp, NULL, i););
                           INDENT;
                           /* to ease the code generation for the next loop */
                           fprintf (outfile, "SAC_i = %d;", shp_size);
                       } else {
                           SET_SHAPES_AUD (to_nt, fprintf (outfile, "SAC_i");
                                           , fprintf (outfile, "0");, shp_size_fun (shp);
                                           , INDENT; fprintf (outfile, "SAC_size *= \n");
                                           , shp_read_fun (shp, "SAC_i", -1););
                       }

                       if (val_dim >= 0) {
                           for (i = 0; i < val_dim; i++) {
                               INDENT;
                               fprintf (outfile, "SAC_size *= \n");
                               SET_SHAPE_AUD (to_nt, fprintf (outfile, "SAC_i + %d", i);
                                              ,
                                              fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)",
                                                       val_nt, i););
                           }
                       } else {
                           FOR_LOOP (fprintf (outfile, "SAC_j = 0");
                                     , fprintf (outfile, "SAC_j < SAC_ND_A_DIM( %s)",
                                                val_nt);
                                     , fprintf (outfile, "SAC_i++, SAC_j++");, INDENT;
                                     fprintf (outfile, "SAC_size *= \n");
                                     SET_SHAPE_AUD (to_nt, fprintf (outfile, "SAC_i");
                                                    ,
                                                    fprintf (outfile,
                                                             "SAC_ND_A_SHAPE( %s, SAC_j)",
                                                             val_nt);););
                       }

                       SET_SIZE (to_nt, fprintf (outfile, "SAC_size");););
        break;

    case C_akd:
        DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
        BLOCK_VARDECS (fprintf (outfile, "int ");
                       if ((val_dim < 0)
                           && (shp_size
                               < 0)) { fprintf (outfile, "SAC_i, "); } fprintf (outfile,
                                                                                "SAC_"
                                                                                "size = "
                                                                                "1;");
                       ,
                       if ((val_dim < 0) && (shp_size < 0)) {
                           FOR_LOOP_INC (fprintf (outfile, "SAC_i");
                                         , fprintf (outfile, "0");, shp_size_fun (shp);
                                         , INDENT; fprintf (outfile, "SAC_size *= \n");
                                         INDENT;
                                         fprintf (outfile,
                                                  "SAC_ND_A_DESC_SHAPE( %s, SAC_i) = ",
                                                  to_nt);
                                         shp_read_fun (shp, "SAC_i", -1);
                                         fprintf (outfile, ";\n"););
                           FOR_LOOP (fprintf (outfile, " ");
                                     , fprintf (outfile, "SAC_i < %d", to_dim);
                                     , fprintf (outfile, "SAC_i++");, INDENT;
                                     fprintf (outfile, "SAC_size *= \n"); INDENT;
                                     fprintf (outfile,
                                              "SAC_ND_A_DESC_SHAPE( %s, SAC_i) = "
                                              "SAC_ND_A_SHAPE( %s, SAC_i - ",
                                              to_nt, val_nt);
                                     shp_size_fun (shp); fprintf (outfile, ");\n"););
                           for (i = 0; i < to_dim; i++) {
                               INDENT;
                               fprintf (outfile,
                                        "SAC_ND_A_MIRROR_SHAPE( %s, %d)"
                                        " = SAC_ND_A_DESC_SHAPE( %s, %d);\n",
                                        to_nt, i, to_nt, i);
                           }
                       } else { /* ((val_dim >= 0) || (shp_size >= 0)) */
                                if (val_dim >= 0) {
                                    if (shp_size >= 0) {
                                        DBUG_ASSERT ((shp_size == to_dim - val_dim),
                                                     "inconsistant dimension found!");
                                    } else {
                                        shp_size = to_dim - val_dim;
                                    }
                                }
                                SET_SHAPES_AKD (to_nt, i, 0, shp_size, INDENT;
                                                fprintf (outfile, "SAC_size *= \n");
                                                , shp_read_fun (shp, NULL, i););
                                SET_SHAPES_AKD (to_nt, i, shp_size, to_dim, INDENT;
                                                fprintf (outfile, "SAC_size *= \n");
                                                , fprintf (outfile,
                                                           "SAC_ND_A_SHAPE( %s, %d)",
                                                           val_nt, i - shp_size););
                       }

                       SET_SIZE (to_nt, fprintf (outfile, "SAC_size");););
        break;

    case C_aks:
        /* here is no break missing */
    case C_scl:
        /* noop */
        break;

    default:
        DBUG_ASSERT ((0), "Unknown shape class found!");
        break;
    }

    /*
     * check constant parts of mirror
     */
    switch (to_sc) {
    case C_scl:
        DBUG_ASSERT ((0), "illegal dimension found!");
        break;

    case C_aks:
        if (shp_size >= 0) {
            for (i = 0; i < shp_size; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d) == ", to_nt,
                                          i);
                                 shp_read_fun (shp, NULL, i);
                                 ,
                                 fprintf (outfile,
                                          "Assignment with incompatible types found!"););
            }
            DBUG_ASSERT ((to_dim >= 0), "illegal dimension found!");
            for (; i < to_dim; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile,
                                          "SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( "
                                          "%s, %d)",
                                          to_nt, i, val_nt, i - shp_size);
                                 ,
                                 fprintf (outfile,
                                          "Assignment with incompatible types found!"););
            }
        } else {
            for (i = 0; i < to_dim; i++) {
                ASSURE_TYPE_ASS (fprintf (outfile, "((%d < ", i); shp_size_fun (shp);
                                 fprintf (outfile, ") && ");
                                 fprintf (outfile, "(SAC_ND_A_SHAPE( %s, %d) == ", to_nt,
                                          i);
                                 shp_read_fun (shp, NULL, i); fprintf (outfile, ")) ||");

                                 fprintf (outfile, "((%d >= ", i); shp_size_fun (shp);
                                 fprintf (outfile, ") && ");
                                 fprintf (outfile,
                                          "(SAC_ND_A_SHAPE( %s, %d) == SAC_ND_A_SHAPE( "
                                          "%s, %d)))",
                                          to_nt, i, val_nt, i - shp_size);
                                 ,
                                 fprintf (outfile,
                                          "Assignment with incompatible types found!"););
            }
        }
        /* here is no break missing */

    case C_akd:
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == ", to_nt);
                         GetAttr (shp, shp_size, shp_size_fun);
                         fprintf (outfile, " + SAC_ND_A_DIM( %s)", val_nt);
                         ,
                         fprintf (outfile, "Assignment with incompatible types found!"););
        break;

    case C_aud:
        /* noop */
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
 *   void ICMCompileND_WL_GENARRAY__SHAPE_id( char *to_nt, int to_sdim,
 *                                            char *shp_nt,
 *                                            char *val_nt, int val_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_WL_GENARRAY__SHAPE_id( to_nt, to_sdim, shp_nt, val_nt, val_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_WL_GENARRAY__SHAPE_id (char *to_nt, int to_sdim, char *shp_nt, char *val_nt,
                                    int val_sdim)
{
    DBUG_ENTER ("ICMCompileND_WL_GENARRAY__SHAPE_id");

#define ND_WL_GENARRAY__SHAPE_id
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_id

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_WL_GENARRAY__SHAPE( %s, %d, ..., %s, %d)\"))\n",
             to_nt, to_sdim, val_nt, val_sdim);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 1", shp_nt);
                     , fprintf (outfile, "Shape of genarray with-loop has (dim != 1)!"););

    WLGenarray_Shape (to_nt, to_sdim, shp_nt, -1, SizeId, ReadId, val_nt, val_sdim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileND_WL_GENARRAY__SHAPE_arr( char *to_nt, int to_sdim,
 *                                             int shp_size, char **shpa_any,
 *                                             char *val_nt, int val_sdim)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   ND_WL_GENARRAY__SHAPE_arr( to_nt, to_sdim, shp_size, shpa_any,
 *                              val_nt, val_sdim)
 *
 ******************************************************************************/

void
ICMCompileND_WL_GENARRAY__SHAPE_arr (char *to_nt, int to_sdim, int shp_size,
                                     char **shpa_any, char *val_nt, int val_sdim)
{
    int i;

    DBUG_ENTER ("ICMCompileND_WL_GENARRAY__SHAPE_arr");

#define ND_WL_GENARRAY__SHAPE_arr
#include "icm_comment.c"
#include "icm_trace.c"
#undef ND_WL_GENARRAY__SHAPE_arr

    /*
     * CAUTION:
     * 'shpa_any[i]' is either a tagged identifier (representing a scalar)
     * or a constant scalar!!
     */

    INDENT;
    fprintf (outfile,
             "SAC_TR_PRF_PRINT("
             " (\"ND_WL_GENARRAY__SHAPE( %s, %d, ..., %s, %d)\"))\n",
             to_nt, to_sdim, val_nt, val_sdim);

    for (i = 0; i < shp_size; i++) {
        if (shpa_any[i][0] == '(') {
            ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", shpa_any[i]);
                             , fprintf (outfile,
                                        "Shape of genarray with-loop has (dim != 1)!"););
        }
    }

    WLGenarray_Shape (to_nt, to_sdim, shpa_any, shp_size, NULL, ReadConstArray, val_nt,
                      val_sdim);

    DBUG_VOID_RETURN;
}

#endif /* TAGGED_ARRAYS */

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_BEGIN__OFFSET( char *to_nt, int to_sdim,
 *                                    char *idx_vec_nt, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_BEGIN__OFFSET( to_nt, to_sdim, idx_vec_nt, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_BEGIN__OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_BEGIN__OFFSET");

#define WL_BEGIN__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_BEGIN__OFFSET

    INDENT;
    fprintf (outfile, "{ int SAC_WL_OFFSET( %s);\n", to_nt);
    indent++;

    for (i = 0; i < dims; i++) {
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_START( %d);\n", i);
        INDENT;
        fprintf (outfile, "int SAC_WL_MT_SCHEDULE_STOP( %d);\n", i);
    }

    INDENT;
    fprintf (outfile, "int SAC_i;\n"); /* for DefineShapeFactor() !!! */
    for (i = 0; i < dims; i++) {
        DefineShapeFactor (to_nt, to_sdim, i);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_BEGIN( char *to_nt, int to_sdim,
 *                            char *idx_vec_nt, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_BEGIN( to_nt, to_sdim, idx_vec_nt, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_BEGIN (char *to_nt, int to_sdim, char *idx_vec_nt, int dims)
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
 *   void ICMCompileWL_END__OFFSET( char *to_nt, int to_sdim,
 *                                  char *idx_vec_nt, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_END__OFFSET( to_nt, to_sdim, idx_vec_nt, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_END__OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt, int dims)
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
 *   void ICMCompileWL_END( char *to_nt, int to_sdim,
 *                          char *idx_vec_nt, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_END( to_nt, to_sdim, idx_vec_nt, dims)
 *
 ******************************************************************************/

void
ICMCompileWL_END (char *to_nt, int to_sdim, char *idx_vec_nt, int dims)
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
 *   void ICMCompileWL_ASSIGN( char *val_nt, int val_sdim,
 *                             char *to_nt, int to_sdim
 *                             char *idx_vec_nt,
 *                             int dims, char **idxa_scl)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN( val_nt, val_sdim, to_nt, to_sdim,
 *              idx_vec_nt, dims, [ idxa_scl ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN (char *val_nt, int val_sdim, char *to_nt, int to_sdim,
                     char *idx_vec_nt, int dims, char **idxa_scl)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);
    int val_dim = DIM_NO_OFFSET (val_sdim);

    DBUG_ENTER ("ICMCompileWL_ASSIGN");

#define WL_ASSIGN
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN

    PrintTraceICM (to_nt, idx_vec_nt, dims, idxa_scl, "assign", TRUE);

    ASSURE_TYPE_ASS (fprintf (outfile, "SAC_ND_A_DIM( %s) == (SAC_ND_A_DIM( %s) - %d)",
                              val_nt, to_nt, dims);
                     , fprintf (outfile, "WL expression with illegal dimension found!"););

    ASSURE_TYPE_ASS (fprintf (outfile,
                              "SAC_ND_A_SIZE( %s) =="
                              " SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d)",
                              val_nt, to_nt, dims - 1);
                     , fprintf (outfile, "WL expression with illegal size found!"););

    if ((val_dim == 0) || (to_dim == dims)) {
        INDENT;
#ifdef TAGGED_ARRAYS
        fprintf (outfile,
                 "SAC_ND_WRITE( %s, SAC_WL_OFFSET( %s)) = "
                 "SAC_ND_READ( %s, 0);\n",
                 to_nt, to_nt, val_nt);
#else
        fprintf (outfile, "SAC_ND_WRITE_ARRAY( %s, SAC_WL_OFFSET( %s)) = %s;\n", to_nt,
                 to_nt, val_nt);
#endif
        INDENT;
        fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt);
    } else {
        FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                             , fprintf (outfile, "SAC_ND_A_SIZE( %s)", val_nt);, INDENT;
                             fprintf (outfile,
                                      "SAC_ND_WRITE( %s, SAC_WL_OFFSET( %s)) = "
                                      "SAC_ND_READ( %s, SAC_i);\n",
                                      to_nt, to_nt, val_nt);
                             INDENT;
                             fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN__INIT( char *to_nt, int to_sdim,
 *                                   char *idx_vec_nt,
 *                                   int dims, char **idxa_scl)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN__INIT( to_nt, to_sdim, idx_vec_nt, dims, [ idxa_scl ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN__INIT (char *to_nt, int to_sdim, char *idx_vec_nt, int dims,
                           char **idxa_scl)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileWL_ASSIGN__INIT");

#define WL_ASSIGN__INIT
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN__INIT

    PrintTraceICM (to_nt, idx_vec_nt, dims, idxa_scl, "init", TRUE);

    /*
     * the size of the area to be initialized can be found in
     *   SAC_WL_SHAPE_FACTOR( NT_NAME( to_nt), dims-1)
     */

    DBUG_ASSERT (((to_dim < 0) || (to_dim >= dims)), "inconsistant WL found!");
    if (to_dim == dims) {
        ASSURE_TYPE_ASS (fprintf (outfile,
                                  "(SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d) == 1)", to_nt,
                                  dims - 1);
                         , fprintf (outfile, "Inconsistent with-loop found!"););
        INDENT;
        fprintf (outfile, "SAC_ND_WRITE( %s, SAC_WL_OFFSET( %s)) = 0;\n", to_nt, to_nt);
        INDENT;
        fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt);
    } else {
        FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                             , fprintf (outfile, "SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d)",
                                        to_nt, dims - 1);
                             , INDENT;
                             fprintf (outfile,
                                      "SAC_ND_WRITE( %s, SAC_WL_OFFSET( %s)) = 0;\n",
                                      to_nt, to_nt);
                             INDENT;
                             fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ASSIGN__COPY( char *from_nt,
 *                                   char *to_nt, int to_sdim,
 *                                   char *idx_vec_nt,
 *                                   int dims, char **idxa_scl)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_ASSIGN__COPY( from_nt, to_nt, to_sdim, idx_vec_nt, dims, [ idxa_scl ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_ASSIGN__COPY (char *from_nt, char *to_nt, int to_sdim, char *idx_vec_nt,
                           int dims, char **idxa_scl)
{
    int to_dim = DIM_NO_OFFSET (to_sdim);

    DBUG_ENTER ("ICMCompileWL_ASSIGN__COPY");

#define WL_ASSIGN__COPY
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ASSIGN__COPY

    PrintTraceICM (to_nt, idx_vec_nt, dims, idxa_scl, "copy", TRUE);

    /*
     * the size of the area to be copied can be found in
     *   SAC_WL_SHAPE_FACTOR( NT_NAME( to_nt), dims-1)
     */

    DBUG_ASSERT (((to_dim < 0) || (to_dim >= dims)), "inconsistant WL found!");
    if (to_dim == dims) {
        ASSURE_TYPE_ASS (fprintf (outfile, "SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d) == 1",
                                  to_nt, dims - 1);
                         , fprintf (outfile, "Inconsistent with-loop found!"););
        INDENT;
        fprintf (outfile,
                 "SAC_ND_WRITE( %s, SAC_WL_OFFSET( %s)) = "
                 "SAC_ND_READ( %s, SAC_WL_OFFSET( %s));\n",
                 to_nt, to_nt, from_nt, to_nt);
        INDENT;
        fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt);
    } else {
        FOR_LOOP_INC_VARDEC (fprintf (outfile, "SAC_i");, fprintf (outfile, "0");
                             , fprintf (outfile, "SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d)",
                                        to_nt, dims - 1);
                             , INDENT; fprintf (outfile,
                                                "SAC_ND_WRITE( %s, SAC_WL_OFFSET( %s)) = "
                                                "SAC_ND_READ( %s, SAC_WL_OFFSET( %s));\n",
                                                to_nt, to_nt, from_nt, to_nt);
                             INDENT;
                             fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt););
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD( char *to_nt, int to_sdim,
 *                           char *idx_vec_nt, int dims, char **idxa_scl)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD( to_nt, to_sdim, idx_vec_nt, dims, [ idxa_scl ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD (char *to_nt, int to_sdim, char *idx_vec_nt, int dims, char **idxa_scl)
{
    DBUG_ENTER ("ICMCompileWL_FOLD");

#define WL_FOLD
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD

    PrintTraceICM (to_nt, idx_vec_nt, dims, idxa_scl, "fold", FALSE);

    INDENT;
    fprintf (outfile, "/* fold operation */\n");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_FOLD__OFFSET( char *to_nt, int to_sdim,
 *                                   char *idx_vec_nt,
 *                                   int dims, char **idxa_scl)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *   WL_FOLD__OFFSET( to_nt, to_sdim, idx_vec_nt, dims, [ idxa_scl ]* )
 *
 ******************************************************************************/

void
ICMCompileWL_FOLD__OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt, int dims,
                           char **idxa_scl)
{
    DBUG_ENTER ("ICMCompileWL_FOLD__OFFSET");

#define WL_FOLD__OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_FOLD__OFFSET

    PrintTraceICM (to_nt, idx_vec_nt, dims, idxa_scl, "fold", TRUE);

    INDENT;
    fprintf (outfile, "/* fold operation */\n");
    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)++;\n", to_nt);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_INIT_OFFSET( char *to_nt, int to_sdim,
 *                                  char *idx_vec_nt, int dims)
 *
 * description:
 *   Implements the compilation of the following ICM:
 *
 *     WL_INIT_OFFSET( to_nt, to_sdim, idx_vec_nt, dims)
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
ICMCompileWL_INIT_OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt, int dims)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_INIT_OFFSET");

#define WL_INIT_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_INIT_OFFSET

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s)\n", to_nt);
    indent++;

    INDENT;
    fprintf (outfile,
             "= SAC_WL_MT_SCHEDULE_START( 0)"
             " * SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d)",
             to_nt, 0);

    for (i = 1; i < dims; i++) {
        fprintf (outfile, "\n");
        INDENT;
        fprintf (outfile, "+ SAC_WL_MT_SCHEDULE_START( %d)", i);
        fprintf (outfile, " * SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d)", to_nt, i);
    }

    fprintf (outfile, ";\n");
    indent--;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_ADJUST_OFFSET( int dim,
 *                                    char *to_nt, int to_sdim,
 *                                    char *idx_vec_nt,
 *                                    int dims, char **idxa_scl)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_ADJUST_OFFSET( dim, to_nt, to_sdim, idx_vec_nt, dims, [ idxa_scl ]* )
 *
 * remark:
 *   This ICM is needed (and usefull) in combination with NOOP N_WL..-nodes
 *   only! It uses the variable 'SAC_diff...' defined by the ICMs
 *   WL_..._NOOP_BEGIN.
 *
 ******************************************************************************/

void
ICMCompileWL_ADJUST_OFFSET (int dim, char *to_nt, int to_sdim, char *idx_vec_nt, int dims,
                            char **idxa_scl)
{
    DBUG_ENTER ("ICMCompileWL_ADJUST_OFFSET");

#define WL_ADJUST_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_ADJUST_OFFSET

    INDENT;
    fprintf (outfile,
             "SAC_WL_OFFSET( %s) += SAC_WL_VAR( diff, %s)"
             " * SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d);\n",
             to_nt, idxa_scl[dim], to_nt, dim);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void ICMCompileWL_SET_OFFSET( int dim, int first_block_dim,
 *                                 char *to_nt, int to_sdim,
 *                                 char *idx_vec_nt, int dims, char **idxa_scl)
 *
 * description:
 *   implements the compilation of the following ICM:
 *
 *   WL_SET_OFFSET( dim, first_block_dim, to_nt, to_sdim, idx_vec_nt,
 *                  dims, [ idxa_scl ]* )
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
ICMCompileWL_SET_OFFSET (int dim, int first_block_dim, char *to_nt, int to_sdim,
                         char *idx_vec_nt, int dims, char **idxa_scl)
{
    int i;

    DBUG_ENTER ("ICMCompileWL_SET_OFFSET");

#define WL_SET_OFFSET
#include "icm_comment.c"
#include "icm_trace.c"
#undef WL_SET_OFFSET

    INDENT;
    fprintf (outfile, "SAC_WL_OFFSET( %s) \n", to_nt);
    indent++;

    INDENT;
    fprintf (outfile, "= ");
    for (i = dims - 1; i > 0; i--) {
        fprintf (outfile, "( SAC_ND_A_SHAPE( %s, %d) * ", to_nt, i);
    }
    fprintf (outfile, "%s \n", idxa_scl[0]);

    INDENT;
    for (i = 1; i < dims; i++) {
        if (i <= dim) {
            fprintf (outfile, "+ %s )", idxa_scl[i]);
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
                fprintf (outfile, " + SAC_WL_VAR( first, %s) )", idxa_scl[i]);
            }
        }
    }
    fprintf (outfile, " * SAC_WL_SHAPE_FACTOR( NT_NAME( %s), %d);\n", to_nt, dims - 1);
    indent--;

    DBUG_VOID_RETURN;
}
