/*
 *
 * $Log$
 * Revision 1.2  1996/01/02 07:57:55  cg
 * first working revision
 *
 * Revision 1.1  1995/12/29  17:19:26  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_cccall_h

#define _sac_cccall_h

extern void InvokeCC (node *modul);

extern node *PrepareLinking (node *syntax_tree);

extern node *LINKfundef (node *arg_node, node *arg_info);
extern node *LINKobjdef (node *arg_node, node *arg_info);
extern node *LINKmodul (node *arg_node, node *arg_info);

#endif /* _sac_cccall_h */
