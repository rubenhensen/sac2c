/*
 *
 * $Log$
 * Revision 3.7  2002/09/09 19:38:31  dkr
 * print.h included for prf_name_string
 *
 * Revision 3.6  2002/09/09 19:35:40  dkr
 * prf_name_str renamed into prf_name_string
 *
 * Revision 3.5  2002/02/20 14:44:34  dkr
 * function DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.4  2001/05/17 11:34:07  sbs
 * return value of Free now used ...
 *
 * Revision 3.3  2001/05/17 09:20:42  sbs
 * MALLOC FREE aliminated
 *
 * Revision 3.2  2001/03/22 20:39:28  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:00:06  sacbase
 * new release made
 *
 * Revision 2.5  2000/10/12 15:45:11  dkr
 * macros in prf.h used
 *
 * Revision 2.4  2000/10/09 19:17:52  dkr
 * CreatePseudoFoldFun():
 *   tmp-var used in body of fold-fun
 *
 * Revision 2.3  2000/07/12 15:13:33  dkr
 * function DuplicateTypes renamed into DupTypes
 *
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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "prf.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "gen_pseudo_fun.h"
#include "typecheck.h"

/******************************************************************************
 *
 * function:
 *   node *CreatePseudoFoldFun( types *elem_type,
 *                              char *fold_fun,
 *                              prf fold_prf,
 *                              char *res_var,
 *                              char *body_expr)
 *
 * description:
 *  - generates an N_fundef node of the following kind:
 *
 *      <elem_type> _FOLD: _type_<n>_<mod>__<fold_fun>( <elem_type> <res_var>,
 *                                                      <elem_type> <body_expr>)
 *      {
 *        tmp__<res_var> = <fold_fun>( <res_var>, <body_expr>)
 *        <res_var> = tmp__<res_var>;
 *        return( <res_var>);
 *      }
 *    NOTE, that all(!) arguments are NOT inserted in the new AST!
 *    Instead, copied versions are used only!
 *    Hence, the calling function has to take care of the memory allocated
 *    for the arguments!
 *    NOTE as well, that fold_prf is valid  iff  (fold_fun == NULL)
 *
 ******************************************************************************/

node *
CreatePseudoFoldFun (types *elem_type, char *fold_fun, prf fold_prf, char *res_var,
                     char *body_expr)
{
    char *pseudo_fold_fun, *buffer, *tmp_res_var;
    node *new_fundef, *application, *args;

    DBUG_ENTER ("CreatePseudoFoldFun");

    args
      = MakeExprs (MakeId (StringCopy (res_var), NULL, ST_regular),
                   MakeExprs (MakeId (StringCopy (body_expr), NULL, ST_regular), NULL));

    if (fold_fun != NULL) {
        application = MakeAp (StringCopy (fold_fun), NULL, args);
        pseudo_fold_fun = fold_fun;
    } else {
        DBUG_ASSERT (LEGAL_PRF (fold_prf), "fold_prf is out of range!");
        application = MakePrf (fold_prf, args);
        pseudo_fold_fun = prf_name_string[fold_prf];
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
    buffer = Free (buffer);
    tmp_res_var = TmpVarName (res_var);

    new_fundef = MakeFundef (
      pseudo_fold_fun, PSEUDO_MOD_FOLD, DupAllTypes (elem_type),
      MakeArg (StringCopy (res_var), DupAllTypes (elem_type), ST_regular, ST_regular,
               MakeArg (StringCopy (body_expr), DupAllTypes (elem_type), ST_regular,
                        ST_regular, NULL)),
      MakeBlock (MakeAssign (MakeLet (application,
                                      MakeIds (tmp_res_var, NULL, ST_regular)),
                             MakeAssign (MakeLet (MakeId (StringCopy (tmp_res_var), NULL,
                                                          ST_regular),
                                                  MakeIds (StringCopy (res_var), NULL,
                                                           ST_regular)),
                                         MakeAssign (MakeReturn (
                                                       MakeExprs (MakeId (StringCopy (
                                                                            res_var),
                                                                          NULL,
                                                                          ST_regular),
                                                                  NULL)),
                                                     NULL))),
                 NULL),
      NULL);
    FUNDEF_STATUS (new_fundef) = ST_foldfun;

    DBUG_RETURN (new_fundef);
}
