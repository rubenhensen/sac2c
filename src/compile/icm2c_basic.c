/*
 *
 * $Log$
 * Revision 1.14  2002/10/29 19:09:40  dkr
 * Vect2Offset2() rewritten
 *
 * Revision 1.13  2002/10/24 16:00:14  dkr
 * minor changes done
 *
 * Revision 1.12  2002/10/08 01:49:52  dkr
 * bug in VectToOffset2() fixed
 *
 * Revision 1.10  2002/08/05 18:08:15  dkr
 * comment corrected
 *
 * Revision 1.9  2002/07/31 15:36:29  dkr
 * types for NT tags reorganized
 *
 * Revision 1.8  2002/07/24 14:34:29  dkr
 * WriteScalar() added
 *
 * Revision 1.7  2002/07/24 08:20:29  dkr
 * VectToOffset2: DBUG_ASSERT corrected
 *
 * Revision 1.6  2002/07/12 17:36:40  dkr
 * some spaces in output added
 *
 * Revision 1.5  2002/07/12 14:34:51  dkr
 * print_comment initialized with 1 now
 *
 * Revision 1.4  2002/07/11 17:29:19  dkr
 * SizeId() added
 *
 * Revision 1.3  2002/07/11 13:37:19  dkr
 * functions renamed
 *
 * Revision 1.2  2002/07/10 19:27:26  dkr
 * TAGGED_ARRAYS: access macros are functions now
 *
 * Revision 1.1  2002/07/02 13:03:19  dkr
 * Initial revision
 *
 */

#include "dbug.h"
#include "globals.h"
#include "print.h"
#include "NameTuples.h"
#include "icm2c_utils.h"
#include "icm2c_basic.h"

int print_comment = 1; /* bool */

#ifdef TAGGED_ARRAYS

