/*
 *
 * $Log$
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
            INDENT;
            fprintf (outfile,
                     "( SAC_ASSURE_TYPE( (SAC_ND_A_DIM( %s) == 0),"
                     " (\"Scalar expected but array with (dim > 0)"
                     " found!\")) , \n",
                     (char *)scl);
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
 *   void VectToOffset2( char *off_any,
 *                       void *v_any, int v_size,
 *                       void (*v_size_fun)( void *),
 *                       void (*v_read_fun)( void *, char *, int),
 *                       void *a_any, int a_dim,
 *                       void (*a_dim_fun)( void *),
 *                       void (*a_shape_fun)( void *, char *, int))
 *
 * Description:
 *
 *
 ******************************************************************************/

void
VectToOffset2 (char *off_any, void *v_any, int v_size, void (*v_size_fun) (void *),
               void (*v_read_fun) (void *, char *, int), void *a_any, int a_dim,
               void (*a_dim_fun) (void *), void (*a_shape_fun) (void *, char *, int))
{
    int i;

    DBUG_ENTER ("VectToOffset2");

    if (v_size != 0) {
        if ((v_size < 0) || (a_dim < 0)) {
            DBUG_ASSERT (((v_size_fun != NULL) && (a_dim_fun != NULL)),
                         "AUD array without access functions found!");
            INDENT;
            fprintf (outfile, "{\n");
            indent++;
            INDENT;
            fprintf (outfile, "int SAC_start;\n");
            INDENT;
            fprintf (outfile, "int SAC_rest = 1;\n");
            INDENT;
            fprintf (outfile, "int SAC_i;\n");

            /*
             * compute SAC_rest
             */
            INDENT;
            fprintf (outfile, "for (SAC_i = ");
            v_size_fun (v_any);
            fprintf (outfile, "; SAC_i < ");
            a_dim_fun (a_any);
            fprintf (outfile, "; SAC_i++) {\n");
            indent++;
            INDENT;
            fprintf (outfile, "SAC_rest *= ");
            a_shape_fun (a_any, "SAC_i", -1);
            fprintf (outfile, ";\n");
            indent--;
            INDENT;
            fprintf (outfile, "}\n");

            /*
             * compute SAC_start
             */
            if (v_size < 0) {
                INDENT;
                fprintf (outfile, "SAC_start = 0;\n");
                INDENT;
                fprintf (outfile, "for (SAC_i = 0; SAC_i < ");
                v_size_fun (v_any);
                fprintf (outfile, "; SAC_i++) {\n");
                indent++;
                INDENT;
                fprintf (outfile, "int SAC_tmp = 1;\n");
                INDENT;
                fprintf (outfile, "int SAC_j;\n");
                INDENT;
                fprintf (outfile, "for (SAC_j = ");
                v_size_fun (v_any);
                fprintf (outfile, " - 1; SAC_j > SAC_i;  SAC_j--) {\n");
                indent++;
                INDENT;
                fprintf (outfile, "SAC_tmp *= ");
                a_shape_fun (a_any, "SAC_i", -1);
                fprintf (outfile, ";\n");
                indent--;
                INDENT;
                fprintf (outfile, "}\n");
                INDENT;
                fprintf (outfile, "SAC_start += SAC_tmp * ");
                v_read_fun (v_any, "SAC_i", -1);
                fprintf (outfile, ";\n");
                indent--;
                INDENT;
                fprintf (outfile, "}\n");
            } else {
                INDENT;
                fprintf (outfile, "SAC_start = ");
                for (i = v_size - 1; i > 0; i--) {
                    fprintf (outfile, "( ");
                    a_shape_fun (a_any, NULL, i);
                    fprintf (outfile, " * ");
                }
                v_read_fun (v_any, NULL, i);
                for (i = 1; i < v_size; i++) {
                    fprintf (outfile, " + ");
                    v_read_fun (v_any, NULL, i);
                    fprintf (outfile, " )");
                }
                fprintf (outfile, ";\n");
            }

            INDENT;
            WriteScalar (off_any);
            fprintf (outfile, " = SAC_start * SAC_rest;\n");
            indent--;
            INDENT;
            fprintf (outfile, "}\n");
        } else {
            DBUG_ASSERT (((v_size >= 0) && (a_dim >= 0)), "illegal dimension found!");
            INDENT;
            WriteScalar (off_any);
            fprintf (outfile, " = ");
            for (i = v_size - 1; i > 0; i--) {
                fprintf (outfile, "( ");
                a_shape_fun (a_any, NULL, i);
                fprintf (outfile, " * ");
            }
            v_read_fun (v_any, NULL, i);
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
        }
    } else {
        INDENT;
        WriteScalar (off_any);
        fprintf (outfile, " = 0;\n");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void VectToOffset( char *off_any,
 *                      void *v_any, int v_size,
 *                      void (*v_size_fun)( void *),
 *                      void (*v_read_fun)( void *, char *, int),
 *                      void *a_nt, int a_dim)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
VectToOffset (char *off_any, void *v_any, int v_size, void (*v_size_fun) (void *),
              void (*v_read_fun) (void *, char *, int), void *a_nt, int a_dim)
{
    DBUG_ENTER ("VectToOffset");

    VectToOffset2 (off_any, v_any, v_size, v_size_fun, v_read_fun, a_nt, a_dim, DimId,
                   ShapeId);

    DBUG_VOID_RETURN;
}

#endif
