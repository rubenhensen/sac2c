/*
 *
 * $Log$
 * Revision 1.15  2005/08/29 11:25:21  ktr
 * NTC may now run in the optimization cycle
 *
 * Revision 1.14  2005/07/26 14:32:08  sah
 * moved creation of special fold funs to
 * dispatchfuncall as new2old is running
 * prior to the module system which again relies
 * on the fact that no foldfuns have been
 * created, yet.
 *
 * Revision 1.13  2005/07/26 12:43:21  sah
 * new2old no longer removes casts
 *
 * Revision 1.12  2005/06/14 09:55:10  sbs
 * support for bottom types integrated.
 *
 * Revision 1.11  2005/03/20 00:22:02  sbs
 * NT2OTpart added.
 *
 * Revision 1.10  2004/12/08 18:00:11  ktr
 * removed ARRAY_TYPE/ARRAY_NTYPE
 *
 * Revision 1.9  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.8  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
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

#ifndef _SAC_NEW2OLD_H_
#define _SAC_NEW2OLD_H_

#include "types.h"

extern node *NT2OTdoTransform (node *arg_node);
extern node *NT2OTdoTransformOneFunction (node *arg_node);

extern node *NT2OTmodule (node *arg_node, info *arg_info);
extern node *NT2OTfundef (node *arg_node, info *arg_info);
extern node *NT2OTap (node *arg_node, info *arg_info);
extern node *NT2OTavis (node *arg_node, info *arg_info);
extern node *NT2OTblock (node *arg_node, info *arg_info);
extern node *NT2OTvardec (node *arg_node, info *arg_info);
extern node *NT2OTlet (node *arg_node, info *arg_info);
extern node *NT2OTassign (node *arg_node, info *arg_info);
extern node *NT2OTpart (node *arg_node, info *arg_info);
extern node *NT2OTwithid (node *arg_node, info *arg_info);
extern node *NT2OTcond (node *arg_node, info *arg_info);
extern node *NT2OTfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_NEW2OLD_H_ */
