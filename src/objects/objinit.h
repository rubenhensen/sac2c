/*
 *
 * $Log$
 * Revision 3.2  2004/07/17 14:30:09  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 18:02:01  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:43:24  sacbase
 * new release made
 *
 * Revision 1.2  1995/10/31 17:19:07  cg
 * Now, multiple includes of this header file are avoided.
 *
 * Revision 1.1  1995/10/16  12:22:44  cg
 * Initial revision
 *
 *
 *
 */

#ifndef _sac_objinit_h
#define _sac_objinit_h

extern node *OImodul (node *arg_node, info *arg_info);
extern node *OIobjdef (node *arg_node, info *arg_info);

extern node *objinit (node *syntax_tree);

#endif /* _sac_objinit_h  */
