/*
 *
 * $Log$
 * Revision 1.7  2002/10/28 14:54:55  sbs
 * NT2OTcast added.
 *
 * Revision 1.6  2002/10/23 06:35:39  sbs
 * NT2OTwithid added. It inserts scalar index variables whenever possible now.
 *
 * Revision 1.5  2002/10/08 16:34:09  dkr
 * NT2OTreturn() removed
 *
 * Revision 1.4  2002/10/08 10:35:57  dkr
 * some new traversal functions added
 *
 * Revision 1.3  2002/08/30 16:33:48  dkr
 * NT2OTarray added
 *
 * Revision 1.2  2002/08/13 12:17:47  dkr
 * NT2OTarg added
 *
 * Revision 1.1  2002/08/05 16:58:33  sbs
 * Initial revision
 *
 */

#ifndef _new2old_h_
#define _new2old_h_

#include "types.h"

extern node *NT2OTTransform (node *arg_node);

extern node *NT2OTmodul (node *arg_node, node *arg_info);
extern node *NT2OTfundef (node *arg_node, node *arg_info);
extern node *NT2OTarg (node *arg_node, node *arg_info);
extern node *NT2OTblock (node *arg_node, node *arg_info);
extern node *NT2OTvardec (node *arg_node, node *arg_info);
extern node *NT2OTarray (node *arg_node, node *arg_info);
extern node *NT2OTcast (node *arg_node, node *arg_info);
extern node *NT2OTlet (node *arg_node, node *arg_info);
extern node *NT2OTwithop (node *arg_node, node *arg_info);
extern node *NT2OTwithid (node *arg_node, node *arg_info);

#endif /* _new2old_h_ */
