/*
 *
 * $Log$
 * Revision 1.3  1995/12/23 17:05:04  cg
 * removed old log messages from sib.h
 *
 * Revision 1.2  1995/12/23  17:02:41  cg
 * first running version for SIB grammar v0.5
 *
 * Revision 1.1  1995/12/21  16:17:49  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_writesib_h

#define _sac_writesib_h

extern node *WriteSib (node *syntax_tree);

extern node *WSIBmodul (node *arg_node, node *arg_info);
extern node *WSIBtypedef (node *arg_node, node *arg_info);
extern node *WSIBfundef (node *arg_node, node *arg_info);
extern node *WSIBexplist (node *arg_node, node *arg_info);

#endif /* _sac_writesib_h */
