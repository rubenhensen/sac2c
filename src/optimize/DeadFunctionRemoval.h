/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:00:30  sacbase
 * new release made
 *
 * Revision 2.3  2000/10/31 18:08:09  cg
 * Dead function removal completely re-implemented.
 *
 * Revision 2.2  2000/07/14 11:51:18  dkr
 * DFRblock added
 *
 * Revision 2.1  1999/02/23 12:41:15  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/07 17:37:03  sbs
 * Initial revision
 *
 */

#ifndef _sac_DeadFunctionRemoval_h
#define _sac_DeadFunctionRemoval_h

extern node *DeadFunctionRemoval (node *arg_node, node *info_node);

extern node *DFRmodul (node *arg_node, node *arg_info);
extern node *DFRfundef (node *arg_node, node *arg_info);
extern node *DFRblock (node *arg_node, node *arg_info);
extern node *DFRap (node *arg_node, node *arg_info);
extern node *DFRwithop (node *arg_node, node *arg_info);

#endif /* _DeadFunctionRemoval_h */
