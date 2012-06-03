/*
 *
 * $Log$
 * Revision 1.1  2005/02/02 18:13:34  rbe
 * Initial revision
 *
 *
 */

#ifndef _SAC_PROPAGATE_EXTREMA_THRU_LACFUNS_H_
#define _SAC_PROPAGATE_EXTREMA_THRU_LACFUNS_H_

#include "types.h"

node *PETLdoPropagateExtremaThruLacfuns (node *arg_node);

node *PETLmodule (node *arg_node, info *arg_info);
node *PETLfundef (node *arg_node, info *arg_info);
node *PETLap (node *arg_node, info *arg_info);
node *PETLblock (node *arg_node, info *arg_info);
node *PETLcond (node *arg_node, info *arg_info);
node *PETLfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_PROPAGATE_EXTREMA_THRU_LACFUNS_H_ */
