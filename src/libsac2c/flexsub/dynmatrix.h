#ifndef _DYNMATRIX_H_
#define _DYNMATRIX_H_

#include "types.h"

extern void initMatrix (matrix *m);
extern void free2DArray (dynarray **d2, int count);
extern void freeMatrix (matrix *m);
extern void setMatrixValue (matrix *m, int x, int y, int value);
extern void setMatrixElem (matrix *m, int x, int y, elem *element);
extern elem *getMatrixElem (matrix *m, int x, int y);
extern int getMatrixValue (matrix *m, int x, int y);

#define MATRIX_ARRAY2D(n) ((n)->array2d)
#define MATRIX_TOTALROWS(n) ((n)->totalrows)
#define MATRIX_TOTALCOLS(n) ((n)->totalcols)

#endif
