/*
 *
 * $Log$
 * Revision 1.9  1997/10/29 14:29:11  srs
 * removed HAVE_MALLOC_O
 *
 * Revision 1.8  1997/04/24 15:00:54  sbs
 * HAVE_MALLOC_O inserted
 *
 * Revision 1.7  1995/12/28  10:32:52  cg
 * added declarations for malloc_debug and malloc_verify
 *
 * Revision 1.6  1994/12/30  16:57:01  sbs
 * commented out #ifndef DBUG_OFF
 *
 * Revision 1.5  1994/12/21  11:33:29  hw
 * added char *mdb_type[]
 *
 * Revision 1.4  1994/12/05  13:05:57  hw
 * removed extern char *type_string[]
 *
 * Revision 1.3  1994/12/02  11:08:24  hw
 * inserted extern char *type_string[]
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

/* #ifndef DBUG_OFF    the following is only used with Fred Fish */

#ifndef _my_debug_h

#define _my_debug_h

extern char *mdb_nodetype[];

extern char *mdb_prf[];

extern char *mdb_type[];

#define P_FORMAT "(%06x)" /* formatstring for pointer address */

#endif /* _my_debug_h */

/* #endif   DBUG_OFF */
