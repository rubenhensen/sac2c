
#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "types.h"

extern Matrix NewMatrix (int dim_x, int dim_y);
extern void FreeMatrix (Matrix m);
extern void MatrixToReducedREForm (Matrix m);
extern int MatrixRank (Matrix m);
extern void MatrixDisplay (Matrix m);

#endif /* _MATRIX_H_ */
