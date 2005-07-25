/*
 *
 * $Log$
 * Revision 1.1  2005/07/25 16:43:24  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_PRIVATE_HEAP_H_
#define _SAC_PRIVATE_HEAP_H_

#include "types.h"

extern heap *PHPcreateHeap (size_t elem_size, int chunk_size);
extern void *PHPmalloc (heap *private_heap);
extern void *PHPfindElem (heap *private_heap, php_cmp_fun fun, void *elem);
extern heap *PHPfreeHeap (heap *private_heap);

#endif /* _SAC_PRIVATE_HEAP_H_ */
