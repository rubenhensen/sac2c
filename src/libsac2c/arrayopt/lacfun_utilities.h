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
extern int LFUindexOfMemberIds (node *arg_node, node *ids);
extern node *LFUinsertAssignIntoLacfun (node *arg_node, node *assign, node *oldavis);
extern node *LFUfindRecursiveCallAssign (node *arg_node);
extern node *LFUfindFundefReturn (node *arg_node);
extern node *LFUarg2Vardec (node *arg_node, node *lacfundef);
extern node *LFUscalarizeArray (node *avis, node **preassigns, node **vardecs,
                                shape *shp);
extern node *LFUcorrectSSAAssigns (node *arg_node, node *nassgn);
extern node *LFUfindAffineFunctionForLIV (node *arg_node, node *lacfundef);
extern node *LFUgetLoopIncrement (node *arg_node, node *rca);
extern prf LFUdualFun (prf nprf);

#endif /* _SAC_LACFUN_UTILITIES_H_ */
