/*
 *
 * $Log$
 * Revision 1.4  1995/02/28 18:34:45  asi
 * added CFwith
 *
 * Revision 1.3  1995/02/22  18:16:34  asi
 * Fuctions CFfundef, CFassign, CFdo, CFwhile, CFcond, CFprf, CFid, CFlet
 * and PushVL, PushDupVL, PopVL for stack-management added
 *
 * Revision 1.2  1995/02/14  09:59:56  asi
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
extern node *CFfundef (node *arg_node, node *arg_info);
extern node *CFwhile (node *arg_node, node *arg_info);
extern node *CFdo (node *arg_node, node *arg_info);
extern node *CFcond (node *arg_node, node *arg_info);
extern node *CFvar (node *arg_node, node *arg_info);
extern node *CFassign (node *arg_node, node *arg_info);
extern node *CFwith (node *arg_node, node *arg_info);

#endif /* _ConstantFolding_h */
