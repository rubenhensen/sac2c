/*
 *
 * $Log$
 * Revision 1.2  1995/02/14 09:59:56  asi
 * added CFid
 *
 * Revision 1.1  1995/02/13  16:55:36  asi
 * Initial revision
 *
 *
 */

#ifndef _ConstantFolding_h

#define _ConstantFolding_h

extern node *ConstantFolding (node *arg_node, node *arg_info);

extern node *CFlet (node *arg_node, node *arg_info);
extern node *CFprf (node *arg_node, node *arg_info);
extern node *CFid (node *arg_node, node *arg_info);

#endif /* _ConstantFolding_h */
