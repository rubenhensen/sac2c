/*
 *
 * $Log$
 * Revision 3.15  2001/03/22 19:44:52  dkr
 * precompile.h now include once
 *
 * Revision 3.14  2001/03/22 19:16:20  dkr
 * include of tree.h eliminated
 *
 * Revision 3.13  2001/03/15 16:12:38  dkr
 * RemoveArtificialIds() added (extracted from RenameIds())
 *
 * Revision 3.12  2001/03/15 11:59:35  dkr
 * ST_inout replaced by ST_reference
 *
 * Revision 3.11  2001/03/05 18:33:30  dkr
 * ups! missing \n in message of DBUG_ASSERT added ...
 *
 * Revision 3.10  2001/03/05 18:30:00  dkr
 * PREC1code: DBUG_ASSERT added
 *
 * Revision 3.9  2001/03/05 15:39:56  dkr
 * PREC1code added
 *
 * Revision 3.8  2001/03/02 16:10:11  dkr
 * generation of individual pseudo fold-funs done in PREC1withop now
 *
 * Revision 3.7  2001/02/14 14:43:54  dkr
 * PREC2fundef: Removal of artificial return types re-implemented
 *
 * Revision 3.6  2001/01/19 11:55:44  dkr
 * PREC2WLseg() and PREC2WLsegVar() replaced by PREC2WLsegx()
 *
 * Revision 3.5  2000/12/12 12:24:06  dkr
 * PREC1fundef: marking of c-functions is done during RC now
 *
 * Revision 3.4  2000/12/06 18:12:16  cg
 * Re-introduced code for the elimination of function bodies
 * of imported, non-specialized functions. This was disabled
 * due to a resource conflict with new MT compilation. However
 * this turned out to be erroneous.
 *
 * Revision 3.3  2000/12/04 13:34:45  dkr
 * PREC2array added: ARRAY_TYPE renamed correctly now
 *
 * Revision 3.2  2000/12/01 18:34:51  dkr
 * no cc warnings '... might be used uninitialized' anymore
 * global comment added
 *
 * Revision 3.1  2000/11/20 18:01:26  sacbase
 * new release made
 *
 * Revision 2.37  2000/11/17 12:52:09  cg
 * External C types are now consistentlt renamed for their usage
 * within SAC to avoid accidental name clashes with other symbols
 * on the C level.
 *
 * Revision 2.36  2000/11/14 13:38:39  dkr
 * some '... might be used uninitialized in this function' warnings
 * removed
 *
 * Revision 2.35  2000/10/31 23:17:51  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 2.34  2000/10/31 13:47:52  dkr
 * bug in PREC1let fixed:
 * dummy fold-funs have unique names now
 *
 * Revision 2.33  2000/10/31 13:05:27  dkr
 * bug in PREC1fundef fixed:
 * status flag of main function is not set to ST_Cfun anymore
 *
 * Revision 2.32  2000/10/30 19:23:10  dkr
 * bug in PREC2ap fixed:
 * applications of class conversion functions to scalar values are
 * excepted now.
 *
 * Revision 2.31  2000/10/26 12:49:36  dkr
 * signature of DupOneIds changed
 *
 * Revision 2.30  2000/10/24 13:23:33  dkr
 * MakeTypes renamed into MakeTypes1
 *
 * Revision 2.29  2000/10/23 10:28:17  dkr
 * MakeId1 replaced by MakeId_Copy
 *
 * Revision 2.28  2000/10/18 09:44:53  dkr
 * PREC1let modified: 2nd argument of F_reshape is never flattened now
 *
 * Revision 2.27  2000/10/17 17:51:09  dkr
 * flattening of applications from from flatten.c to precompile.c
 *
 * Revision 2.26  2000/10/09 19:16:20  dkr
 * PREC1fundef() added
 *
 * Revision 2.25  2000/09/20 18:19:44  dkr
 * ID_MAKEUNIQUE renamed into ID_CLSCONV
 *
 * Revision 2.24  2000/08/17 10:12:13  dkr
 * all the NT stuff is now in a separate modul (NameTuples.[ch])
 *
 * Revision 2.23  2000/07/19 16:39:32  nmw
 * objinit function modified to work with ICMs
 *
 * [...]
 *
 */

/******************************************************************************
 *
 * This module does some precompilation.
 *
 * It carries out two separate tree traversals.
 * Things done during first traversal:
 *   - Arguments of function applications are abstracted if needed:
 *       a = fun( a)   =>   tmp_a = a; a = fun( tmp_a)
 *     This can only be done *after* type-checking, because types are needed
 *     to decide if abstraction is needed or not, and *after* code optimizations
 *     are finished, because some optimizations (especially CF) might undo these
 *     abstractions.
 *   - For each with-loop a unique and adjusted dummy fold-function is
 *     generated.
 *   - It is checked whether NCODE_CEXPR is identical for all N_Ncode nodes
 *     of a single with-loop.
 * Things done during second traversal:
 *   - Artificial arguments and return values are removed.
 *   - All names and identifiers are renamed in order to avoid name clashes.
 *
 ******************************************************************************/

#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "DupTree.h"
#include "free.h"
#include "prf.h"
#include "traverse.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "typecheck.h"
#include "scheduling.h"
#include "adjust_ids.h"
#include "map_cwrapper.h"
#include "refcount.h"
#include "compile.h"
#include "precompile.h"

/******************************************************************************
 *
 * function:
 *   char *ObjInitFunctionName()
 *
 * description:
 *   Returns new allocated string with objinitfunction name
 *
 * parameters:
 *   uses global variable modulename!
 *
 ******************************************************************************/

