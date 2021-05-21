/**
 * @brief Provides the interface to profile.c
 */
#ifndef _SAC_PROFILE_PRINT_H
#define _SAC_PROFILE_PRINT_H

#include "libsac/profile/profile.h"

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

#define BYTES_TO_KBYTES(bytes) (bytes / 1024)

/*
 * External declarations of library functions defined in libsac
 */

SAC_C_EXTERN void SAC_PF_PrintHeader (const char *title);
SAC_C_EXTERN void SAC_PF_PrintHeaderNode (const char *title, size_t rank);
SAC_C_EXTERN void SAC_PF_PrintSubHeader (const char *title, size_t lineno);
SAC_C_EXTERN void SAC_PF_PrintSection (const char *title);
SAC_C_EXTERN void SAC_PF_PrintTime (const char *title, const char *space, const SAC_PF_TIMER *time);
SAC_C_EXTERN void SAC_PF_PrintCount (const char *title, const char *space, unsigned long count);
SAC_C_EXTERN void SAC_PF_PrintSize (const char *title, const char *space, unsigned long size, const char *unit);
SAC_C_EXTERN void SAC_PF_PrintTimePercentage (const char *title, const char *space,
                                              const SAC_PF_TIMER *time1, const SAC_PF_TIMER *time2);
#endif /* _SAC_PROFILE_PRINT_H */


