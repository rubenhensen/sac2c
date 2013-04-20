#ifndef _SAC_PROPAGATE_EXTREMA_THRU_LACFUNS_H_
#define _SAC_PROPAGATE_EXTREMA_THRU_LACFUNS_H_

#include "types.h"

extern node *PETLdoPropagateExtremaThruLacfuns (node *arg_node);

extern node *PETLmodule (node *arg_node, info *arg_info);
extern node *PETLfundef (node *arg_node, info *arg_info);
extern node *PETLap (node *arg_node, info *arg_info);
extern node *PETLblock (node *arg_node, info *arg_info);
extern node *PETLcond (node *arg_node, info *arg_info);

extern node *PETLprefixFunctionArgument (node *arg_node, node *calleravis,
                                         node **callerapargs);
#endif /* _SAC_PROPAGATE_EXTREMA_THRU_LACFUNS_H_ */
