/*
 * $Log$
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

#ifndef _SAC_SSADEADCODEREMOVAL_H

#define _SAC_SSADEADCODEREMOVAL_H

extern node *SSADeadCodeRemoval (node *fundef);

extern node *SSADCRfundef (node *arg_node, node *arg_info);
extern node *SSADCRarg (node *arg_node, node *arg_info);
extern node *SSADCRblock (node *arg_node, node *arg_info);
extern node *SSADCRvardec (node *arg_node, node *arg_info);
extern node *SSADCRassign (node *arg_node, node *arg_info);
extern node *SSADCRlet (node *arg_node, node *arg_info);
extern node *SSADCRid (node *arg_node, node *arg_info);
extern node *SSADCRcond (node *arg_node, node *arg_info);
extern node *SSADCRreturn (node *arg_node, node *arg_info);
extern node *SSADCRap (node *arg_node, node *arg_info);
extern node *SSADCRNwith (node *arg_node, node *arg_info);
extern node *SSADCRNpart (node *arg_node, node *arg_info);
extern node *SSADCRNcode (node *arg_node, node *arg_info);
extern node *SSADCRNwithid (node *arg_node, node *arg_info);

#endif /* SAC_SSADEADCODEREMOVAL_H */
