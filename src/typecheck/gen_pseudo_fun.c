/*
 * $Log$
 * Revision 2.2  2000/02/17 17:04:17  cg
 * Prototype of function DuplicateTypes() is now included from
 * DupTree.h instead of typecheck.h.
 *
 * Revision 2.1  1999/02/23 12:40:44  sacbase
 * new release made
 *
 * Revision 1.5  1999/01/07 10:50:49  cg
 * The module name is now encoded in the name of a fold-function.
 * This is necessary to avoid name clashes with fold-functions from
 * different modules although their actual module name is always '_FOLD'
 *
 * Revision 1.4  1998/12/03 09:40:28  sbs
 * DBUG_ASSERT inserted.
 *
 * Revision 1.3  1998/06/04 09:10:15  sbs
 * indenting adjusted
 *
 * Revision 1.2  1998/05/28 23:49:56  dkr
 * FUNDEF_STATUS of the pseudo-fun for a fold with-loop is set to
 * ST_foldfun
 *
 * Revision 1.1  1998/05/28 14:55:25  sbs
 * Initial revision
 *
 */

#include <stdlib.h>

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree.h"
#include "free.h"
#include "DupTree.h"
#include "internal_lib.h"
#include "gen_pseudo_fun.h"
#include "typecheck.h"

/******************************************************************************
 *
 * function:
 *   node *CreatePseudoFoldFun( types *elem_type,
 *                              char *fold_fun,
 *                              prf   fold_prf,
 *                              char *res_var,
 *                              char *body_expr)
 *
 * description:
 *  - generates an N_fundef node of the following kind:
 *
 *      <elem_type> _FOLD: _type_<n>_<mod>__<fold_fun>( <elem_type> <res_var>,
 *                                                      <elem_type> <body_expr>)
 *      {
 *        <res_var> = <fold_fun>( <res_var>, <body_expr>);
 *        return( <res_var>);
 *      }
 *    NOTE, that all(!) arguments are NOT inserted in the new AST!
 *    Instead, copied versions are used only!
 *    Hence, the calling function has to take care of the memory allocated
 *    for the arguments!
 *    NOTE as well, that fold_prf is valid  iff  (fold_fun == NULL)
 *
 *
 ******************************************************************************/

node *
CreatePseudoFoldFun (types *elem_type, char *fold_fun, prf fold_prf, char *res_var,
                     char *body_expr)
{
    char *pseudo_fold_fun, *buffer;
    node *new_fundef, *application, *args;

    DBUG_ENTER ("CreatePseudoFoldFun");

    args
      = MakeExprs (MakeId (StringCopy (res_var), NULL, ST_regular),
                   MakeExprs (MakeId (StringCopy (body_expr), NULL, ST_regular), NULL));

    if (fold_fun != NULL) {
        application = MakeAp (StringCopy (fold_fun), NULL, args);
        pseudo_fold_fun = fold_fun;
    } else {
        DBUG_ASSERT (((fold_prf >= F_toi) && (fold_prf <= F_genarray)),
                     "fold_prf is out of range!");
        application = MakePrf (fold_prf, args);
        pseudo_fold_fun = prf_name_str[fold_prf];
    }

    /*
     * Since the module name of the fold function is always '_FOLD', the
     * actual module name must be encoded within the name in order to guarantee
     * that no two fold functions from different modules may have the same
     * internal name.
     */
    buffer = (char *)Malloc (strlen (pseudo_fold_fun) + strlen (module_name) + 3);
    strcpy (buffer, module_name);
    strcat (buffer, "__");
    strcat (buffer, pseudo_fold_fun);
    pseudo_fold_fun = TmpVarName (buffer);
    FREE (buffer);

    new_fundef
      = MakeFundef (pseudo_fold_fun, PSEUDO_MOD_FOLD, DuplicateTypes (elem_type, 1),
                    MakeArg (StringCopy (res_var), DuplicateTypes (elem_type, 1),
                             ST_regular, ST_regular,
                             MakeArg (StringCopy (body_expr),
                                      DuplicateTypes (elem_type, 1), ST_regular,
                                      ST_regular, NULL)),
                    MakeBlock (MakeAssign (MakeLet (application,
                                                    MakeIds (StringCopy (res_var), NULL,
                                                             ST_regular)),
                                           MakeAssign (MakeReturn (
                                                         MakeExprs (MakeId (StringCopy (
                                                                              res_var),
                                                                            NULL,
                                                                            ST_regular),
                                                                    NULL)),
                                                       NULL)),
                               NULL),
                    NULL);
    FUNDEF_STATUS (new_fundef) = ST_foldfun;

    DBUG_RETURN (new_fundef);
}
