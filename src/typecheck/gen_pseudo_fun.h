/*
 *
 * $Log$
 * Revision 3.2  2002/10/08 10:39:47  dkr
 * function CreateFoldFun() for new type system added
 *
 * Revision 3.1  2000/11/20 18:00:07  sacbase
 * new release made
 *
 * Revision 2.1  1999/02/23 12:40:46  sacbase
 * new release made
 *
 * Revision 1.1  1998/05/28 14:55:40  sbs
 * Initial revision
 *
 */

#ifndef _gen_pseudo_fun_h_
#define _gen_pseudo_fun_h_

#define PSEUDO_MOD_FOLD "_FOLD"

extern /* for old typechecker */
  node *
  CreatePseudoFoldFun (types *elem_type, char *fold_fun, prf fold_prf, char *res_var,
                       char *body_expr);

extern /* for new typechecker */
  node *
  CreateFoldFun (types *elem_type, node *fold_fundef, prf fold_prf, char *res_name,
                 char *cexpr_name);

#endif /* _gen_pseudo_fun_h_ */
