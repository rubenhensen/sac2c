/*
 *
 * $Log$
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

int print_comment = 0; /* bool */

#ifdef TAGGED_ARRAYS

/******************************************************************************
 *
 * Function:
 *   void AccessVect( void *v, char *i_str, int i)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
AccessVect (void *v, char *i_str, int i)
{
    DBUG_ENTER ("AccessVect");

    if (i_str != NULL) {
        fprintf (outfile, "SAC_ND_READ( %s, %s)", (char *)v, i_str);
    } else {
        fprintf (outfile, "SAC_ND_READ( %s, %d)", (char *)v, i);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void AccessConst( void *v, char *i_str, int i)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
AccessConst (void *v, char *i_str, int i)
{
    DBUG_ENTER ("AccessConst");

    if (i_str != NULL) {
        DBUG_ASSERT ((0), "illegal argument for AccessConst() found!");
    } else {
        fprintf (outfile, "%s", ((char **)v)[i]);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void AccessDim( void *v)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
AccessDim (void *v)
{
    DBUG_ENTER ("AccessDim");

    fprintf (outfile, "SAC_ND_A_DIM( %s)", (char *)v);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   void AccessShape( void *v, char *i_str, int i)
 *
 * Description:
 *
 *
 ******************************************************************************/

void
AccessShape (void *v, char *i_str, int i)
{
    DBUG_ENTER ("AccessShape");

    if (i_str != NULL) {
        fprintf (outfile, "SAC_ND_A_SHAPE( %s, %s)", (char *)v, i_str);
    } else {
        fprintf (outfile, "SAC_ND_A_SHAPE( %s, %d)", (char *)v, i);
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

    VectToOffset2 (offset, v, dimv, v_acc_dim, v_acc_shp, a, dima, AccessDim,
                   AccessShape);

    DBUG_VOID_RETURN;
}

#endif
