/* *
 * $Log$
 * Revision 1.4  2003/02/24 17:40:36  mwe
 * removed some functions
 *
 * Revision 1.3  2003/02/10 18:01:30  mwe
 * removed needles functions
 *
 * Revision 1.2  2003/02/09 22:32:19  mwe
 * removed bugs
 *
 * Revision 1.1  2003/02/08 16:08:16  mwe
 * Initial revision
 *
 *
 */

#ifndef _DistributiveLaw_h_
#define _DistributiveLaw_h_

extern node *DistributiveLaw (node *, node *);
extern node *DLblock (node *, node *);
extern node *DLassign (node *, node *);
extern node *DLlet (node *, node *);
extern node *DLPrfOrAp (node *, node *);

#endif
