/*
 *
 * $Log$
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
 * Revision 2.22  2000/07/14 15:44:01  nmw
 * some dead code removed
 *
 * Revision 2.21  2000/07/14 15:19:33  dkr
 * FUNDEF_INLINE is bool!
 *
 * Revision 2.20  2000/07/14 14:46:25  nmw
 * init code for global objects moved from beginning og main()
 * to a separate init function that is called during the startup
 * it can be used in sac programms and c libraries, too
 *
 * Revision 2.15  2000/06/28 09:12:23  nmw
 * added traversal for mapping of functions to c wrappers
 *
 * Revision 2.14  2000/06/23 15:11:19  dkr
 * signature of DupTree changed
 *
 * Revision 2.13  2000/05/29 14:32:13  dkr
 * precompile() renamed into Precompile()
 * precompile now performs *two* tree traversals
 *
 * Revision 2.12  2000/05/29 11:56:00  dkr
 * fixed a bug in PREClet:
 * AdjustFoldFundef() is called now *before* the traversal of the
 * let-node is started.
 *
 * Revision 2.11  2000/05/26 19:26:53  dkr
 * signature of AdjustFoldFundef modified
 *
 * Revision 2.10  2000/05/25 23:04:44  dkr
 * PREClet(): call of AdjustFoldFundef() added
 *
 * Revision 2.9  2000/04/18 14:00:48  jhs
 * Added DBUGs.
 *
 * Revision 2.8  2000/03/23 16:04:49  dkr
 * DBUG_ASSERT in RenameId() added
 *
 * Revision 2.7  2000/03/02 13:07:44  jhs
 * Commented out usage of FUNDEF_ATTRIB (search it to find the place)
 * because these flag is cleared and reused bei multithreaded.
 *
 * Revision 2.6  2000/02/23 20:16:34  cg
 * Node status ST_imported replaced by ST_imported_mod and
 * ST_imported_class in order to allow distinction between enteties
 * that are imported from a module and those that are imported from a
 * class.
 *
 * Revision 2.5  2000/02/23 17:47:34  cg
 * Header file refcount.h no longer included.
 * Type property functions IsUnique(<type>), IsBoxed(<type>)
 * moved from refcount.c to tree_compound.c.
 *
 * Revision 2.4  1999/09/01 17:12:39  jhs
 * Expanded COMPSync to refcounters in barriers.
 *
 * Revision 2.3  1999/06/25 15:19:43  rob
 * Avoid genning GetUniFromTypes if not TAGGED_ARRAYS
 *
 * Revision 2.2  1999/06/25 14:52:25  rob
 * Introduce definitions and utility infrastructure for tagged array support.
 *
 * Revision 2.1  1999/02/23 12:42:52  sacbase
 * new release made
 *
 * Revision 1.76  1998/11/08 14:31:15  dkr
 * PRECNcode:
 *   NCODE_CBLOCK should be never NULL
 *   (if so, we will get an assert now :-)
 *
 * Revision 1.75  1998/08/07 16:06:13  dkr
 * PRECWLsegVar added
 *
 * [...]
 *
 * Revision 1.2  1995/12/01  17:23:26  cg
 * first working revision
 *
 * Revision 1.1  1995/11/28  12:23:34  cg
 * Initial revision
 *
 */

#include <string.h>

#include "dbug.h"
#include "types.h"
#include "tree.h"
#include "DupTree.h"
#include "free.h"
#include "prf.h"
#include "traverse.h"
#include "internal_lib.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "typecheck.h"
#include "scheduling.h"
#include "adjust_ids.h"
#include "precompile.h"
#include "map_cwrapper.h"
#include "compile.h"
#include "precompile.h"

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

    INFO_PREC_OBJINITFUNDEF (arg_info) = fundef;

    DBUG_RETURN (module);
}

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

    INFO_PREC_FUNDEF (arg_info) = arg_node;

    /*
     * no module name -> must be an external C-fun
     */
