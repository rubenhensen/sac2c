/*****************************************************************************
 *
 * file:   types.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides type and macro definitions for the implementation of
 *   the SAC standard data types.
 *
 *****************************************************************************/

#ifndef _SAC_TYPES_H
#define _SAC_TYPES_H

#define MUTC 1
#if SAC_BACKEND == MUTC || defined SAC_BACKEND_MUTC
#ifndef __mt_freestanding__
#define USHORT_DEFINED
#define UINT_DEFINED
#define ULONG_DEFINED
#endif /* __mt_freestanding__ */
#endif /* SAC_BACKEND */
#undef MUTC

#ifndef FLOATVEC_DEFINED
#if HAVE_GCC_SIMD_OPERATIONS
typedef float __attribute__ ((vector_size (4 * sizeof (float)))) floatvec;
#define FLOATVEC_IDX(vec, idx) (vec)[(idx)]
#else
/* You cannot assign static arrays, so we go with struct.  Stupid C!  */
typedef union {
    float __flvec_array[4];
} floatvec;
#define FLOATVEC_IDX(vec, idx) (vec).__flvec_array[(idx)]
#endif
#define FLOATVEC_DEFINED
#endif

#ifndef BYTE_DEFINED
typedef char byte;
#define BYTE_DEFINED
#endif

#ifndef LONGLONG_DEFINED
typedef long long longlong;
#define LONGLONG_DEFINED
#endif

#ifndef UBYTE_DEFINED
typedef unsigned char ubyte;
#define UBYTE_DEFINED
#endif

#ifndef ULONGLONG_DEFINED
typedef unsigned long long ulonglong;
#define ULONGLONG_DEFINED
#endif

#ifndef USHORT_DEFINED
typedef unsigned short ushort;
#define USHORT_DEFINED
#endif

#ifndef UINT_DEFINED
typedef unsigned int uint;
#define UINT_DEFINED
#endif

#ifndef ULONG_DEFINED
typedef unsigned long ulong;
#define ULONG_DEFINED
#endif

#endif /* _TYPES_H */