char *
ObjInitFunctionName ()
{
    char *new_name;

    DBUG_ENTER ("ObjInitFunctionName");

    new_name = StringConcat ("SACf_GlobalObjInit_for_", modulename);

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *   char *RenameLocalIdentifier( char *id)
 *
 * description:
 *   This function renames a given local identifier name for precompiling
 *   purposes. If the identifier has been inserted by sac2c, i.e. it starts
 *   with an underscore, it is prefixed by SACp. Otherwise, it is prefixed
 *   by SACl.
 *
 *   It also maps the name into an nt (Name Tuple) for tagged arrays.
 *
 ******************************************************************************/

char *
RenameLocalIdentifier (char *id)
{
    char *name_prefix;
    char *new_name;

    DBUG_ENTER ("RenameLocalIdentifier");

    if (id[0] == '_') {
        /*
         * This local identifier was inserted by sac2c.
         */
        name_prefix = "SACp";
        /*
         * Here, we don't need an underscore after the prefix because the name
         * already starts with one.
         */
    } else {
        /*
         * This local identifier originates from the source code.
         */
        name_prefix = "SACl_";
    }

    new_name = (char *)MALLOC (sizeof (char) * (strlen (id) + strlen (name_prefix) + 1));
    sprintf (new_name, "%s%s", name_prefix, id);

    FREE (id);

    DBUG_RETURN (new_name);
}

/*
 *
 * FIRST TRAVERSAL
 *
 */

/******************************************************************************
 *
 * Function:
 *   node *AddVardec( node *fundef, types *type, char *name)
 *
 * Description:
 *   Generates a new declaration, inserts it into the AST, updates the DFMbase
 *   and returns the new declaration.
 *
 ******************************************************************************/

static node *
AddVardec (node *fundef, types *type, char *name)
{
    node *new_vardec;

    DBUG_ENTER ("AddVardec");

    /*
     * generate new vardec node
     */
    new_vardec = MakeVardec (StringCopy (name), DupTypes (type), NULL);

    /*
     * insert new vardec into AST
     */
    VARDEC_NEXT (new_vardec) = FUNDEF_VARDEC (fundef);
    FUNDEF_VARDEC (fundef) = new_vardec;

    /*
     * we must update FUNDEF_DFM_BASE!!
     */
    FUNDEF_DFM_BASE (fundef)
      = DFMUpdateMaskBase (FUNDEF_DFM_BASE (fundef), FUNDEF_ARGS (fundef),
                           FUNDEF_VARDEC (fundef));

    DBUG_RETURN (new_vardec);
}

/******************************************************************************
 *
 * function:
 *   node *AdjustFoldFundef( node *fundef,
 *                           ids *acc, node *cexpr)
 *
 * description:
 *   Returns the given fold-fun definition 'fundef' with adjusted var-names.
 *
 * parameters:
 *   'acc' is the accumulator variable.
 *   'cexpr' is the expression in the operation part.
 *
 ******************************************************************************/

static node *
AdjustFoldFundef (node *fundef, ids *acc, node *cexpr)
{
    node *accvar, *funap, *fold_let;

    DBUG_ENTER ("AdjustFoldFundef");

    DBUG_ASSERT ((fundef != NULL), "fundef is NULL!");
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "no fundef found!");

    /*
     * first, we create a let-expression of the form
     *    <acc> = <funname>( <acc>, <cexpr>);
     */

    accvar = MakeId (StringCopy (IDS_NAME (acc)), StringCopy (IDS_MOD (acc)), ST_regular);
    ID_VARDEC (accvar) = IDS_VARDEC (acc);
    DBUG_ASSERT ((ID_VARDEC (accvar) != NULL), "vardec is missing");

    funap = MakeAp (StringCopy (FUNDEF_NAME (fundef)), StringCopy (FUNDEF_MOD (fundef)),
                    MakeExprs (accvar, MakeExprs (DupTree (cexpr), NULL)));
    AP_FUNDEF (funap) = fundef;

    fold_let = MakeLet (funap, DupOneIds (acc));

    /*
     * then we use this dummy let-expression to adjust the fundef
     */
    fundef = AdjustIdentifiers (fundef, fold_let);

    /*
     * let-expression is useless now
     */
    fold_let = FreeTree (fold_let);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1fundef( node *arg_node, node *arg_info)
 *
 * Description:
 *   Traverses the function body and the next function.
 *
 ******************************************************************************/

node *
PREC1fundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC1fundef");

    INFO_PREC1_FUNDEF (arg_info) = arg_node;

    /*
     * The following is done during reference counting now
     */
#if 0
  /*
   * no module name -> must be an external C-fun
   */
#ifdef MAIN_HAS_MODNAME
  if (FUNDEF_MOD( arg_node) == NULL) {
#else
  if ((FUNDEF_MOD( arg_node) == NULL) &&
      strcmp( FUNDEF_NAME( arg_node), "main")) {
#endif
    FUNDEF_STATUS( arg_node) = ST_Cfun;
  }
#endif

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*
         * The result of Trav() is NOT assigned to FUNDEF_NEXT, because this
         * pointer might be modified by PREC1let() ...
         */
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1block( node *arg_node, node *arg_info)
 *
 * Description:
 *   Saves and restores INFO_PREC1_LASTASSIGN( arg_info).
 *
 ******************************************************************************/

node *
PREC1block (node *arg_node, node *arg_info)
{
    node *old_lastassign;

    DBUG_ENTER ("PREC1fundef");

    old_lastassign = INFO_PREC1_LASTASSIGN (arg_info);

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    INFO_PREC1_LASTASSIGN (arg_info) = old_lastassign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1assign( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC1assign (node *arg_node, node *arg_info)
{
    node *return_node;

    DBUG_ENTER ("PREC1assign");

    INFO_PREC1_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_PREC1_LASTASSIGN( arg_info). To properly insert these nodes,
     * that pointer has to be returned!
     */
    return_node = INFO_PREC1_LASTASSIGN (arg_info);

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (return_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC1let( node *arg_node, node *arg_info)
 *
 * description:
 *   For each id from the LHS:
 *     If we have   a ... a = fun( ... a ... a ... )   ,
 *     were   fun   is a user-defined function,
 *       and   a   is a regular argument representing a refcounted data object,
 *       and the refcounting is *not* done by the function itself,
 *     or   fun   is a primitive function (on arrays) except F_dim or F_ids_psi,
 *       and   a   is a regular argument representing a refcounted data object,
 *     then we rename each RHS   a   into a temp-var   __tmp<n>   and insert
 *     an assignment   __tmp<n> = a;   in front of the function application.
 *
 ******************************************************************************/

node *
PREC1let (node *arg_node, node *arg_info)
{
    node *old_let;
    node *let_expr;
    node *arg, *arg_id;
    ids *let_ids, *tmp_ids;
    char *ids_name, *tmp_name;
    int arg_idx;
    bool flatten, is_prf;
    node *tmp_vardec = NULL;
    prf prf = 0;

    DBUG_ENTER ("PREC1let");

    old_let = INFO_PREC1_LET (arg_info);
    INFO_PREC1_LET (arg_info) = arg_node;

    let_ids = LET_IDS (arg_node);
    let_expr = LET_EXPR (arg_node);

    /*
     * flattening of applications
     */
    while (let_ids != NULL) {
        ids_name = IDS_NAME (let_ids);

        flatten = FALSE;
        is_prf = FALSE;
        if (NODE_TYPE (let_expr) == N_prf) {
            prf = PRF_PRF (let_expr);
            if (ARRAY_ARGS (prf) && (prf != F_dim) && (prf != F_idx_psi)) {
                flatten = TRUE;
            }
            is_prf = TRUE;
        } else if (NODE_TYPE (let_expr) == N_ap) {
            flatten = TRUE;

            DBUG_ASSERT ((AP_FUNDEF (let_expr) != NULL),
                         "no definition of user-defined function found!");
        }

        if (flatten) {
            /*
             * does 'ids_name' occur as an argument of the function application??
             */
            arg = AP_OR_PRF_ARGS (let_expr);
#if 1
            arg_idx = 0; /*
                          * dkr:
                          * not correct yet!!!!!
                          * should be the number of LHS vars???
                          */
#endif
            tmp_name = NULL;
            while (arg != NULL) {
                arg_id = EXPRS_EXPR (arg);
                if ((NODE_TYPE (arg_id) == N_id) && IS_REFCOUNTED (ID, arg_id)
                    && (ID_ATTRIB (arg_id) == ST_regular) &&
                    /* 2nd argument of F_reshape need not to be flattened! */
                    ((is_prf && ((prf != F_reshape) || (arg_idx != 1)))
                     || ((!is_prf)
                         && (!FUN_DOES_REFCOUNT (AP_FUNDEF (let_expr), arg_idx))))) {
                    /*
                     * dkr:
                     * Here, the interpretation of the refcouting-pragma is not correct
                     * yet!!!!!
                     */
                    if (!strcmp (ID_NAME (arg_id), ids_name)) {
                        if (tmp_name == NULL) {
                            /*
                             * first occur of the var of the LHS
                             */
                            DBUG_PRINT ("RENAME",
                                        ("abtracting LHS (%s) of function application",
                                         ids_name));

                            tmp_name = TmpVarName (ids_name);
                            /*
                             * Insert vardec for new var
                             */
                            tmp_vardec = AddVardec (INFO_PREC1_FUNDEF (arg_info),
                                                    IDS_TYPE (let_ids), tmp_name);

                            /*
                             * Abstract the found argument:
                             *   A:n = prf( A:1, ...);
                             *   ... A:n ... A:1 ...    // n references of 'A'
                             * is transformed into
                             *   __A:1 = A:1;
                             *   A:n = prf( __A:1, ...);
                             *   ... A:n ... A:1 ...
                             */
                            tmp_ids = MakeIds (tmp_name, NULL, ST_regular);
                            IDS_VARDEC (tmp_ids) = tmp_vardec;
                            INFO_PREC1_LASTASSIGN (arg_info)
                              = MakeAssign (MakeLet (arg_id, tmp_ids),
                                            INFO_PREC1_LASTASSIGN (arg_info));

                            EXPRS_EXPR (arg) = MakeId_Copy (tmp_name);
                            ID_VARDEC (EXPRS_EXPR (arg)) = tmp_vardec;

                            if (IS_REFCOUNTED (IDS, let_ids)) {
                                IDS_REFCNT (tmp_ids) = 1;
                                ID_REFCNT (EXPRS_EXPR (arg)) = 1;
                            } else {
                                IDS_REFCNT (tmp_ids) = -1;
                                ID_REFCNT (EXPRS_EXPR (arg)) = -1;
                            }
                        } else {
                            /*
                             * temporary var already generated
                             * -> just replace the current arg by 'tmp_name'
                             */
                            FreeTree (EXPRS_EXPR (arg));
                            EXPRS_EXPR (arg) = MakeId_Copy (tmp_name);
                            ID_VARDEC (EXPRS_EXPR (arg)) = tmp_vardec;

                            if (IS_REFCOUNTED (IDS, let_ids)) {
                                ID_REFCNT (EXPRS_EXPR (arg)) = 1;
                            } else {
                                ID_REFCNT (EXPRS_EXPR (arg)) = -1;
                            }
                        }
                    }
                }
                arg = EXPRS_NEXT (arg);
                arg_idx++;
            }
        }
        let_ids = IDS_NEXT (let_ids);
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_PREC1_LET (arg_info) = old_let;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC1with2(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC1with2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC1with2");

    INFO_PREC1_CEXPR (arg_info) = NULL;

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    /*
     * CODE must be traversed before WITHOP!!
     */

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1withop( node *arg_node, node *arg_info)
 *
 * Description:
 *   New, unique and adjusted pseudo fold-funs are created.
 *
 ******************************************************************************/

node *
PREC1withop (node *arg_node, node *arg_info)
{
    ids *let_ids;
    node *let_expr;
    node *new_foldfun;
    char *old_name;

    DBUG_ENTER ("PREC1withop");

    let_ids = LET_IDS (INFO_PREC1_LET (arg_info));
    let_expr = LET_EXPR (INFO_PREC1_LET (arg_info));

    DBUG_ASSERT ((NODE_TYPE (let_expr) == N_Nwith2), "no N_Nwith2 node found!");
    if (NWITH2_IS_FOLD (let_expr)) {
        /*
         * We have to make the formal parameters of each pseudo fold-fun identical
         * to the corresponding application in order to allow for simple code
         * inlining during compilation. But after inlining a single fold-fun might
         * be called multiple times within the code.
         * Therefore we have to create new unique fold-funs first!
         */
        new_foldfun = DupNode (NWITHOP_FUNDEF (arg_node));

        /*
         * create a unique name for the new fold-fun
         * (this is needed for SearchFoldImplementation() in icm2c_mt.c !!)
         */
        old_name = FUNDEF_NAME (new_foldfun);
        FUNDEF_NAME (new_foldfun) = TmpVarName (FUNDEF_NAME (new_foldfun));
        FREE (old_name);

        new_foldfun = AdjustFoldFundef (new_foldfun, let_ids,
                                        /*
                                         * It is sufficient to take the CEXPR of the first
                                         * code-node, because in a fold with-loop all
                                         * CEXPR-ids have the same name!
                                         */
                                        NWITH2_CEXPR (let_expr));

        /*
         * insert new dummy function into fundef chain
         */
        FUNDEF_NEXT (new_foldfun) = FUNDEF_NEXT (NWITHOP_FUNDEF (arg_node));
        FUNDEF_NEXT (NWITHOP_FUNDEF (arg_node)) = new_foldfun;

        NWITHOP_FUNDEF (arg_node) = new_foldfun;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1code( node *arg_node, node *arg_info)
 *
 * Description:
 *   New, unique and adjusted pseudo fold-funs are created.
 *
 ******************************************************************************/

node *
PREC1code (node *arg_node, node *arg_info)
{
    node *let_expr;

    DBUG_ENTER ("PREC1code");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    let_expr = LET_EXPR (INFO_PREC1_LET (arg_info));

    DBUG_ASSERT ((NODE_TYPE (let_expr) == N_Nwith2), "no N_Nwith2 node found!");
    if (NWITH2_IS_FOLD (let_expr)) {
        /*
         * fold with-loop:
         * check whether all NCODE_CEXPR nodes have identical names
         */
        if (INFO_PREC1_CEXPR (arg_info) != NULL) {
            DBUG_ASSERT (((NODE_TYPE (INFO_PREC1_CEXPR (arg_info)) == N_id)
                          && (NODE_TYPE (NCODE_CEXPR (arg_node)) == N_id)),
                         "NCODE_CEXPR must be a N_id node!");

            DBUG_ASSERT ((!strcmp (ID_NAME (INFO_PREC1_CEXPR (arg_info)),
                                   ID_NAME (NCODE_CEXPR (arg_node)))),
                         "Not all NCODE_CEXPR nodes of the fold with-loop have"
                         " identical names!\n"
                         "This is probably due to an error during undo-SSA.");
        } else {
            INFO_PREC1_CEXPR (arg_info) = NCODE_CEXPR (arg_node);
        }
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 * SECOND TRAVERSAL
 *
 */

/******************************************************************************
 *
 * function:
 *   node *CreateObjInitFundef(node *module, node *arg_info)
 *
 * description:
 *   builds up new fundef with empty block, that will contain all init calls
 *   for global objects. This functions will be called during the startup in
 *   the main function or from a separate init functions when used in
 *   a c library
 *
 * parameters:
 *   module where to add fundef
 *   arg_info, to tag new inserted fundef
 *
 * returns:
 *   modified module
 *
 ******************************************************************************/

static node *
CreateObjInitFundef (node *module, node *arg_info)
{
    node *fundef;
    node *assign;
    node *returns;

    DBUG_ENTER ("CreateObjInitFundef");

    returns = MakeReturn (NULL);
    assign = MakeAssign (returns, NULL);

    /* create void procedure without args and with empty return in body */
    fundef = MakeFundef (ObjInitFunctionName (), MODUL_NAME (module), MakeTypes1 (T_void),
                         NULL, MakeBlock (assign, NULL), MODUL_FUNS (module));

    FUNDEF_RETURN (fundef) = returns;

    MODUL_FUNS (module) = fundef;

    INFO_PREC2_OBJINITFUNDEF (arg_info) = fundef;

    DBUG_RETURN (module);
}

/******************************************************************************
 *
 * Function:
 *   node *InsertObjInit( node *block, node *objdef)
 *
 * Description:
 *   For this global object defined in SAC an application of its generic
 *   initialization function is inserted at the beginning of block.
 *
 * Remarks:
 *   The Let_nodes are generated by PRECObjdef.
 *
 ******************************************************************************/

static node *
InsertObjInit (node *block, node *objdef)
{
    ids *new_ids;
    node *assign_end;
    node *assign_ap;
    node *assign_begin;

    DBUG_ENTER ("InsertObjInit");

    new_ids = MakeIds (StringCopy (OBJDEF_NAME (objdef)), NULL, ST_regular);
    IDS_VARDEC (new_ids) = objdef;
    IDS_ATTRIB (new_ids) = ST_global;
    if (IsArray (OBJDEF_TYPE (objdef))) {
        IDS_REFCNT (new_ids) = 1;
    } else {
        IDS_REFCNT (new_ids) = -1;
    }

    assign_end = MakeAssignIcm0 ("INITGLOBALOBJECT_END");
    ICM_END_OF_STATEMENT (ASSIGN_INSTR (assign_end)) = TRUE;

    assign_ap = MakeAssign (MakeLet (OBJDEF_EXPR (objdef), new_ids), BLOCK_INSTR (block));
    assign_begin = MakeAssignIcm1 ("INITGLOBALOBJECT_BEGIN",
                                   MakeId_Copy (StringConcat ("SAC_INIT_FLAG_",
                                                              OBJDEF_NAME (objdef))));

    ASSIGN_NEXT (assign_end) = BLOCK_INSTR (block);
    ASSIGN_NEXT (assign_ap) = assign_end;
    ASSIGN_NEXT (assign_begin) = assign_ap;

    BLOCK_INSTR (block) = assign_begin;

    OBJDEF_EXPR (objdef) = NULL;

    DBUG_RETURN (block);
}

/******************************************************************************
 *
 * function:
 *   node *RenameId( node *idnode)
 *
 * description:
 *   This function performs the renaming of identifiers on the right hand
 *   side of assignments, i.e. the original identifiers are prefixed with
 *   SACl or SACp or are renamed according to the renaming conventions of
 *   global objects.
 *
 ******************************************************************************/

static node *
RenameId (node *idnode)
{
    DBUG_ENTER ("RenameId");

    DBUG_ASSERT ((NODE_TYPE (idnode) == N_id), "Wrong argument to function RenameId()");

    DBUG_PRINT ("PREC", ("id-name == %s", ID_NAME (idnode)));

    DBUG_ASSERT ((ID_VARDEC (idnode) != NULL), "Vardec not found in function RenameId()");

    if (NODE_TYPE (ID_VARDEC (idnode)) == N_objdef) {
        FREE (ID_NAME (idnode));
        ID_NAME (idnode) = StringCopy (OBJDEF_NAME (ID_VARDEC (idnode)));
        /*
         * The global object's definition has already been renamed.
         */
    } else {
        ID_NAME (idnode) = RenameLocalIdentifier (ID_NAME (idnode));
    }

    DBUG_RETURN (idnode);
}

/******************************************************************************
 *
 * Function:
 *   types *RenameTypes( types *type)
 *
 * Description:
 *   Renames the given type if it is a user-defined SAC-type.
 *   Chains of types structures are considered.
 *
 * Remarks:
 *   The complete new name is stored in NAME while MOD is set to NULL.
 *
 ******************************************************************************/

static types *
RenameTypes (types *type)
{
    char *tmp;

    DBUG_ENTER ("RenameTypes");

    if (TYPES_BASETYPE (type) == T_user) {
        if (TYPES_MOD (type) != NULL) {
            /*
             * This is a SAC data type.
             */
            if (!strcmp (TYPES_MOD (type), MAIN_MOD_NAME)) {
                tmp = (char *)MALLOC (sizeof (char) * (strlen (TYPES_NAME (type)) + 6));
                sprintf (tmp, "SACt_%s", TYPES_NAME (type));
            } else {
                tmp = (char *)MALLOC (
                  sizeof (char)
                  * (strlen (TYPES_NAME (type)) + strlen (TYPES_MOD (type)) + 8));
                sprintf (tmp, "SACt_%s__%s", TYPES_MOD (type), TYPES_NAME (type));
            }

            DBUG_PRINT ("PREC", ("renaming type %s:%s to %s", TYPES_MOD (type),
                                 TYPES_NAME (type), tmp));

            FREE (TYPES_NAME (type));
            TYPES_NAME (type) = tmp;
            TYPES_MOD (type) = NULL;
        } else {
            /*
             * This is an imported C data type.
             */
            tmp = (char *)MALLOC (sizeof (char) * (strlen (TYPES_NAME (type)) + 6));
            sprintf (tmp, "SACe_%s", TYPES_NAME (type));

            DBUG_PRINT ("PREC", ("renaming type %s to %s", TYPES_NAME (type), tmp));

            FREE (TYPES_NAME (type));
            TYPES_NAME (type) = tmp;
        }
    }

    if (TYPES_NEXT (type) != NULL) {
        TYPES_NEXT (type) = RenameTypes (TYPES_NEXT (type));
    }

    DBUG_RETURN (type);
}

/******************************************************************************
 *
 * Function:
 *   node *RenameFun(node *fun)
 *
 * Description:
 *   Renames the given function.
 *   For SAC-functions, a new name is created from the module name, the
 *   original name and the argument's types.
 *   For C-functions, a new name is taken from the pragma 'linkname' if present.
 *
 ******************************************************************************/

static node *
RenameFun (node *fun)
{
    node *args;
    char *new_name;
    int length = 0;

    DBUG_ENTER ("RenameFun");

    if (FUNDEF_MOD (fun) != NULL) {
        /*
         * These are SAC-functions which may be overloaded.
         */

        if (FUNDEF_STATUS (fun) == ST_spmdfun) {
            new_name = (char *)MALLOC (sizeof (char) * (strlen (FUNDEF_NAME (fun)) + 6));
            sprintf (new_name, "SACf_%s", FUNDEF_NAME (fun));
        } else {
            args = FUNDEF_ARGS (fun);

            while (args != NULL) {
                length += strlen (ARG_TYPESTRING (args)) + 1;
                args = ARG_NEXT (args);
            }

            if (!strcmp (FUNDEF_MOD (fun), MAIN_MOD_NAME)) {
                length += (strlen (FUNDEF_NAME (fun)) + 7);

                new_name = (char *)MALLOC (sizeof (char) * length);

                sprintf (new_name, "SACf_%s_", FUNDEF_NAME (fun));
            } else {
                length += (strlen (FUNDEF_NAME (fun)) + strlen (FUNDEF_MOD (fun)) + 9);

                new_name = (char *)MALLOC (sizeof (char) * length);

                sprintf (new_name, "SACf_%s__%s_", FUNDEF_MOD (fun), FUNDEF_NAME (fun));
            }

            args = FUNDEF_ARGS (fun);

            while (args != NULL) {
                strcat (new_name, "_");
                strcat (new_name, ARG_TYPESTRING (args));
                FREE (ARG_TYPESTRING (args));
                args = ARG_NEXT (args);
            }
        }

        DBUG_PRINT ("PREC", ("renaming function %s:%s to %s", FUNDEF_MOD (fun),
                             FUNDEF_NAME (fun), new_name));

        FREE (FUNDEF_NAME (fun));

        /* don't free FUNDEF_MOD(fun) because it is shared !! */

        FUNDEF_NAME (fun) = new_name;
        FUNDEF_MOD (fun) = NULL;
    } else {
        if ((FUNDEF_PRAGMA (fun) != NULL) && (FUNDEF_LINKNAME (fun) != NULL)) {
            /*
             * These are C-functions with additional pragma 'linkname'.
             */

            DBUG_PRINT ("PREC", ("renaming function %s to %s", FUNDEF_NAME (fun),
                                 FUNDEF_LINKNAME (fun)));

            FREE (FUNDEF_NAME (fun));

            /* don't free FUNDEF_MOD(fun) because it is shared !! */

            FUNDEF_NAME (fun) = StringCopy (FUNDEF_LINKNAME (fun));
        }
    }

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   ids *RenameIds( ids *arg)
 *
 * description:
 *   This function performs the renaming of identifiers stored within ids-chains.
 *
 ******************************************************************************/

static ids *
RenameIds (ids *arg)
{
    DBUG_ENTER ("RenameIds");

    if (arg != NULL) {
        if (NODE_TYPE (IDS_VARDEC (arg)) == N_objdef) {
            FREE (IDS_NAME (arg));
            IDS_NAME (arg) = StringCopy (OBJDEF_NAME (IDS_VARDEC (arg)));
            /*
             * The global object's definition has already been renamed.
             */
        } else {
            IDS_NAME (arg) = RenameLocalIdentifier (IDS_NAME (arg));
        }

        if (IDS_NEXT (arg) != NULL) {
            IDS_NEXT (arg) = RenameIds (IDS_NEXT (arg));
        }
    }

    DBUG_RETURN (arg);
}

/******************************************************************************
 *
 * function:
 *   ids *RemoveOneArtificialIds( ids *arg, node *rhs)
 *
 * description:
 *   This function removes the first identifier from the given IDS chain.
 *   If the identifier is found on the LHS of a function application it is
 *   checked that it indeed have a counterpart of the same name on the RHS.
 *
 ******************************************************************************/

static ids *
RemoveOneArtificialIds (ids *arg, node *rhs)
{
    node *exprs;
    bool found;

    DBUG_ENTER ("RemoveOneArtificialIds");

    if ((rhs != NULL) && (NODE_TYPE (rhs) == N_ap)) {
        found = FALSE;
        exprs = AP_ARGS (rhs);
        while ((!found) && (exprs != NULL)) {
            if (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id) {
                found = (!strcmp (ID_NAME (EXPRS_EXPR (exprs)), IDS_NAME (arg)));
            }
            exprs = EXPRS_NEXT (exprs);
        }
    } else {
        found = TRUE;
    }

    if (found) {
        arg = FreeOneIds (arg);
    } else {
        DBUG_ASSERT ((0), "application with corrupted reference argument found!");
    }

    DBUG_RETURN (arg);
}

/******************************************************************************
 *
 * function:
 *   ids *RemoveArtificialIds( ids *arg, node *rhs)
 *
 * description:
 *   This function removes those identifiers from the chain which are marked
 *   as 'artificial'.
 *
 ******************************************************************************/

static ids *
RemoveArtificialIds (ids *arg, node *rhs)
{
    ids *tmp;

    DBUG_ENTER ("RemoveArtificialIds");

    /*
     * remove artificial ids: head
     */
    while ((arg != NULL) && (IDS_STATUS (arg) == ST_artificial)) {
        arg = RemoveOneArtificialIds (arg, rhs);
    }

    /*
     * remove artificial ids: tail
     */
    if (arg != NULL) {
        tmp = arg;
        while (IDS_NEXT (tmp) != NULL) {
            if (IDS_STATUS (IDS_NEXT (tmp)) == ST_artificial) {
                IDS_NEXT (tmp) = RemoveOneArtificialIds (IDS_NEXT (tmp), rhs);
            } else {
                tmp = IDS_NEXT (tmp);
            }
        }
    }

    DBUG_RETURN (arg);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2modul( node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal mechanism for objdef and fundef nodes.
 *
 ******************************************************************************/

node *
PREC2modul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2modul");

    arg_node = CreateObjInitFundef (arg_node, arg_info);

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2typedef(node *arg_node, node *arg_info)
 *
 * Description:
 *   Renames types. All types defined in SAC get the prefix "SAC_" to avoid
 *   name clashes with C identifiers.
 *
 ******************************************************************************/

node *
PREC2typedef (node *arg_node, node *arg_info)
{
    char *tmp;

    DBUG_ENTER ("PREC2typedef");

    if (TYPEDEF_MOD (arg_node) != NULL) {
        /*
         * This is a SAC typedef.
         */
        if (!strcmp (TYPEDEF_MOD (arg_node), MAIN_MOD_NAME)) {
            tmp = (char *)MALLOC (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
            sprintf (tmp, "SACt_%s", TYPEDEF_NAME (arg_node));
        } else {
            tmp = (char *)MALLOC (
              sizeof (char)
              * (strlen (TYPEDEF_NAME (arg_node)) + strlen (TYPEDEF_MOD (arg_node)) + 8));
            sprintf (tmp, "SACt_%s__%s", TYPEDEF_MOD (arg_node), TYPEDEF_NAME (arg_node));
        }

        FREE (TYPEDEF_NAME (arg_node));
        TYPEDEF_NAME (arg_node) = tmp;
        TYPEDEF_MOD (arg_node) = NULL;

        TYPEDEF_TYPE (arg_node) = RenameTypes (TYPEDEF_TYPE (arg_node));
    } else {
        /*
         * This is an imported C typedef.
         */
        tmp = (char *)MALLOC (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
        sprintf (tmp, "SACe_%s", TYPEDEF_NAME (arg_node));

        FREE (TYPEDEF_NAME (arg_node));
        TYPEDEF_NAME (arg_node) = tmp;
        /*
         * Why are imported C renamed unlike imported C functions or global objects ?
         *
         * Imported C types do not have a real counterpart in the C module/class
         * implementation. So, there must be no coincidence at link time.
         * As the type name actually does only exist for the sake of the SAC world,
         * which maps it directly to either void* or some basic type, its renaming
         * avoids potential name clashes with other external symbols.
         */
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2objdef( node *arg_node, node *arg_info)
 *
 * Description:
 *   Renames global objects.
 *   For SAC-functions the VARNAME, a combination of module name and object
 *   name is used, for C-functions the optional 'linkname' is used if present.
 *   Additionally, the object's type is renamed as well.
 *   After all the init code is added in the body of the ObjInitFunction.
 *
 ******************************************************************************/

node *
PREC2objdef (node *arg_node, node *arg_info)
{
    char *new_name;

    DBUG_ENTER ("PREC2objdef");

    DBUG_PRINT ("PREC", ("precompiling object %s", ItemName (arg_node)));

    if (OBJDEF_MOD (arg_node) == NULL) {
        if (OBJDEF_LINKNAME (arg_node) != NULL) {
            FREE (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
            FREE (OBJDEF_PRAGMA (arg_node));
        }
    } else {
        FREE (OBJDEF_VARNAME (arg_node));
        /*
         * OBJDEF_VARNAME is no longer used for the generation of the final C code
         * identifier of a global object.
         */

        if (!strcmp (OBJDEF_MOD (arg_node), MAIN_MOD_NAME)) {
            new_name
              = (char *)MALLOC (sizeof (char) * (strlen (OBJDEF_NAME (arg_node)) + 6));

            sprintf (new_name, "SACo_%s", OBJDEF_NAME (arg_node));
        } else {
            new_name = (char *)MALLOC (
              sizeof (char)
              * (strlen (OBJDEF_NAME (arg_node)) + strlen (OBJDEF_MOD (arg_node)) + 8));

            sprintf (new_name, "SACo_%s__%s", OBJDEF_MOD (arg_node),
                     OBJDEF_NAME (arg_node));
        }

        FREE (OBJDEF_NAME (arg_node));
        OBJDEF_MOD (arg_node) = NULL;
        OBJDEF_NAME (arg_node) = new_name;
    }

    OBJDEF_TYPE (arg_node) = RenameTypes (OBJDEF_TYPE (arg_node));

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    /*insert init code */
    FUNDEF_BODY (INFO_PREC2_OBJINITFUNDEF (arg_info))
      = InsertObjInit (FUNDEF_BODY (INFO_PREC2_OBJINITFUNDEF (arg_info)), arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2fundef(node *arg_node, node *arg_info)
 *
 * description:
 *   precompilation of an N_fundef node.
 *
 ******************************************************************************/

node *
PREC2fundef (node *arg_node, node *arg_info)
{
    types *ret_types;
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;

    DBUG_ENTER ("PREC2fundef");

    DBUG_PRINT ("PREC", ("entering %s", FUNDEF_NAME (arg_node)));

    /*
     * The body of an imported inline function is removed.
     */
    if (((FUNDEF_STATUS (arg_node) == ST_imported_mod)
         || (FUNDEF_STATUS (arg_node) == ST_imported_class))
        && (FUNDEF_ATTRIB (arg_node) != ST_generic) && (FUNDEF_BODY (arg_node) != NULL)) {
        FUNDEF_BODY (arg_node) = FreeTree (FUNDEF_BODY (arg_node));
        FUNDEF_RETURN (arg_node) = NULL;
    }

    /*
     * unset inline flag
     */
    FUNDEF_INLINE (arg_node) = FALSE;

    /*
     * The function body is traversed in order to remove artificial return
     * values and parameters of function applications.
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_ASSERT (((FUNDEF_RETURN (arg_node) != NULL)
                      && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_return)),
                     "N_fundef node has no reference to N_return node");

        /*
         * The reference checked above is actually not needed by the
         * precompiler. This is done to check consistency of the syntax
         * tree for further compilation steps.
         */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Now, traverse the following functions.
     * All function bodies must be traversed before arguments and
     * return values of functions are modified.
     */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * The function arguments are traversed, artificial arguments are removed
     * and the number of reference parameters (including global objects)
     * is counted and stored in 'cnt_artificial'
     */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     * All artificial return types are removed.
     * It is necessary to keep name, module name, status, and attrib
     * because in the real syntax tree these are stored within the types
     * structure and not as part of the fundef node as in the virtual
     * syntax tree.
     * Artificial return types are tagged ST_artificial.
     */
    keep_name = FUNDEF_NAME (arg_node);
    keep_mod = FUNDEF_MOD (arg_node);
    keep_status = FUNDEF_STATUS (arg_node);
    keep_attrib = FUNDEF_ATTRIB (arg_node);

    ret_types = FUNDEF_TYPES (arg_node);
    DBUG_ASSERT ((ret_types != NULL), "no return type found!");
    /* remove artificial types at head of TYPES chain */
    while ((ret_types != NULL) && (TYPES_STATUS (ret_types) == ST_artificial)) {
        ret_types = FreeOneTypes (ret_types);
    }
    if (ret_types == NULL) {
        /* all return types removed -> create T_void */
        FUNDEF_TYPES (arg_node) = MakeTypes1 (T_void);
    } else {
        /* store new head in FUNDEF node */
        FUNDEF_TYPES (arg_node) = ret_types;
        /* remove artificial types in inner of TYPES chain */
        while (TYPES_NEXT (ret_types) != NULL) {
            if (TYPES_STATUS (TYPES_NEXT (ret_types)) == ST_artificial) {
                TYPES_NEXT (ret_types) = FreeOneTypes (TYPES_NEXT (ret_types));
            } else {
                ret_types = TYPES_NEXT (ret_types);
            }
        }
    }

    FUNDEF_NAME (arg_node) = keep_name;
    FUNDEF_MOD (arg_node) = keep_mod;
    FUNDEF_STATUS (arg_node) = keep_status;
    FUNDEF_ATTRIB (arg_node) = keep_attrib;

    if (arg_node != INFO_PREC2_OBJINITFUNDEF (arg_info)) {
        /*
         * Now, the data flow mask base is updated.
         * This is necessary because some local identifiers are removed while all
         * others are renamed. This is skipped, when a object-init-function is
         * processed
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMUpdateMaskBaseAfterRenaming (FUNDEF_DFM_BASE (arg_node),
                                                FUNDEF_ARGS (arg_node),
                                                FUNDEF_VARDEC (arg_node));
        }

        /* no renaming of obj-init-functions */
        arg_node = RenameFun (arg_node);
    }

    FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2arg(node *arg_node, node *arg_info)
 *
 * description:
 *   An artificial argument is removed, the attribs are switched:
 *       ST_readonly_reference -> ST_regular
 *       ST_was_reference -> ST_reference
 *
 ******************************************************************************/

node *
PREC2arg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2arg");

    if (ARG_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        ARG_TYPESTRING (arg_node) = Type2String (ARG_TYPE (arg_node), 2, TRUE);
        ARG_TYPE (arg_node) = RenameTypes (ARG_TYPE (arg_node));
        /*
         * ARG_TYPESTRING is only used for renaming functions, so the
         * type's actual name may be changed afterwards.
         */

        if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
            ARG_ATTRIB (arg_node) = ST_regular;
        } else {
            if (ARG_ATTRIB (arg_node) == ST_was_reference) {
                ARG_ATTRIB (arg_node) = ST_reference;
            }
        }

        if (ARG_NAME (arg_node) != NULL) {
            /*
             * The attribute ARG_NAME may not be set in the case of imported function
             * declarations.
             */
            ARG_NAME (arg_node) = RenameLocalIdentifier (ARG_NAME (arg_node));
        }

        if (ARG_NEXT (arg_node) != NULL) {
            ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2vardec(node *arg_node, node *arg_info)
 *
 * Description:
 *   Renames types of declared variables.
 *   Remove artificial variable declarations.
 *
 ******************************************************************************/

node *
PREC2vardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2vardec");

    if (VARDEC_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        VARDEC_TYPE (arg_node) = RenameTypes (VARDEC_TYPE (arg_node));
        VARDEC_NAME (arg_node) = RenameLocalIdentifier (VARDEC_NAME (arg_node));

        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2assign( node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC2assign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2assign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_INSTR (arg_node) == NULL) {
        arg_node = FreeNode (arg_node);
        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2let( node *arg_node, node *arg_info)
 *
 * description:
 *   removes all artificial identifiers on the left hand side of a let.
 *
 ******************************************************************************/

node *
PREC2let (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2let");

    LET_IDS (arg_node) = RemoveArtificialIds (LET_IDS (arg_node), LET_EXPR (arg_node));

    LET_IDS (arg_node) = RenameIds (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_EXPR (arg_node) == NULL) {
        arg_node = FreeTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2exprs_ap(node *current, node *formal)
 *
 * Description:
 *   Removes all artificial parameters.
 *   The status of those current parameters which belong to formal reference
 *   parameters is modified to ST_reference.
 *   Global objects given as parameters to the applied function get a reference
 *   to the object definition and are renamed with the new name of the global
 *   object.
 *
 ******************************************************************************/

static node *
PREC2exprs_ap (node *current, node *formal)
{
    node *expr;

    DBUG_ENTER ("PREC2exprs_ap");

    if (EXPRS_NEXT (current) != NULL) {
        EXPRS_NEXT (current)
          = PREC2exprs_ap (EXPRS_NEXT (current), (ARG_BASETYPE (formal) == T_dots)
                                                   ? formal
                                                   : ARG_NEXT (formal));
    }

    expr = EXPRS_EXPR (current);

    if (NODE_TYPE (expr) == N_id) {
        if (ID_STATUS (expr) == ST_artificial) {
            current = FreeNode (current);
        } else {
            if (ARG_ATTRIB (formal) == ST_was_reference) {
                ID_ATTRIB (expr) = ST_reference;
            }

            if ((VARDEC_OR_ARG_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                ID_VARDEC (expr) = VARDEC_OR_ARG_OBJDEF (ID_VARDEC (expr));
            }

            EXPRS_EXPR (current) = RenameId (EXPRS_EXPR (current));
        }
    }

    DBUG_RETURN (current);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2exprs_return(node *ret_exprs, node *ret_node)
 *
 * Description:
 *   Removes all artificial return values from the chain.
 *   A new chain is built up for all those return values which belong to
 *   original reference parameters. These are stored in RETURN_REFERENCE for
 *   later use in compile.c.
 *
 ******************************************************************************/

static node *
PREC2exprs_return (node *ret_exprs, node *ret_node)
{
    node *tmp;

    DBUG_ENTER ("PREC2exprs_return");

    if (EXPRS_NEXT (ret_exprs) != NULL) {
        EXPRS_NEXT (ret_exprs) = PREC2exprs_return (EXPRS_NEXT (ret_exprs), ret_node);
    }

    if (ID_STATUS (EXPRS_EXPR (ret_exprs)) == ST_artificial) {
        if (VARDEC_OR_ARG_STATUS (ID_VARDEC (EXPRS_EXPR (ret_exprs))) == ST_artificial) {
            /*
             * This artificial return value belongs to a global object,
             * so it can be removed.
             */

            ret_exprs = FreeNode (ret_exprs);
        } else {
            /*
             * This artificial return value belongs to an original reference
             * parameter, so it is stored in RETURN_REFERENCE to be compiled
             * to an "inout" parameter.
             */

            tmp = ret_exprs;
            ret_exprs = EXPRS_NEXT (ret_exprs);
            EXPRS_EXPR (tmp) = RenameId (EXPRS_EXPR (tmp));
            EXPRS_NEXT (tmp) = RETURN_REFERENCE (ret_node);
            RETURN_REFERENCE (ret_node) = tmp;
        }
    } else {
        /*
         * All expressions in a return-statement are guaranteed to be of
         * node type N_id.
         */
        EXPRS_EXPR (ret_exprs) = RenameId (EXPRS_EXPR (ret_exprs));
    }

    DBUG_RETURN (ret_exprs);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2ap( node *arg_node, node *arg_info)
 *
 * Description:
 *   Traverses the current arguments using function PREC2exprs_ap that is given
 *   a pointer to the first formal parameter of the applied function.
 *
 ******************************************************************************/

node *
PREC2ap (node *arg_node, node *arg_info)
{
    node *ap;

    DBUG_ENTER ("PREC2ap");

    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_classfun) {
        ap = arg_node;
        arg_node = EXPRS_EXPR (AP_ARGS (arg_node));

        if (NODE_TYPE (arg_node) == N_id) {
            if (!strncmp (AP_NAME (ap), "to_", 3)) {
                arg_node = RenameId (arg_node);
                DBUG_ASSERT ((!IsUnique (ID_TYPE (arg_node))),
                             "Argument of to_class function is unique already!");

                ID_CLSCONV (arg_node) = TO_CLASS;
            } else {
                /*
                 * This must be a "from" function. So, the argument is of a class
                 * type which implies that it is an identifier.
                 */
                arg_node = RenameId (arg_node);

                ID_CLSCONV (arg_node) = FROM_CLASS;
            }
        } else {
            /* argument of class conversion function is no N_id node */

            /*
             * -> argument is a scalar value
             * -> simply remove the conversion function
             */
        }

        FREE (AP_NAME (ap));
        FREE (ap);
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node)
              = PREC2exprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2return(node *arg_node, node *arg_info)
 *
 * Description:
 *   Traverses the return values using function PREC2exprs_return.
 *
 ******************************************************************************/

node *
PREC2return (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2return");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = PREC2exprs_return (RETURN_EXPRS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2id( node *arg_node, node *arg_info)
 *
 * Description:
 *   Applied occurrences of global objects may be renamed, if the global
 *   object was renamed.
 *
 ******************************************************************************/

node *
PREC2id (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2id");

    if (ID_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeTree (arg_node);
    } else {
        if (ID_VARDEC (arg_node) != NULL) { /* is NULL for IDs in ObjInitFunctions */
            if (VARDEC_OR_ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial) {
                ID_VARDEC (arg_node) = VARDEC_OR_ARG_OBJDEF (ID_VARDEC (arg_node));
            }

            arg_node = RenameId (arg_node);

            ID_CLSCONV (arg_node) = NO_CLSCONV;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2array(node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC2array (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2array");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }

    ARRAY_TYPE (arg_node) = RenameTypes (ARRAY_TYPE (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2withid( node *arg_node, node *arg_info)
 *
 * description:
 *   This function does the renaming of the index vector variable
 *   as well as its scalar counterparts for the new with-loop.
 *
 ******************************************************************************/

node *
PREC2withid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2withid");

    NWITHID_VEC (arg_node) = RenameIds (NWITHID_VEC (arg_node));
    NWITHID_IDS (arg_node) = RenameIds (NWITHID_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2do(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2do (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2do");

    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    DO_USEVARS (arg_node) = RenameIds (DO_USEVARS (arg_node));
    DO_DEFVARS (arg_node) = RenameIds (DO_DEFVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2while(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2while (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2while");

    WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    WHILE_USEVARS (arg_node) = RenameIds (WHILE_USEVARS (arg_node));
    WHILE_DEFVARS (arg_node) = RenameIds (WHILE_DEFVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2cond(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2cond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2cond");

    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);
    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    COND_THENVARS (arg_node) = RenameIds (COND_THENVARS (arg_node));
    COND_ELSEVARS (arg_node) = RenameIds (COND_ELSEVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2with2(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2with2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2with2");

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    NWITH2_DEC_RC_IDS (arg_node) = RenameIds (NWITH2_DEC_RC_IDS (arg_node));

    /*
     * Since the scheduling specification may contain the names of local
     * identifiers, these have to be renamed according to the general renaming
     * scheme implemented by this compiler phase.
     */
    if (NWITH2_SCHEDULING (arg_node) != NULL) {
        NWITH2_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (NWITH2_SCHEDULING (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2code( node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2code (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2code");

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    NCODE_INC_RC_IDS (arg_node) = RenameIds (NCODE_INC_RC_IDS (arg_node));

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2sync( node *arg_node, node *arg_info)
 *
 * description:
 *   Was used for renaming SYNC_SCHEDULE, since this has move to the
 *   with-loops it's not done here anymore, but i kept the function, not
 *   knowing if there would be any problems if if i killed it. (jhs)
 *
 ******************************************************************************/

node *
PREC2sync (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2sync");

    SYNC_REGION (arg_node) = Trav (SYNC_REGION (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2WLsegx(node *arg_node, node *arg_info)
 *
 * description:
 *   Since the scheduling specification may contain the names of local
 *   identifiers, these have to be renamed according to the general renaming
 *   scheme implemented by this compiler phase.
 *
 ******************************************************************************/

node *
PREC2WLsegx (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2WLsegx");

    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        WLSEGX_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (WLSEGX_SCHEDULING (arg_node));
    }

    WLSEGX_CONTENTS (arg_node) = Trav (WLSEGX_CONTENTS (arg_node), arg_info);

    if (WLSEGX_NEXT (arg_node) != NULL) {
        WLSEGX_NEXT (arg_node) = Trav (WLSEGX_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Precompile( node *syntax_tree)
 *
 * description:
 *   Prepares syntax tree for code generation.
 *
 *   optional traversal of AST when generating c-library:
 *     - look for overloaded functions and build up a list of wrappers
 *     remark: this has to be done before the following steps, because
 *             of the renaming
 *
 *   First traversal of the AST:
 *     - renaming of the local identifiers of each dummy fold-function
 *       definition in order to prepare them for naive inlining during
 *       compilation.
 *   Second traversal of the AST:
 *     - renames functions and global objects
 *     - removes all casts
 *     - inserts extern declarations for function definitions
 *     - removes all artificial parameters and return values
 *     - marks reference parameters in function applications
 *   Unfortunately it is impossible to merge these two traversals in a concise
 *   way ...
 *
 ******************************************************************************/

node *
Precompile (node *syntax_tree)
{
    node *info;

    DBUG_ENTER ("Precompile");

    syntax_tree = MapCWrapper (syntax_tree);

    info = MakeInfo ();

    act_tab = precomp1_tab;
    syntax_tree = Trav (syntax_tree, info);

    act_tab = precomp2_tab;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeTree (info);

    DBUG_RETURN (syntax_tree);
}
