/*
 *
 * $Log$
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

extern void CTIinstallInterruptHandlers ();
extern void CTIerror (const char *format, ...);
extern void CTIerrorLine (int line, const char *format, ...);
extern void CTIerrorContinued (const char *format, ...);
extern void CTIabort (const char *format, ...);
extern void CTIabortLine (int line, const char *format, ...);
extern void CTIabortOnError ();
extern void CTIwarn (const char *format, ...);
extern void CTIwarnLine (int line, const char *format, ...);
extern void CTIstate (const char *format, ...);
extern void CTInote (const char *format, ...);

#endif /* _SAC_CTINFO_H_ */
