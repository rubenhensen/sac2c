/*
 *
 * $Log$
 * Revision 1.10  2004/12/05 16:45:38  sah
 * added SPIds SPId SPAp in frontend
 *
 * Revision 1.9  2004/11/26 23:45:18  khf
 * corrected
 *
 * Revision 1.8  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.7  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.6  2002/09/11 23:22:42  dkr
 * HMAdjustFunNames() removed
 *
 * Revision 1.5  2002/08/14 11:51:22  sbs
 * HMAdjustFunNames debugged....
 *
 * Revision 1.4  2002/08/13 17:14:34  sbs
 * HMfundef changed into HMAdjustFundef
 *
 * Revision 1.3  2002/08/13 16:34:08  sbs
 * HMfundef added.
 *
 * Revision 1.2  2002/08/13 14:41:00  sbs
 * HMNwithop added.
 * ./
 *
 * Revision 1.1  2002/08/13 10:22:40  sbs
 * Initial revision
 *
 */

#ifndef _SAC_HANDLE_MOPS_H_
#define _SAC_HANDLE_MOPS_H_

#include "types.h"

/******************************************************************************
 *
 * Handle mops traversal ( hm_tab)
 *
 * Prefix: HM
 *
 *****************************************************************************/
extern node *HMdoHandleMops (node *arg_node);

extern node *HMspmop (node *arg_node, info *arg_info);

#endif /* _SAC_HANDLE_MOPS_H_ */
