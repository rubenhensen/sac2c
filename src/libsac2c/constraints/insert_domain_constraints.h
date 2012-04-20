/*
 * $Id$
 *
 */

#ifndef _SAC_INSERT_DOMAIN_CONSTRAINTS_H_
#define _SAC_INSERT_DOMAIN_CONSTRAINTS_H_

#include "types.h"

extern node *IDCinitialize (node *fundef, bool all);
extern node *IDCaddTypeConstraint (ntype *type, node *avis);
extern node *IDCaddFunConstraint (node *expr);
extern node *IDCinsertConstraints (node *fundef, bool all);
extern node *IDCfinalize (node *fundef, bool all);

extern node *IDCfundef (node *arg_node, info *arg_info);
extern node *IDCblock (node *arg_node, info *arg_info);
extern node *IDCassign (node *arg_node, info *arg_info);
extern node *IDClet (node *arg_node, info *arg_info);
extern node *IDCids (node *arg_node, info *arg_info);
extern node *IDCcond (node *arg_node, info *arg_info);
extern node *IDCwith (node *arg_node, info *arg_info);
extern node *IDCpart (node *arg_node, info *arg_info);
extern node *IDCcode (node *arg_node, info *arg_info);
extern node *IDCid (node *arg_node, info *arg_info);
extern node *IDCavis (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_DOMAIN_CONSTRAINTS_H_ */
