/*
 *
 * $Log$
 * Revision 3.4  2004/11/23 21:27:36  skt
 * kill the n of nwith and nwithop
 *
 * Revision 3.3  2004/11/21 23:01:01  ktr
 * ISMOP 2004!!!!!!!
 *
 * Revision 3.2  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.1  2000/11/20 18:02:31  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:44:15  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_SPMD_LIFT_H_
#define _SAC_SPMD_LIFT_H_

#include "types.h"

/*****************************************************************************
 *
 * SPMD lift ( spmdlift_tab)
 *
 * prefix: SPMDL
 *
 *****************************************************************************/
extern node *SPMDLspmd (node *arg_node, info *arg_info);
extern node *SPMDLid (node *arg_node, info *arg_info);
extern node *SPMDLlet (node *arg_node, info *arg_info);
extern node *SPMDLwith2 (node *arg_node, info *arg_info);
extern node *SPMDLwithid (node *arg_node, info *arg_info);
extern node *SPMDLids (node *arg_node, info *arg_info);

#endif /* _SAC_SPMD_LIFT_H_ */
