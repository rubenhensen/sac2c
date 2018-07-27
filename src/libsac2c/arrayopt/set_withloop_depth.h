/** <!--********************************************************************-->
 *
 * Template traversal ( swld)
 *
 * Prefix: SWLD
 *
 *****************************************************************************/

#ifndef _SAC_SET_WITHLOOP_DEPTH_H_
#define _SAC_SET_WITHLOOP_DEPTH_H_

#include "types.h"

extern node *SWLDdoSetWithloopDepth (node *arg_node);
extern node *SWLDfundef (node *arg_node, info *arg_info);
extern node *SWLDassign (node *arg_node, info *arg_info);
extern node *SWLDwith (node *arg_node, info *arg_info);
extern node *SWLDpart (node *arg_node, info *arg_info);
extern node *SWLDwithid (node *arg_node, info *arg_info);
extern node *SWLDids (node *arg_node, info *arg_info);
extern node *SWLDvardec (node *arg_node, info *arg_info);
extern node *SWLDarg (node *arg_node, info *arg_info);

#if 0
// Commented out due to a bug in it and lack of use in the compiler.
extern bool SWLDisDefinedInNextOuterBlock (node *avis, int wldepth);
#endif

extern bool SWLDisDefinedInThisBlock (node *avis, int wldepth);

#endif // _SAC_SET_WITHLOOP_DEPTH_H_
