#ifndef __STRUCTURES_H__
#define __STRUCTURES_H__

#include <stdio.h>
#include <stdlib.h>
#include "types.h"

void initElem (elem *elem);
void initDynarray (dynarray *arrayd);
void initMatrix (matrix *m);
void initElemstack (elemstack *s);
void pushElemstack (elemstack **s, elem *e);
elem *popElemstack (elemstack **s);

void freeElem (elem *e);
void freeElemArray (elem **e, int count);
void freeDynarray (dynarray *arrayd);
void free2DArray (dynarray **d2, int count);
void freeMatrix (matrix *m);
void *MEMrealloc (void *src, int allocsize, int oldsize);

int addToArray (dynarray *arrayd, elem *item);
int addToArrayAtPos (dynarray *arrayd, elem *item, int pos);
void sortArray (elem **elems, int lower, int upper, int desc);
void setMatrixValue (matrix *m, int x, int y, int value);
int getMatrixValue (matrix *m, int x, int y);
void printMatrix (matrix *m);
void printMatrixInDotFormat (matrix *m);
void printTransitiveLinkTable (dynarray *arrayd);
void buildTransitiveLinkTable (dynarray *arrayd);
void setSrcTarArrays (dynarray *arrayd, dynarray **arrX, dynarray **arrY);
matrix *computeTLCMatrix (dynarray *arrayd, dynarray *arrX, dynarray *arrY);

#define ELEM_IDX(n) ((n)->idx)
#define ELEM_DATA(n) ((n)->data)

#define DYNARRAY_ELEMS(n) ((n)->elems)
#define DYNARRAY_ELEMS_POS(n, i) ((n)->elems[i])
#define DYNARRAY_TOTALELEMS(n) ((n)->totalelems)
#define DYNARRAY_ALLOCELEMS(n) ((n)->allocelems)

#define MATRIX_ARRAY2D(n) ((n)->array2d)
#define MATRIX_TOTALROWS(n) ((n)->totalrows)
#define MATRIX_TOTALCOLS(n) ((n)->totalcols)

#define ELEMSTACK_CURR(n) ((n)->curr)
#define ELEMSTACK_NEXT(n) ((n)->next)

#define COMPINFO_CSRC(n) ((n)->csrc)
#define COMPINFO_CTAR(n) ((n)->ctar)
#define COMPINFO_TLTABLE(n) ((n)->tltable)
#define COMPINFO_CROSSCLOS(n) ((n)->crossclos)
#define COMPINFO_TLC(n) ((n)->tlc)
#define COMPINFO_LUB(n) ((n)->lub)
#define COMPINFO_LUBPOS(n, i) ((n)->lub[i])
#define COMPINFO_DIST(n) ((n)->dist)
#define COMPINFO_TOPOLIST(n) ((n)->topolist)

#endif
