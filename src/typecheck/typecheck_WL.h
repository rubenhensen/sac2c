/*      $Id$
 *
 * $Log$
 * Revision 1.2  1998/04/29 08:51:56  srs
 * *** empty log message ***
 *
 * Revision 1.1  1998/04/28 15:48:01  srs
 * Initial revision
 *
 */
#ifndef _typecheck_WL_h

#define _typecheck_WL_h

extern node *TCWLnull (node *arg_node, node *arg_info);
extern node *TCWLarray (node *arg_node, node *arg_info);
extern node *TCWLprf (node *arg_node, node *arg_info);

extern node *ReduceGenarrayShape (node *arg_node, types *expr_type);

#endif
