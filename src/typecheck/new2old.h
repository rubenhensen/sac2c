/*
 *
 * $Log$
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

extern node *NT2OTfundef (node *arg_node, node *arg_info);
extern node *NT2OTarg (node *arg_node, node *arg_info);
extern node *NT2OTvardec (node *arg_node, node *arg_info);
extern node *NT2OTarray (node *arg_node, node *arg_info);

#endif /* _new2old_h_ */
