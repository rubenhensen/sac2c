/* *
 * $Log$
 * Revision 1.2  2004/07/07 15:57:05  mwe
 * former log-messages added
 *
 *
 *
 * revision 1.1    locked by: mwe;
 * date: 2003/04/26 20:58:53;  author: mwe;  state: Exp;
 * Initial revision
 */

#ifndef _UndoElimSubDiv_h_
#define _UndoElimSubDiv_h_

extern node *UndoElimSubDiv (node *, node *);
extern node *UESDblock (node *, node *);
extern node *UESDassign (node *, node *);
extern node *UESDlet (node *, node *);
extern node *UESDprf (node *, node *);

#endif
