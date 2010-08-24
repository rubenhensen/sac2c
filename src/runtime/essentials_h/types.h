/*
 *
 * $Id: bool.h 16313 2009-08-12 12:09:00Z jgo $
 *
 */

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
