/*
 *
 * $Log$
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

int print_comment = 0; /* bool */

#ifdef TAGGED_ARRAYS

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
        data_class_t dc = ICUGetDataClass (scl);

        DBUG_ASSERT (((dc == C_scl) || (dc == C_aud)), "tagged id is no scalar!");
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
        data_class_t dc = ICUGetDataClass (scl);

        DBUG_ASSERT (((dc == C_scl) || (dc == C_aud)), "tagged id is no scalar!");
        if (dc == C_aud) {
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
 *   void VectToOffset2( char *offset,
 *                       void *v_any, int dimv,
 *                       void (*v_acc_dim)( void *),
 *                       void (*v_acc_shp)( void *, char *, int),
 *                       void *a_any, int dima,
 *                       void (*a_acc_dim)( void *),
 *                       void (*a_acc_shp)( void *, char *, int))
 *
 * Description:
 *
 *
 ******************************************************************************/

void
VectToOffset2 (char *offset, void *v_any, int dimv, void (*v_acc_dim) (void *),
               void (*v_acc_shp) (void *, char *, int), void *a_any, int dima,
               void (*a_acc_dim) (void *), void (*a_acc_shp) (void *, char *, int))
{
    int i;

    DBUG_ENTER ("VectToOffset2");

    if (dimv != 0) {
        if ((dimv < 0) || (dima < 0)) {
            DBUG_ASSERT (((v_acc_dim != NULL) && (a_acc_dim != NULL)),
                         "AUD array without access functions found!");
            INDENT;
            fprintf (outfile, "{\n");
            indent++;
            INDENT;
            fprintf (outfile, "int SAC__start;\n");
            INDENT;
            fprintf (outfile, "int SAC__rest = 1;\n");
            INDENT;
            fprintf (outfile, "int SAC__i;\n");

            /*
             * compute SAC__rest
             */
            INDENT;
            fprintf (outfile, "for (SAC__i = ");
            v_acc_dim (v_any);
            fprintf (outfile, "; SAC__i < ");
            a_acc_dim (a_any);
            fprintf (outfile, "; SAC__i++) {\n");
            indent++;
            INDENT;
            fprintf (outfile, "SAC__rest *= ");
            a_acc_shp (a_any, "SAC__i", -1);
            fprintf (outfile, ";\n");
            indent--;
            INDENT;
            fprintf (outfile, "}\n");

            /*
             * compute SAC__start
             */
            if (dimv < 0) {
                INDENT;
                fprintf (outfile, "SAC__start = 0;\n");
                INDENT;
                fprintf (outfile, "for (SAC__i = 0; SAC__i < ");
                v_acc_dim (v_any);
                fprintf (outfile, "; SAC__i++) {\n");
                indent++;
                INDENT;
                fprintf (outfile, "int SAC__tmp = 1;\n");
                INDENT;
                fprintf (outfile, "int SAC__j;\n");
                INDENT;
                fprintf (outfile, "for (SAC__j = ");
                v_acc_dim (v_any);
                fprintf (outfile, " - 1; SAC__j > SAC__i;  SAC__j--) {\n");
                indent++;
                INDENT;
                fprintf (outfile, "SAC__tmp *= ");
                a_acc_shp (a_any, "SAC__i", -1);
                fprintf (outfile, ";\n");
                indent--;
                INDENT;
                fprintf (outfile, "}\n");
                INDENT;
                fprintf (outfile, "SAC__start += SAC__tmp * ");
                v_acc_shp (v_any, "SAC__i", -1);
                fprintf (outfile, ";\n");
                indent--;
                INDENT;
                fprintf (outfile, "}\n");
            } else {
                INDENT;
                fprintf (outfile, "SAC__start =");
                for (i = dimv - 1; i > 0; i--) {
                    fprintf (outfile, " ( ");
                    a_acc_shp (a_any, NULL, i);
                    fprintf (outfile, " *");
                }
                v_acc_shp (v_any, NULL, i);
                for (i = 1; i < dimv; i++) {
                    fprintf (outfile, " + ");
                    v_acc_shp (v_any, NULL, i);
                    fprintf (outfile, " )");
                }
                fprintf (outfile, ";\n");
            }

            INDENT;
            fprintf (outfile, "%s = SAC__start * SAC__rest;\n", offset);
            indent--;
            INDENT;
            fprintf (outfile, "}\n");
        } else {
            DBUG_ASSERT (((dimv >= 0) && (dima >= 0)), "illegal dimension found!");
            INDENT;
            fprintf (outfile, "%s = ", offset);
            for (i = dimv - 1; i > 0; i--) {
                fprintf (outfile, "( ");
                a_acc_shp (a_any, NULL, i);
                fprintf (outfile, "* ");
            }
            v_acc_shp (v_any, NULL, i);
            for (i = 1; i < dimv; i++) {
                fprintf (outfile, "+");
                v_acc_shp (v_any, NULL, i);
                fprintf (outfile, ") ");
            }
            for (i = dimv; i < dima; i++) {
                fprintf (outfile, "*");
                a_acc_shp (a_any, NULL, i);
                fprintf (outfile, " ");
            }
            fprintf (outfile, ";\n");
        }
    } else {
        INDENT;
        fprintf (outfile, "%s = 0;\n", offset);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void VectToOffset( char *offset,
 *                      void *v, int dimv,
 *                      void (*v_acc_dim)( void *),
 *                      void (*v_acc_shp)( void *, char *, int),
 *                      void *a, int dima)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
VectToOffset (char *offset, void *v, int dimv, void (*v_acc_dim) (void *),
              void (*v_acc_shp) (void *, char *, int), void *a, int dima)
{
    DBUG_ENTER ("VectToOffset");

    VectToOffset2 (offset, v, dimv, v_acc_dim, v_acc_shp, a, dima, DimId, ShapeId);

    DBUG_VOID_RETURN;
}

#endif
