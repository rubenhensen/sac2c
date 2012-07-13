/*
 *
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   sac_bool.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides type and macro definitions for the implementation of
 *   the SAC standard data type bool.
 *
 *****************************************************************************/

#ifndef _SAC_BOOL_H
#define _SAC_BOOL_H

#define MUTC 1
#if SAC_BACKEND == MUTC
#include "stdbool.h"
#else

#ifdef __bool_true_false_are_defined /* C99 standard */
#undef bool
#undef true
#undef false
#endif /*  __bool_true_false_are_defined */

#ifndef SAC_SIMD_COMPILATION
#if SAC_MUTC_MACROS
typedef int boolbool;
#elif SAC_CUDA_MACROS
typedef int boolboolbool;
#else
#ifndef __cplusplus
typedef int bool;
#endif
#endif
#endif

#define true 1
#define false 0

#endif
#undef MUTC

#endif /* _SAC_BOOL_H */
