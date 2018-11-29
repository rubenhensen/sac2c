#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "int_matrix.h"

#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "math_utils.h"
#include "str_buffer.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "namespaces.h"
#include "print.h"
#include "memory.h"

#define MatrixGet(m, rix, cix) m->mtx[rix][cix]

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
static void
MatrixMulAndAddRows (IntMatrix m, unsigned int ixrdest, unsigned int ixrsrc, int mplr)
{
    unsigned int ix;
    int *drow, *srow;

    DBUG_ENTER ();

    drow = m->mtx[ixrdest];
    srow = m->mtx[ixrsrc];
    for (ix = 0; ix < m->dim_x; ix++) {
        drow[ix] += mplr * srow[ix];
    }

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
static void
MatrixSwapRows (IntMatrix m, unsigned int rix1, unsigned int rix2)
{
    int *r1, *r2, temp;
    unsigned int ix;

    DBUG_ENTER ();

    if (rix1 == rix2)
        return;
    r1 = m->mtx[rix1];
    r2 = m->mtx[rix2];
    for (ix = 0; ix < m->dim_x; ix++) {
        temp = r1[ix];
        r1[ix] = r2[ix];
        r2[ix] = temp;
    }

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
static void
MatrixNormalizeRow (IntMatrix m, unsigned int rix, unsigned int lead)
{
    unsigned ix;
    int *drow;
    int lv;

    DBUG_ENTER ();

    drow = m->mtx[rix];
    lv = drow[lead];
    for (ix = 0; ix < m->dim_x; ix++) {
        drow[ix] /= lv;
    }

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
static unsigned int
NumOfZeroRows (IntMatrix m)
{
    unsigned int rows, cols;
    unsigned int i, j, count = 0;
    bool nzr;

    DBUG_ENTER ();

    rows = m->dim_y;
    cols = m->dim_x;

    for (i = 0; i < rows; i++) {
        nzr = TRUE;
        for (j = 0; j < cols; j++) {
            if (MatrixGet (m, i, j) != 0) {
                nzr = FALSE;
                break;
            }
        }
        if (nzr)
            count++;
    }

    DBUG_RETURN (count);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
IntMatrix
NewMatrix (unsigned int dim_x, unsigned int dim_y)
{
    unsigned int n, i, j;
    IntMatrix m;

    DBUG_ENTER ();

    m = (IntMatrix)MEMmalloc (sizeof (sMatrix));
    n = dim_x * dim_y;
    m->dim_x = dim_x;
    m->dim_y = dim_y;
    m->m_stor = (int *)MEMmalloc (n * sizeof (int));
    m->mtx = (int **)MEMmalloc (m->dim_y * sizeof (int *));
    for (n = 0; n < dim_y; n++) {
        m->mtx[n] = m->m_stor + n * dim_x;
    }

    for (i = 0; i < dim_y; i++) {
        for (j = 0; j < dim_x; j++) {
            m->mtx[i][j] = 0;
        }
    }
    DBUG_RETURN (m);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
IntMatrix
DupMatrix (IntMatrix m)
{
    unsigned int n, i, j;
    IntMatrix new_m;

    DBUG_ENTER ();

    new_m = (IntMatrix)MEMmalloc (sizeof (sMatrix));
    n = m->dim_x * m->dim_y;
    new_m->dim_x = m->dim_x;
    new_m->dim_y = m->dim_y;
    new_m->m_stor = (int *)MEMmalloc (n * sizeof (int));
    new_m->mtx = (int **)MEMmalloc (new_m->dim_y * sizeof (int *));
    for (n = 0; n < new_m->dim_y; n++) {
        new_m->mtx[n] = new_m->m_stor + n * new_m->dim_x;
    }

    for (i = 0; i < new_m->dim_y; i++) {
        for (j = 0; j < new_m->dim_x; j++) {
            new_m->mtx[i][j] = m->mtx[i][j];
        }
    }
    DBUG_RETURN (new_m);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
void
FreeMatrix (IntMatrix m)
{
    DBUG_ENTER ();

    if (!m)
        DBUG_RETURN ();

    MEMfree (m->m_stor);
    MEMfree (m->mtx);
    MEMfree (m);

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
void
MatrixSetEntry (IntMatrix m, unsigned int x, unsigned int y, int elem)
{
    DBUG_ENTER ();

    m->mtx[y][x] = elem;

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
int
MatrixGetEntry (IntMatrix m, unsigned int x, unsigned int y)
{
    int elem;

    DBUG_ENTER ();

    elem = m->mtx[y][x];

    DBUG_RETURN (elem);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
void
MatrixToReducedREForm (IntMatrix m)
{
    unsigned int lead;
    unsigned int rix, iix;
    int lv;
    unsigned int rowCount = m->dim_y;

    DBUG_ENTER ();

    lead = 0;
    for (rix = 0; rix < rowCount; rix++) {
        if (lead >= m->dim_x)
            return;
        iix = rix;
        while (0 == MatrixGet (m, iix, lead)) {
            iix++;
            if (iix == rowCount) {
                iix = rix;
                lead++;
                if (lead == m->dim_x)
                    return;
            }
        }
        MatrixSwapRows (m, iix, rix);
        MatrixNormalizeRow (m, rix, lead);
        for (iix = 0; iix < rowCount; iix++) {
            if (iix != rix) {
                lv = MatrixGet (m, iix, lead);
                MatrixMulAndAddRows (m, iix, rix, -lv);
            }
        }
        lead++;
    }

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
unsigned int
MatrixRank (IntMatrix m)
{
    IntMatrix tmp;
    unsigned int rank;

    DBUG_ENTER ();

    tmp = DupMatrix (m);

    MatrixToReducedREForm (tmp);

    rank = m->dim_y - NumOfZeroRows (tmp);

    FreeMatrix (tmp);

    DBUG_RETURN (rank);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
void
MatrixDisplay (IntMatrix m, FILE *file)
{
    unsigned int iy, ix;
    const char *sc;

    DBUG_ENTER ();

    for (iy = 0; iy < m->dim_y; iy++) {
        fprintf (file, "   ");
        sc = " ";

        for (ix = 0; ix < m->dim_x; ix++) {
            if (ix == 0) {
                fprintf (file, "%s   |%d", sc, m->mtx[iy][ix]);
            } else {
                fprintf (file, "%s %3d", sc, m->mtx[iy][ix]);
            }
            sc = ",";
        }
        fprintf (file, "|\n");
    }
    // fprintf( file, "\n");

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
void
MatrixToFile (IntMatrix m, FILE *file)
{
    unsigned int iy, ix;

    DBUG_ENTER ();

    fprintf (file, "%u %u\n", MatrixRows (m), MatrixCols (m));
    for (iy = 0; iy < m->dim_y; iy++) {
        for (ix = 0; ix < m->dim_x; ix++) {
            fprintf (file, "%d ", m->mtx[iy][ix]);
        }
        fprintf (file, "\n");
    }

    DBUG_RETURN ();
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
unsigned int
MatrixRows (IntMatrix m)
{
    unsigned int rows;

    DBUG_ENTER ();

    rows = m->dim_y;

    DBUG_RETURN (rows);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
unsigned int
MatrixCols (IntMatrix m)
{
    unsigned int cols;

    DBUG_ENTER ();

    cols = m->dim_x;

    DBUG_RETURN (cols);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
bool
MatrixEqual (IntMatrix m1, IntMatrix m2)
{
    unsigned int iy, ix;
    bool equal = TRUE;

    DBUG_ENTER ();

    if ((m1->dim_y != m2->dim_y) || (m1->dim_x != m2->dim_x)) {
        equal = FALSE;
    } else {
        for (iy = 0; iy < m1->dim_y; iy++) {
            for (ix = 0; ix < m1->dim_x; ix++) {
                if (m1->mtx[iy][ix] != m2->mtx[iy][ix]) {
                    equal = FALSE;
                    break;
                }
            }
            if (!equal)
                break;
        }
    }

    DBUG_RETURN (equal);
}

/*******************************************************************************
 *
 * Description:
 *
 * Parameters:
 *
 * Return:g
 *
 *******************************************************************************/
void
MatrixClearRow (IntMatrix m, unsigned int row)
{
    unsigned int ix;

    DBUG_ENTER ();

    for (ix = 0; ix < m->dim_x; ix++) {
        m->mtx[row][ix] = 0;
    }

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
