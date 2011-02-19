#include "DupTree.h"
#include "str.h"
#include "free.h"
#include "ctinfo.h"
#include "tree_basic.h"
#include "traverse.h"
#include "globals.h"
#include "dbug.h"
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

    DBUG_ENTER ("printDepthAndPre");

    DBUG_ASSERT ((d != NULL), "Cannot print information for a NULL array");

    int i;

    printf ("\n----------\n");

    for (i = 0; i < DYNARRAY_TOTALELEMS (d); i++) {
        printf ("{%d,", *(int *)ELEM_DATA (DYNARRAY_ELEMS_POS (d, i)));
        printf ("%d} ", ELEM_IDX (DYNARRAY_ELEMS_POS (d, i)));
    }

    printf ("\n----------\n");

    DBUG_VOID_RETURN;
}

void
printLubInfo (lubinfo *linfo)
{

    DBUG_ENTER ("printLubInfo");

    int i;

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
            printMatrix (LUBINFO_PCPTMAT (linfo));
        }

        if (LUBINFO_PCPCMAT (linfo) != NULL) {
            printMatrix (LUBINFO_PCPCMAT (linfo));
        }
    }

    DBUG_VOID_RETURN;
}
