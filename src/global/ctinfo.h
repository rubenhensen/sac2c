/*
 *
 * $Log$
 * Revision 1.5  2005/03/10 09:41:09  cg
 * Added CTIterminateCompilation()
 *
 * Revision 1.4  2005/01/12 15:50:46  cg
 * Added CTIterminateCompilation.
 *
 * Revision 1.3  2005/01/11 15:11:46  cg
 * Added some useful functionality.
 *
 * Revision 1.2  2005/01/07 19:54:13  cg
 * Some streamlining done.
 *
 * Revision 1.1  2005/01/07 16:48:36  cg
 * Initial revision
 *
 *
 */

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

extern void CTIinstallInterruptHandlers ();
extern void CTIerror (const char *format, ...);
extern void CTIerrorLine (int line, const char *format, ...);
extern void CTIerrorContinued (const char *format, ...);
extern int CTIgetErrorMessageLineLength ();
extern void CTIabort (const char *format, ...);
extern void CTIabortLine (int line, const char *format, ...);
extern void CTIabortOnError ();
extern void CTIwarn (const char *format, ...);
extern void CTIwarnLine (int line, const char *format, ...);
extern void CTIwarnContinued (const char *format, ...);
extern int CTIgetWarnMessageLineLength ();
extern void CTIstate (const char *format, ...);
extern void CTInote (const char *format, ...);
extern void CTIterminateCompilation (compiler_phase_t phase, char *break_specifier,
                                     node *syntax_tree);

#endif /* _SAC_CTINFO_H_ */
