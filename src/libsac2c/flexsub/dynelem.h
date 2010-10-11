#ifndef _ELEM_H_
#define _ELEM_H_

#define ELEM_IDX(n) ((n)->idx)
#define ELEM_DATA(n) ((n)->data)

extern void initElem (elem *e);
extern void freeElem (elem *e);

#endif
