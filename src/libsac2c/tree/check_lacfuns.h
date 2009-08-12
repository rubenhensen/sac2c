/*
 * $Id$
 */

#ifndef _SAC_CHECK_LACFUNS_H_
#define _SAC_CHECK_LACFUNS_H_

#include "types.h"

extern node *CHKLACFdoCheckLacFuns (node *arg_node);

extern node *CHKLACFmodule (node *arg_node, info *arg_info);
extern node *CHKLACFfundef (node *arg_node, info *arg_info);
extern node *CHKLACFblock (node *arg_node, info *arg_info);
extern node *CHKLACFap (node *arg_node, info *arg_info);

#endif /* _CHECK_LACFUNS_H_ */
