/*
 *
 * $Log$
 * Revision 1.2  1995/10/22 17:38:26  cg
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

#ifndef _sib_h

#define _sib_h

extern node *WriteSib (node *syntax_tree);

extern node *SIBmodul (node *arg_node, node *arg_info);
extern node *SIBtypedef (node *arg_node, node *arg_info);

#endif /* _sib_h */
