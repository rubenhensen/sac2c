/*
 *
 * $Log$
 * Revision 1.3  1995/05/24 13:58:04  sbs
 * ctrl-l eliminated
 *
 * Revision 1.2  1995/05/22  12:06:51  sbs
 * TRACE_MEM inserted
 *
 * Revision 1.1  1995/05/04  11:48:35  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_trace_h

#define _sac_trace_h

extern int traceflag; /* imported from main.c */

#define NO_TRACE 0x0000
#define TRACE_UDF 0x0001
#define TRACE_PRF 0x0002
#define TRACE_WST 0x0004
#define TRACE_REF 0x0008
#define TRACE_MEM 0x0010
#define TRACE_ALL 0xffff

#endif /* _sac_trace_h */
