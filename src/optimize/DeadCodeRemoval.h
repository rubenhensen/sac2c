/*
 *
 * $Log$
 * Revision 1.5  1996/09/11 14:13:08  asi
 * DFRmodul added
 *
 * Revision 1.4  1996/08/09  16:42:52  asi
 * dead function removal added
 *
 * Revision 1.3  1995/12/21  13:27:48  asi
 * New algorithm implemented
 *
 * Revision 1.2  1995/03/07  10:23:59  asi
 * added DEADwith
 *
 * Revision 1.1  1995/02/13  16:38:56  asi
 * Initial revision
 *
 *
 */

#ifndef _DeadCodeRemoval_h

#define _DeadCodeRemoval_h

extern node *DeadCodeRemoval (node *arg_node, node *arg_info);

extern node *DFRmodul (node *arg_node, node *arg_info);
extern node *ACTfundef (node *arg_node, node *arg_info);
extern node *DCRfundef (node *arg_node, node *arg_info);
extern node *DFRfundef (node *arg_node, node *arg_info);
extern node *DFRap (node *arg_node, node *arg_info);
extern node *ACTassign (node *arg_node, node *arg_info);
extern node *DCRassign (node *arg_node, node *arg_info);
extern node *DCRvardec (node *arg_node, node *arg_info);
extern node *ACTcond (node *arg_node, node *arg_info);
extern node *DCRcond (node *arg_node, node *arg_info);
extern node *ACTdo (node *arg_node, node *arg_info);
extern node *DCRdo (node *arg_node, node *arg_info);
extern node *ACTwhile (node *arg_node, node *arg_info);
extern node *DCRwhile (node *arg_node, node *arg_info);
extern node *ACTwith (node *arg_node, node *arg_info);
extern node *DCRwith (node *arg_node, node *arg_info);

#endif /* _DeadCodeRemoval_h */
