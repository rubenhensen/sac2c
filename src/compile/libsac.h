/*
 *
 * $Log$
 * Revision 1.2  1996/01/09 08:54:00  cg
 * __SAC__Runtime_malloc(int size) now returns void*
 *
 * Revision 1.1  1996/01/09  08:31:41  cg
 * Initial revision
 *
 *
 *
 *
 */

#ifndef _sac_libsac_h

#define _sac_libsac_h

extern void __SAC__RuntimeError (char *format, ...);
extern void __SAC__Runtime_Print (char *format, ...);
extern void __SAC__Runtime_PrintTraceHeader (char *format, ...);
extern void __SAC__Runtime_PrintTraceInfo (char *format, ...);
extern void *__SAC__Runtime_malloc (int size);

#endif /* _sac_libsac_h */
