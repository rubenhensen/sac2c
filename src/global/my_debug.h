/*
 *
 * $Log$
 * Revision 1.2  1994/11/10 15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef DBUG_OFF /* Dies Datenstruktur wird nur mit Fred Fish DBUG benutzt */

#ifndef _my_debug_h

#define _my_debug_h

extern char *mdb_nodetype[];

extern char *mdb_prf[];

#define P_FORMAT "(%06x)" /* formatstring for pointer address */

#endif /* _my_debug_h */

#endif /* DBUG_OFF */
