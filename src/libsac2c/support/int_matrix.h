
#ifndef _INT_MATRIX_H_
#define _INT_MATRIX_H_

#include "types.h"

extern IntMatrix NewMatrix (int dim_x, int dim_y);
extern IntMatrix DupMatrix (IntMatrix m);
extern void FreeMatrix (IntMatrix m);
extern void MatrixToReducedREForm (IntMatrix m);
extern int MatrixRank (IntMatrix m);
extern void MatrixDisplay (IntMatrix m, FILE *file);
extern void MatrixToFile (IntMatrix m, FILE *file);
extern void MatrixSetEntry (IntMatrix m, int x, int y, int elem);
extern int MatrixGetEntry (IntMatrix m, int x, int y);
extern int MatrixRows (IntMatrix m);
extern int MatrixCols (IntMatrix m);
extern bool MatrixEqual (IntMatrix m1, IntMatrix m2);
extern void MatrixClearRow (IntMatrix m, int row);

#endif /* _INT_MATRIX_H_ */
