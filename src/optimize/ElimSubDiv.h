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
 * date: 2003/04/26 20:54:49;  author: mwe;  state: Exp;
 * Initial revision
 */

#ifndef _ElimSubDiv_h_
#define _ElimSubDiv_h_

extern node *ElimSubDiv (node *);
extern node *ESDblock (node *, info *);
extern node *ESDassign (node *, info *);
extern node *ESDlet (node *, info *);
extern node *ESDprf (node *, info *);

#endif
