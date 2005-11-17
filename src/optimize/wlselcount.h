/**<!--******************************************************************-->
 *
 * $Id$
 *
 * @file wlselcount.h
 *
 *
 **************************************************************************/

#ifndef _SAC_WLSELCOUNT_H_
#define _SAC_WLSELCOUNT_H_

#include "types.h"

extern node *WLSELCdoWithloopSelectionCount (node *fundef);
extern node *WLSELCfundef (node *arg_node, info *arg_info);
extern node *WLSELCwith (node *arg_node, info *arg_info);
extern node *WLSELCcode (node *arg_node, info *arg_info);
extern node *WLSELCprf (node *arg_node, info *arg_info);
extern node *WLSELCap (node *arg_node, info *arg_info);

#endif
