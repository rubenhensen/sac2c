/**
 * @brief Provides the interface to profile.c
 */
#ifndef _SAC_PROFILE_PRINT_H
#define _SAC_PROFILE_PRINT_H

#include "libsac/profile/profile.h"

#ifndef SAC_C_EXTERN
#define SAC_C_EXTERN extern
#endif /* SAC_C_EXTERN */

/*
 * External declarations of library functions defined in libsac
 */

SAC_C_EXTERN void SAC_PF_PrintHeader (char *title);
SAC_C_EXTERN void SAC_PF_PrintHeaderNode (char *title, size_t rank);
SAC_C_EXTERN void SAC_PF_PrintSubHeader (char *title, int lineno);
SAC_C_EXTERN void SAC_PF_PrintSection (char *title);
SAC_C_EXTERN int SAC_PF_Printf (const char *format, ...);
SAC_C_EXTERN void SAC_PF_PrintTime (char *title, char *space, SAC_PF_TIMER *time);
SAC_C_EXTERN void SAC_PF_PrintCount (char *title, char *space, unsigned long count);
SAC_C_EXTERN void SAC_PF_PrintSize (char *title, char *space, unsigned long size, char *unit);
SAC_C_EXTERN void SAC_PF_PrintTimePercentage (char *title, char *space,
                                              SAC_PF_TIMER *time1, SAC_PF_TIMER *time2);
#endif /* _SAC_PROFILE_PRINT_H */


