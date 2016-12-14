#include "regression.h"

#include <stdlib.h>
#include <math.h>

#include "memory.h"

#define DBUG_PREFIX "COMP"
#include "debug.h"

float **
Matrix (int r, int c)
{
    DBUG_ENTER ();

    float **m = MEMmalloc (r * sizeof (float *));

    for (int i = 0; i < r; i++) {
        m[i] = MEMmalloc (c * sizeof (float));
    }

    DBUG_RETURN (m);
}

void
DelMatrix (float **a, int r, int c)
{
    DBUG_ENTER ();

    for (int i = 0; i < r; i++) {
        MEMfree (a[i]);
    }
    MEMfree (a);

    DBUG_RETURN ();
}

/*
  Transpose of a matrix
*/
void
Transpose (float **a, int r, int c, float **trans)
{
    DBUG_ENTER ();

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            trans[j][i] = a[i][j];
        }
    }

    DBUG_RETURN ();
}

void
MxMMultiply (float **a, int r1, int c1, float **b, int r2, int c2, float **mult)
{
    DBUG_ENTER ();

    for (int i = 0; i < r1; i++) {
        for (int j = 0; j < c2; j++) {
            mult[i][j] = 0;
            for (int k = 0; k < c1; k++) {
                mult[i][j] += a[i][k] * b[k][j];
            }
        }
    }

    DBUG_RETURN ();
}

void
MxVMultiply (float **a, int r, int c, float *b, float *mult)
{
    DBUG_ENTER ();

    for (int i = 0; i < r; i++) {
        mult[i] = 0;
        for (int j = 0; j < c; j++) {
            mult[i] += a[i][j] * b[j];
        }
    }

    DBUG_RETURN ();
}

void
Inverse (float **a, int n, float **inv)
{
    DBUG_ENTER ();

    float det;
    float **cof = Matrix (n, n);

    det = Determinant (a, n);
    CoFactor (a, n, cof);

    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            inv[i][j] = 1.0 / det * cof[j][i];
        }
    }

    DelMatrix (cof, n, n);

    DBUG_RETURN ();
}

/*
  Recursive definition of determinate using expansion by minors.
*/
float
Determinant (float **a, int n)
{
    DBUG_ENTER ();

    int i, j, j1, j2;
    float det = 0;
    float **m = NULL;

    if (n < 1) {         /* Error */
    } else if (n == 1) { /* Shouldn't get used */
        det = a[0][0];
    } else if (n == 2) {
        det = a[0][0] * a[1][1] - a[1][0] * a[0][1];
    } else {
        det = 0;

        m = Matrix (n - 1, n - 1);
        for (j1 = 0; j1 < n; j1++) {

            for (i = 1; i < n; i++) {
                j2 = 0;

                for (j = 0; j < n; j++) {
                    if (j == j1) {
                        continue;
                    }

                    m[i - 1][j2] = a[i][j];
                    j2++;
                }
            }

            det += pow (-1.0, j1 + 2.0) * a[0][j1] * Determinant (m, n - 1);
        }
        DelMatrix (m, n - 1, n - 1);
    }

    DBUG_RETURN (det);
}

/*
  Find the cofactor matrix of a square matrix
*/
void
CoFactor (float **a, int n, float **b)
{
    DBUG_ENTER ();

    int i, j, ii, jj, i1, j1;
    float det;
    float **c = Matrix (n - 1, n - 1);

    for (j = 0; j < n; j++) {
        for (i = 0; i < n; i++) {

            /* Form the adjoint a_ij */
            i1 = 0;

            for (ii = 0; ii < n; ii++) {
                if (ii == i) {
                    continue;
                }
                j1 = 0;

                for (jj = 0; jj < n; jj++) {
                    if (jj == j) {
                        continue;
                    }
                    c[i1][j1] = a[ii][jj];
                    j1++;
                }
                i1++;
            }

            /* Calculate the determinate */
            det = Determinant (c, n - 1);

            /* Fill in the elements of the cofactor */
            b[i][j] = pow (-1.0, i + j + 2.0) * det;
        }
    }

    DelMatrix (c, n - 1, n - 1);

    DBUG_RETURN ();
}

void
PolyRegression (float **X, int r, int c, float *y, float *reg)
{
    DBUG_ENTER ();

    float **TX = Matrix (c, r);
    float **XX = Matrix (c, c);
    float **IX = Matrix (c, c);
    float **X3 = Matrix (c, r);

    Transpose (X, r, c, TX);
    MxMMultiply (TX, c, r, X, r, c, XX);
    Inverse (XX, c, IX);
    MxMMultiply (IX, c, c, TX, c, r, X3);
    MxVMultiply (X3, c, r, y, reg);

    DelMatrix (TX, c, r);
    DelMatrix (XX, c, c);
    DelMatrix (IX, c, c);
    DelMatrix (X3, c, r);

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
