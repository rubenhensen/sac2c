/*
 *
 * $Log$
 * Revision 3.90  2004/09/22 18:42:00  ktr
 * Small bugfix.
 *
 * Revision 3.89  2004/09/22 18:11:37  khf
 * moved renaming of cexprs from markmemvals
 * to third traversal of precompile
 *
 * Revision 3.88  2004/09/22 17:38:25  ktr
 * Specialized fold functions are now built if emm and MT are activated.
 *
 * Revision 3.87  2004/08/13 13:50:56  khf
 * substituted NCODE_CEXPR by NCODE_CEXPRS
 *
 * Revision 3.86  2004/08/09 12:26:32  khf
 * PREC1prf: break in default case inserted
 *
 * Revision 3.85  2004/08/08 15:48:59  ktr
 * PREC1prf added in order to remove inc_rc/dec_rc instructions for global
 * objects.
 *
 * Revision 3.84  2004/08/06 13:13:49  ktr
 * MakeMergeAssign does not need to care about RC in EMM
 *
 * Revision 3.83  2004/08/04 12:04:58  ktr
 * substituted eacc by emm
 *
 * Revision 3.82  2004/07/27 12:17:58  khf
 * PREC2apORprf(): args of F_alloc are not traversed
 *
 * Revision 3.81  2004/07/23 15:25:17  khf
 * if explicit accumulation was applied, the generatsted
 * dummy fold-functions is no longer needed
 *
 * Revision 3.80  2004/07/22 14:48:15  ktr
 * markmemvals.h added.
 *
 * Revision 3.79  2004/07/22 14:13:50  ktr
 * - DO_SKIP is traversed by new traversal function PRECdo
 * - Third traversal now lifts constant args from funp
 * funaps correctly (inserts alloc assignment).
 *
 * Revision 3.78  2004/07/21 16:53:43  ktr
 * added markmemval traversal
 *
 * Revision 3.77  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.76  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.75  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.74  2004/02/20 08:25:40  mwe
 * now functions with (MODUL_FUNS) and without (MODUL_FUNDECS) body are separated
 * changed tree traversal according to that
 *
 * Revision 3.73  2003/09/29 19:08:23  dkr
 * PREC2apORprf() corrected
 *
 * Revision 3.72  2003/09/25 10:54:33  dkr
 * to_unq() and from_unq() are prfs now
 *
 * Revision 3.71  2003/06/17 16:56:23  dkr
 * PREC3with() and PREC4with() added.
 * Bug in PREC3withop() fixed.
 *
 * Revision 3.70  2003/04/20 21:19:15  dkr
 * PREC2ap renamed into PREC2apORprf
 * some prf args are flattened as well now
 *
 * Revision 3.69  2003/04/15 12:29:17  dkr
 * bug in MakeMergeAssigns() fixed:
 * assignments in INFO_PREC2_PRE_ASSIGNS are in correct order now
 *
 * Revision 3.68  2003/04/14 14:57:11  sbs
 * comparison partner for strlen result casted to size_t
 *
 * Revision 3.67  2002/10/29 19:49:30  dkr
 * PREC1fundef() modified for new backend
 *
 * Revision 3.66  2002/10/28 19:05:01  dkr
 * type conversion for applications of external functions added
 *
 * Revision 3.65  2002/10/18 16:53:35  dkr
 * support for (..._MOD == EXTERN_MOD_NAME) finished 8-)
 *
 * Revision 3.64  2002/10/18 16:08:04  dkr
 * some DBUG_ASSERTs added
 *
 * Revision 3.63  2002/10/18 15:52:20  dkr
 * support for (..._MOD == EXTERN_MOD_NAME) added
 *
 * Revision 3.62  2002/10/18 13:30:04  sbs
 * ID_ATTRIB replaced by accesses to the FLAGS of N_id
 *
 * Revision 3.61  2002/10/08 01:50:33  dkr
 * ReplaceSpecialCharacters() modified
 *
 * Revision 3.59  2002/09/16 13:29:39  dkr
 * RenameFunName(): individual names for wrapper functions
 *
 * Revision 3.58  2002/09/11 23:09:20  dkr
 * rf_node_info.mac modified.
 *
 * Revision 3.57  2002/09/09 17:50:05  dkr
 * PREC3code(), PREC3withop(): support for N_Nwith added
 *
 * Revision 3.56  2002/08/12 10:31:11  dkr
 * no more cc warnings (hopefully)
 *
 * Revision 3.55  2002/07/31 15:33:54  dkr
 * new hidden tag added
 *
 * Revision 3.54  2002/07/24 18:55:06  dkr
 * new backend: scalar args are flattened during precompile
 * now (in PREC2ap())
 *
 * Revision 3.53  2002/07/24 13:17:30  dkr
 * MUST_REFCOUNT renamed into TYPE_MUST_REFCOUNT
 *
 * Revision 3.52  2002/07/15 19:16:23  dkr
 * PREC3let: modification for new backend done:
 * F_reshape is also lifted now
 *
 * Revision 3.51  2002/07/15 17:24:38  dkr
 * LiftArg() moved to tree_compound.c
 *
 * Revision 3.50  2002/07/15 15:24:59  dkr
 * comment in file header corrected
 *
 * Revision 3.49  2002/07/15 15:24:35  dkr
 * - LiftIds() and LiftArg() modified
 * - bug in PREC2let() fixed: args for T_dots are not lifted
 *
 * Revision 3.48  2002/07/12 17:16:03  dkr
 * RenameId(): ID_NT_TAG is renamed as well
 *
 * Revision 3.47  2002/07/03 17:11:16  dkr
 * - PREC1ap(): assert added
 * - ID_UNQCONV added again #$%-(
 *
 * Revision 3.46  2002/07/03 16:55:29  dkr
 * ID_UNQCONV removed for new backend
 *
 * Revision 3.45  2002/06/27 17:22:32  dkr
 * bug in PREC3code() fixed: INFO_PREC3_CEXPR is handled correctly now
 *
 * Revision 3.44  2002/06/20 15:24:19  dkr
 * signature of AddVardecs modified
 * AddVardecs() moved to tree_compound.[ch]
 *
 * Revision 3.43  2002/06/07 15:50:41  dkr
 * new ATG_... arrays used
 *
 * Revision 3.42  2002/06/06 19:36:26  dkr
 * bug with old backend fixed
 *
 * Revision 3.41  2002/06/06 18:13:19  dkr
 * bug about LiftIds() and LiftArg() corrected
 *
 * Revision 3.40  2002/06/06 12:41:23  dkr
 * LiftIds() for new backend added
 *
 * Revision 3.39  2002/05/31 17:35:52  dkr
 * bug with old backend fixed
 *
 * Revision 3.38  2002/05/31 17:26:14  dkr
 * new argtags for new backend used
 *
 * Revision 3.37  2002/04/17 15:47:36  dkr
 * InsertObjInit() modified:
 * usage of INITGLOBALOBJECT_BEGIN modified in order to ease identifier
 * renaming.
 *
 * Revision 3.36  2002/04/16 21:12:34  dkr
 * renaming of identifiers is done in a separate traversal now
 *
 * Revision 3.35  2002/04/09 16:35:16  dkr
 * break specifier added
 *
 * Revision 3.34  2002/04/03 14:12:36  dkr
 * - some comments added
 * - LiftArg() corrected
 *
 * Revision 3.33  2002/03/07 20:29:21  dkr
 * ICM_END_OF_STATEMENT removed
 *
 * Revision 3.32  2002/03/07 02:22:50  dkr
 * traversals reordered
 *
 * Revision 3.31  2002/03/01 17:20:37  dkr
 * fixed a bug in InsertOut():
 * the C return type is marked correctly now (ST_crettype)
 *
 * Revision 3.30  2002/03/01 03:17:55  dkr
 * third traversal added:
 * reorganisation of fundefs and aps (argument remapping, mapping to
 * C-signature, ...) is done here already and not in compile
 *
 * Revision 3.28  2002/02/20 15:02:47  dkr
 * fundef DupTypes() renamed into DupAllTypes()
 *
 * Revision 3.27  2002/02/06 17:08:53  dkr
 * PREC1let() modified: *all* primitive functions are flattened now
 *
 * [...]
 *
 */

/******************************************************************************
 *
 * This module does some precompilation.
 *
 * It carries out two separate tree traversals.
 *
 * Things done during first traversal:
 *   - Artificial arguments and return values are removed.
 *   - A function with code for object initialization is created.
 *
 * Things done during second traversal:
 *   - Constant arguments of function applications are abstracted out:
 *       a = fun( 1);   =>   _tmp = 1; a = fun( _tmp);
 *     This can be done *after* type-checking only, because this modification
 *     should not be done for (most) primitive functions. (It is superfluous
 *     for most prfs and we should minimize the number of local variables used.)
 *     Unfortunately, only the type system can decide whether an application
 *     uses a primitive or a user-defined function (overloading!).
 *     Moreover, several optimizations (WLUR, IVE, ...) produce unflattened SAC
 *     code for the time being anyway!!!
 *   - Function signatures are transformed into the final form:
 *     At most a single return value, remapping because of a linksign pragma,
 *     parameter tags (in, out, inout, ...).
 *     The reorganized layout is stored in FUNDEF_ARGTAB and AP_ARGTAB
 *     respectively. Note, that the node information of the AST is left as is.
 *   - Arguments of function applications are lifted if formal and actual types
 *     differ:
 *       b = fun( a);   =>   _tmp_a = a; b = fun( _tmp_a);
 *     Return values of function applications are lifted if formal and actual
 *     types differ:
 *       b = fun( a);   =>   _tmp_b = fun( a); b = _tmp_b;
 *
 * Things done during third traversal:
 *   - Arguments of function applications are abstracted out if needed for
 *     generating correct code for reference counting:
 *       a = fun( a)   =>   tmp_a = a; a = fun( tmp_a)
 *     This can only be done *after* type-checking, because types are needed
 *     to decide if abstraction is needed or not, and *after* code optimizations
 *     are finished, because some optimizations (especially CF) might undo these
 *     modifications.
 *   - For each with-loop a unique and adjusted dummy fold-function is
 *     generated.
 *   - It is checked whether NCODE_CEXPR is identical for all N_Ncode nodes
 *     of a single fold-with-loop.
 *
 * Things done during fourth traversal:
 *   - All names and identifiers are renamed in order to avoid name clashes.
 *
 * Remark:
 *   It would be possible to combine the third and fourth traversal, but
 *   separate traversals are more easy to understand and can be swapped
 *   if necessary one day.
 *
 ******************************************************************************/

#define NEW_INFO

