/*
 * $Log$
 * Revision 3.2  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 18:01:02  sacbase
 * new release made
 *
 * Revision 1.1  2000/06/13 13:49:50  dkr
 * Initial revision
 *
 */

#ifndef _sac_PatchWith_h

#define _sac_PatchWith_h

extern node *PatchWith (node *syntaxtree);

extern node *PWwith (node *arg_node, info *arg_info);
extern node *PWpart (node *arg_node, info *arg_info);

#endif /* _sac_PatchWith_h */
