#ifndef _SAC_DETECTDEPENDENCIES_H_
#define _SAC_DETECTDEPENDENCIES_H_

#include "types.h"

/******************************************************************************
 *
 * Detect dependencies traversal ( ddepend_tab)
 *
 * Prefix: DDEPEND
 *
 *****************************************************************************/
extern node *DDEPENDdoDetectDependencies (node *with, node *fusionable_wl,
                                          nodelist *references_fwl);

extern node *DDEPENDassign (node *arg_node, info *arg_info);
extern node *DDEPENDprf (node *arg_node, info *arg_info);
extern node *DDEPENDid (node *arg_node, info *arg_info);

extern node *DDEPENDwith (node *arg_node, info *arg_info);
extern node *DDEPENDcode (node *arg_node, info *arg_info);

#endif /* _SAC_DETECTDEPENDENCIES_H_ */
