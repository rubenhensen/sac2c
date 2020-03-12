#ifndef _SAC_CUDA_HWLOC_H_
#define _SAC_CUDA_HWLOC_H_

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief check if HWLOC command has failed
 *
 * HWLOC functions return 0 for true, and -1 (or other non-zero) value for
 * false. This is opposite to normal C conditional checks, as such we define a
 * macro to make this distinction less confusing.
 *
 * @param ret the return value (assumed to be signed int)
 */
#define HWLOC_IS_TRUE(ret) ret == 0

extern bool SAC_CUDA_HWLOC_init (int cuda_ordinal, char *str, size_t str_size);

#endif /* _SAC_CUDA_HWLOC_H_ */
