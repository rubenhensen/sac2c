/* *
 * $Log$
 * Revision 1.2  2003/02/09 22:32:19  mwe
 * removed bugs
 *
 * Revision 1.1  2003/02/08 16:08:16  mwe
 * Initial revision
 *
 *
 */

#ifndef _DistributiveLaw_h_
#define _DistributiveLaw_h_

extern node *DistributiveLaw (node *, node *);
extern node *DLblock (node *, node *);
extern node *DLassign (node *, node *);
extern node *DLlet (node *, node *);
extern node *DLPrfOrAp (node *, node *);

extern node *SearchTravElems (node *, node *);
extern bool CheckOperator (node *, node *);
extern bool DLIsConstant (node *);
extern bool DLReachedArgument (node *);
extern bool DLReachedDefinition (node *);
extern node *RegisterNode (node *, node *);
extern bool IsSupportedOperator (node *, node *);
extern bool IsValidSecondOperator (node *, node *);
extern node *GetUsedOperator (node *, node *);
extern node *SearchMostFrequentNode (node *, node *);
extern int GetPriority (node *);
extern bool IsThirdOperatorReached (node *, node *);
extern bool IsSameOperator (node *, node *);
extern nodelist *FindNodeInList (nodelist *, node *, node *);
extern node *FindAndSetMostFrequentNode (node *);
extern nodelist *DeleteNodelist (nodelist *);
extern node *CreateOptLists (node *, node *);
extern node *OptTravElems (node *, node *);
extern node *CheckNode (node *, node *);
extern bool IsIdenticalNode (node *, node *);
extern node *RemoveMostFrequentNode (node *);
extern node *ResetFlags (node *);
extern nodelist *CommitAssignNodes (nodelist *, node *);
extern node *IncludeMostFrequentNode (node *);
extern node *MakeAllNodelistnodesToAssignNodes (nodelist *, node *);
extern node *IntegrateResults (node *);
extern node *GetNeutralElement (node *);
extern bool ExistKnownNeutralElement (node *);
extern node *CreateAssignNodes (node *);
extern node *MakeExprsNodeFromAssignNode (node *);
extern node *MakeAssignLetNodeFromCurrentNode (node *, node *);
extern node *MakeExprNodes (node *);
extern node *MakeOperatorNode (node *, node *);
extern node *AddNodeToOptimizedNodes (node *, node *);
extern node *RegisterMultipleUsableNodes (node *);
extern node *GetOperator (node *);
extern bool OperatorIsSupported (node *);
extern node *AddToOptimizedNodes (node *, node *);

#endif
