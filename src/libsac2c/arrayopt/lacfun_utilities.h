#ifndef _SAC_LACFUN_UTILITIES_H_
#define _SAC_LACFUN_UTILITIES_H_

#include "types.h"

extern node *LFUprefixFunctionArgument (node *arg_node, node *calleravis,
                                        node **callerapargs);
// extern node *LFUgetRecursiveCallVariableFromArgs( node *var,
// dead              node *fundef, node *reccallargs);
extern node *LFUgetRecursiveCallVariableFromArg (node *arg, node *fundef);
extern node *LFUgetCallerVariableFromArg (node *var, node *fundef);
extern node *LFUgetArgFromRecursiveCallVariable (node *rcv, node *fundef);
extern node *LFUfindRecursiveCallAssign (node *arg_node);
extern node *LFUgetStrideForAffineFun (node *rcv, node *lcv);
extern int LFUgetMathSignumForAffineFun (node *rcv, node *lcv);
extern node *LFUfindLacfunConditional (node *arg_node);
extern node *LFUfindAssignForCond (node *arg_node);
extern node *LFUfindAssignBeforeCond (node *arg_node);
extern node *LFUfindAssignOfType (node *assigns, nodetype n);
extern bool LFUisLURPredicate (node *arg_node);
extern bool LFUisAvisMemberIds (node *arg_node, node *ids);
extern bool LFUisAvisMemberArg (node *arg_node, node *arg);
extern int LFUindexOfMemberIds (node *arg_node, node *ids);
extern bool LFUisAvisMemberExprs (node *arg_node, node *exprs);
extern node *LFUinsertAssignIntoLacfun (node *arg_node, node *assign, node *oldavis);
extern bool LFUisLoopfunInvariant (node *arg, node *fundef);
extern node *LFUfindFundefReturn (node *arg_node);
extern node *LFUarg2Vardec (node *arg_node, node *lacfundef);
extern node *LFUscalarizeArray (node *avis, node **preassigns, node **vardecs,
                                shape *shp);
extern node *LFUcorrectSSAAssigns (node *arg_node, node *nassgn);
extern node *LFUfindAffineFunctionForLIV (node *arg_node, node *lacfundef);
extern node *LFUgetLoopIncrementFromCondprf (node *arg_node, node *rca);
extern node *LFUgetLoopIncrementFromIslChain (node *rca, node *islchain);
extern prf LFUdualFun (prf nprf);
extern node *LFUgetStrideInfo (node *expn, node *lcv, int *stridesgn);

#endif /* _SAC_LACFUN_UTILITIES_H_ */
