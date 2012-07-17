/*
 *
 * $Log$
 * Revision 1.1  2005/02/02 18:13:34  rbe
 * Initial revision
 *
 *
 */

#ifndef _SAC_LACFUN_UTILITIES_H_
#define _SAC_LACFUN_UTILITIES_H_

#include "types.h"

extern node *LFUprefixFunctionArgument (node *arg_node, node *calleravis,
                                        node **callerapargs);
extern bool LFUisLoopFunInvariant (node *arg_node, node *argid, node *rca);
extern node *LFUgetCallArg (node *id, node *fundef, node *ext_assign);
extern node *LFUgetLoopVariable (node *var, node *fundef, node *params);
extern node *LFUfindAssignBeforeCond (node *arg_node);
extern node *LFUfindAssignOfType (node *assigns, nodetype n);
extern bool LFUisLURPredicate (node *arg_node);
extern bool LFUisAvisMemberIds (node *arg_node, node *ids);

#endif /* _SAC_LACFUN_UTILITIES_H_ */
