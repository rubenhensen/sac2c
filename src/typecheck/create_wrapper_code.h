/*
 * $Log$
 * Revision 1.8  2004/11/25 18:01:40  sbs
 * compiles
 *
 * Revision 1.7  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
 * Revision 1.6  2004/08/26 18:12:21  sbs
 * node *CWCwith( node *arg_node, info *arg_info);
 * added
 *
 * Revision 1.5  2004/07/30 17:29:21  sbs
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.4  2002/09/05 13:26:17  dkr
 * CWCwithop() added
 *
 * Revision 1.3  2002/08/09 14:50:46  dkr
 * CWCap added
 *
 * Revision 1.2  2002/08/09 13:15:29  dkr
 * CWCmodul, CWCfundef added
 *
 * Revision 1.1  2002/08/09 13:00:03  dkr
 * Initial revision
 *
 */

#ifndef _SAC_CREATE_WRAPPER_CODE_H_
#define _SAC_CREATE_WRAPPER_CODE_H_

#include "types.h"

node *CWCdoCreateWrapperCode (node *ast);

node *CWCmodule (node *arg_node, info *arg_info);
node *CWCfundef (node *arg_node, info *arg_info);
node *CWCap (node *arg_node, info *arg_info);
node *CWCwith (node *arg_node, info *arg_info);
node *CWCgenarray (node *arg_node, info *arg_info);
node *CWCfold (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_WRAPPER_CODE_H_ */
