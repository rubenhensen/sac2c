/*
 *
 * $Log$
 * Revision 1.3  2004/11/21 20:42:14  ktr
 * Ismop
 *
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/21 16:18:25  ktr
 * Initial revision
 *
 */
#ifndef _SAC_STATICREUSE_H_
#define _SAC_STATICREUSE_H_

#include "types.h"

/******************************************************************************
 *
 * Static reuse traversal ( emsr_tab)
 *
 * Prefix: EMSR
 *
 *****************************************************************************/
extern node *EMSRdoStaticReuse (node *syntax_tree);

extern node *EMSRprf (node *arg_node, info *arg_info);

#endif /* _SAC_STATICREUSE_H_ */
