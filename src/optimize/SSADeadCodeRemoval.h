/*
 * $Log$
 * Revision 1.6  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 1.5  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.4  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.3  2001/03/23 09:30:33  nmw
 * SSADCRdo/while removed
 *
 * Revision 1.2  2001/02/27 16:05:35  nmw
 * SSADeadCodeRemoval for intraprocedural code implemented
 *
 * Revision 1.1  2001/02/23 13:37:50  nmw
 * Initial revision
 *

 */

#ifndef _SAC_SSADEADCODEREMOVAL_H_
#define _SAC_SSADEADCODEREMOVAL_H_

#include "types.h"

extern node *DCRdoDeadCodeRemoval (node *fundef, node *modul);

extern node *DCRfundef (node *arg_node, info *arg_info);
extern node *DCRarg (node *arg_node, info *arg_info);
extern node *DCRblock (node *arg_node, info *arg_info);
extern node *DCRvardec (node *arg_node, info *arg_info);
extern node *DCRassign (node *arg_node, info *arg_info);
extern node *DCRlet (node *arg_node, info *arg_info);
extern node *DCRid (node *arg_node, info *arg_info);
extern node *DCRcond (node *arg_node, info *arg_info);
extern node *DCRreturn (node *arg_node, info *arg_info);
extern node *DCRap (node *arg_node, info *arg_info);
extern node *DCRwith (node *arg_node, info *arg_info);
extern node *DCRpart (node *arg_node, info *arg_info);
extern node *DCRcode (node *arg_node, info *arg_info);
extern node *DCRwithid (node *arg_node, info *arg_info);

#endif /* SAC_SSADEADCODEREMOVAL_H_ */
