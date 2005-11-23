/*
 *
 * $Id$
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
extern node *NT2OTarray (node *arg_node, info *arg_info);
extern node *NT2OTlet (node *arg_node, info *arg_info);
extern node *NT2OTassign (node *arg_node, info *arg_info);
extern node *NT2OTpart (node *arg_node, info *arg_info);
extern node *NT2OTwithid (node *arg_node, info *arg_info);
extern node *NT2OTcond (node *arg_node, info *arg_info);
extern node *NT2OTfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_NEW2OLD_H_ */