#ifdef MAIN_HAS_MODNAME
    if (FUNDEF_MOD (arg_node) == NULL) {
#else
    if ((FUNDEF_MOD (arg_node) == NULL) && strcmp (FUNDEF_NAME (arg_node), "main")) {
#endif
        FUNDEF_STATUS (arg_node) = ST_Cfun;
    }

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
 *   Saves and restores INFO_PREC_LASTASSIGN( arg_info).
 *
 ******************************************************************************/

node *
PREC1block (node *arg_node, node *arg_info)
{
    node *old_lastassign;

    DBUG_ENTER ("PREC1fundef");

    old_lastassign = INFO_PREC_LASTASSIGN (arg_info);

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    INFO_PREC_LASTASSIGN (arg_info) = old_lastassign;

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

    INFO_PREC_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_PREC_LASTASSIGN( arg_info). To properly insert these nodes,
     * that pointer has to be returned!
     */
    return_node = INFO_PREC_LASTASSIGN (arg_info);

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
 *   Moreover new, unique and adjusted pseudo fold-funs are created.
 *
 ******************************************************************************/

node *
PREC1let (node *arg_node, node *arg_info)
{
    node *expr, *new_foldfun, *arg, *arg_id, *tmp_vardec;
    ids *let_ids, *tmp_ids;
    char *ids_name, *tmp_name, *old_name;
    prf prf;
    int arg_idx;
    bool flatten, is_prf;

    DBUG_ENTER ("PREC1let");

    let_ids = LET_IDS (arg_node);
    expr = LET_EXPR (arg_node);

    /*
     * flattening of applications
     */
    while (let_ids != NULL) {
        ids_name = IDS_NAME (let_ids);

        flatten = FALSE;
        is_prf = FALSE;
        if (NODE_TYPE (expr) == N_prf) {
            prf = PRF_PRF (expr);
            if (ARRAY_ARGS (prf) && (prf != F_dim) && (prf != F_idx_psi)) {
                flatten = TRUE;
            }
            is_prf = TRUE;
        } else if (NODE_TYPE (expr) == N_ap) {
            flatten = TRUE;

            DBUG_ASSERT ((AP_FUNDEF (expr) != NULL),
                         "no definition of user-defined function found!");
        }

        if (flatten) {
            /*
             * does 'ids_name' occur as an argument of the function application??
             */
            arg = AP_OR_PRF_ARGS (expr);
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
                         && (!FUN_DOES_REFCOUNT (AP_FUNDEF (expr), arg_idx))))) {
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
                            tmp_vardec = AddVardec (INFO_PREC_FUNDEF (arg_info),
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
                            INFO_PREC_LASTASSIGN (arg_info)
                              = MakeAssign (MakeLet (arg_id, tmp_ids),
                                            INFO_PREC_LASTASSIGN (arg_info));

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

    let_ids = LET_IDS (arg_node);
    if ((NODE_TYPE (expr) == N_Nwith2)
        && ((NWITH2_TYPE (expr) == WO_foldfun) || (NWITH2_TYPE (expr) == WO_foldprf))) {
        /*
         * We have to make the formal parameters of each pseudo fold-fun identical
         * to the corresponding application in order to allow for simple code
         * inlining during compilation. But after inlining a single fold-fun might
         * be called multiple times within the code.
         * Therefore we have to create new unique fold-funs first!
         */
        new_foldfun = DupNode (NWITH2_FUNDEF (expr));

        /*
         * Create a unique name for the new fold-fun
         * (This is needed for SearchFoldImplementation() in icm2c_mt.c !!)
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
                                        NWITH2_CEXPR (expr));

        FUNDEF_NEXT (new_foldfun) = FUNDEF_NEXT (NWITH2_FUNDEF (expr));
        FUNDEF_NEXT (NWITH2_FUNDEF (expr)) = new_foldfun;
        NWITH2_FUNDEF (expr) = new_foldfun;
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

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
 * function:
 *   ids *PrecompileIds( ids *id)
 *
 * description:
 *   This function performs the renaming of identifiers stored within ids-chains.
 *   It also removes those identifiers from the chain which are marked as
 *   'artificial'.
 *
 ******************************************************************************/

static ids *
PrecompileIds (ids *id)
{
    DBUG_ENTER ("PrecompileIds");

    while ((id != NULL) && (IDS_STATUS (id) == ST_artificial)) {
        id = FreeOneIds (id);
    }

    if (id != NULL) {
        if (NODE_TYPE (IDS_VARDEC (id)) == N_objdef) {
            FREE (IDS_NAME (id));
            IDS_NAME (id) = StringCopy (OBJDEF_NAME (IDS_VARDEC (id)));
            /*
             * The global object's definition has already been renamed.
             */
        } else {
            IDS_NAME (id) = RenameLocalIdentifier (IDS_NAME (id));
        }

        if (IDS_NEXT (id) != NULL) {
            IDS_NEXT (id) = PrecompileIds (IDS_NEXT (id));
        }
    }

    DBUG_RETURN (id);
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
            if (0 == strcmp (TYPES_MOD (type), MAIN_MOD_NAME)) {
                tmp = (char *)Malloc (sizeof (char) * (strlen (TYPES_NAME (type)) + 6));
                sprintf (tmp, "SACt_%s", TYPES_NAME (type));
            } else {
                tmp = (char *)Malloc (
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
            tmp = (char *)Malloc (sizeof (char) * (strlen (TYPES_NAME (type)) + 6));
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
            new_name = (char *)Malloc (sizeof (char) * (strlen (FUNDEF_NAME (fun)) + 6));
            sprintf (new_name, "SACf_%s", FUNDEF_NAME (fun));
        } else {
            args = FUNDEF_ARGS (fun);

            while (args != NULL) {
                length += strlen (ARG_TYPESTRING (args)) + 1;
                args = ARG_NEXT (args);
            }

            if (0 == strcmp (FUNDEF_MOD (fun), MAIN_MOD_NAME)) {
                length += (strlen (FUNDEF_NAME (fun)) + 7);

                new_name = (char *)Malloc (sizeof (char) * length);

                sprintf (new_name, "SACf_%s_", FUNDEF_NAME (fun));
            } else {
                length += (strlen (FUNDEF_NAME (fun)) + strlen (FUNDEF_MOD (fun)) + 9);

                new_name = (char *)Malloc (sizeof (char) * length);

                sprintf (new_name, "SACf_%s__%s_", FUNDEF_MOD (fun), FUNDEF_NAME (fun));
            }

            args = FUNDEF_ARGS (fun);

            while (args != NULL) {
                strcat (new_name, "_");
                strcat (new_name, ARG_TYPESTRING (args));
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

    INFO_PREC_MODUL (arg_info) = arg_node;

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

/*
 *
 *  functionname  : PREC2typedef
 *  arguments     : 1) N_typedef node
 *                  2) arg_info unused
 *  description   : renames types,
 *                  All types defined in SAC get the prefix "SAC_"
 *                  to avoid name clashes with C identifiers.
 *  global vars   : mod_name_con
 *  internal funs : ---
 *  external funs : Malloc, sprintf, strlen
 *  macros        : DBUG, FREE, TREE
 *
 *  remarks       :
 *
 */

node *
PREC2typedef (node *arg_node, node *arg_info)
{
    char *tmp;

    DBUG_ENTER ("PREC2typedef");

    if (TYPEDEF_MOD (arg_node) != NULL) {
        /*
         * This is a SAC typedef.
         */
        if (0 == strcmp (TYPEDEF_MOD (arg_node), MAIN_MOD_NAME)) {
            tmp = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
            sprintf (tmp, "SACt_%s", TYPEDEF_NAME (arg_node));
        } else {
            tmp = (char *)Malloc (
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
        tmp = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
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
 *   node *PREC2objdef(node *arg_node, node *arg_info)
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

        if (0 == strcmp (OBJDEF_MOD (arg_node), MAIN_MOD_NAME)) {
            new_name
              = (char *)Malloc (sizeof (char) * (strlen (OBJDEF_NAME (arg_node)) + 6));

            sprintf (new_name, "SACo_%s", OBJDEF_NAME (arg_node));
        } else {
            new_name = (char *)Malloc (
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
    FUNDEF_BODY (INFO_PREC_OBJINITFUNDEF (arg_info))
      = InsertObjInit (FUNDEF_BODY (INFO_PREC_OBJINITFUNDEF (arg_info)), arg_node);

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
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;
    int i;

    DBUG_ENTER ("PREC2fundef");

    DBUG_PRINT ("PREC", ("entering %s", FUNDEF_NAME (arg_node)));

    /*
     * The body of an imported inline function is removed.
     */
#if 0
  /*
   * FUNDEF_ATTRIB is reused by multithread ...
   * this here seems to be superfluous (?)
   * if problems occur we need another way to store ST_call_[rep|any|mt|st]
   * (jhs)
   */
  if (((FUNDEF_STATUS(arg_node) == ST_imported_mod)
       || (FUNDEF_STATUS(arg_node) == ST_imported_class)) &&
      (FUNDEF_ATTRIB(arg_node) != ST_generic) &&
      (FUNDEF_BODY(arg_node) != NULL)) {
    FUNDEF_BODY(arg_node) = FreeTree(FUNDEF_BODY(arg_node));
    FUNDEF_RETURN(arg_node) = NULL;
  }
#endif

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
    INFO_PREC_CNT_ARTIFICIAL (arg_info) = 0;
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /*
     * All artificial return types are removed.
     * It is necessary to keep name, module name, status, and attrib
     * because in the real syntax tree these are stored within the types
     * structure and not as part of the fundef node as in the virtual
     * syntax tree.
     * Nowadays, artificial return types are tagged ST_artificial. This
     * would allow a different implementation of removing them without
     * first counting the corresponding arguments.
     */
    if (INFO_PREC_CNT_ARTIFICIAL (arg_info) > 0) {
        keep_name = FUNDEF_NAME (arg_node);
        keep_mod = FUNDEF_MOD (arg_node);
        keep_status = FUNDEF_STATUS (arg_node);
        keep_attrib = FUNDEF_ATTRIB (arg_node);

        for (i = 1; i < INFO_PREC_CNT_ARTIFICIAL (arg_info); i++) {
            FUNDEF_TYPES (arg_node) = FreeOneTypes (FUNDEF_TYPES (arg_node));
        }

        if (TYPES_NEXT (FUNDEF_TYPES (arg_node)) == NULL) {
            FUNDEF_BASETYPE (arg_node) = T_void;
            FREE (FUNDEF_TNAME (arg_node));
            FUNDEF_TNAME (arg_node) = NULL;
            FUNDEF_TMOD (arg_node) = NULL;
        } else {
            FUNDEF_TYPES (arg_node) = FreeOneTypes (FUNDEF_TYPES (arg_node));
        }

        FUNDEF_NAME (arg_node) = keep_name;
        FUNDEF_MOD (arg_node) = keep_mod;
        FUNDEF_STATUS (arg_node) = keep_status;
        FUNDEF_ATTRIB (arg_node) = keep_attrib;
    }

    /*
     * Now, the data flow mask base is updated.
     * This is necessary because some local identifiers are removed while all
     * others are renamed. This is skipped, when the ObjInitFunctions is processed
     */
    if ((FUNDEF_BODY (arg_node) != NULL)
        && (arg_node != INFO_PREC_OBJINITFUNDEF (arg_info))) {
        FUNDEF_DFM_BASE (arg_node)
          = DFMUpdateMaskBaseAfterRenaming (FUNDEF_DFM_BASE (arg_node),
                                            FUNDEF_ARGS (arg_node),
                                            FUNDEF_VARDEC (arg_node));
    }

    /* no renaming of objinitfunction */
    if (arg_node != INFO_PREC_OBJINITFUNDEF (arg_info)) {
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
 *       ST_was_reference -> ST_inout
 *
 *   INFO_PREC_CNT_ARTIFICIAL is used to count the number of artificial
 *   return values.
 *
 ******************************************************************************/

node *
PREC2arg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2arg");

    if (ARG_ATTRIB (arg_node) == ST_was_reference) {
        INFO_PREC_CNT_ARTIFICIAL (arg_info)++;
    }

    if (ARG_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        ARG_TYPESTRING (arg_node) = Type2String (ARG_TYPE (arg_node), 2);
        ARG_TYPE (arg_node) = RenameTypes (ARG_TYPE (arg_node));
        /*
         * ARG_TYPESTRING is only used for renaming functions, so the
         * type's actual name may be changed afterwards.
         */

        if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
            ARG_ATTRIB (arg_node) = ST_regular;
        } else {
            if (ARG_ATTRIB (arg_node) == ST_was_reference) {
                ARG_ATTRIB (arg_node) = ST_inout;
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

    LET_IDS (arg_node) = PrecompileIds (LET_IDS (arg_node));
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
 *   parameters is modified to ST_inout.
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
          = PREC2exprs_ap (EXPRS_NEXT (current),
                           ARG_BASETYPE (formal) == T_dots ? formal : ARG_NEXT (formal));
    }

    expr = EXPRS_EXPR (current);

    if (NODE_TYPE (expr) == N_id) {
        if (ID_STATUS (expr) == ST_artificial) {
            current = FreeNode (current);
        } else {
            if (ARG_ATTRIB (formal) == ST_was_reference) {
                ID_ATTRIB (expr) = ST_inout;
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
        if (ARG_STATUS (ID_VARDEC (EXPRS_EXPR (ret_exprs))) == ST_artificial) {
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
 *   node *PREC2id(node *arg_node, node *arg_info)
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
 * function:
 *   node *PREC2Nwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   This function does the renaming of the index vector variable
 *   as well as its scalar counterparts for the new with-loop.
 *
 ******************************************************************************/

node *
PREC2Nwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2Nwithid");

    NWITHID_VEC (arg_node) = PrecompileIds (NWITHID_VEC (arg_node));
    NWITHID_IDS (arg_node) = PrecompileIds (NWITHID_IDS (arg_node));

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

    DO_USEVARS (arg_node) = PrecompileIds (DO_USEVARS (arg_node));
    DO_DEFVARS (arg_node) = PrecompileIds (DO_DEFVARS (arg_node));

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

    WHILE_USEVARS (arg_node) = PrecompileIds (WHILE_USEVARS (arg_node));
    WHILE_DEFVARS (arg_node) = PrecompileIds (WHILE_DEFVARS (arg_node));

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

    COND_THENVARS (arg_node) = PrecompileIds (COND_THENVARS (arg_node));
    COND_ELSEVARS (arg_node) = PrecompileIds (COND_ELSEVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2Nwith2(node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2Nwith2 (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2Nwith2");

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    NWITH2_DEC_RC_IDS (arg_node) = PrecompileIds (NWITH2_DEC_RC_IDS (arg_node));

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
 *   node *PREC2Ncode( node *arg_node, node *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC2Ncode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2Ncode");

    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);

    NCODE_INC_RC_IDS (arg_node) = PrecompileIds (NCODE_INC_RC_IDS (arg_node));

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
 *   node *PREC2WLseg(node *arg_node, node *arg_info)
 *
 * description:
 *   Since the scheduling specification may contain the names of local
 *   identifiers, these have to be renamed according to the general renaming
 *   scheme implemented by this compiler phase.
 *
 ******************************************************************************/

node *
PREC2WLseg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2WLseg");

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (WLSEG_SCHEDULING (arg_node));
    }

    WLSEG_CONTENTS (arg_node) = Trav (WLSEG_CONTENTS (arg_node), arg_info);

    if (WLSEG_NEXT (arg_node) != NULL) {
        WLSEG_NEXT (arg_node) = Trav (WLSEG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2WLsegVar(node *arg_node, node *arg_info)
 *
 * description:
 *   Since the scheduling specification may contain the names of local
 *   identifiers, these have to be renamed according to the general renaming
 *   scheme implemented by this compiler phase.
 *
 ******************************************************************************/

node *
PREC2WLsegVar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("PREC2WLsegVar");

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (WLSEGVAR_SCHEDULING (arg_node));
    }

    WLSEGVAR_CONTENTS (arg_node) = Trav (WLSEGVAR_CONTENTS (arg_node), arg_info);

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = Trav (WLSEGVAR_NEXT (arg_node), arg_info);
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
