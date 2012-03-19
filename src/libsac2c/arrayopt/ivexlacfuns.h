/*
 * $Id$
 */
#ifndef _SAC_IVEXLACFUNS_H_
#define _SAC_IVEXLACFUNS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Index vector extreme lacfuns support traversal
 *
 * Prefix: IVEXL
 *
 *****************************************************************************/
extern node *IVEXLfundef (node *arg_node, info *arg_info);
extern node *IVEXLavis (node *arg_node, info *arg_info);
extern node *IVEXLblock (node *arg_node, info *arg_info);
extern node *IVEXLassign (node *arg_node, info *arg_info);
extern node *IVEXLlet (node *arg_node, info *arg_info);
extern node *IVEXLids (node *arg_node, info *arg_info);
extern node *IVEXLid (node *arg_node, info *arg_info);
extern node *IVEXLwith (node *arg_node, info *arg_info);
extern node *IVEXLpart (node *arg_node, info *arg_info);
extern node *IVEXLcode (node *arg_node, info *arg_info);
extern node *IVEXLcond (node *arg_node, info *arg_info);
extern node *IVEXLfuncond (node *arg_node, info *arg_info);
extern node *IVEXLap (node *arg_node, info *arg_info);

#endif /* _SAC_IVEXLACFUNS_H_ */
