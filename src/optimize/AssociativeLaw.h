#ifndef _AssociativeLaw_h_
#define _AssociativeLaw_h_

extern node *AssociativeLaw (node *arg_node, node *);
extern node *ALblock (node *, node *);
extern node *ALassign (node *, node *);
extern node *ALinstr (node *, node *);
extern node *ALprf (node *, node *);
extern node *AssociativeLawOptimize (node *, node *);
extern node *TravElems (node *, node *);
extern int ContainOptInformation (node *);
extern int IsAssociativeAndCommutative (node *);
extern int IsConstant (node *);
extern node *AddNode (node *, node *, bool);
extern bool OtherPrfOp (node *, node *);
extern int CountConst (node *);
extern int CountAll (node *);
extern node *SortList (node *);
extern node *CreateNAssignNodes (node *);
extern node *CommitNAssignNodes (node *);

#endif