#include <string.h>

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "prf.h"
#include "free.h"
#include "DupTree.h"
#include "traverse.h"
#include "convert.h"
#include "DataFlowMask.h"
#include "adjust_ids.h"
#include "typecheck.h"
#include "refcount.h"
#include "NameTuplesUtils.h"
#include "map_cwrapper.h"
#include "scheduling.h"
#include "compile.h"
#include "precompile.h"
#include "markmemvals.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *objinitfundef;
    node *modul;
    node *preassigns;
    node *postassigns;
    node *let;
    node *lastassign;
    node *cexpr;
};

/*
 * INFO macros
 */
#define INFO_PREC_FUNDEF(n) (n->fundef)
#define INFO_PREC1_OBJINITFUNDEF(n) (n->objinitfundef)
#define INFO_PREC1_MODUL(n) (n->modul)
#define INFO_PREC2_PREASSIGNS(n) (n->preassigns)
#define INFO_PREC2_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREC3_LET(n) (n->let)
#define INFO_PREC3_LASTASSIGN(n) (n->lastassign)
#define INFO_PREC3_CEXPR(n) (n->cexpr)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_PREC_FUNDEF (result) = NULL;
    INFO_PREC1_OBJINITFUNDEF (result) = NULL;
    INFO_PREC1_MODUL (result) = NULL;
    INFO_PREC2_PREASSIGNS (result) = NULL;
    INFO_PREC2_POSTASSIGNS (result) = NULL;
    INFO_PREC3_LET (result) = NULL;
    INFO_PREC3_LASTASSIGN (result) = NULL;
    INFO_PREC3_CEXPR (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

#define FUNDEF_NO_DESC(n, idx)                                                           \
    ((FUNDEF_STATUS (n) == ST_Cfun)                                                      \
     && ((FUNDEF_PRAGMA (n) == NULL) || (FUNDEF_REFCOUNTING (n) == NULL)                 \
         || (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) <= (idx))                              \
         || (!(FUNDEF_REFCOUNTING (n)[idx]))))

#define FUNDEF_HAS_LINKSIGN(n, idx)                                                      \
    (((FUNDEF_STATUS (n) == ST_Cfun) && (FUNDEF_PRAGMA (n) != NULL)                      \
      && (FUNDEF_LINKSIGN (n) != NULL)                                                   \
      && (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) > (idx)))                                 \
       ? TRUE                                                                            \
       : FALSE)

#define FUNDEF_GET_LINKSIGN(n, idx, dots)                                                \
    (((FUNDEF_STATUS (n) == ST_Cfun) && (FUNDEF_PRAGMA (n) != NULL)                      \
      && (FUNDEF_LINKSIGN (n) != NULL)                                                   \
      && (PRAGMA_NUMPARAMS (FUNDEF_PRAGMA (n)) > (idx)))                                 \
       ? (FUNDEF_LINKSIGN (fundef))[idx]                                                 \
       : ((dots) ? (idx) : ((idx) + 1)))

/******************************************************************************
 *
 * Function:
 *   node *PRECdo
 *
 * Description:
 *   A special traversal function for do-loops is necessary as the do-loop's
 *   SKIPBLOCK could be placed in node[2] where it belongs
 *
 *****************************************************************************/
