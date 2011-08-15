
#ifndef _INT_MATRIX_H_
#define _INT_MATRIX_H_

#include "types.h"

extern IntMatrix NewMatrix (int dim_x, int dim_y);
extern IntMatrix DupMatrix (IntMatrix m);
extern void FreeMatrix (IntMatrix m);
extern void MatrixToReducedREForm (IntMatrix m);
extern int MatrixRank (IntMatrix m);
extern void MatrixDisplay (IntMatrix m, FILE *file);
extern void MatrixSetEntry (IntMatrix m, int x, int y, int elem);
extern int MatrixGetEntry (IntMatrix m, int x, int y);

#endif /* _INT_MATRIX_H_ */
