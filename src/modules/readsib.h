/*
 *
 * $Log$
 * Revision 1.2  1995/12/29 10:41:52  cg
 * first running revision for new SIBs
 *
 * Revision 1.1  1995/12/23  17:28:39  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_readsib_h

#define _sac_readsib_h

extern node *ReadSib (node *);

extern node *RSIBmodul (node *arg_node, node *arg_info);
extern node *RSIBtypedef (node *arg_node, node *arg_info);
extern node *RSIBfundef (node *arg_node, node *arg_info);

#endif /* _sac_readsib_h  */
