/*
 *
 * $Log$
 * Revision 1.5  1996/02/05 09:21:48  sbs
 * RuntimError => Runtime_Error
 *
 * Revision 1.4  1996/01/25  15:04:57  cg
 * added __SAC__Runtime_hidden_memcnt and __SAC__Runtime_array_memcnt
 *
 * Revision 1.3  1996/01/21  18:07:34  cg
 * added declaration of __SAC__Runtime_trace_memcnt
 *
 * Revision 1.2  1996/01/09  08:54:00  cg
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

extern int __SAC__Runtime_hidden_memcnt;
extern int __SAC__Runtime_array_memcnt;

extern void __SAC__Runtime_Error (char *format, ...);
extern void __SAC__Runtime_Print (char *format, ...);
extern void __SAC__Runtime_PrintTraceHeader (char *format, ...);
extern void __SAC__Runtime_PrintTraceInfo (char *format, ...);
extern void *__SAC__Runtime_malloc (int size);

#endif /* _sac_libsac_h */
