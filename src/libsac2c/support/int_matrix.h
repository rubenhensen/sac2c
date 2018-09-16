
#ifndef _INT_MATRIX_H_
#define _INT_MATRIX_H_

#include "types.h"

extern IntMatrix NewMatrix (unsigned int dim_x, unsigned int dim_y);
extern IntMatrix DupMatrix (IntMatrix m);
extern void FreeMatrix (IntMatrix m);
extern void MatrixToReducedREForm (IntMatrix m);
extern unsigned int MatrixRank (IntMatrix m);
extern void MatrixDisplay (IntMatrix m, FILE *file);
extern void MatrixToFile (IntMatrix m, FILE *file);
extern void MatrixSetEntry (IntMatrix m, unsigned int x, unsigned int y, int elem);
extern int MatrixGetEntry (IntMatrix m, unsigned int x, unsigned int y);
extern unsigned int MatrixRows (IntMatrix m);
extern unsigned int MatrixCols (IntMatrix m);
extern bool MatrixEqual (IntMatrix m1, IntMatrix m2);
extern void MatrixClearRow (IntMatrix m, unsigned int row);

#endif /* _INT_MATRIX_H_ */
