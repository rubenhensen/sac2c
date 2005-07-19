/*
 *
 * $Log$
 * Revision 3.21  2005/07/19 17:09:59  sbs
 * eliminated module encoding
 *
 * Revision 3.20  2005/07/18 15:01:46  sbs
 * removed the PSEUDE namespace FOLD for fold funs.
 * Considered safe by sah due to _ prefix in fun-name :-)
 *
 * Revision 3.19  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.18  2004/12/09 00:38:18  sbs
 * swapped the ars of TBmakeLet
 *
 * Revision 3.17  2004/11/25 17:52:55  sbs
 * compiles
 *
 * Revision 3.16  2004/11/23 15:07:12  sbs
 * SacSacDevCamp 04 done
 *
 * Revision 3.15  2004/11/22 13:43:54  sbs
 * CreatePseudoFoldFun eliminated
 *
 * Revision 3.14  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * Revision 3.13  2004/10/05 13:49:35  sah
 * added some defines for NEW_AST mode
 *
 * Revision 3.12  2004/07/21 12:46:12  khf
 * set flag FUNDEF_INLINE true
 *
 * Revision 3.11  2003/06/17 20:26:27  sbs
 * changed the generation of fold funs for the new TC.
 * Now, a SSA safe form is generated (no surplus assignment).
 *
 * Revision 3.10  2002/10/18 14:30:58  sbs
 * several FLAGS set appropriately after creating fresh N_id nodes
 *
 * Revision 3.9  2002/10/08 10:39:51  dkr
 * function CreateFoldFun() for new type system added
 *
 * Revision 3.8  2002/09/11 23:15:35  dkr
 * prf_node_info.mac modified
 *
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

#include <string.h>

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
#include "new_types.h"
#include "namespaces.h"

/******************************************************************************
 *
 * function:
 *   node *GPFcreateFoldFun( ntype *elem_type,
 *                           node  *fold_fundef,
 *                           prf    fold_prf,
 *                           char  *res_name,
 *                           char  *cexpr_name)
 *
 * description:
 *  - generates an N_fundef node of the following kind:
 *
 *      <elem_type> actNS:_type_<n>__<fold_fun>( <elem_type> <res_name>,
 *                                               <elem_type> <cexpr_name>)
 *      {
 *        tmp__<res_name> = <fold_fun>( <res_name>, <cexpr_name>)
 *        return( tmp__<res_name>);
 *      }
 *    NOTE, that all(!) arguments are NOT inserted in the new AST!
 *    Instead, copied versions are used only!
 *    Hence, the calling function has to take care of the memory allocated
 *    for the arguments!
 *    NOTE as well, that fold_prf is valid iff (fold_fun == NULL).
 *
 ******************************************************************************/

node *
GPFcreateFoldFun (ntype *elem_type, node *fold_fundef, prf fold_prf, char *res_name,
                  char *cexpr_name)
{
    const char *pseudo_fold_fun_name;
    char *tmp_res_name;
    char *buffer;
    node *new_fundef, *formal_args, *application, *args, *ret_ass;
    node *tmp_res_vardec;
    node *res_avis, *cexpr_avis, *tmp_res_avis;

    DBUG_ENTER ("GPFcreateFoldFun");

    res_avis = TBmakeAvis (ILIBstringCopy (res_name), TYcopyType (elem_type));
    cexpr_avis = TBmakeAvis (ILIBstringCopy (cexpr_name), TYcopyType (elem_type));
    formal_args = TBmakeArg (res_avis, TBmakeArg (cexpr_avis, NULL));

    args = TBmakeExprs (TBmakeId (res_avis), TBmakeExprs (TBmakeId (cexpr_avis), NULL));

    if (fold_fundef != NULL) {
        application = TBmakeAp (fold_fundef, args);
        pseudo_fold_fun_name = FUNDEF_NAME (fold_fundef);
    } else {
        DBUG_ASSERT (LEGAL_PRF (fold_prf), "fold_prf is out of range!");
        application = TBmakePrf (fold_prf, args);
        pseudo_fold_fun_name = global.prf_string[fold_prf];
    }

    /*
     * Since the module name of the fold function is always '_FOLD', the
     * actual module name must be encoded within the name in order to guarantee
     * that no two fold functions from different modules may have the same
     * internal name.
     */
    buffer = (char *)ILIBmalloc (strlen (pseudo_fold_fun_name) + 3);
    strcat (buffer, "__");
    strcat (buffer, pseudo_fold_fun_name);
    pseudo_fold_fun_name = ILIBtmpVarName (buffer);
    buffer = ILIBfree (buffer);

    tmp_res_name = ILIBtmpVarName (res_name);
    tmp_res_avis = TBmakeAvis (ILIBstringCopy (tmp_res_name), TYcopyType (elem_type));
    tmp_res_vardec = TBmakeVardec (tmp_res_avis, NULL);

    ret_ass = TBmakeReturn (TBmakeExprs (TBmakeId (tmp_res_avis), NULL));

    new_fundef
      = TBmakeFundef (ILIBstringCopy (pseudo_fold_fun_name),
                      NSgetNamespace (global.modulename),
                      TBmakeRet (TYcopyType (elem_type), NULL), formal_args,
                      TBmakeBlock (TBmakeAssign (TBmakeLet (TBmakeIds (tmp_res_avis,
                                                                       NULL),
                                                            application),
                                                 TBmakeAssign (ret_ass, NULL)),
                                   tmp_res_vardec),
                      NULL);
    FUNDEF_ISFOLDFUN (new_fundef) = TRUE;
    FUNDEF_ISINLINE (new_fundef) = TRUE;
    FUNDEF_RETURN (new_fundef) = ret_ass;

    DBUG_RETURN (new_fundef);
}
