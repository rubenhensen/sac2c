#ifndef _SAC_TAGDEPENDENCIES_H_
#define _SAC_TAGDEPENDENCIES_H_

#include "types.h"

/******************************************************************************
 *
 * Tag dependencies traversal ( tdepend_tab)
 *
 * Prefix: TDEPEND
 *
 *****************************************************************************/
extern node *TDEPENDdoTagDependencies (node *with, node *fusionable_wl);

extern node *TDEPENDassign (node *arg_node, info *arg_info);
extern node *TDEPENDid (node *arg_node, info *arg_info);

extern node *TDEPENDwith (node *arg_node, info *arg_info);

#endif /* _SAC_TAGDEPENDENCIES_H_ */
