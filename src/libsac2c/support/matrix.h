
#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "types.h"

extern Matrix NewMatrix (int dim_x, int dim_y);
extern Matrix DupMatrix (Matrix m);
extern void FreeMatrix (Matrix m);
extern void MatrixToReducedREForm (Matrix m);
extern int MatrixRank (Matrix m);
extern void MatrixDisplay (Matrix m, FILE *file);
extern void MatrixSetEntry (Matrix m, int x, int y, int elem);
extern int MatrixGetEntry (Matrix m, int x, int y);

#endif /* _MATRIX_H_ */
