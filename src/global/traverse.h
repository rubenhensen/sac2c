#ifndef _sac_traverse_h

#define _sac_traverse_h

typedef node *(*funptr) (node *, node *);

extern node *Trav (node *arg_node, node *arg_info);

extern node *DummyFun (node *arg_node, node *arg_info);

extern funptr *act_tab;

extern funptr flat_tab[];

extern funptr prnt_tab[];

#endif /* _sac_traverse_h */
