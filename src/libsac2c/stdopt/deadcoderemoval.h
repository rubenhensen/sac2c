/*
 * $Log$
 * Revision 1.3  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 1.2  2005/08/02 14:24:37  ktr
 * a seperate dead code inference traversal is now employed
 *
 * Revision 1.1  2005/07/19 16:56:53  ktr
 * Initial revision
 *
 */
#ifndef _SAC_DEADCODEREMOVAL_H_
#define _SAC_DEADCODEREMOVAL_H_

#include "types.h"

/******************************************************************************
 *
 * Dead code removal traversal (dcr_tab)
 *
 * prefix: DCR
 *
 *****************************************************************************/
extern node *DCRdoDeadCodeRemovalOneFundef (node *fundef);
extern node *DCRdoDeadCodeRemovalModule (node *module);

extern node *DCRfundef (node *arg_node, info *arg_info);
extern node *DCRarg (node *arg_node, info *arg_info);
extern node *DCRret (node *arg_node, info *arg_info);
extern node *DCRblock (node *arg_node, info *arg_info);
extern node *DCRvardec (node *arg_node, info *arg_info);
extern node *DCRassign (node *arg_node, info *arg_info);
extern node *DCRannotate (node *arg_node, info *arg_info);
extern node *DCRlet (node *arg_node, info *arg_info);
extern node *DCRids (node *arg_node, info *arg_info);
extern node *DCRcond (node *arg_node, info *arg_info);
extern node *DCRreturn (node *arg_node, info *arg_info);
extern node *DCRap (node *arg_node, info *arg_info);
extern node *DCRcode (node *arg_node, info *arg_info);

#endif /* SAC_DEADCODEREMOVAL_H_ */
