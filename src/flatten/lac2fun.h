/*
 *
 * $Log$
 * Revision 3.3  2001/02/28 15:44:16  nmw
 * L2Fassign included in traversal to set correct FUNDEF_EXT_ASSIGN
 *
 * Revision 3.2  2000/12/06 19:58:08  dkr
 * inference parts are moved into a new module infer_dfms.[ch]
 *
 * Revision 3.1  2000/11/20 17:59:23  sacbase
 * new release made
 *
 * Revision 1.8  2000/03/23 17:38:05  dkr
 * L2F_INFERarg and L2F_INFERap added
 *
 * Revision 1.7  2000/02/23 23:07:40  dkr
 * some new functions added
 *
 * Revision 1.6  2000/02/09 14:54:08  dkr
 * LAC2FUNcode added
 *
 * Revision 1.5  2000/02/08 16:40:16  dkr
 * LAC2FUNwith() and LAC2FUNwith2() added
 *
 * Revision 1.3  2000/02/08 14:43:51  dkr
 * LAC2FUNwithid() added
 *
 * Revision 1.2  2000/02/03 17:29:40  dkr
 * conditions are lifted now correctly :)
 *
 * Revision 1.1  2000/01/21 12:49:01  dkr
 * Initial revision
 *
 */

#ifndef _sac_lac2fun_h
#define _sac_lac2fun_h

extern node *Lac2Fun (node *syntaxtree);

extern node *L2Ffundef (node *arg_node, node *arg_info);
extern node *L2Fcond (node *arg_node, node *arg_info);
extern node *L2Fwhile (node *arg_node, node *arg_info);
extern node *L2Fdo (node *arg_node, node *arg_info);
extern node *L2Fassign (node *arg_node, node *arg_info);

#endif /* _sac_lac2fun_h */
