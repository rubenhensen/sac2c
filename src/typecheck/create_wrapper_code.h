/*
 * $Log$
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

node *CWCmodul (node *arg_node, node *arg_info);
node *CWCfundef (node *arg_node, node *arg_info);

#endif /* _create_wrapper_code_h_ */
