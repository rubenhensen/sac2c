/*****************************************************************************
 *
 * file:   misc.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides external declarations for global variables and functions
 *   defined in libsac_misc.c
 *
 *****************************************************************************/

#ifndef _SAC_MISC_H_
#define _SAC_MISC_H_

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#if SAC_MUTC_MACROS
#define SAC_String2Array(array, string) SAC_STRING2ARRAY (array, string)
#else
SAC_C_EXTERN void SAC_String2Array (unsigned char *array, const char *string);
#endif

#endif /* _SAC_MISC_H_ */

