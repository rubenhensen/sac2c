/*
 *
 * $Log$
 * Revision 3.4  2004/11/25 18:01:40  sbs
 * compiles
 *
 * Revision 3.3  2004/11/22 15:36:00  sbs
 * SacDevCamp04
 *
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

#ifndef _SAC_GEN_PSEUDO_FUN_H_
#define _SAC_GEN_PSEUDO_FUN_H_

#include "types.h"

#define PSEUDO_MOD_FOLD "_FOLD"

extern node *GPFcreateFoldFun (ntype *elem_type, node *fold_fundef, prf fold_prf,
                               char *res_name, char *cexpr_name);

#endif /* _SAC_GEN_PSEUDO_FUN_H_ */
