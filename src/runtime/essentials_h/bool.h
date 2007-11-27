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

#ifndef SAC_SIMD_COMPILATION
#ifndef _sac_types_h
typedef int bool;
#endif
#endif

#define true 1
#define false 0

#endif /* _SAC_BOOL_H */
