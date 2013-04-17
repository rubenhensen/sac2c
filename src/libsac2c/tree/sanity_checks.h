#ifndef _SANITY_CHECKS_H_
#define _SANITY_CHECKS_H_

#include "types.h"

extern void SANCHKdoSanityChecksPreTraversal (node *arg_node, info *arg_info,
                                              void *travstack);

extern void SANCHKdoSanityChecksPostTraversal (node *arg_node, info *arg_info,
                                               void *travstack);

#endif /* _SANITY_CHECKS_H_ */
