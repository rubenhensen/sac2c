/*
 *
 * $Log$
 * Revision 1.2  1995/12/23 17:02:41  cg
 * first running version for SIB grammar v0.5
 *
 * Revision 1.1  1995/12/21  16:17:49  cg
 * Initial revision
 *
 * Revision 1.3  1995/10/30  10:22:01  cg
 * added declaration of function SIBfundef.
 *
 * Revision 1.2  1995/10/22  17:38:26  cg
 * Totally modified revision:
 * A lot of code moved to checkdec.c
 * Making a new start with writing SIBs in the context of new
 * compiler modules such as analysis or checkdec.
 *
 * Revision 1.1  1995/08/31  08:37:48  cg
 * Initial revision
 *
 *
 */

#ifndef _sac_sib_h

#define _sac_sib_h

extern node *WriteSib (node *syntax_tree);

extern node *WSIBmodul (node *arg_node, node *arg_info);
extern node *WSIBtypedef (node *arg_node, node *arg_info);
extern node *WSIBfundef (node *arg_node, node *arg_info);
extern node *WSIBexplist (node *arg_node, node *arg_info);

#endif /* _sac_sib_h */
