#ifndef _ELEMQUEUE_H_
#define _ELEMQUEUE_H_

#define ELEMQUEUE_HEAD(n) ((n)->head)
#define ELEMQUEUE_TAIL(n) ((n)->tail)

void EQenqueue (elemqueue *q, elem *e);
elem *EQdequeue (elemqueue *q);

#endif
