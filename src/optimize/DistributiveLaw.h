/* *
 * $Log$
 * Revision 1.3  2003/02/10 18:01:30  mwe
 * removed needles functions
 *
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

node *SearchTravElems (node *, node *);
bool CheckOperator (node *, node *);
bool DLIsConstant (node *, node *);
bool DLReachedArgument (node *);
bool DLReachedDefinition (node *);
node *RegisterNode (node *, node *);
bool IsSupportedOperator (node *, node *);
bool IsValidSecondOperator (node *, node *);
node *GetUsedOperator (node *, node *);
node *SearchMostFrequentNode (node *, node *);
int GetPriority (node *);
bool IsThirdOperatorReached (node *, node *);
bool IsSameOperator (node *, node *);
nodelist *FindNodeInList (nodelist *, node *, node *);
node *FindAndSetMostFrequentNode (node *);
nodelist *DeleteNodelist (nodelist *);
node *CreateOptLists (node *, node *);
node *OptTravElems (node *, node *);
node *CheckNode (node *, node *);
bool IsIdenticalNode (node *, node *);
node *RemoveMostFrequentNode (node *);
node *ResetFlags (node *);
nodelist *CommitAssignNodes (nodelist *, node *);
node *IncludeMostFrequentNode (node *);
node *MakeAllNodelistnodesToAssignNodes (nodelist *, node *);
node *IntegrateResults (node *);
node *GetNeutralElement (node *);
bool ExistKnownNeutralElement (node *);
node *CreateAssignNodes (node *);
node *MakeExprsNodeFromAssignNode (node *);
node *MakeAssignLetNodeFromCurrentNode (node *, node *);
node *MakeExprNodes (node *);
node *MakeOperatorNode (node *, node *);
node *AddNodeToOptimizedNodes (node *, node *);
node *RegisterMultipleUsableNodes (node *);
node *GetOperator (node *);
bool OperatorIsSupported (node *);
node *AddToOptimizedNodes (node *, node *);

#endif
