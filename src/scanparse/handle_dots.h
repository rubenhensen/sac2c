/*
 *
 * $Log$
 * Revision 1.7  2003/12/10 16:00:15  sah
 * added HDid
 *
 * Revision 1.6  2003/11/12 14:32:30  sbs
 * HDpart inserted.
 *
 * Revision 1.5  2002/09/06 11:45:32  sah
 * added support for N_selwl.
 *
 * Revision 1.4  2002/09/05 12:48:21  sah
 * added HDassign
 *
 * Revision 1.3  2002/08/13 09:54:52  sah
 * added several traversalfunctions.
 *
 * Revision 1.2  2002/07/19 13:24:49  sah
 * added functions for traversal.
 *
 * Revision 1.1  2002/07/09 12:54:26  sbs
 * Initial revision
 *
 *
 */

#ifndef _handle_dots_h
#define _handle_dots_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"

extern node *EliminateSelDots (node *arg_node);
extern node *HDwith (node *arg_node, node *arg_info);
extern node *HDwithop (node *arg_node, node *arg_info);
extern node *HDpart (node *arg_node, node *arg_info);
extern node *HDgenerator (node *arg_node, node *arg_info);
extern node *HDdot (node *arg_node, node *arg_info);
extern node *HDap (node *arg_node, node *arg_info);
extern node *HDprf (node *arg_node, node *arg_info);
extern node *HDassign (node *arg_node, node *arg_info);
extern node *HDsetwl (node *arg_node, node *arg_info);
extern node *HDid (node *arg_node, node *arg_info);

#endif /* _handle_dots_h */
