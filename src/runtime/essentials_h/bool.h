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

#ifndef __bool_true_false_are_defined /* C99 standard */

#ifndef SAC_SIMD_COMPILATION
typedef int bool;
#endif

#define true 1
#define false 0

#endif /*  __bool_true_false_are_defined */

#endif /* _SAC_BOOL_H */
