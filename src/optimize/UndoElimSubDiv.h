/* *
 * $Log$
 * Revision 1.3  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
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

extern node *UndoElimSubDiv (node *);
extern node *UESDblock (node *, info *);
extern node *UESDassign (node *, info *);
extern node *UESDlet (node *, info *);
extern node *UESDprf (node *, info *);

#endif
