/*
 * $Log$
 * Revision 1.10  2005/07/17 20:10:47  sbs
 * inserts bodies only; the splitted has been outsourced to split_wrappers
 *
 * Revision 1.9  2005/07/15 15:52:18  sah
 * splitted create_wrapper_code and dispatchfuncalls
 * introduced namespaces
 *
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

extern node *CWCdoCreateWrapperCode (node *ast);

extern bool CWChasWrapperCode (node *fundef);
extern node *CWCfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_WRAPPER_CODE_H_ */