/******************************************************************************
 *
 * Function:
 *   void WriteScalar( char *scl)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
WriteScalar (char *scl)
{
    DBUG_ENTER ("WriteScalar");

    if (scl[0] == '(') {
        /* 'scl' is a tagged id */
        shape_class_t dc = ICUGetShapeClass (scl);

        DBUG_ASSERT (((dc == C_scl) || (dc == C_aud)), "tagged id is no scalar!");
        fprintf (outfile, "SAC_ND_WRITE( %s, 0)", scl);
    } else {
        /* 'scl' is a scalar constant */
        fprintf (outfile, "%s", scl);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ReadId( void *nt, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadId (void *nt, char *idx_str, int idx)
{
    DBUG_ENTER ("ReadId");

    DBUG_ASSERT ((((char *)nt)[0] == '('), "no tag found!");

    if (idx_str != NULL) {
        fprintf (outfile, "SAC_ND_READ( %s, %s)", (char *)nt, idx_str);
    } else {
        DBUG_ASSERT ((idx >= 0), "illegal index found!");
        fprintf (outfile, "SAC_ND_READ( %s, %d)", (char *)nt, idx);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ReadScalar( void *scl, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadScalar (void *scl, char *idx_str, int idx)
{
    DBUG_ENTER ("ReadScalar");

    if (((char *)scl)[0] == '(') {
        /* 'scl' is a tagged id */
        shape_class_t sc = ICUGetShapeClass (scl);

        DBUG_ASSERT (((sc == C_scl) || (sc == C_aud)), "tagged id is no scalar!");
        ReadId (scl, idx_str, idx);
    } else {
        if (idx_str == NULL) {
            DBUG_ASSERT ((idx == 0), "illegal index found!");
        }

        /* 'scl' is a scalar constant */
        fprintf (outfile, "%s", (char *)scl);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ReadScalar_Check( void *scl, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadScalar_Check (void *scl, char *idx_str, int idx)
{
    DBUG_ENTER ("ReadScalar_Check");

    if (((char *)scl)[0] == '(') {
        /* 'scl' is a tagged id */
        shape_class_t sc = ICUGetShapeClass (scl);

        DBUG_ASSERT (((sc == C_scl) || (sc == C_aud)), "tagged id is no scalar!");
        if (sc == C_aud) {
            fprintf (outfile, "\n");
            indent++;
            fprintf (outfile, "( ");
            ASSURE_TYPE_EXPR (fprintf (outfile, "SAC_ND_A_DIM( %s) == 0", (char *)scl);
                              , fprintf (outfile, "Scalar expected but array with (dim > "
                                                  "0) found!"););
            fprintf (outfile, " , \n");
            INDENT;
            fprintf (outfile, "  ");
            ReadId (scl, idx_str, idx);
            fprintf (outfile, " )");
            indent--;
        } else {
            ReadId (scl, idx_str, idx);
        }
    } else {
        /* 'scl' is a scalar constant */
        fprintf (outfile, "%s", (char *)scl);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ReadConstArray( void *v, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ReadConstArray (void *v, char *idx_str, int idx)
{
    DBUG_ENTER ("ReadConstArray");

    if (idx_str != NULL) {
        DBUG_ASSERT ((0), "illegal argument for ReadConstArray() found!");
    } else {
        ReadScalar (((char **)v)[idx], NULL, 0);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void DimId( void *nt)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
DimId (void *nt)
{
    DBUG_ENTER ("DimId");

    fprintf (outfile, "SAC_ND_A_DIM( %s)", (char *)nt);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void ShapeId( void *nt, char *idx_str, int idx)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
ShapeId (void *nt, char *idx_str, int idx)
{
    DBUG_ENTER ("ShapeId");

    if (idx_str != NULL) {
        fprintf (outfile, "SAC_ND_A_SHAPE( %s, %s)", (char *)nt, idx_str);
    } else {
        fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", (char *)nt, idx);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void SizeId( void *nt)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
SizeId (void *nt)
{
    DBUG_ENTER ("SizeId");

    fprintf (outfile, "SAC_ND_A_SIZE( %s)", (char *)nt);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   char *PrintAttr( void *v, int v_attr, void (*v_attr_fun)( void *))
 *
 * Description:
 *
 *
 ******************************************************************************/

void
GetAttr (void *v, int v_attr, void (*v_attr_fun) (void *))
{
    DBUG_ENTER ("GetAttr");

    if (v_attr < 0) {
        DBUG_ASSERT ((v_attr_fun != NULL), "access function not found!");
        v_attr_fun (v);
    } else {
        fprintf (outfile, "%d", v_attr);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void Vect2Offset2( char *off_any,
 *                      void *v_any, int v_size,
 *                      void (*v_size_fun)( void *),
 *                      void (*v_read_fun)( void *, char *, int),
 *                      void *a_any, int a_dim,
 *                      void (*a_dim_fun)( void *),
 *                      void (*a_shape_fun)( void *, char *, int))
 *
 * Description:
 *
 *
 ******************************************************************************/

void
Vect2Offset2 (char *off_any, void *v_any, int v_size, void (*v_size_fun) (void *),
              void (*v_read_fun) (void *, char *, int), void *a_any, int a_dim,
              void (*a_dim_fun) (void *), void (*a_shape_fun) (void *, char *, int))
{
    int i;

    DBUG_ENTER ("Vect2Offset2");

    DBUG_ASSERT ((v_read_fun != NULL), "access function not found!");
    DBUG_ASSERT ((a_shape_fun != NULL), "access function not found!");

    DBUG_ASSERT ((((v_size >= 0) || (v_size_fun != NULL))
                  && ((a_dim >= 0) || (a_dim_fun != NULL))),
                 "access function not found!");

    if (v_size == 0) {
        INDENT;
        WriteScalar (off_any);
        fprintf (outfile, " = 0;\n");
    } else if ((v_size >= 0) && (a_dim >= 0)) {
        INDENT;
        WriteScalar (off_any);
        fprintf (outfile, " = ");
        for (i = v_size - 1; i > 0; i--) {
            fprintf (outfile, "( ");
            a_shape_fun (a_any, NULL, i);
            fprintf (outfile, " * ");
        }
        v_read_fun (v_any, NULL, 0);
        for (i = 1; i < v_size; i++) {
            fprintf (outfile, " + ");
            v_read_fun (v_any, NULL, i);
            fprintf (outfile, " )");
        }
        for (i = v_size; i < a_dim; i++) {
            fprintf (outfile, " * ");
            a_shape_fun (a_any, NULL, i);
        }
        fprintf (outfile, ";\n");
    } else if (a_dim < 0) {
        BLOCK_VARDECS (
          fprintf (outfile, "int SAC_i;");,
                                          /*
                                           * init offset
                                           */
                                          INDENT;
          WriteScalar (off_any); fprintf (outfile, " = 0;\n");

          /*
           * compute offset for indices (0 <= .. < v_size)
           */
          if (v_size < 0) {
              FOR_LOOP (fprintf (outfile, "SAC_i = 0");, fprintf (outfile, "SAC_i < ");
                        v_size_fun (v_any);, fprintf (outfile, "SAC_i++");, INDENT;
                        WriteScalar (off_any); fprintf (outfile, " = ");
                        a_shape_fun (a_any, "SAC_i", -1); fprintf (outfile, " * ");
                        WriteScalar (off_any); fprintf (outfile, " + ");
                        v_read_fun (v_any, "SAC_i", -1); fprintf (outfile, ";\n"););
          } else {
              INDENT;
              WriteScalar (off_any);
              fprintf (outfile, " = ");
              for (i = v_size - 1; i > 0; i--) {
                  fprintf (outfile, "( ");
                  a_shape_fun (a_any, NULL, i);
                  fprintf (outfile, " * ");
              }
              v_read_fun (v_any, NULL, 0);
              for (i = 1; i < v_size; i++) {
                  fprintf (outfile, " + ");
                  v_read_fun (v_any, NULL, i);
                  fprintf (outfile, " )");
              }
              fprintf (outfile, ";\n");
          }

          /*
           * compute offset for indices (v_size <= .. < a_dim)
           */
          FOR_LOOP (fprintf (outfile, "SAC_i = "); GetAttr (v_any, v_size, v_size_fun);
                    , fprintf (outfile, "SAC_i < "); GetAttr (a_any, a_dim, a_dim_fun);
                    , fprintf (outfile, "SAC_i++");, INDENT; WriteScalar (off_any);
                    fprintf (outfile, " *= "); a_shape_fun (a_any, "SAC_i", -1);
                    fprintf (outfile, ";\n");););
    } else { /* ((a_dim >= 0) && (v_size < 0)) */
        BLOCK_VARDECS (fprintf (outfile, "int SAC_i;");, INDENT; WriteScalar (off_any);
                       fprintf (outfile, " = 0;\n"); for (i = 0; i < a_dim; i++) {
                           INDENT;
                           WriteScalar (off_any);
                           fprintf (outfile, " *= ");
                           a_shape_fun (a_any, NULL, i);
                           fprintf (outfile, ";\n");
                           COND1 (fprintf (outfile, "%d < ", i); v_size_fun (v_any);
                                  , WriteScalar (off_any); fprintf (outfile, " += ");
                                  v_read_fun (v_any, NULL, i); fprintf (outfile, ";\n"););
                       });
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void Vect2Offset( char *off_any,
 *                     void *v_any, int v_size,
 *                     void (*v_size_fun)( void *),
 *                     void (*v_read_fun)( void *, char *, int),
 *                     void *a_nt, int a_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
Vect2Offset (char *off_any, void *v_any, int v_size, void (*v_size_fun) (void *),
             void (*v_read_fun) (void *, char *, int), void *a_nt, int a_dim)
{
    DBUG_ENTER ("Vect2Offset");

    Vect2Offset2 (off_any, v_any, v_size, v_size_fun, v_read_fun, a_nt, a_dim, DimId,
                  ShapeId);

    DBUG_VOID_RETURN;
}

#endif
