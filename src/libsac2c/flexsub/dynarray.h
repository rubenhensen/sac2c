#ifndef _DYNARRAY_H_
#define _DYNARRAY_H_

#define DYNARRAY_ELEMS(n) ((n)->elems)
#define DYNARRAY_ELEMS_POS(n, i) ((n)->elems[i])
#define DYNARRAY_TOTALELEMS(n) ((n)->totalelems)
#define DYNARRAY_ALLOCELEMS(n) ((n)->allocelems)

extern void initDynarray (dynarray *arrayd);
extern void freeElemArray (elem **e, int count);
extern void freeDynarray (dynarray *arrayd);
extern int addToArray (dynarray *arrayd, elem *item);
extern int indexExistsInArray (dynarray *arrayd, int idx);
extern int addToArrayAtPos (dynarray *arrayd, elem *item, int pos);
extern void merge (elem **elems, int lower, int upper, int desc);
extern void sortArray (elem **elems, int lower, int upper, int desc);

#endif
