/*
 * $Id$
 */

#ifndef _SAC_MEMORY_H_
#define _SAC_MEMORY_H_

extern void MEMdbugMemoryLeakCheck (void);
extern void *MEMmallocAt (int size, char *file, int line);
#define MEMmalloc(size) MEMmallocAt (size, __FILE__, __LINE__)

template <typename T> T __MEMfree (T address);
#define MEMfree(x) __MEMfree<__typeof(x)> (x)

extern void *MEMcopy (int size, void *mem);

extern void *__MEMrealloc (void *, int);

#endif /* _SAC_MEMORY_H_ */
