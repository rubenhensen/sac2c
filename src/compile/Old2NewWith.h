/*
 * $Log$
 * Revision 2.2  2000/06/13 13:18:24  dkr
 * function for old with-loop removed
 * -> now, the only purpose of this module is to transform a N_Nwith node
 *    into a concrete and arbitrary form
 * -> this module should be renamed into PatchWith.[ch]
 *
 */

#ifndef _sac_Old2NewWith_h

#define _sac_Old2NewWith_h

extern node *Old2NewWith (node *syntaxtree);

extern node *O2Nnwith (node *arg_node, node *arg_info);
extern node *O2Nnpart (node *arg_node, node *arg_info);

#endif /* _sac_Old2NewWith_h */
