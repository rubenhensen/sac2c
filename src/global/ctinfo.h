/*
 *
 * $Log$
 * Revision 1.1  2005/01/07 16:48:36  cg
 * Initial revision
 *
 *
 */

/**
 *
 * @file
 *
 * header file
 *
 */

#ifndef _SAC_CTINFO_H_
#define _SAC_CTINFO_H_

extern void CTIinstallInterruptHandlers ();
extern void CTIerror (int line, const char *format, ...);
extern void CTIsyserror (const char *format, ...);
extern void CTIabort (int line, const char *format, ...);
extern void CTIsysabort (const char *format, ...);
extern void CTIabortOnError ();
extern void CTIwarning (int line, const char *format, ...);
extern void CTIsyswarning (const char *format, ...);
extern void CTInote (const char *format, ...);
extern void CTItell (const char *format, ...);

#endif /* _SAC_CTINFO_H_ */
