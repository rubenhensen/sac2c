/*
 *
 * $Log$
 * Revision 1.4  1994/12/09 10:45:33  sbs
 * opt_tab inserted
 *
 * Revision 1.3  1994/12/01  17:41:09  hw
 * added extern funptr type_tab[];
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _sac_traverse_h

#define _sac_traverse_h

typedef node *(*funptr) (node *, node *);

extern node *Trav (node *arg_node, node *arg_info);

extern node *DummyFun (node *arg_node, node *arg_info);

extern funptr *act_tab;

extern funptr flat_tab[];

extern funptr prnt_tab[];

extern funptr type_tab[];

extern funptr opt_tab[];

#endif /* _sac_traverse_h */
