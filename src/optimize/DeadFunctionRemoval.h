/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 3.2  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
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

#ifndef _SAC_DEADFUNCTIONREMOVAL_H_
#define _SAC_DEADFUNCTIONREMOVAL_H_

#include "types.h"

extern node *DFRdoDeadFunctionRemoval (node *arg_node);

extern node *DFRmodule (node *arg_node, info *arg_info);
extern node *DFRfundef (node *arg_node, info *arg_info);
extern node *DFRblock (node *arg_node, info *arg_info);
extern node *DFRap (node *arg_node, info *arg_info);
extern node *DFRfold (node *arg_node, info *arg_info);

#endif /* _DEADFUNCTIONREMOVAL_H_ */
