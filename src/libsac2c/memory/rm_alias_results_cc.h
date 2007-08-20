/*
 * $Id$
 */
#ifndef _SAC_RM_ALIAS_RESULTS_CC_H_
#define _SAC_RC_ALIAS_RESULTS_CC_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Remove Alias Results from Conformity Checks
 *
 * Prefix: EMRACC
 *
 *****************************************************************************/
extern node *EMRACCdoRemoveAliasResultsFromConformityChecks (node *syntax_tree);

extern node *EMRACCfundef (node *arg_node, info *arg_info);
extern node *EMRACCarg (node *arg_node, info *arg_info);
extern node *EMRACCblock (node *arg_node, info *arg_info);
extern node *EMRACCvardec (node *arg_node, info *arg_info);
extern node *EMRACCassign (node *arg_node, info *arg_info);
extern node *EMRACClet (node *arg_node, info *arg_info);
extern node *EMRACCprf (node *arg_node, info *arg_info);
extern node *EMRACCid (node *arg_node, info *arg_info);

#endif /* _SAC_RM_ALIAS_RESULTS_CC_H_ */