node *
PRECdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PRECdo");

    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    if (DO_SKIP (arg_node) != NULL) {
        DO_SKIP (arg_node) = Trav (DO_SKIP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   void LiftIds( ids *ids_arg, node *fundef, types *new_type,
 *                 node **new_assigns)
 *
 * Description:
 *   Lifts the given return value of a function application:
 *    - Generates a new and fresh varname.
 *    - Generates a new vardec and inserts it into the vardec chain of 'fundef'.
 *      If 'new_type' is not NULL, 'new_type' is used as VARDEC_TYPE instead
 *      of IDS_VARDEC(ids_arg).
 *    - Builds a new assignment and inserts it into the assignment chain
 *      'new_assigns'.
 *    - Adjusts the name and vardec of 'ids_arg'.
 *
 ******************************************************************************/

static void
LiftIds (ids *ids_arg, node *fundef, types *new_type, node **new_assigns)
{
    char *new_name;
    node *new_vardec;
    node *new_id;

    DBUG_ENTER ("LiftIds");

    new_name = TmpVarName (IDS_NAME (ids_arg));
    /* Insert vardec for new var */
    if (new_type == NULL) {
        new_type = IDS_TYPE (ids_arg);
    }
    new_vardec = MakeVardec (StringCopy (new_name), DupAllTypes (new_type), NULL);
    fundef = AddVardecs (fundef, new_vardec);

    /*
     * Abstract the found return value out:
     *   A:n = fun( ...);
     *   ... A:n ... A:1 ...    // n references of A
     * is transformed into
     *   __A:1 = fun( ...);
     *   A:n = __A:1;
     *   ... A:n ... A:1 ...    // n references of A
     */
    new_id = MakeId (new_name, NULL, ST_regular);
    ID_VARDEC (new_id) = new_vardec;
    (*new_assigns) = MakeAssign (MakeLet (new_id, DupOneIds (ids_arg)), (*new_assigns));

    IDS_NAME (ids_arg) = Free (IDS_NAME (ids_arg));
    IDS_NAME (ids_arg) = StringCopy (new_name);
    IDS_VARDEC (ids_arg) = new_vardec;

    if (RC_IS_ACTIVE (IDS_REFCNT (ids_arg))) {
        ID_REFCNT (new_id) = IDS_REFCNT (ids_arg) = 1;
    } else if (RC_IS_INACTIVE (IDS_REFCNT (ids_arg))) {
        ID_REFCNT (new_id) = IDS_REFCNT (ids_arg) = RC_INACTIVE;
    } else {
        DBUG_ASSERT (emm, "illegal RC value found!");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *LiftOrReplaceArg( node *arg_id, node *fundef,
 *                           node *new_id, types *new_type,
 *                           node **new_assigns)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
LiftOrReplaceArg (node *arg_id, node *fundef, node *new_id, types *new_type,
                  node **new_assigns)
{
    DBUG_ENTER ("LiftOrReplaceArg");

    if (new_id == NULL) {
        arg_id = LiftArg (arg_id, fundef, new_type, TRUE, new_assigns);
    } else {
        /*
         * temporary var already generated
         * -> just replace the current arg by 'new_id'
         */
        ID_NAME (arg_id) = Free (ID_NAME (arg_id));
        ID_NAME (arg_id) = StringCopy (ID_NAME (new_id));
        ID_VARDEC (arg_id) = ID_VARDEC (new_id);
    }

    DBUG_RETURN (arg_id);
}

/*
 *
 * FIRST TRAVERSAL
 *
 */

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
 *   node *CreateObjInitFundef( node *module, info *arg_info)
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
CreateObjInitFundef (node *module, info *arg_info)
{
    node *fundef;
    node *assign;
    node *returns;

    DBUG_ENTER ("CreateObjInitFundef");

    returns = MakeReturn (NULL);
    assign = MakeAssign (returns, NULL);

    /* create void procedure without args and with empty return in body */
    fundef
      = MakeFundef (ObjInitFunctionName (TRUE), MODUL_NAME (module), MakeTypes1 (T_void),
                    NULL, MakeBlock (assign, NULL), MODUL_FUNS (module));

    FUNDEF_RETURN (fundef) = returns;

    MODUL_FUNS (module) = fundef;

    INFO_PREC1_OBJINITFUNDEF (arg_info) = fundef;

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
 ******************************************************************************/

static node *
InsertObjInit (node *block, node *objdef)
{
    ids *new_ids;
    node *new_id;

    DBUG_ENTER ("InsertObjInit");

    new_ids = MakeIds (StringCopy (OBJDEF_NAME (objdef)), NULL, ST_regular);
    IDS_VARDEC (new_ids) = objdef;
    IDS_ATTRIB (new_ids) = ST_global;
    if (TYPE_MUST_REFCOUNT (OBJDEF_TYPE (objdef))) {
        IDS_REFCNT (new_ids) = 1;
    } else {
        IDS_REFCNT (new_ids) = RC_INACTIVE;
    }

    new_id = MakeId_Copy (OBJDEF_NAME (objdef));
    ID_VARDEC (new_id) = objdef;
    SET_FLAG (ID, new_id, IS_GLOBAL, TRUE);

    BLOCK_INSTR (block)
      = MakeAssignIcm1 ("INITGLOBALOBJECT_BEGIN", new_id,
                        MakeAssign (MakeLet (OBJDEF_EXPR (objdef), new_ids),
                                    MakeAssignIcm0 ("INITGLOBALOBJECT_END",
                                                    BLOCK_INSTR (block))));

    OBJDEF_EXPR (objdef) = NULL;

    DBUG_RETURN (block);
}

/******************************************************************************
 *
 * function:
 *   node *PREC1modul( node *arg_node, info *arg_info)
 *
 * description:
 *   Creates fundef for object-initialization code.
 *
 ******************************************************************************/

node *
PREC1modul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1modul");

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

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1objdef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC1objdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1objdef");

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    /* insert init code */
    FUNDEF_BODY (INFO_PREC1_OBJINITFUNDEF (arg_info))
      = InsertObjInit (FUNDEF_BODY (INFO_PREC1_OBJINITFUNDEF (arg_info)), arg_node);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC1fundef(node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC1fundef (node *arg_node, info *arg_info)
{
    types *ret_types;
    char *keep_name, *keep_mod;
    statustype keep_status, keep_attrib;

    DBUG_ENTER ("PREC1fundef");

    /*
     * unset inline flag
     */
    FUNDEF_INLINE (arg_node) = FALSE;

    /*
     * The function body is traversed in order to remove artificial return
     * values and parameters of function applications.
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        DBUG_ASSERT (((FUNDEF_RETURN (arg_node) != NULL)
                      && (NODE_TYPE (FUNDEF_RETURN (arg_node)) == N_return)),
                     "N_fundef node has no reference to N_return node");

        /*
         * The reference checked above is actually not needed by the
         * precompiler. This is done to check consistency of the syntax
         * tree for further compilation steps.
         */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    } else {
        DBUG_ASSERT ((FUNDEF_DFM_BASE (arg_node) == NULL),
                     "FUNDEF_DFM_BASE without body found!");
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

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC1arg( node *arg_node, info *arg_info)
 *
 * description:
 *   An artificial argument is removed, the attribs are switched:
 *       ST_readonly_reference -> ST_regular
 *       ST_was_reference -> ST_reference
 *
 ******************************************************************************/

node *
PREC1arg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1arg");

    if (ARG_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        if (ARG_ATTRIB (arg_node) == ST_readonly_reference) {
            ARG_ATTRIB (arg_node) = ST_regular;
        } else {
            if (ARG_ATTRIB (arg_node) == ST_was_reference) {
                ARG_ATTRIB (arg_node) = ST_reference;
            }
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
 *   node *PREC1vardec( node *arg_node, info *arg_info)
 *
 * Description:
 *   Removes artificial variable declarations.
 *
 ******************************************************************************/

node *
PREC1vardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1vardec");

    if (VARDEC_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeNode (arg_node);

        if (arg_node != NULL) {
            arg_node = Trav (arg_node, arg_info);
        }
    } else {
        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC1assign( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC1assign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1assign");

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
 *   node *PREC1let( node *arg_node, info *arg_info)
 *
 * description:
 *   Removes all artificial identifiers on the left hand side of a let.
 *
 ******************************************************************************/

node *
PREC1let (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1let");

    LET_IDS (arg_node) = RemoveArtificialIds (LET_IDS (arg_node), LET_EXPR (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_EXPR (arg_node) == NULL) {
        arg_node = FreeTree (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1icm( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC1icm (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1icm");

    /*
     * content of ICMs is not traversed!!
     */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1exprs_ap( node *current, node *formal)
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
PREC1exprs_ap (node *current, node *formal)
{
    node *expr;

    DBUG_ENTER ("PREC1exprs_ap");

    if (EXPRS_NEXT (current) != NULL) {
        EXPRS_NEXT (current)
          = PREC1exprs_ap (EXPRS_NEXT (current), (ARG_BASETYPE (formal) == T_dots)
                                                   ? formal
                                                   : ARG_NEXT (formal));
    }

    expr = EXPRS_EXPR (current);

    if (NODE_TYPE (expr) == N_id) {
        if (ID_STATUS (expr) == ST_artificial) {
            current = FreeNode (current);
        } else {
            if (ARG_ATTRIB (formal) == ST_was_reference) {
                SET_FLAG (ID, expr, IS_REFERENCE, TRUE);
                SET_FLAG (ID, expr, IS_READ_ONLY, FALSE);
            }

            if ((VARDEC_OR_ARG_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
                ID_VARDEC (expr) = VARDEC_OR_ARG_OBJDEF (ID_VARDEC (expr));
            }
        }
    }

    DBUG_RETURN (current);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1exprs_prf( node *current, node *formal)
 *
 * Description:
 *   Removes all artificial parameters.
 *
 ******************************************************************************/

static node *
PREC1exprs_prf (node *current)
{
    node *expr;

    DBUG_ENTER ("PREC1exprs_ap");

    if (EXPRS_NEXT (current) != NULL) {
        EXPRS_NEXT (current) = PREC1exprs_prf (EXPRS_NEXT (current));
    }

    expr = EXPRS_EXPR (current);

    if (NODE_TYPE (expr) == N_id) {
        if ((ID_STATUS (expr) == ST_artificial)
            && (VARDEC_OR_ARG_STATUS (ID_VARDEC (expr)) == ST_artificial)) {
            current = FreeNode (current);
        }
    }

    DBUG_RETURN (current);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1exprs_return( node *ret_exprs, node *ret_node)
 *
 * Description:
 *   Removes all artificial return values from the chain.
 *   A new chain is built up for all those return values which belong to
 *   original reference parameters. These are stored in RETURN_REFERENCE for
 *   later use in compile.c.
 *
 ******************************************************************************/

static node *
PREC1exprs_return (node *ret_exprs, node *ret_node)
{
    node *tmp;

    DBUG_ENTER ("PREC1exprs_return");

    if (EXPRS_NEXT (ret_exprs) != NULL) {
        EXPRS_NEXT (ret_exprs) = PREC1exprs_return (EXPRS_NEXT (ret_exprs), ret_node);
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
            EXPRS_NEXT (tmp) = RETURN_REFERENCE (ret_node);
            RETURN_REFERENCE (ret_node) = tmp;
        }
    }

    DBUG_RETURN (ret_exprs);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1ap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses the current arguments using function PREC1exprs_ap that is given
 *   a pointer to the first formal parameter of the applied function.
 *
 ******************************************************************************/

node *
PREC1ap (node *arg_node, info *arg_info)
{
    node *arg;
    node *ret_node;

    DBUG_ENTER ("PREC1ap");

    if (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_classfun) {
        arg = EXPRS_EXPR (AP_ARGS (arg_node));

        if (NODE_TYPE (arg) == N_id) {
            if (!strncmp (AP_NAME (arg_node), "to_", 3)) {
                DBUG_ASSERT ((!IsUnique (ID_TYPE (arg))),
                             "Argument of to_class function is unique already!");

                ret_node = MakePrf (F_to_unq, AP_ARGS (arg_node));
                AP_ARGS (arg_node) = NULL;
            } else {
                /*
                 * This must be a "from" function. So, the argument is of a class
                 * type which implies that it is an identifier.
                 */
                DBUG_ASSERT ((IsUnique (ID_TYPE (arg))),
                             "Argument of from_class function not unique!");

                ret_node = MakePrf (F_from_unq, AP_ARGS (arg_node));
                AP_ARGS (arg_node) = NULL;
            }
        } else {
            /* argument of class conversion function is no N_id node */

            /*
             * -> argument is a scalar value
             * -> simply remove the conversion function
             */
            ret_node = arg;
            EXPRS_EXPR (AP_ARGS (arg_node)) = NULL;
        }

        arg_node = Free (arg_node);
        arg_node = ret_node;
    } else {
        if (AP_ARGS (arg_node) != NULL) {
            AP_ARGS (arg_node)
              = PREC1exprs_ap (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1prf( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses the prf arguments using function PREC1exprs_prf iff
 *   this prf is a inc_rc/dec_rc prf.
 *
 ******************************************************************************/

node *
PREC1prf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1prf");

    switch (PRF_PRF (arg_node)) {
    case F_inc_rc:
        PRF_ARGS (arg_node) = PREC1exprs_prf (PRF_ARGS (arg_node));
        if (CountExprs (PRF_ARGS (arg_node)) == 1) {
            arg_node = FreeTree (arg_node);
        }
        break;

    case F_dec_rc:
        PRF_ARGS (arg_node) = PREC1exprs_prf (PRF_ARGS (arg_node));
        if (PRF_ARGS (arg_node) == NULL) {
            arg_node = FreeTree (arg_node);
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1return( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses the return values using function PREC1exprs_return.
 *
 ******************************************************************************/

node *
PREC1return (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1return");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = PREC1exprs_return (RETURN_EXPRS (arg_node), arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC1id( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC1id (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC1id");

    if (ID_STATUS (arg_node) == ST_artificial) {
        arg_node = FreeTree (arg_node);
    } else {
        DBUG_ASSERT ((ID_VARDEC (arg_node) != NULL), "no ID_VARDEC found!");

        if (VARDEC_OR_ARG_STATUS (ID_VARDEC (arg_node)) == ST_artificial) {
            ID_VARDEC (arg_node) = VARDEC_OR_ARG_OBJDEF (ID_VARDEC (arg_node));
        }
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
 * Function:
 *   argtab_t *CompressArgtab( argtab_t *argtab)
 *
 * Description:
 *   Empty entries in 'argtab' are moved to the end of the table.
 *
 ******************************************************************************/

static argtab_t *
CompressArgtab (argtab_t *argtab)
{
    int pos, idx;
    int old_size;

    DBUG_ENTER ("CompressArgtab");

    pos = idx = 1;
    while (pos < argtab->size) {
        if (argtab->tag[pos] != ATG_notag) {
            if (pos > idx) {
                argtab->tag[idx] = argtab->tag[pos];
                argtab->ptr_in[idx] = argtab->ptr_in[pos];
                argtab->ptr_out[idx] = argtab->ptr_out[pos];
            }
            idx++;
        } else {
            DBUG_ASSERT ((argtab->ptr_in[pos] == NULL), "argtab inconsistent");
            DBUG_ASSERT ((argtab->ptr_out[pos] == NULL), "argtab inconsistent");
        }
        pos++;
    }
    /* adjust size */
    old_size = argtab->size;
    argtab->size = idx;

    /* clear superfluous entries */
    for (; idx < old_size; idx++) {
        argtab->tag[idx] = ATG_notag;
        argtab->ptr_in[idx] = NULL;
        argtab->ptr_out[idx] = NULL;
    }

    DBUG_RETURN (argtab);
}

/******************************************************************************
 *
 * function:
 *   node *PREC2modul( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC2modul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC2modul");

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *InsertOut( argtab_t argtab, node *fundef,
 *                        int param_id, types *rettype,
 *                        bool *dots, node *ret)
 *
 * Description:
 *   Inserts an out-argument into the argtab.
 *
 ******************************************************************************/

static argtab_t *
InsertOut (argtab_t *argtab, node *fundef, int param_id, types *rettype, bool *dots,
           node *ret)
{
    argtag_t argtag;
    int idx;
    int line;

    DBUG_ENTER ("InsertOut");

    line = NODE_LINE (fundef);

    idx = FUNDEF_GET_LINKSIGN (fundef, param_id, *dots);
    if (TYPES_BASETYPE (rettype) != T_dots) {
        if (FUNDEF_NO_DESC (fundef, param_id)) {
            argtag = ATG_out_nodesc;
        } else {
            argtag = ATG_out;

            if (idx == 0) {
                ERROR (line, ("Pragma 'linksign' or 'refcounting' illegal"));
                CONT_ERROR (("Return value must not use a descriptor"));
            }
        }

        if ((argtab->ptr_out[0] == NULL) && (argtag == ATG_out_nodesc)
            && ((FUNDEF_PRAGMA (fundef) == NULL) || (FUNDEF_LINKSIGN (fundef) == NULL))) {
            node *ret_exprs;
            int i;

            /*
             * no linksign pragma given and no C return value found yet?
             *  -> use this out-param as C return value!
             */
            idx = 0;

            /*
             * set RETURN_CRET(ret)
             */
            if (ret != NULL) {
                DBUG_ASSERT ((NODE_TYPE (ret) == N_return), "no N_return node found!");

                ret_exprs = RETURN_EXPRS (ret);
                for (i = 0; i < param_id; i++) {
                    DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");
                    ret_exprs = EXPRS_NEXT (ret_exprs);
                }
                DBUG_ASSERT ((ret_exprs != NULL), "not enough return values found!");

                RETURN_CRET (ret) = ret_exprs;
            }
        }
    } else {
        DBUG_ASSERT (((*dots) == FALSE), "more than one T_dots parameter found");

        if ((idx != argtab->size - 1) && FUNDEF_HAS_LINKSIGN (fundef, param_id)) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Parameter '...' must be mapped to the last position"));
        }
        (*dots) = TRUE;

        idx = argtab->size - 1;
        argtag = FUNDEF_NO_DESC (fundef, param_id) ? ATG_out_nodesc : ATG_out;
    }

    if (idx == 0) {
        /* mark the C return value */
        TYPES_STATUS (rettype) = ST_crettype;
    }

    if ((idx >= 0) && (idx < argtab->size)) {
        DBUG_ASSERT ((argtab->ptr_in[idx] == NULL), "argtab is inconsistent");

        if (argtab->tag[idx] == ATG_notag) {
            DBUG_ASSERT ((argtab->ptr_out[idx] == NULL), "argtab is inconsistent");

            argtab->ptr_out[idx] = rettype;
            argtab->tag[idx] = argtag;

            DBUG_PRINT ("PREC", ("%s(): out-arg " F_PTR
                                 " (TYPE) inserted at position %d with tag %s.",
                                 FUNDEF_NAME (fundef), rettype, idx, ATG_string[argtag]));
        } else if (idx == 0) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Return value found twice"));
        } else {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Out-parameter at position %d found twice", idx));
        }
    } else {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Entry at position %d contains illegal value %d", param_id, idx));
    }

    DBUG_RETURN (argtab);
}

/******************************************************************************
 *
 * Function:
 *   argtab_t *InsertIn( argtab_t *argtab, node *fundef,
 *                       int param_id, node *arg,
 *                       bool *dots)
 *
 * Description:
 *   Inserts an in-argument into the argtab.
 *
 ******************************************************************************/

static argtab_t *
InsertIn (argtab_t *argtab, node *fundef, int param_id, node *arg, bool *dots)
{
    argtag_t argtag;
    int idx;
    int line;

    DBUG_ENTER ("InsertIn");

    line = NODE_LINE (fundef);

    idx = FUNDEF_GET_LINKSIGN (fundef, param_id, *dots);
    if (ARG_BASETYPE (arg) != T_dots) {
        if (FUNDEF_NO_DESC (fundef, param_id)) {
            if (ARG_ATTRIB (arg) == ST_reference) {
                if ((FUNDEF_STATUS (fundef) == ST_Cfun) && (IsBoxed (ARG_TYPE (arg)))) {
                    argtag = ATG_inout_nodesc_bx;
                } else {
                    argtag = ATG_inout_nodesc;
                }
            } else {
                argtag = ATG_in_nodesc;
            }
        } else {
            argtag = (ARG_ATTRIB (arg) == ST_reference) ? ATG_inout : ATG_in;
        }
    } else {
        DBUG_ASSERT (((*dots) == FALSE), "more than one T_dots parameter found");

        if ((idx != argtab->size - 2) && FUNDEF_HAS_LINKSIGN (fundef, param_id)) {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("Parameter '...' must be mapped to the last position"));
        }
        (*dots) = TRUE;

        idx = argtab->size - 1;
        argtag = FUNDEF_NO_DESC (fundef, param_id) ? ATG_in_nodesc : ATG_in;
    }

    if (idx == 0) {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (
          ("In-parameter at position %d cannot be used as return value", param_id));
    } else if ((idx > 0) && (idx < argtab->size)) {
        if (argtab->ptr_in[idx] == NULL) {
            if (argtab->ptr_out[idx] == NULL) {
                DBUG_ASSERT ((argtab->tag[idx] == ATG_notag), "argtab is inconsistent");

                argtab->ptr_in[idx] = arg;
                argtab->tag[idx] = argtag;

                DBUG_PRINT ("PREC", ("%s(): in-arg " F_PTR "," F_PTR
                                     " (ARG,TYPE) inserted at position %d with tag %s.",
                                     FUNDEF_NAME (fundef), arg, ARG_TYPE (arg), idx,
                                     ATG_string[argtag]));
            } else if ((argtab->tag[idx] == ATG_out_nodesc)
                       && (argtag == ATG_in_nodesc)) {
                /*
                 * merge 'argtab->ptr_out[idx]' and 'arg'
                 */
                if (CmpTypes (argtab->ptr_out[idx], ARG_TYPE (arg)) == CMP_equal) {
                    argtag
                      = IsBoxed (ARG_TYPE (arg)) ? ATG_inout_nodesc_bx : ATG_inout_nodesc;
                    argtab->ptr_in[idx] = arg;
                    argtab->tag[idx] = argtag;

                    DBUG_PRINT ("PREC", ("%s(): in-arg " F_PTR "," F_PTR
                                         " (ARG,TYPE) merged with out-arg " F_PTR
                                         " (TYPE) at position %d with tag %s.",
                                         FUNDEF_NAME (fundef), arg, ARG_TYPE (arg),
                                         argtab->ptr_out[idx], idx, ATG_string[argtag]));
                } else {
                    ERROR (line, ("Pragma 'linksign' illegal"));
                    CONT_ERROR (("Mappings allowed exclusively between parameters"
                                 " with identical types"));
                }
            } else {
                ERROR (line, ("Pragma 'linksign' illegal"));
                CONT_ERROR (("Mappings allowed exclusively between parameters"
                             " without descriptor"));
            }
        } else {
            ERROR (line, ("Pragma 'linksign' illegal"));
            CONT_ERROR (("In-parameter at position %d found twice", idx));
        }
    } else {
        ERROR (line, ("Pragma 'linksign' illegal"));
        CONT_ERROR (("Entry at position %d contains illegal value %d", param_id, idx));
    }

    DBUG_RETURN (argtab);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2fundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds FUNDEF_ARGTAB.
 *
 ******************************************************************************/

node *
PREC2fundef (node *arg_node, info *arg_info)
{
    types *rettypes;
    node *args;
    argtab_t *argtab;
    int param_id;
    bool dots;

    DBUG_ENTER ("PREC2fundef");

    if (FUNDEF_STATUS (arg_node) != ST_zombiefun) {
        rettypes = FUNDEF_TYPES (arg_node);
        args = FUNDEF_ARGS (arg_node);
        param_id = 0;
        dots = FALSE;
        argtab = MakeArgtab (CountFunctionParams (arg_node) + 1);

        while (rettypes != NULL) {
            if (TYPES_BASETYPE (rettypes) != T_void) {
                argtab = InsertOut (argtab, arg_node, param_id, rettypes, &dots,
                                    FUNDEF_RETURN (arg_node));
                param_id++;
            }

            rettypes = TYPES_NEXT (rettypes);
        }

        while (args != NULL) {
            argtab = InsertIn (argtab, arg_node, param_id, args, &dots);
            param_id++;

            args = ARG_NEXT (args);
        }

        ABORT_ON_ERROR;
        FUNDEF_ARGTAB (arg_node) = CompressArgtab (argtab);

        /*
         * traverse next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }

        INFO_PREC_FUNDEF (arg_info) = arg_node;

        /*
         * all FUNDEF_ARGTABs are build now
         *  -> traverse body
         */
        INFO_PREC2_POSTASSIGNS (arg_info) = NULL;
        INFO_PREC2_PREASSIGNS (arg_info) = NULL;
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }

        /*
         * FUNDEF_PRAGMA is still needed during the last traversal!!!
         *  -> it is removed later
         */
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2assign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Inserts the assignments found in INFO_PREC2_PRE/POSTASSIGNS into the AST.
 *
 ******************************************************************************/

node *
PREC2assign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC2assign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREC2_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = AppendAssign (INFO_PREC2_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_PREC2_POSTASSIGNS (arg_info) = NULL;
    }
    if (INFO_PREC2_PREASSIGNS (arg_info) != NULL) {
        arg_node = AppendAssign (INFO_PREC2_PREASSIGNS (arg_info), arg_node);
        INFO_PREC2_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *MakeMergeAssigns( argtab_t *argtab, info *arg_info)
 *
 * Description:
 *   Builds merging assignments for inout-arguments if needed and stores them
 *   in 'arg_info'.
 *
 ******************************************************************************/

static info *
MakeMergeAssigns (argtab_t *argtab, info *arg_info)
{
    node *expr;
    int i;
    node *pre_assigns = INFO_PREC2_PREASSIGNS (arg_info);
    node *post_assigns = INFO_PREC2_POSTASSIGNS (arg_info);

    DBUG_ENTER ("MakeMergeAssigns");

    DBUG_ASSERT ((argtab != NULL), "no argtab found!");

    for (i = argtab->size - 1; i >= 1; i--) {
        if ((argtab->ptr_out[i] != NULL) && (argtab->ptr_in[i] != NULL)) {
            DBUG_ASSERT ((NODE_TYPE (argtab->ptr_in[i]) == N_exprs),
                         "no N_exprs node found in argtab!");
            expr = EXPRS_EXPR (argtab->ptr_in[i]);

            if ((NODE_TYPE (expr) != N_id)
                || strcmp (IDS_NAME (((ids *)argtab->ptr_out[i])), ID_NAME (expr))) {
                if (emm) {
                    ids *out_ids;

                    DBUG_ASSERT (NODE_TYPE (expr) == N_id,
                                 "Non N_id function argument found!!!");
                    /*
                     * argument is a refcounted ID node
                     *  -> must be converted into a unique object
                     *
                     ******
                     *
                     *  a = fun( b);   ->   a' = to_unq( b);
                     *                           fun( a');
                     *                      a  = from_unq( a');
                     */

                    /* a' */
                    out_ids = DupOneIds (argtab->ptr_out[i]);

                    /* to_unq( b) */
                    expr = MakePrf (F_to_unq, MakeExprs (DupTree (expr), NULL));

                    /* a = to_unq( b); */
                    /*
                     * append at tail of 'pre_assigns'!!!
                     */
                    pre_assigns
                      = AppendAssign (pre_assigns,
                                      MakeAssign (MakeLet (expr, out_ids), NULL)),

                      /* a */
                      out_ids = DupOneIds (argtab->ptr_out[i]);

                    /* from_unq( a) */
                    expr = DupIds_Id (argtab->ptr_out[i]);
                    expr = MakePrf (F_from_unq, MakeExprs (expr, NULL));

                    /*
                     * append at head of 'post_assigns'!!!
                     */
                    post_assigns = MakeAssign (MakeLet (expr, out_ids), post_assigns);

                    DBUG_PRINT ("PREC", ("Assignments %s = to_unq(...) added",
                                         IDS_NAME (((ids *)argtab->ptr_out[i]))));
                } else {
                    if ((NODE_TYPE (expr) == N_id) && RC_IS_ACTIVE (ID_REFCNT (expr))) {
                        /*
                         * argument is a refcounted ID node
                         *  -> must be converted into a unique object
                         *
                         ******
                         *
                         *  a:4 = fun( b:3);   ->   a':-1 = to_unq( b:3);
                         *                          fun( a':-1);
                         *                          a:4 = from_unq( a':-1);
                         */
                        ids *out_ids;

                        /* a:-1 */
                        out_ids = DupOneIds (argtab->ptr_out[i]);
                        IDS_REFCNT (out_ids) = 1 /* RC_INACTIVE */;

                        /* to_unq( b:3) */
                        expr = MakePrf (F_to_unq, MakeExprs (DupTree (expr), NULL));

                        /* a:-1 = to_unq( b:3); */
                        /*
                         * append at tail of 'pre_assigns'!!!
                         */
                        pre_assigns
                          = AppendAssign (pre_assigns,
                                          MakeAssign (MakeLet (expr, out_ids), NULL)),

                          /* a:4 */
                          out_ids = DupOneIds (argtab->ptr_out[i]);

                        /* from_unq( a:-1) */
                        expr = DupIds_Id (argtab->ptr_out[i]);
                        ID_REFCNT (expr) = RC_INACTIVE;
                        expr = MakePrf (F_from_unq, MakeExprs (expr, NULL));

                        /*
                         * append at head of 'post_assigns'!!!
                         */
                        post_assigns = MakeAssign (MakeLet (expr, out_ids), post_assigns);

                        DBUG_PRINT ("PREC", ("Assignments %s = to_unq(...) added",
                                             IDS_NAME (((ids *)argtab->ptr_out[i]))));
                    } else {
                        /*
                         * argument is no ID node or not refcounted
                         *  -> no conversion into a unique object is necessary
                         *  -> create a plain assignment
                         *
                         ******
                         *
                         *  a = fun( b);   ->   a = b;
                         *                      fun( a);
                         */
                        pre_assigns
                          = MakeAssign (MakeLet (DupTree (expr),
                                                 DupOneIds (argtab->ptr_out[i])),
                                        pre_assigns);

                        DBUG_PRINT ("PREC", ("Assignment %s = ... added",
                                             IDS_NAME (((ids *)argtab->ptr_out[i]))));
                    }
                }
            }
        }
    }

    INFO_PREC2_PREASSIGNS (arg_info) = pre_assigns;
    INFO_PREC2_POSTASSIGNS (arg_info) = post_assigns;

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2let( node *arg_node, info *arg_info)
 *
 * Description:
 *   Builds AP_ARGTAB and generates merging assignments for inout-arguments
 *   if needed.
 *
 ******************************************************************************/

node *
PREC2let (node *arg_node, info *arg_info)
{
    node *ap, *fundef;
    argtab_t *ap_argtab, *argtab;
    ids *ap_ids;
    types *rettypes;
    node *ap_exprs;
    node *args;
    int idx, dots_off;
    node *ap_id;
    shape_class_t actual_cls, formal_cls;

    DBUG_ENTER ("PREC2let");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    ap = LET_EXPR (arg_node);
    if (NODE_TYPE (ap) == N_ap) {
        fundef = AP_FUNDEF (ap);
        DBUG_ASSERT ((fundef != NULL), "AP_FUNDEF not found!");

        DBUG_PRINT ("PREC", ("Application of %s().", FUNDEF_NAME (fundef)));

        ap_ids = LET_IDS (arg_node);
        rettypes = FUNDEF_TYPES (fundef);
        ap_exprs = AP_ARGS (ap);
        args = FUNDEF_ARGS (fundef);

        ap_argtab = MakeArgtab (CountIds (ap_ids) + CountExprs (ap_exprs) + 1);
        argtab = FUNDEF_ARGTAB (fundef);
        DBUG_ASSERT ((argtab != NULL), "FUNDEF_ARGTAB not found!");

        dots_off = 0;
        idx = ap_argtab->size; /* to avoid a CC warning */
        while (ap_ids != NULL) {
            DBUG_ASSERT ((rettypes != NULL), "application is inconsistant");

            if (dots_off == 0) {
                idx = GetArgtabIndexOut (rettypes, argtab);
            }
            DBUG_ASSERT ((idx + dots_off < ap_argtab->size), "illegal index");
            DBUG_ASSERT ((idx < argtab->size), "illegal index");
            ap_argtab->ptr_out[idx + dots_off] = ap_ids;
            ap_argtab->tag[idx + dots_off] = argtab->tag[idx];

            if (TYPES_BASETYPE (rettypes) != T_dots) {
                actual_cls = GetShapeClassFromTypes (IDS_TYPE (ap_ids));
                formal_cls = GetShapeClassFromTypes (rettypes);
                if ((actual_cls != formal_cls)
                    && (ATG_has_shp[argtab->tag[idx]] || (actual_cls == C_scl)
                        || (formal_cls == C_scl))) {
                    DBUG_PRINT ("PREC",
                                ("Return value with inappropriate shape class found:"));
                    DBUG_PRINT ("PREC", ("   ... %s ... = %s( ... ), %s instead of %s",
                                         FUNDEF_NAME (fundef), IDS_NAME (ap_ids),
                                         nt_shape_string[actual_cls],
                                         nt_shape_string[formal_cls]));
                    LiftIds (ap_ids, INFO_PREC_FUNDEF (arg_info), rettypes,
                             &(INFO_PREC2_POSTASSIGNS (arg_info)));
                }
            }

            ap_ids = IDS_NEXT (ap_ids);
            if (TYPES_BASETYPE (rettypes) != T_dots) {
                rettypes = TYPES_NEXT (rettypes);
            } else {
                dots_off++;
            }
        }

        dots_off = 0;
        idx = ap_argtab->size; /* to avoid a CC warning */
        while (ap_exprs != NULL) {
            DBUG_ASSERT ((args != NULL), "application is inconsistant");

            if (dots_off == 0) {
                idx = GetArgtabIndexIn (ARG_TYPE (args), argtab);
            }
            DBUG_ASSERT ((idx + dots_off < ap_argtab->size), "illegal index");
            DBUG_ASSERT ((idx < argtab->size), "illegal index");
            ap_argtab->ptr_in[idx + dots_off] = ap_exprs;
            ap_argtab->tag[idx + dots_off] = argtab->tag[idx];

            ap_id = EXPRS_EXPR (ap_exprs);

            DBUG_ASSERT ((NODE_TYPE (ap_id) == N_id), "no N_id node found!");
            if (ARG_BASETYPE (args) != T_dots) {
                formal_cls = GetShapeClassFromTypes (ARG_TYPE (args));
                actual_cls = GetShapeClassFromTypes (ID_TYPE (ap_id));
                if ((actual_cls != formal_cls)
                    && (ATG_has_shp[argtab->tag[idx]] || (actual_cls == C_scl)
                        || (formal_cls == C_scl))) {
                    DBUG_PRINT ("PREC",
                                ("Argument with inappropriate shape class found:"));
                    DBUG_PRINT ("PREC", ("   ... = %s( ... %s ...), %s instead of %s",
                                         FUNDEF_NAME (fundef), ID_NAME (ap_id),
                                         nt_shape_string[actual_cls],
                                         nt_shape_string[formal_cls]));
                    EXPRS_EXPR (ap_exprs)
                      = LiftArg (ap_id, INFO_PREC_FUNDEF (arg_info), ARG_TYPE (args),
                                 TRUE, &(INFO_PREC2_PREASSIGNS (arg_info)));
                }
            }

            ap_exprs = EXPRS_NEXT (ap_exprs);
            if (ARG_BASETYPE (args) != T_dots) {
                args = ARG_NEXT (args);
            } else {
                dots_off++;
            }
        }

        ABORT_ON_ERROR;
        AP_ARGTAB (ap) = CompressArgtab (ap_argtab);

        /*
         * builds merging assignments if needed and stores them in 'arg_info'
         */
        arg_info = MakeMergeAssigns (ap_argtab, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC2apORprf( node *arg_node, info *arg_info)
 *
 * Description:
 *   Lifts scalars from all N_ap and some N_prf applications.
 *
 ******************************************************************************/

node *
PREC2apORprf (node *arg_node, info *arg_info)
{
    prf *prf;
    node *args, *arg;
    int arg_cnt;
    types *type;

    DBUG_ENTER ("PREC2apORprf");

    /*
     * prf F_alloc is only pseudo syntax which will be eliminated in compile.c.
     * It's args must not be traversed.
     */
    if ((PRF_PRF (arg_node) != F_alloc) && (AP_OR_PRF_ARGS (arg_node) != NULL)) {
        L_AP_OR_PRF_ARGS (arg_node, Trav (AP_OR_PRF_ARGS (arg_node), arg_info));
    }

    if (NODE_TYPE (arg_node) == N_ap) {
        prf = NULL;
    } else {
        prf = &(PRF_PRF (arg_node));
    }

    args = AP_OR_PRF_ARGS (arg_node);
    arg_cnt = 0;
    while (args != NULL) {
        arg = EXPRS_EXPR (args);
        if ((NODE_TYPE (arg) != N_id) && (NODE_TYPE (arg) != N_array)) {
            if ((prf == NULL) ||                            /* all N_ap args */
                (*prf == F_dim) ||                          /* F_dim arg */
                (*prf == F_shape) ||                        /* F_shape arg */
                ((*prf == F_modarray) && (arg_cnt == 0))) { /* 1st F_modarray arg */
                switch (NODE_TYPE (arg)) {
                case N_num:
                    type = MakeTypes1 (T_int);
                    break;
                case N_float:
                    type = MakeTypes1 (T_float);
                    break;
                case N_double:
                    type = MakeTypes1 (T_double);
                    break;
                case N_bool:
                    type = MakeTypes1 (T_bool);
                    break;
                case N_char:
                    type = MakeTypes1 (T_char);
                    break;
                default:
                    DBUG_ASSERT ((0), "illegal node type found!");
                    type = NULL;
                    break;
                }

                EXPRS_EXPR (args) = LiftArg (arg, INFO_PREC_FUNDEF (arg_info), type, TRUE,
                                             &(INFO_PREC2_PREASSIGNS (arg_info)));
            }
        }
        args = EXPRS_NEXT (args);
        arg_cnt++;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 * THIRD TRAVERSAL
 *
 */

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
    DBUG_ASSERT ((acc != NULL), "no ids found!");
    DBUG_ASSERT ((cexpr != NULL), "no cexpr found!");

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

    /*
     * create DFM base
     */
    if (FUNDEF_DFM_BASE (fundef) == NULL) {
        FUNDEF_DFM_BASE (fundef)
          = DFMGenMaskBase (FUNDEF_ARGS (fundef), FUNDEF_VARDEC (fundef));
    }

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC3fundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC3fundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC3fundef");

    INFO_PREC_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        /*
         * The result of Trav() is NOT assigned to FUNDEF_NEXT, because this
         * pointer might be modified by PREC3let() ...
         */
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC3block( node *arg_node, info *arg_info)
 *
 * Description:
 *   Saves and restores INFO_PREC3_LASTASSIGN.
 *
 ******************************************************************************/

node *
PREC3block (node *arg_node, info *arg_info)
{
    node *old_lastassign;

    DBUG_ENTER ("PREC3block");

    old_lastassign = INFO_PREC3_LASTASSIGN (arg_info);

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    INFO_PREC3_LASTASSIGN (arg_info) = old_lastassign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC3assign( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC3assign (node *arg_node, info *arg_info)
{
    node *return_node;

    DBUG_ENTER ("PREC3assign");

    INFO_PREC3_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /*
     * Newly inserted assignments are prepanded in front of
     * INFO_PREC3_LASTASSIGN.
     * In order to insert these nodes properly, that pointer has to be returned!
     */
    return_node = INFO_PREC3_LASTASSIGN (arg_info);

    if (ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (return_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC3let( node *arg_node, info *arg_info)
 *
 * description:
 *   For each id from the LHS:
 *     If we have   ... a ... = fun( ... a ... a ... )   ,
 *     where   fun   is a user-defined function
 *       and   a   is a regular argument representing a refcounted data object,
 *       and the refcounting is *not* done by the function itself,
 *     or   fun   is a primitive function
 *       and   a   is a regular argument representing a refcounted data object,
 *     then we rename each RHS   a   into a temp-var   __tmp<n>   and insert
 *     an assignment   __tmp<n> = a;   in front of the function application.
 *
 ******************************************************************************/

node *
PREC3let (node *arg_node, info *arg_info)
{
    node *old_let;
    node *let_expr;
    node *arg, *arg_id;
    ids *let_ids;
    node *new_id;
    int arg_idx;

    DBUG_ENTER ("PREC3let");

    old_let = INFO_PREC3_LET (arg_info);
    INFO_PREC3_LET (arg_info) = arg_node;

    let_expr = LET_EXPR (arg_node);

    /*
     * N_prf
     */
    if (NODE_TYPE (let_expr) == N_prf) {
        let_ids = LET_IDS (arg_node);
        while (let_ids != NULL) {
            arg = PRF_ARGS (let_expr);
            arg_idx = 0;
            new_id = NULL;
            while (arg != NULL) {
                arg_id = EXPRS_EXPR (arg);
                if ((NODE_TYPE (arg_id) == N_id) && (RC_IS_ACTIVE (ID_REFCNT (arg_id)))
                    && (!strcmp (ID_NAME (arg_id), IDS_NAME (let_ids)))) {
                    new_id
                      = LiftOrReplaceArg (arg_id, INFO_PREC_FUNDEF (arg_info), new_id,
                                          NULL, &(INFO_PREC3_LASTASSIGN (arg_info)));
                    EXPRS_EXPR (arg) = new_id;
                }

                arg = EXPRS_NEXT (arg);
                arg_idx++;
            }

            let_ids = IDS_NEXT (let_ids);
        }
    }
    /*
     * N_ap
     */
    else if (NODE_TYPE (let_expr) == N_ap) {
        argtab_t *argtab = AP_ARGTAB (let_expr);
        int ids_idx;

        DBUG_ASSERT ((argtab != NULL), "no argtab found!");

        for (ids_idx = 0; ids_idx < argtab->size; ids_idx++) {
            let_ids = argtab->ptr_out[ids_idx];
            if (let_ids != NULL) {
                DBUG_ASSERT ((argtab->ptr_in[0] == NULL), "argtab inconsistent");

                new_id = NULL;
                for (arg_idx = 1; arg_idx < argtab->size; arg_idx++) {
                    arg = argtab->ptr_in[arg_idx];
                    if ((ids_idx != arg_idx) && (arg != NULL)) {
                        DBUG_ASSERT ((NODE_TYPE (arg) == N_exprs),
                                     "no N_exprs node found in argtab!");

                        arg_id = EXPRS_EXPR (arg);
                        if ((NODE_TYPE (arg_id) == N_id)
                            && (RC_IS_ACTIVE (ID_REFCNT (arg_id)))
                            && (!strcmp (ID_NAME (arg_id), IDS_NAME (let_ids)))) {
                            DBUG_ASSERT (ATG_is_in[argtab->tag[arg_idx]],
                                         "illegal tag found!");

                            if (!ATG_has_rc[argtab->tag[arg_idx]]) {
                                new_id
                                  = LiftOrReplaceArg (arg_id, INFO_PREC_FUNDEF (arg_info),
                                                      new_id, NULL,
                                                      &(INFO_PREC3_LASTASSIGN (
                                                        arg_info)));
                                EXPRS_EXPR (arg) = new_id;
                            }
                        }
                    }
                }
            }
        }
    }

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    INFO_PREC3_LET (arg_info) = old_let;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC3with( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC3with (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC3with");

    INFO_PREC3_CEXPR (arg_info) = NULL;

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    if (!emm) {
        /*
         * CODE must be traversed before WITHOP!!
         */

        if (NWITH_CODE (arg_node) != NULL) {
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        }

        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    } else {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        if (NWITH_CODE (arg_node) != NULL) {
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC3with2( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC3with2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC3with2");

    INFO_PREC3_CEXPR (arg_info) = NULL;

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (!emm) {
        /*
         * CODE must be traversed before WITHOP!!
         */

        if (NWITH2_CODE (arg_node) != NULL) {
            NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
        }

        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    } else {

        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

        if (NWITH2_CODE (arg_node) != NULL) {
            NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC3withop( node *arg_node, info *arg_info)
 *
 * description:
 *   New, unique and adjusted pseudo fold-funs are created,
 *   if Explicit Accumulate wasn't applied (flag emm activated)
 *
 * caution:
 *   The N_Nwithop node may be part of a N_Nwith *or* a N_Nwith2 node!!
 *
 ******************************************************************************/

node *
PREC3withop (node *arg_node, info *arg_info)
{
    ids *let_ids;
    node *let_expr;
    node *new_foldfun;
    char *old_name;

    DBUG_ENTER ("PREC3withop");

    let_ids = LET_IDS (INFO_PREC3_LET (arg_info));
    let_expr = LET_EXPR (INFO_PREC3_LET (arg_info));

    DBUG_ASSERT (((NODE_TYPE (let_expr) == N_Nwith)
                  || (NODE_TYPE (let_expr) == N_Nwith2)),
                 "neither N_Nwith nor N_Nwith2 node found!");

    if (((!emm) || (mtmode != MT_none)) && NWITH_OR_NWITH2_IS_FOLD (let_expr)) {
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
        old_name = Free (old_name);
        DBUG_ASSERT ((NWITH_OR_NWITH2_CEXPR (let_expr) != NULL),
                     "NWITH_OR_NWITH2_CEXPR not found!");

        new_foldfun = AdjustFoldFundef (new_foldfun, let_ids,
                                        /*
                                         * It is sufficient to take the CEXPR of the first
                                         * code-node, because in a fold with-loop all
                                         * CEXPR-ids have the same name!
                                         */
                                        NWITH_OR_NWITH2_CEXPR (let_expr));

        if (emm) {
            /*
             * Vardecs from foldfun must be put into current function
             */
            FUNDEF_VARDEC (INFO_PREC_FUNDEF (arg_info))
              = AppendVardec (FUNDEF_VARDEC (INFO_PREC_FUNDEF (arg_info)),
                              DupTree (FUNDEF_VARDEC (new_foldfun)));

            FUNDEF_DFM_BASE (INFO_PREC_FUNDEF (arg_info))
              = DFMUpdateMaskBase (FUNDEF_DFM_BASE (INFO_PREC_FUNDEF (arg_info)),
                                   FUNDEF_ARGS (INFO_PREC_FUNDEF (arg_info)),
                                   FUNDEF_VARDEC (INFO_PREC_FUNDEF (arg_info)));
        }

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
 * function:
 *   node *PREC3code( node *arg_node, info *arg_info)
 *
 * description:
 *   Checks whether all NCODE_CEXPR nodes of fold-WLs have identical names.
 *   (Only necessary if ExplicitAccumulation wasn't applied
 *    (flag emm activated))
 *
 * caution:
 *   The N_Nwithop node may be part of a N_Nwith *or* a N_Nwith2 node!!
 *
 ******************************************************************************/

node *
PREC3code (node *arg_node, info *arg_info)
{
    node *cexprs, *withop, *cexpr, *nassign, *tmp;
    node *let_expr;
    ids *_ids;

    DBUG_ENTER ("PREC3code");

    let_expr = LET_EXPR (INFO_PREC3_LET (arg_info));

    DBUG_ASSERT (((NODE_TYPE (let_expr) == N_Nwith)
                  || (NODE_TYPE (let_expr) == N_Nwith2)),
                 "neither N_Nwith nor N_Nwith2 node found!");

    if ((!emm && NWITH_OR_NWITH2_IS_FOLD (let_expr))) {
        /*
         * fold with-loop:
         * check whether all NCODE_CEXPR nodes have identical names
         */
        if (INFO_PREC3_CEXPR (arg_info) != NULL) {
            DBUG_ASSERT (((NODE_TYPE (INFO_PREC3_CEXPR (arg_info)) == N_id)
                          && (NODE_TYPE (NCODE_CEXPR (arg_node)) == N_id)),
                         "NCODE_CEXPR must be a N_id node!");

            DBUG_ASSERT ((!strcmp (ID_NAME (INFO_PREC3_CEXPR (arg_info)),
                                   ID_NAME (NCODE_CEXPR (arg_node)))),
                         "Not all NCODE_CEXPR nodes of the fold with-loop have"
                         " identical names!\n"
                         "This is probably due to an error during undo-SSA.");
        } else {
            INFO_PREC3_CEXPR (arg_info) = NCODE_CEXPR (arg_node);
        }
    }

    /*
     * NCODE_NEXT should be traversed first,
     * otherwise INFO_PREC3_CEXPR must be stacked!!
     */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (NCODE_CEXPRS (arg_node) != NULL) {
        NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);
    }

    if (emm) {
        /* if ExplicitAccumulate was applied
         * cexprs of fold operators have to be renamed to the
         * name of the corresponding return values
         * A,B = with(iv)
         *        gen:{res1 = ...;
         *             res2 = ...;
         *            }:res1,res2
         *       fold( op1, n1)
         *            ...
         *
         *     rename cexpr: res1 -> A
         *     add assign  : A = res1
         *
         */

        _ids = LET_IDS (INFO_PREC3_LET (arg_info));
        cexprs = NCODE_CEXPRS (arg_node);
        withop = NWITH2_WITHOP (LET_EXPR (INFO_PREC3_LET (arg_info)));
        nassign = NULL;
        tmp = NULL;

        while (withop != NULL) {
            if (NWITHOP_IS_FOLD (withop)) {
                DBUG_ASSERT ((_ids != NULL), "ids is missing");
                cexpr = EXPRS_EXPR (cexprs);
                DBUG_ASSERT ((cexpr != NULL), "CEXPR is missing");
                DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "CEXPR is not a N_id");

                if (IDS_VARDEC (_ids) != ID_VARDEC (cexpr)) {
                    tmp = MakeAssign (MakeLet (DupNode (cexpr), DupOneIds (_ids)), NULL);
                    nassign = AppendAssign (nassign, tmp);

                    EXPRS_EXPR (cexprs) = FreeNode (EXPRS_EXPR (cexprs));
                    EXPRS_EXPR (cexprs) = DupIds_Id (_ids);
                }
            }

            _ids = IDS_NEXT (_ids);
            cexprs = EXPRS_NEXT (cexprs);
            withop = NWITHOP_NEXT (withop);
        }

        if (nassign != NULL) {
            BLOCK_INSTR (NCODE_CBLOCK (arg_node))
              = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)), nassign);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 * FOURTH TRAVERSAL
 *
 */

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

    DBUG_PRINT ("PREC", ("renaming id %s", ID_NAME (idnode)));

    DBUG_ASSERT ((ID_VARDEC (idnode) != NULL), "Vardec not found in function RenameId()");

    if (NODE_TYPE (ID_VARDEC (idnode)) == N_objdef) {
        ID_NAME (idnode) = Free (ID_NAME (idnode));
        ID_NAME (idnode) = StringCopy (OBJDEF_NAME (ID_VARDEC (idnode)));
        /*
         * The global object's definition has already been renamed.
         */
    } else {
        ID_NAME (idnode) = RenameLocalIdentifier (ID_NAME (idnode));

        if (ID_NT_TAG (idnode) != NULL) {
            /*
             * there is already a tag -> it must be recreated
             */
            DBUG_ASSERT ((ID_VARDEC (idnode) != NULL), "no vardec found!");

            ID_NT_TAG (idnode) = Free (ID_NT_TAG (idnode));
            ID_NT_TAG (idnode) = CreateNtTag (ID_NAME (idnode), ID_TYPE (idnode));
        }
    }

    DBUG_RETURN (idnode);
}

/******************************************************************************
 *
 * function:
 *   types *RenameTypes( types *type)
 *
 * description:
 *   Renames the given type if it is a user-defined SAC-type.
 *   Chains of types structures are considered.
 *
 * remarks:
 *   The complete new name is stored in NAME while MOD is set to NULL.
 *
 ******************************************************************************/

static types *
RenameTypes (types *type)
{
    char *tmp;

    DBUG_ENTER ("RenameTypes");

    if (TYPES_BASETYPE (type) == T_user) {
        if (((sbs == 1) && (strcmp (TYPES_MOD (type), EXTERN_MOD_NAME) != 0))
            || ((sbs == 0) && (TYPES_MOD (type) != NULL))) {
            /*
             * SAC data type
             */
            if (!strcmp (TYPES_MOD (type), MAIN_MOD_NAME)) {
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

            TYPES_NAME (type) = Free (TYPES_NAME (type));
            TYPES_NAME (type) = tmp;
            TYPES_MOD (type) = NULL;
        } else {
            /*
             * imported C data type
             */
            tmp = (char *)Malloc (sizeof (char) * (strlen (TYPES_NAME (type)) + 6));
            sprintf (tmp, "SACe_%s", TYPES_NAME (type));

            DBUG_PRINT ("PREC", ("renaming type %s to %s", TYPES_NAME (type), tmp));

            TYPES_NAME (type) = Free (TYPES_NAME (type));
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
 * function:
 *   char *ReplaceSpecialCharacters( char *name)
 *
 * description:
 *
 *
 ******************************************************************************/

static char *
ReplaceSpecialCharacters (char *name)
{
    char *new_name;
    char *tmp;
    int i, j;

    DBUG_ENTER ("ReplaceSpecialCharacters");

    new_name = Malloc ((3 * strlen (name)) * sizeof (char));
    new_name[0] = '\0';

    for (i = 0, j = 0; (size_t)i < strlen (name); i++, j++) {
        switch (name[i]) {
        case '.':
            tmp = "_DO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '+':
            tmp = "_PL";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '-':
            tmp = "_MI";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '*':
            tmp = "_ST";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '/':
            tmp = "_DI";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '%':
            tmp = "_PR";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '<':
            tmp = "_LT";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '>':
            tmp = "_GT";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '=':
            tmp = "_EQ";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '&':
            tmp = "_AM";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '|':
            tmp = "_VE";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '!':
            tmp = "_EX";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '[':
            tmp = "_BL";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case ']':
            tmp = "_BR";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '~':
            tmp = "_TI";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '@':
            tmp = "_AT";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '#':
            tmp = "_HA";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '$':
            tmp = "_DO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '^':
            tmp = "_PO";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        case '\\':
            tmp = "_BS";
            strcat (&(new_name[j]), tmp);
            j += strlen (tmp) - 1;
            break;
        default:
            new_name[j] = name[i];
            break;
        }
    }

    new_name[j] = '\0';

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *   char *RenameFunName( char *mod, char *name,
 *                        statustype status, node *args)
 *
 * description:
 *   Renames the given name of a SAC-function.
 *   A new name is created from the module name, the original name and the
 *   argument's types.
 *
 ******************************************************************************/

static char *
RenameFunName (char *mod, char *name, statustype status, node *args)
{
    char *prefix;
    char *tmp_name;
    char *new_name;

    DBUG_ENTER ("RenameFunName");

    tmp_name = ReplaceSpecialCharacters (name);

    if (status == ST_spmdfun) {
        new_name = (char *)Malloc ((strlen (tmp_name) + 6) * sizeof (char));
        sprintf (new_name, "SACf_%s", tmp_name);
    } else {
        node *arg;
        int length = 0;

        arg = args;
        while (arg != NULL) {
            length += strlen (ARG_TYPESTRING (arg)) + 2;
            arg = ARG_NEXT (arg);
        }

        prefix = (status == ST_wrapperfun) ? "SACwf" : "SACf";

        if (!strcmp (mod, MAIN_MOD_NAME)) {
            length += (strlen (prefix) + strlen (tmp_name) + 2);
            new_name = (char *)Malloc (length * sizeof (char));
            sprintf (new_name, "%s_%s", prefix, tmp_name);
        } else {
            length += (strlen (prefix) + strlen (mod) + strlen (tmp_name) + 4);
            new_name = (char *)Malloc (length * sizeof (char));
            sprintf (new_name, "%s_%s__%s", prefix, mod, tmp_name);
        }

        arg = args;
        while (arg != NULL) {
            strcat (new_name, "__");
            strcat (new_name, ARG_TYPESTRING (arg));
            ARG_TYPESTRING (arg) = Free (ARG_TYPESTRING (arg));
            arg = ARG_NEXT (arg);
        }
    }

    tmp_name = Free (tmp_name);

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *   node *RenameFun( node *fun)
 *
 * description:
 *   Renames the given function.
 *   For SAC-functions, a new name is created from the module name, the
 *   original name and the argument's types.
 *   For C-functions, a new name is taken from the pragma 'linkname' if present.
 *
 ******************************************************************************/

static node *
RenameFun (node *fun)
{
    char *new_name;

    DBUG_ENTER ("RenameFun");

    if (FUNDEF_STATUS (fun) == ST_Cfun) {
        if ((FUNDEF_PRAGMA (fun) != NULL) && (FUNDEF_LINKNAME (fun) != NULL)) {
            /*
             * C functions with additional pragma 'linkname'
             */

            DBUG_PRINT ("PREC", ("renaming C function %s to %s", FUNDEF_NAME (fun),
                                 FUNDEF_LINKNAME (fun)));

            FUNDEF_NAME (fun) = Free (FUNDEF_NAME (fun));
            FUNDEF_NAME (fun) = StringCopy (FUNDEF_LINKNAME (fun));
        } else {
            DBUG_PRINT ("PREC",
                        ("C function %s has not been renamed", FUNDEF_NAME (fun)));
        }
    } else {
        /*
         * SAC functions which may be overloaded
         */

        new_name = RenameFunName (FUNDEF_MOD (fun), FUNDEF_NAME (fun),
                                  FUNDEF_STATUS (fun), FUNDEF_ARGS (fun));

        DBUG_PRINT ("PREC", ("renaming SAC function %s:%s to %s", FUNDEF_MOD (fun),
                             FUNDEF_NAME (fun), new_name));

        FUNDEF_NAME (fun) = Free (FUNDEF_NAME (fun));
        FUNDEF_NAME (fun) = new_name;
        /* don't free FUNDEF_MOD because it is shared !! */
        FUNDEF_MOD (fun) = NULL;
    }

    DBUG_RETURN (fun);
}

/******************************************************************************
 *
 * function:
 *   ids *RenameIds( ids *arg)
 *
 * description:
 *   This function performs the renaming of identifiers stored within
 *   ids-chains.
 *
 ******************************************************************************/

static ids *
RenameIds (ids *arg)
{
    DBUG_ENTER ("RenameIds");

    if (arg != NULL) {
        if (NODE_TYPE (IDS_VARDEC (arg)) == N_objdef) {
            IDS_NAME (arg) = Free (IDS_NAME (arg));
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
 *   node *PREC4modul( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC4modul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4modul");

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4typedef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Renames types. All types defined in SAC get the prefix "SAC_" to avoid
 *   name clashes with C identifiers.
 *
 ******************************************************************************/

node *
PREC4typedef (node *arg_node, info *arg_info)
{
    char *tmp;

    DBUG_ENTER ("PREC4typedef");

    if (((sbs == 1) && (strcmp (TYPEDEF_MOD (arg_node), EXTERN_MOD_NAME) != 0))
        || ((sbs == 0) && (TYPEDEF_MOD (arg_node) != NULL))) {
        /*
         * SAC typedef
         */

        if (!strcmp (TYPEDEF_MOD (arg_node), MAIN_MOD_NAME)) {
            tmp = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
            sprintf (tmp, "SACt_%s", TYPEDEF_NAME (arg_node));
        } else {
            tmp = (char *)Malloc (
              sizeof (char)
              * (strlen (TYPEDEF_NAME (arg_node)) + strlen (TYPEDEF_MOD (arg_node)) + 8));
            sprintf (tmp, "SACt_%s__%s", TYPEDEF_MOD (arg_node), TYPEDEF_NAME (arg_node));
        }

        TYPEDEF_NAME (arg_node) = Free (TYPEDEF_NAME (arg_node));
        TYPEDEF_NAME (arg_node) = tmp;
        TYPEDEF_MOD (arg_node) = NULL;

        TYPEDEF_TYPE (arg_node) = RenameTypes (TYPEDEF_TYPE (arg_node));
    } else {
        /*
         * imported C typedef
         */

        tmp = (char *)Malloc (sizeof (char) * (strlen (TYPEDEF_NAME (arg_node)) + 6));
        sprintf (tmp, "SACe_%s", TYPEDEF_NAME (arg_node));

        TYPEDEF_NAME (arg_node) = Free (TYPEDEF_NAME (arg_node));
        TYPEDEF_NAME (arg_node) = tmp;
        /*
         * Why are imported C types renamed unlike imported C functions or
         * global objects?
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
 *   node *PREC4objdef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Renames global objects.
 *   For SAC-functions the VARNAME, a combination of module name and object
 *   name is used, for C-functions the optional 'linkname' is used if present.
 *   Additionally, the object's type is renamed as well.
 *
 ******************************************************************************/

node *
PREC4objdef (node *arg_node, info *arg_info)
{
    char *new_name;

    DBUG_ENTER ("PREC4objdef");

    if (((sbs == 1) && (strcmp (OBJDEF_MOD (arg_node), EXTERN_MOD_NAME) != 0))
        || ((sbs == 0) && (OBJDEF_MOD (arg_node) != NULL))) {
        /*
         * SAC objdef
         */

        OBJDEF_VARNAME (arg_node) = Free (OBJDEF_VARNAME (arg_node));
        /*
         * OBJDEF_VARNAME is no longer used for the generation of the final C code
         * identifier of a global object.
         */

        if (!strcmp (OBJDEF_MOD (arg_node), MAIN_MOD_NAME)) {
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

        OBJDEF_NAME (arg_node) = Free (OBJDEF_NAME (arg_node));
        OBJDEF_NAME (arg_node) = new_name;
        OBJDEF_MOD (arg_node) = NULL;
    } else {
        /*
         * imported C objdef
         */

        if (OBJDEF_LINKNAME (arg_node) != NULL) {
            OBJDEF_NAME (arg_node) = Free (OBJDEF_NAME (arg_node));
            OBJDEF_NAME (arg_node) = OBJDEF_LINKNAME (arg_node);
            OBJDEF_PRAGMA (arg_node) = Free (OBJDEF_PRAGMA (arg_node));
        }
    }

    OBJDEF_TYPE (arg_node) = RenameTypes (OBJDEF_TYPE (arg_node));

    if (OBJDEF_NEXT (arg_node) != NULL) {
        OBJDEF_NEXT (arg_node) = Trav (OBJDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4fundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC4fundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4fundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    /*
     * Now, the data flow mask base is updated.
     * This is necessary because some local identifiers are removed while all
     * others are renamed.
     */
    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) != NULL),
                     "FUNDEF_DFM_BASE without body found!");

        FUNDEF_DFM_BASE (arg_node)
          = DFMUpdateMaskBaseAfterRenaming (FUNDEF_DFM_BASE (arg_node),
                                            FUNDEF_ARGS (arg_node),
                                            FUNDEF_VARDEC (arg_node));
    }

    arg_node = RenameFun (arg_node);

    FUNDEF_TYPES (arg_node) = RenameTypes (FUNDEF_TYPES (arg_node));

    /*
     * FUNDEF_PRAGMA is no longer needed
     */
    if (FUNDEF_PRAGMA (arg_node) != NULL) {
        FUNDEF_PRAGMA (arg_node) = FreeNode (FUNDEF_PRAGMA (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4arg( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC4arg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4arg");

    ARG_TYPESTRING (arg_node) = Type2String (ARG_TYPE (arg_node), 2, TRUE);
    ARG_TYPE (arg_node) = RenameTypes (ARG_TYPE (arg_node));
    /*
     * ARG_TYPESTRING is only used for renaming functions, so the
     * type's actual name may be changed afterwards.
     */

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

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4vardec( node *arg_node, info *arg_info)
 *
 * Description:
 *   Renames types of declared variables.
 *
 ******************************************************************************/

node *
PREC4vardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4vardec");

    VARDEC_TYPE (arg_node) = RenameTypes (VARDEC_TYPE (arg_node));
    VARDEC_NAME (arg_node) = RenameLocalIdentifier (VARDEC_NAME (arg_node));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4let( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
PREC4let (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4let");

    LET_IDS (arg_node) = RenameIds (LET_IDS (arg_node));

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4return( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC4return (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4return");

    if (RETURN_REFERENCE (arg_node) != NULL) {
        RETURN_REFERENCE (arg_node) = Trav (RETURN_REFERENCE (arg_node), arg_info);
    }

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4ap( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC4ap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4ap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4icm( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC4icm (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4icm");

    if (ICM_ARGS (arg_node) != NULL) {
        ICM_ARGS (arg_node) = Trav (ICM_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4array( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC4array (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4array");

    if (ARRAY_AELEMS (arg_node) != NULL) {
        ARRAY_AELEMS (arg_node) = Trav (ARRAY_AELEMS (arg_node), arg_info);
    }

    ARRAY_TYPE (arg_node) = RenameTypes (ARRAY_TYPE (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *PREC4id( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
PREC4id (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4id");

    /*
     * ID_VARDEC is NULL for ICMs inside of ObjInitFunctions!!!
     */
    if (ID_VARDEC (arg_node) != NULL) {
        arg_node = RenameId (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4do( node *arg_node, info *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC4do (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4do");

    DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
    if (DO_SKIP (arg_node) != NULL) {
        DO_SKIP (arg_node) = Trav (DO_SKIP (arg_node), arg_info);
    }

    DO_USEVARS (arg_node) = RenameIds (DO_USEVARS (arg_node));
    DO_DEFVARS (arg_node) = RenameIds (DO_DEFVARS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4cond( node *arg_node, info *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC4cond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4cond");

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
 *   node *PREC4with( node *arg_node, info *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC4with (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4with");

    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    NWITH_DEC_RC_IDS (arg_node) = RenameIds (NWITH_DEC_RC_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4with2( node *arg_node, info *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 ******************************************************************************/

node *
PREC4with2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4with2");

    NWITH2_WITHID (arg_node) = Trav (NWITH2_WITHID (arg_node), arg_info);
    NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);

    NWITH2_DEC_RC_IDS (arg_node) = RenameIds (NWITH2_DEC_RC_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4withid( node *arg_node, info *arg_info)
 *
 * description:
 *   This function does the renaming of the index vector variable
 *   as well as its scalar counterparts for the new with-loop.
 *
 * caution:
 *   The N_Nwithop node may be part of a N_Nwith *or* a N_Nwith2 node!!
 *
 ******************************************************************************/

node *
PREC4withid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4withid");

    NWITHID_VEC (arg_node) = RenameIds (NWITHID_VEC (arg_node));
    NWITHID_IDS (arg_node) = RenameIds (NWITHID_IDS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *PREC4code( node *arg_node, info *arg_info)
 *
 * description:
 *   The compiler phase refcount unfortunately produces chains of identifiers
 *   for which refcounting operations must be inserted during code generation.
 *   These must be renamed in addition to those identifiers that are "really"
 *   part of the code.
 *
 * caution:
 *   The N_Nwithop node may be part of a N_Nwith *or* a N_Nwith2 node!!
 *
 ******************************************************************************/

node *
PREC4code (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PREC4code");

    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);
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
 *   node *PREC4WLsegx( node *arg_node, info *arg_info)
 *
 * description:
 *   Since the scheduling specification and WLSEGVAR_IDX_MIN, WLSEGVAR_IDX_MAX
 *   may contain the names of local identifiers, these have to be renamed
 *   according to the general renaming scheme implemented by this compiler
 *   phase.
 *
 ******************************************************************************/

node *
PREC4WLsegx (node *arg_node, info *arg_info)
{
    int d;

    DBUG_ENTER ("PREC4WLsegx");

    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        WLSEGX_SCHEDULING (arg_node)
          = SCHPrecompileScheduling (WLSEGX_SCHEDULING (arg_node));
        WLSEGX_TASKSEL (arg_node) = SCHPrecompileTasksel (WLSEGX_TASKSEL (arg_node));
    }

    if (NODE_TYPE (arg_node) == N_WLsegVar) {
        DBUG_ASSERT ((WLSEGVAR_IDX_MIN (arg_node) != NULL),
                     "WLSEGVAR_IDX_MIN not found!");
        DBUG_ASSERT ((WLSEGVAR_IDX_MAX (arg_node) != NULL),
                     "WLSEGVAR_IDX_MAX not found!");
        for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
            (WLSEGVAR_IDX_MIN (arg_node))[d]
              = Trav ((WLSEGVAR_IDX_MIN (arg_node))[d], arg_info);
            (WLSEGVAR_IDX_MAX (arg_node))[d]
              = Trav ((WLSEGVAR_IDX_MAX (arg_node))[d], arg_info);
        }
    }

    WLSEGX_CONTENTS (arg_node) = Trav (WLSEGX_CONTENTS (arg_node), arg_info);

    if (WLSEGX_NEXT (arg_node) != NULL) {
        WLSEGX_NEXT (arg_node) = Trav (WLSEGX_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 * Precompile()
 *
 */

/******************************************************************************
 *
 * function:
 *   node *Precompile( node *syntax_tree)
 *
 * description:
 *   Prepares syntax tree for code generation.
 *   (For more information see the comments at the beginning of this file)
 *
 *   Optional traversal of AST when generating c-library:
 *     - Look for overloaded functions and build up a list of wrappers
 *   (This has to be done before the following steps, because of the renaming.)
 *
 ******************************************************************************/

node *
Precompile (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("Precompile");

    syntax_tree = MapCWrapper (syntax_tree);

    DBUG_EXECUTE ("PREC", NOTE (("step 0: renaming MemVals\n")));
    syntax_tree = MarkMemVals (syntax_tree);
    if (!strcmp (break_specifier, "mmv")) {
        goto DONE;
    }

    DBUG_EXECUTE ("PREC", NOTE (("step 1: remove artificial args\n")));
    act_tab = precomp1_tab;
    info = MakeInfo ();
    syntax_tree = Trav (syntax_tree, info);
    info = FreeInfo (info);

    if (strcmp (break_specifier, "prec1")) {
        DBUG_EXECUTE ("PREC", NOTE (("step 2: transform fundefs/aps\n")));
        act_tab = precomp2_tab;
        info = MakeInfo ();
        syntax_tree = Trav (syntax_tree, info);
        info = FreeInfo (info);

        if (strcmp (break_specifier, "prec2")) {
            DBUG_EXECUTE ("PREC", NOTE (("step 3: flatten code\n")));
            act_tab = precomp3_tab;
            info = MakeInfo ();
            syntax_tree = Trav (syntax_tree, info);
            info = FreeInfo (info);

            if (strcmp (break_specifier, "prec3")) {
                DBUG_EXECUTE ("PREC", NOTE (("step 4: rename identifiers\n")));
                act_tab = precomp4_tab;
                info = MakeInfo ();
                syntax_tree = Trav (syntax_tree, info);
                info = FreeInfo (info);
            }
        }
    }

DONE:
    DBUG_RETURN (syntax_tree);
}

/*
 *
 * Other exported functions
 *
 */

/******************************************************************************
 *
 * function:
 *   char *ObjInitFunctionName( bool before_rename)
 *
 * description:
 *   Returns new allocated string with objinitfunction name
 *
 * parameters:
 *   uses global variable modulename!
 *
 ******************************************************************************/

char *
ObjInitFunctionName (bool before_rename)
{
    char *name = "GlobalObjInit";
    char *new_name;

    DBUG_ENTER ("ObjInitFunctionName");

    if (before_rename) {
        new_name = (char *)Malloc (strlen (name) + 1);

        strcpy (new_name, name);
    } else {
        new_name = RenameFunName (modulename, name, ST_regular, NULL);
    }

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

    new_name = (char *)Malloc (sizeof (char) * (strlen (id) + strlen (name_prefix) + 1));
    sprintf (new_name, "%s%s", name_prefix, id);

    id = Free (id);

    DBUG_RETURN (new_name);
}
