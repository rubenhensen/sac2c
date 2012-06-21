/*
 * $Id$
 */

#ifndef _SAC_MEMORY_H_
#define _SAC_MEMORY_H_

extern void MEMdbugMemoryLeakCheck (void);
extern void *MEMmallocAt (int size, char *file, int line);
#define MEMmalloc(size) MEMmallocAt (size, __FILE__, __LINE__)

extern void *MEMfree (void *address);

extern void *MEMcopy (int size, void *mem);

extern void *__MEMrealloc (void *, int);

#endif /* _SAC_MEMORY_H_ */
