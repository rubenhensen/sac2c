/* *
 * $Log$
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

extern node *ElimSubDiv (node *, node *);
extern node *ESDblock (node *, node *);
extern node *ESDassign (node *, node *);
extern node *ESDlet (node *, node *);
extern node *ESDprf (node *, node *);

#endif
