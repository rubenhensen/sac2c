/*
 * $Log$
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

#ifndef _create_wrapper_code_h_
#define _create_wrapper_code_h_

node *CreateWrapperCode (node *ast);

node *CWCmodul (node *arg_node, info *arg_info);
node *CWCfundef (node *arg_node, info *arg_info);
node *CWCap (node *arg_node, info *arg_info);
node *CWCwith (node *arg_node, info *arg_info);
node *CWCwithop (node *arg_node, info *arg_info);

#endif /* _create_wrapper_code_h_ */
