/*
 * $Id:$
 *
 */

#ifndef _SAC_INSERT_DOMAIN_CONSTRAINTS_H_
#define _SAC_INSERT_DOMAIN_CONSTRAINTS_H_

#include "types.h"

extern node *IDCinit (node *fundef, bool all);
extern node *IDCaddUserConstraint (node *expr);
extern node *IDCaddTypeConstraint (ntype *type, node *avis);
extern node *IDCaddPrfConstraint (node *expr, int num_rets);
extern node *IDCinsertConstraints (node *fundef, bool all);
extern node *IDCfinalize (node *fundef, bool all);

#endif /* _SAC_INSERT_DOMAIN_CONSTRAINTS_H_ */
