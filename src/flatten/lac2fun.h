/*
 *
 * $Log$
 * Revision 1.2  2000/02/03 17:29:40  dkr
 * conditions are lifted now correctly :)
 *
 * Revision 1.1  2000/01/21 12:49:01  dkr
 * Initial revision
 *
 */

#ifndef _sac_lac2fun_h
#define _sac_lac2fun_h

extern node *LaC2Fun (node *syntaxtree);

extern node *LAC2FUNfundef (node *arg_node, node *arg_info);
extern node *LAC2FUNassign (node *arg_node, node *arg_info);
extern node *LAC2FUNlet (node *arg_node, node *arg_info);
extern node *LAC2FUNid (node *arg_node, node *arg_info);
extern node *LAC2FUNcond (node *arg_node, node *arg_info);
extern node *LAC2FUNdo (node *arg_node, node *arg_info);
extern node *LAC2FUNwhile (node *arg_node, node *arg_info);

#endif /* _sac_lac2fun_h */
