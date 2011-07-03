#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "memory.h"
#include "tree_compound.h"
#include "graphtypes.h"
#include "types.h"
#include "dynelem.h"
#include "elemstack.h"
#include "dynarray.h"
#include "dynmatrix.h"
#include "tfprintutils.h"

void
printDynarray (dynarray *arrayd)
{

    int i;

    printf ("[");

    for (i = 0; i < DYNARRAY_TOTALELEMS (arrayd); i++) {
        printf ("%d,", ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i)));
    }

    printf ("]\n");
}

void
printMatrix (matrix *m)
{

    int i, j;
    dynarray *arrayd;

    dynarray **array2d = MATRIX_ARRAY2D (m);

    printf ("\n");

    for (i = 0; i < MATRIX_TOTALROWS (m); i++) {

        arrayd = array2d[i];

        if (arrayd != NULL) {

            for (j = 0; j < DYNARRAY_TOTALELEMS (arrayd); j++) {

                if (DYNARRAY_ELEMS_POS (arrayd, j) != NULL) {
                    printf ("%d,", ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, j)));
                    fflush (stdout);
                } else
                    printf ("-,");
            }

            for (j = DYNARRAY_TOTALELEMS (arrayd); j < MATRIX_TOTALCOLS (m); j++) {
                printf ("-,");
            }

        } else {

            for (j = 0; j < MATRIX_TOTALCOLS (m); j++)
                printf ("-,");
        }

        printf ("\n");
    }
}

void
printMatrixInDotFormat (matrix *m)
{

    int i, j;
    static int id = 0;
    dynarray **array2d = MATRIX_ARRAY2D (m);
    dynarray *arrayd;

    fprintf (global.outfile, "struct%d [label=\"", id++);

    for (i = 0; i < MATRIX_TOTALROWS (m); i++) {

        arrayd = array2d[i];

        if (arrayd != NULL) {

            fprintf (global.outfile, "{");

            for (j = 0; j < DYNARRAY_TOTALELEMS (arrayd); j++) {

                if (DYNARRAY_ELEMS_POS (arrayd, j) != NULL) {

                    fprintf (global.outfile, "%d",
                             ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, j)));

                } else {

                    fprintf (global.outfile, "-");
                }

                if (j != DYNARRAY_TOTALELEMS (arrayd) - 1)
                    printf ("|");
            }

            for (j = DYNARRAY_TOTALELEMS (arrayd); j < MATRIX_TOTALCOLS (m); j++) {

                fprintf (global.outfile, "-");
                if (j != MATRIX_TOTALCOLS (m) - 1)
                    fprintf (global.outfile, "|");
            }

        } else {

            for (j = 0; j < MATRIX_TOTALCOLS (m); j++) {

                fprintf (global.outfile, "-");
                if (j != MATRIX_TOTALCOLS (m) - 1)
                    fprintf (global.outfile, "|");
            }
        }

        fprintf (global.outfile, "}");

        if (i != MATRIX_TOTALROWS (m) - 1)
            fprintf (global.outfile, "|");
    }

    fprintf (global.outfile, "\"];\n");
}

void
printTransitiveLinkTable (dynarray *arrayd)
{

    int i;

    for (i = 0; i < DYNARRAY_TOTALELEMS (arrayd); i++) {

        printf ("%d->[%d,%d)\n", ELEM_IDX (DYNARRAY_ELEMS_POS (arrayd, i)),
                *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, i))),
                *((int *)ELEM_DATA (DYNARRAY_ELEMS_POS (arrayd, i)) + 1));
    }
}

void
printDepthAndPre (dynarray *d)
{

    DBUG_ENTER ();

    DBUG_ASSERT (d != NULL, "Cannot print information for a NULL array");

    int i;

    printf ("\n----------\n");

    for (i = 0; i < DYNARRAY_TOTALELEMS (d); i++) {
        printf ("{%d,", *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (d, i)));
        printf ("%d} ", ELEM_IDX (DYNARRAY_ELEMS_POS (d, i)));
    }

    printf ("\n----------\n");

    DBUG_RETURN ();
}

void
printPCPTMat (matrix *pcptmat, dynarray *csrc, dynarray *ctar)
{

    DBUG_ENTER ();

    printf ("\n");
    printf ("PCPT Matrix \n");
    printf ("----------- \n");

    int i, j;
    elem *e;
    int lower, upper;

    for (i = -1; i < DYNARRAY_TOTALELEMS (csrc); i++) {

        if (i == -1) {

            printf ("| \t");

            for (j = 0; j < DYNARRAY_TOTALELEMS (csrc); j++) {

                printf ("| %d\t\t", ELEM_IDX (DYNARRAY_ELEMS_POS (csrc, j)));
            }

        } else {

            for (j = -1; j < DYNARRAY_TOTALELEMS (ctar); j++) {

                if (j == -1) {

                    printf ("| %d\t", ELEM_IDX (DYNARRAY_ELEMS_POS (ctar, i)));

                } else {

                    e = getMatrixElem (pcptmat, i, j);

                    lower = ((int *)ELEM_DATA (e))[0];
                    upper = ((int *)ELEM_DATA (e))[1];

                    printf ("| (%d, %d)\t", lower, upper);
                }
            }
        }

        printf ("|\n");
    }

    DBUG_RETURN ();
}

void
printPCPCMat (matrix *pcpcmat, dynarray *ctar)
{

    DBUG_ENTER ();

    int i, j;

    printf ("\n");
    printf ("PCPC Matrix \n");
    printf ("----------- \n");

    for (i = -1; i < DYNARRAY_TOTALELEMS (ctar); i++) {

        if (i == -1) {

            printf ("| \t");

            for (j = 0; j < DYNARRAY_TOTALELEMS (ctar); j++) {

                printf ("| %d\t", ELEM_IDX (DYNARRAY_ELEMS_POS (ctar, j)));
            }

        } else {

            for (j = -1; j < DYNARRAY_TOTALELEMS (ctar); j++) {

                if (j == -1) {

                    printf ("| %d\t", ELEM_IDX (DYNARRAY_ELEMS_POS (ctar, i)));

                } else {

                    printf ("| %d\t", getMatrixValue (pcpcmat, i, j));
                }
            }
        }

        printf ("|\n");
    }

    DBUG_RETURN ();
}

void
printLubInfo (compinfo *ci)
{

    DBUG_ENTER ();

    lubinfo *linfo;
    int i;

    linfo = COMPINFO_LUB (ci);

    if (linfo != NULL) {

        if (LUBINFO_BLOCKMIN (linfo) != NULL) {
            printDepthAndPre (LUBINFO_BLOCKMIN (linfo));
        }

        if (LUBINFO_INTRAMATS (linfo) != NULL) {

            for (i = 0; i < LUBINFO_NUMINTRA (linfo); i++) {

                if (LUBINFO_INTRAMATS_POS (linfo, i) != NULL) {
                    printMatrix (LUBINFO_INTRAMATS_POS (linfo, i));
                }
            }
        }

        if (LUBINFO_INTERMAT (linfo) != NULL) {
            printMatrix (LUBINFO_INTERMAT (linfo));
        }

        if (LUBINFO_PCPTMAT (linfo) != NULL) {
            printPCPTMat (LUBINFO_PCPTMAT (linfo), COMPINFO_CSRC (ci),
                          COMPINFO_CTAR (ci));
        }

        if (LUBINFO_PCPCMAT (linfo) != NULL) {
            printPCPCMat (LUBINFO_PCPCMAT (linfo), COMPINFO_CTAR (ci));
        }
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
