/*
 * $Id$
 */

#ifndef _SAC_SPMD_TRAV_H_
#define _SAC_SPMD_TRAV_H_

#include "types.h"

/******************************************************************************
 *
 * SPMD delete nested traversal( spmddn_tab)
 *
 * Prefix: SPMDDN
 *
 *****************************************************************************/
extern node *SPMDDNdoDeleteNested (node *arg_node);
extern node *SPMDDNspmd (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * SPMD produce masks traversal( spmdpm_tab)
 *
 * Prefix: SPMDPM
 *
 *****************************************************************************/
extern void SPMDPMdoProduceMasks (node *arg_node, node *spmd, node *fundef);
extern node *SPMDPMassign (node *arg_node, info *arg_info);

#endif /* _SAC_SPMD_TRAV_H_ */
