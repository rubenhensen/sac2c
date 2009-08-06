/*****************************************************************************
 *
 * $Id$
 *
 *****************************************************************************/

#ifndef _SAC_MTDCI_H_
#define _SAC_MTDCI_H_

#include "types.h"

extern node *MTDCIfundef (node *arg_node, info *arg_info);
extern node *MTDCIblock (node *arg_node, info *arg_info);
extern node *MTDCIlet (node *arg_node, info *arg_info);
extern node *MTDCIprf (node *arg_node, info *arg_info);
extern node *MTDCIids (node *arg_node, info *arg_info);
extern node *MTDCIid (node *arg_node, info *arg_info);

extern node *MTDCIdoMtDeadCodeInference (node *fundef);

#endif /* _SAC_MTDCI_H_ */
