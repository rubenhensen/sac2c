/**
 *
 * @file
 *
 * header file for ctinfo.c
 *
 */

#ifndef _SAC_CTINFO_H_
#define _SAC_CTINFO_H_

#include "types.h"
#include <stdarg.h>

extern int CTIgetErrorCount (void);
extern void CTIexit (int);
extern void CTIinstallInterruptHandlers (void);
extern char *CTIgetErrorMessageVA (int line, const char *file, const char *format,
                                   va_list arg_p);
extern void CTIerror (const char *format, ...);
extern void CTIerrorLoc (struct location loc, const char *format, ...);
extern void CTIerrorLine (int line, const char *format, ...);
extern void CTIerrorContinued (const char *format, ...);
extern void CTIerrorInternal (const char *format, ...);
extern int CTIgetErrorMessageLineLength (void);
extern void CTIabortOnBottom (char *err_msg);
extern void CTIabort (const char *format, ...);
extern void CTIabortLine (int line, const char *format, ...);
extern void CTIabortOutOfMemory (unsigned int request);
extern void CTIabortOnError (void);
extern void CTIwarn (const char *format, ...);
extern void CTIwarnLoc (struct location loc, const char *format, ...);
extern void CTIwarnLine (int line, const char *format, ...);
extern void CTIwarnContinued (const char *format, ...);
extern int CTIgetWarnMessageLineLength (void);
extern void CTIstate (const char *format, ...);
extern void CTInote (const char *format, ...);
extern void CTInoteLine (int line, const char *format, ...);
extern void CTItell (int level, const char *format, ...);
extern void CTIterminateCompilation (node *syntax_tree);
extern void CTIterminateCompilationSilent (void);
extern const char *CTIitemName (node *item);
extern const char *CTIfunParams (node *fundef);

#endif /* _SAC_CTINFO_H_ */
