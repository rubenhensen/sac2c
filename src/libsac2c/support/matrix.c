#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "matrix.h"

#include "memory.h"
#include "dbug.h"
#include "math_utils.h"
#include "str_buffer.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "namespaces.h"
#include "print.h"

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
MatrixMulAndAddRows (Matrix m, int ixrdest, int ixrsrc, int mplr)
{
    int ix;
    int *drow, *srow;

    DBUG_ENTER ("MatrixMulAndAddRows");

    drow = m->mtx[ixrdest];
    srow = m->mtx[ixrsrc];
    for (ix = 0; ix < m->dim_x; ix++) {
        drow[ix] += mplr * srow[ix];
    }

    DBUG_VOID_RETURN;
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
MatrixSwapRows (Matrix m, int rix1, int rix2)
{
    int *r1, *r2, temp;
    int ix;

    DBUG_ENTER ("MatrixSwapRows");

    if (rix1 == rix2)
        return;
    r1 = m->mtx[rix1];
    r2 = m->mtx[rix2];
    for (ix = 0; ix < m->dim_x; ix++) {
        temp = r1[ix];
        r1[ix] = r2[ix];
        r2[ix] = temp;
    }

    DBUG_VOID_RETURN;
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
MatrixNormalizeRow (Matrix m, int rix, int lead)
{
    int ix;
    int *drow;
    int lv;

    DBUG_ENTER ("MatrixNormalizeRow");

    drow = m->mtx[rix];
    lv = drow[lead];
    for (ix = 0; ix < m->dim_x; ix++) {
        drow[ix] /= lv;
    }

    DBUG_VOID_RETURN;
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
static int
NumOfZeroRows (Matrix m)
{
    int rows, cols;
    int i, j, count = 0;
    int nzr;

    DBUG_ENTER ("NumOfZeroRows");

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
Matrix
NewMatrix (int dim_x, int dim_y)
{
    int n, i, j;
    Matrix m;

    DBUG_ENTER ("NewMatrix");

    m = malloc (sizeof (sMatrix));
    n = dim_x * dim_y;
    m->dim_x = dim_x;
    m->dim_y = dim_y;
    m->m_stor = malloc (n * sizeof (int));
    m->mtx = malloc (m->dim_y * sizeof (int *));
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
void
FreeMatrix (Matrix m)
{
    DBUG_ENTER ("FreeMatrix");

    free (m->m_stor);
    free (m->mtx);
    free (m);

    DBUG_VOID_RETURN;
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
MatrixSetEntry (Matrix m, int x, int y, int elem)
{
    DBUG_ENTER ("MatrixSetEntry");

    m->mtx[y][x] = elem;

    DBUG_VOID_RETURN;
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
MatrixGetEntry (Matrix m, int x, int y)
{
    int elem;

    DBUG_ENTER ("MatrixGetEntry");

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
MatrixToReducedREForm (Matrix m)
{
    int lead;
    int rix, iix;
    int lv;
    int rowCount = m->dim_y;

    DBUG_ENTER ("MatrixToReducedREForm");

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

    DBUG_VOID_RETURN;
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
MatrixRank (Matrix m)
{
    Matrix tmp;
    int n, rank;

    DBUG_ENTER ("MatrixRank");

    n = m->dim_x * m->dim_y;
    tmp = NewMatrix (m->dim_x, m->dim_y);
    memcpy (tmp->m_stor, m->m_stor, sizeof (int) * n);
    memcpy (tmp->mtx, m->mtx, sizeof (int *) * m->dim_y);

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
MatrixDisplay (Matrix m, FILE *file)
{
    int iy, ix;
    const char *sc;

    DBUG_ENTER ("MatrixDisplay");

    for (iy = 0; iy < m->dim_y; iy++) {
        fprintf (file, "   ");
        sc = " ";
        INDENT;
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

    DBUG_VOID_RETURN;
}
