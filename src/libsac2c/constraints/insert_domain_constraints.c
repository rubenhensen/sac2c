/**
 *
 * @file insert_domain_constraints.c
 *
 * This file provides a helper traversal for ICC. While ICC identifies
 * the constraints that are needed, this traversal is responsible for
 * inserting them at the right position. For a more detailled description
 * of the relation between ICC and IDC and why they need to be separate
 * in the first place, see the comments in ICC.
 * Here, we only describe the API that IDC provides to ICC.
 *
 * In essence, IDC needs to be called three times:
 *
 * 1 First it needs to be initialised on functin level using
 *     IDCinitialize(...)
 *
 * 2 Then, constraints can be added "asynchronously" by calling
 *   either
 *     node *IDCaddTypeConstraint( ntype *type, node *avis))
 *   or
 *     node *IDCaddFunConstraint( node * expr)
 *   Both add constraints in an abstract way, ie the constraints
 *   are only aggregated in the avis nodes but not yet inserted
 *   into the data-flow. In case of IDCaddTypeConstraint expr
 *   can either be an application of a prf or a udf! As the
 *   prf/udf constraints typically relate to more than one avis
 *   the avis that is defined last wrt the data-flow is being chosen.
 *   For convenience, both these functions return a pointer to the
 *   avis of a freshly generated variable that, later, will contain
 *   the boolean obtained from the check.
 *
 *   The actual code insertion is being done through a call of
 *     IDCinsertConstraints(...)
 *   which happens on fundef level again.
 *
 * 3 Finally, IDC needs to perform some cleaning up which happens
 *   through a call to
 *     IDCfinalize(...)
 *
 *
 * Implementation notes:
 * =====================
 */

#define DBUG_PREFIX "IDC"
#include "debug.h"

#include "types.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "constants.h"
#include "globals.h"
#include "memory.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "ctinfo.h"
#include "ptr_buffer.h"

#include "insert_domain_constraints.h"

/**
 *
 * Static global variables
 *
 */

static const int ndf_rets[] = {
#define PRFnum_dataflow_returns(ndf_rets) ndf_rets
#include "prf_info.mac"
};

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

typedef enum { IDC_init, IDC_insert, IDC_finalize } trav_mode;

struct INFO {
    bool all;
    trav_mode mode;
    int counter;
    node *post;
    node *vardecs;
    ptr_buf *ren_stack;
    node *args;
    node *branch;
    bool iuib_res;
    node *iuib_avis;
};

#define INFO_ALL(n) ((n)->all)
#define INFO_MODE(n) ((n)->mode)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_POSTASSIGN(n) ((n)->post)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_RENAME_STACK(n) ((n)->ren_stack)
#define INFO_FUNARGS(n) ((n)->args)
#define INFO_CONDBRANCH(n) ((n)->branch)
#define INFO_IUIB_RES(n) ((n)->iuib_res)
#define INFO_IUIB_AVIS(n) ((n)->iuib_avis)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ALL (result) = FALSE;
    INFO_MODE (result) = IDC_init;
    INFO_COUNTER (result) = 0;
    INFO_POSTASSIGN (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RENAME_STACK (result) = PBUFcreate (50);
    INFO_FUNARGS (result) = NULL;
    INFO_CONDBRANCH (result) = NULL;
    INFO_IUIB_RES (result) = FALSE;
    INFO_IUIB_AVIS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_RENAME_STACK (info) = (ptr_buf *)PBUFfree (INFO_RENAME_STACK (info));
    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

static node *
ATravIUIBid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT_TAG ("IDC_IUIB", "   ...found %s", ID_NAME (arg_node));
    INFO_IUIB_RES (arg_info)
      = (INFO_IUIB_RES (arg_info) || (ID_AVIS (arg_node) == INFO_IUIB_AVIS (arg_info)));
    DBUG_RETURN (arg_node);
}

static bool
IsUsedInBranch (node *avis, info *arg_info)
{
    anontrav_t iuib_trav[2] = {{N_id, &ATravIUIBid}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (iuib_trav, &TRAVsons);

    INFO_IUIB_RES (arg_info) = FALSE;
    INFO_IUIB_AVIS (arg_info) = avis;

    DBUG_PRINT_TAG ("IDC_IUIB", "looking for %s:", AVIS_NAME (avis));

    INFO_CONDBRANCH (arg_info) = TRAVopt (INFO_CONDBRANCH (arg_info), arg_info);

    TRAVpop ();

    DBUG_RETURN (INFO_IUIB_RES (arg_info));
}

static node *
FindAvisOfLastDefinition (node *exprs)
{
    node *avis, *last_avis = NULL;
    node *expr;

    DBUG_ENTER ();

    while (exprs != NULL) {
        expr = EXPRS_EXPR (exprs);
        if (NODE_TYPE (expr) == N_id) {
            avis = ID_AVIS (expr);
            DBUG_ASSERT (AVIS_POS (avis) > 0, "IDCaddConstraint used before IDCinit()!");
            if ((last_avis == NULL) || (AVIS_POS (last_avis) < AVIS_POS (avis))) {
                last_avis = avis;
            }
        }

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (last_avis);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateNewVarAndInitiateRenaming( node *id, info *arg_info)
 *
 * @brief Creates a new variable, inserts the corresponding avis and marks
 *        the avis of id to be substituted by the new variable. The type of
 *        the new variable is the type of id lifted to AKS. This is
 *        essential, as guards introduce the possibility of the new
 *        variable evaluating to _|_, which would not be subtype of the
 *        AKV but of the corresponding AKS!
 *
 * @param id       N_id node to create clone of
 * @param arg_info info structure for vardecs etc.
 *
 * @return avis node of new variable
 ******************************************************************************/
static node *
CreateNewVarAndInitiateRenaming (node *id, info *arg_info)
{
    node *old_avis, *avis;

    DBUG_ENTER ();

    old_avis = ID_AVIS (id);
    avis = TBmakeAvis (TRAVtmpVar (), TYeliminateAKV (AVIS_TYPE (old_avis)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    AVIS_SUBST (old_avis) = avis;
    INFO_RENAME_STACK (arg_info) = PBUFadd (INFO_RENAME_STACK (arg_info), old_avis);

    DBUG_RETURN (avis);
}

static ptr_buf *
EraseRenamings (ptr_buf *stack, int pos)
{
    DBUG_ENTER ();
    int i;
    node *avis;

    for (i = PBUFpos (stack) - 1; i >= pos; i--) {
        avis = (node *)PBUFptr (stack, i);
        AVIS_SUBST (avis) = NULL;
    }

    PBUFflushFrom (stack, pos);

    DBUG_RETURN (stack);
}

static node *
DupIdExprsWithoutDuplicates (node *exprs)
{
    node *args;
    bool found;
    node *tmp, *avis;

    DBUG_ENTER ();

    if (exprs != NULL) {
        args = DupIdExprsWithoutDuplicates (EXPRS_NEXT (exprs));
        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id,
                     "non N_id argument in requires expression found");
        avis = ID_AVIS (EXPRS_EXPR (exprs));
        tmp = args;
        found = FALSE;
        while (tmp != NULL) {
            if (ID_AVIS (EXPRS_EXPR (tmp)) == avis) {
                found = TRUE;
                tmp = NULL;
            } else {
                tmp = EXPRS_NEXT (tmp);
            }
        }
        if (!found) {
            args = TBmakeExprs (TBmakeId (avis), args);
        }
    } else {
        args = NULL;
    }

    DBUG_RETURN (args);
}

static node *
BuildDataFlowHook (node *ids, node *expr, info *arg_info)
{
    node *exprs, *assign, *avis, *new_ids = NULL;
    ;
    int i;

    DBUG_ENTER ();

    exprs = PRF_ARGS (expr);

    if (PRF_PRF (expr) == F_type_constraint) {
        exprs = EXPRS_NEXT (exprs);
    }

    assign = TBmakeAssign (NULL, NULL);

    for (i = 0; i < ndf_rets[PRF_PRF (expr)]; i++) {
        avis = CreateNewVarAndInitiateRenaming (EXPRS_EXPR (exprs), arg_info);
        new_ids = TCappendIds (new_ids, TBmakeIds (avis, NULL));
        AVIS_SSAASSIGN (avis) = assign;
        exprs = EXPRS_NEXT (exprs);
    }
    ids = TCappendIds (new_ids, ids);

    ASSIGN_STMT (assign) = TBmakeLet (ids, expr);

    /**
     * assign needs to be put at the very end due to potential data
     * dependencies!
     */
    INFO_POSTASSIGN (arg_info) = TCappendAssign (INFO_POSTASSIGN (arg_info), assign);

    DBUG_RETURN (assign);
}

static info *
BuildPrfConstraint (node *pavis, node *expr, info *arg_info)
{
    node *assign;

    DBUG_ENTER ();

    INFO_VARDECS (arg_info) = TBmakeVardec (pavis, INFO_VARDECS (arg_info));
    assign = BuildDataFlowHook (TBmakeIds (pavis, NULL), expr, arg_info);
    AVIS_SSAASSIGN (pavis) = assign;

    DBUG_RETURN (arg_info);
}

static info *
BuildUdfConstraint (node *pavis, node *expr, info *arg_info)
{
    node *assign;

    DBUG_ENTER ();

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (pavis, NULL), expr), NULL);
    AVIS_SSAASSIGN (pavis) = assign;

    INFO_POSTASSIGN (arg_info) = TCappendAssign (INFO_POSTASSIGN (arg_info), assign);

    expr = TBmakePrf (F_guard, DupIdExprsWithoutDuplicates (AP_ARGS (expr)));
    assign = BuildDataFlowHook (NULL, expr, arg_info);

    DBUG_RETURN (arg_info);
}

static info *
HandleConstraints (node *avis, info *arg_info)
{
    node *expr, *constraint;

    DBUG_ENTER ();

    if (AVIS_CONSTRTYPE (avis) != NULL) {
        DBUG_PRINT ("Handling constraint on %s:", AVIS_NAME (avis));
        if ((INFO_CONDBRANCH (arg_info) == NULL)
            || IsUsedInBranch (AVIS_CONSTRVAR (avis), arg_info)) {
            expr = TCmakePrf2 (F_type_constraint, TBmakeType (AVIS_CONSTRTYPE (avis)),
                               TBmakeId (avis));
            expr = TRAVdo (expr, arg_info);
            arg_info = BuildPrfConstraint (AVIS_CONSTRVAR (avis), expr, arg_info);
            AVIS_CONSTRVAR (avis) = NULL;
            AVIS_CONSTRTYPE (avis) = NULL;
            DBUG_PRINT ("    ...inserted");
        } else {
            DBUG_PRINT ("    ...wrong branch");
        }
    }

    if (AVIS_CONSTRSET (avis) != NULL) {
        constraint = AVIS_CONSTRSET (avis);
        AVIS_CONSTRSET (avis) = CONSTRAINT_NEXT (constraint);
        CONSTRAINT_NEXT (constraint) = NULL;

        arg_info = HandleConstraints (avis, arg_info);

        DBUG_PRINT ("Handling constraint on %s:", AVIS_NAME (avis));
        if ((INFO_CONDBRANCH (arg_info) == NULL)
            || IsUsedInBranch (CONSTRAINT_PREDAVIS (constraint), arg_info)) {
            CONSTRAINT_EXPR (constraint)
              = TRAVdo (CONSTRAINT_EXPR (constraint), arg_info);
            if (NODE_TYPE (CONSTRAINT_EXPR (constraint)) == N_prf) {
                arg_info = BuildPrfConstraint (CONSTRAINT_PREDAVIS (constraint),
                                               CONSTRAINT_EXPR (constraint), arg_info);
            } else {
                arg_info = BuildUdfConstraint (CONSTRAINT_PREDAVIS (constraint),
                                               CONSTRAINT_EXPR (constraint), arg_info);
            }
            CONSTRAINT_PREDAVIS (constraint) = NULL;
            CONSTRAINT_EXPR (constraint) = NULL;
            constraint = FREEdoFreeNode (constraint);
            DBUG_PRINT ("    ...inserted");
        } else {
            /*
             * re-insert the constraint!
             */
            CONSTRAINT_NEXT (constraint) = AVIS_CONSTRSET (avis);
            AVIS_CONSTRSET (avis) = constraint;
            DBUG_PRINT ("    ...wrong branch");
        }
    }
    DBUG_RETURN (arg_info);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("----- %s ----- %s:", CTIitemName (arg_node),
                (INFO_MODE (arg_info) == IDC_finalize
                   ? "IDC_finalize"
                   : (INFO_MODE (arg_info) == IDC_insert ? "IDC_insert" : "IDC_init")));

    INFO_COUNTER (arg_info) = 1;

    if ((INFO_MODE (arg_info) == IDC_insert) && FUNDEF_ISCONDFUN (arg_node)) {
        /*
         * in Cond-Funs, we need to insert argument constraints in the corres
         * ponding branches (see bug 944 for details). Hence, we deal with the
         * arguments within the individual branches in IDCcond !
         */
        INFO_FUNARGS (arg_info) = FUNDEF_ARGS (arg_node);
    } else {
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    }

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        arg_node = TCaddVardecs (arg_node, INFO_VARDECS (arg_info));
        DBUG_PRINT ("...inserting vardecs");
    }

    if (INFO_ALL (arg_info) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCblock (node *arg_node, info *arg_info)
{
    node *post_assign;
    DBUG_ENTER ();

    post_assign = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    if (post_assign != NULL) {
        BLOCK_ASSIGNS (arg_node) = TCappendAssign (post_assign, BLOCK_ASSIGNS (arg_node));
        DBUG_PRINT ("...inserting assignments at beginning of N_block");
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCassign (node *arg_node, info *arg_info)
{
    node *post_assign;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    post_assign = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (post_assign != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (post_assign, ASSIGN_NEXT (arg_node));
        DBUG_PRINT ("...inserting assignments");
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDClet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCids (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_node);

    switch (INFO_MODE (arg_info)) {
    case IDC_init:
        AVIS_POS (avis) = INFO_COUNTER (arg_info);
        INFO_COUNTER (arg_info)++;
        break;

    case IDC_insert:
        arg_info = HandleConstraints (avis, arg_info);
        break;

    case IDC_finalize:
        AVIS_POS (avis) = 0;
        break;

    default:
        break;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCcond( node *arg_node, info *arg_info)
 *
 * @brief The sole purpose of this code is to make sure
 *        renamings are confined to the branch where there are
 *        initiated! (cf. bug 944 for details)
 *
 *****************************************************************************/
node *
IDCcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    int rename_stack_pos;

    rename_stack_pos = PBUFpos (INFO_RENAME_STACK (arg_info));

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    if (INFO_FUNARGS (arg_info) != NULL) {
        /*
         * we are in a CondFun and we are inserting now. Therefore, we have to
         * deal with the constraints of arguments that are required for this
         * branch, right now! (cf bug 944)
         * In order to be able to identify those required in this branch
         * we keep a pointer to the branch in arg_info:
         */
        INFO_CONDBRANCH (arg_info) = COND_THEN (arg_node);
        INFO_FUNARGS (arg_info) = TRAVopt (INFO_FUNARGS (arg_info), arg_info);
        INFO_CONDBRANCH (arg_info) = NULL;
    }

    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);

    INFO_RENAME_STACK (arg_info)
      = EraseRenamings (INFO_RENAME_STACK (arg_info), rename_stack_pos);

    if (INFO_FUNARGS (arg_info) != NULL) {
        /*
         * we are in a CondFun and we are inserting now. Therefore, we have to
         * deal with the constraints of arguments that are required for this
         * branch, right now! (cf bug 944)
         * In order to be able to identify those required in this branch
         * we keep a pointer to the branch in arg_info:
         */
        INFO_CONDBRANCH (arg_info) = COND_ELSE (arg_node);
        INFO_FUNARGS (arg_info) = TRAVopt (INFO_FUNARGS (arg_info), arg_info);
        INFO_CONDBRANCH (arg_info) = NULL;
    }

    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

    INFO_RENAME_STACK (arg_info)
      = EraseRenamings (INFO_RENAME_STACK (arg_info), rename_stack_pos);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    int rename_stack_pos;

    rename_stack_pos = PBUFpos (INFO_RENAME_STACK (arg_info));

    /**
     * part needs to be traversed BEFORE code so that the N_ids of
     * the generator variable obtain the right AVIS_POS, i.e., one
     * that is smaller than any variable defined in the code.
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_RENAME_STACK (arg_info)
      = EraseRenamings (INFO_RENAME_STACK (arg_info), rename_stack_pos);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * for proper renaming of upper and lower bounds, we need to make sure
     * that no further constraints are built between them and any withid.
     * Hence, we traverse the withid(s) AFTER seeing all parts.
     * See bug 417 for details.
     */
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    while (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
        ID_AVIS (arg_node) = AVIS_SUBST (ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (INFO_MODE (arg_info)) {
    case IDC_init:
        AVIS_SUBST (arg_node) = NULL;
        if (NODE_TYPE (AVIS_DECL (arg_node)) == N_arg) {
            AVIS_POS (arg_node) = INFO_COUNTER (arg_info);
            INFO_COUNTER (arg_info)++;
            /**
             * Although we do not need to increment the counter here as all args
             * could just be tagged as 1, we do so in order to make sure that
             * the first assigned var has a higher number than the args
             * (see bug 877 as a relevant example).
             */
        }
        break;

    case IDC_insert:
        if (NODE_TYPE (AVIS_DECL (arg_node)) == N_arg) {
            arg_info = HandleConstraints (arg_node, arg_info);
        }
        break;

    case IDC_finalize:
        AVIS_SUBST (arg_node) = NULL;
        AVIS_POS (arg_node) = 0;
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn  node *IDCinitialize( node *fundef, bool all)
 *
 *  @brief initializes the constraint system for the given fundef.
 *         Iff all is true apply initialization to the entire fundef chain
 *
 *  @param fundef: fundef to be initialized
 *         all:    indicator whether to be used on the entire chain
 *                 or on one fundef only
 *  @return the initialized fundef (chain)
 *
 ***************************************************************************/

node *
IDCinitialize (node *fundef, bool all)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "IDCinitialize called on nun-fundef!");

    TRAVpush (TR_idc);

    arg_info = MakeInfo ();
    INFO_ALL (arg_info) = all;
    INFO_MODE (arg_info) = IDC_init;

    fundef = TRAVdo (fundef, arg_info);

    arg_info = FreeInfo (arg_info);
    TRAVpop ();

    DBUG_RETURN (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCaddTypeConstraint( ntype *type, node *avis);
 *
 *  @brief add a constraint that by means of a call to IDCinsertConstraints()
 *     will lead to the following code:
 *         id`, p = _type_constraint_( type, id)
 *     where id is an N_id that points to the given N_avis avis and p is the
 *     returned predicate.
 *     Issues a runtime error  in case IDCinit() has not been called yet.
 *     Note here, that subsequent calls may return the same predicate and
 *     it may sharpen the constraining type.
 *
 *  @param type: type constraint
 *         avis: variable that is to be constrained
 *
 *  @return N_avis node of the predicate or NULL of none generated
 *
 ***************************************************************************/

node *
IDCaddTypeConstraint (ntype *type, node *avis)
{
    node *res = NULL;
    ntype *act_type;
#ifndef DBUG_OFF
    char *tmp_str = NULL;
#endif

    DBUG_ENTER ();

    DBUG_EXECUTE (tmp_str = TYtype2String (type, FALSE, 0));
    DBUG_PRINT ("type constraint requested: %s for %s", tmp_str, AVIS_NAME (avis));
    DBUG_EXECUTE (tmp_str = MEMfree (tmp_str));

    act_type = AVIS_TYPE (avis);
    if (TYleTypes (act_type, type)) {
        DBUG_PRINT ("inferred type is precise enough");
    } else {
        if (AVIS_CONSTRTYPE (avis) != NULL) {
            if (TYleTypes (AVIS_CONSTRTYPE (avis), type)) {
                DBUG_PRINT ("strong enough constraint exists already");
            } else {
                AVIS_CONSTRTYPE (avis) = TYfreeType (AVIS_CONSTRTYPE (avis));
                AVIS_CONSTRTYPE (avis) = TYcopyType (type);
                /**
                 * for getting half-decent error-msgs, we copy the pos info
                 * into the  avis from where we will spread it upon code
                 * generation
                 */
                res = AVIS_CONSTRVAR (avis);
                NODE_LINE (res) = NODE_LINE (avis);
                NODE_FILE (res) = NODE_FILE (avis);
                DBUG_PRINT ("replacing existing constraint");
            }
        } else {
            AVIS_CONSTRTYPE (avis) = TYcopyType (type);
            res = TBmakeAvis (TRAVtmpVarName ("pred"),
                              TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
            AVIS_CONSTRVAR (avis) = res;
            /**
             * for getting half-decent error-msgs, we copy the pos info
             * into the  avis from where we will spread it upon code
             * generation
             */
            NODE_LINE (res) = NODE_LINE (avis);
            NODE_FILE (res) = NODE_FILE (avis);
            DBUG_PRINT ("constraint added");
        }
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCaddFunConstraint( node *expr);
 *
 *  @brief add a function constraint expr to the free variables contained in
 *     it. expr either is a (flattened) N_prf or an N_ap! By means of a call
 *     to IDCinsertConstraints() this will either lead to:
 *         a1`, ..., am`, p = prf( a1, ..., an);
 *     or it will lead to:
 *         p = udf( a1, ..., an);
 *         a1`, ..., an` = _guard_( p, a1, ..., an);
 *     depending on whether expr is  prf( a1, ..., an) or udf( a1, ..., an).
 *     Requires IDCinit() to have been called on the function that the
 *     expression is being located in.
 *
 *  @param expr: prf-expression or ap-expression that introduces a constraint
 *               to its arguments.
 *
 *  @return N_avis node of the predicate p
 *
 ***************************************************************************/

node *
IDCaddFunConstraint (node *expr)
{
    node *args, *avis, *res;

    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap),
                 "illegal expr in IDCaddFunConstraint");

    DBUG_PRINT ("constraint requested: %s", (NODE_TYPE (expr) == N_prf ? "prf" : "udf"));

    args = AP_OR_PRF_ARGS (expr);
    avis = FindAvisOfLastDefinition (args);

    if (avis == NULL) {
        CTIwarnLoc (NODE_LOCATION (expr), "illegal requirement ignored!");
        res = NULL;
    } else {
        res = TBmakeAvis (TRAVtmpVarName ("pred"),
                          TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        AVIS_CONSTRSET (avis) = TBmakeConstraint (res, expr, AVIS_CONSTRSET (avis));
        /**
         * for getting half-decent error-msgs, we copy the pos info
         * into the  avis from where we will spread it upon code
         * generation
         */
        NODE_LINE (res) = NODE_LINE (expr);
        NODE_FILE (res) = NODE_FILE (expr);
        DBUG_PRINT ("constraint added to %s", AVIS_NAME (avis));
    }

    DBUG_RETURN (res);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCinsertConstraints( node *fundef, bool all)
 *
 *  @brief inserts all constraints that have been added since IDCinit().
 *     This includes appropriate vardec insertions and appropriate
 *     renamings of bound identifiers.
 *     The new code should be inserted as early as possible in order
 *     to maximise the "back-propagation-effect" of the constraints.
 *     Issues a runtime error  in case IDCinit() has not been called yet.
 *
 *  @param fundef: the function to be modified
 *         all:    indicator whether to be used on the entire chain
 *                 or on one fundef only
 *
 *  @return the modified function
 *
 ***************************************************************************/

node *
IDCinsertConstraints (node *fundef, bool all)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "IDCinsertConstraints called on nun-fundef!");

    TRAVpush (TR_idc);

    arg_info = MakeInfo ();
    INFO_ALL (arg_info) = all;
    INFO_MODE (arg_info) = IDC_insert;

    fundef = TRAVdo (fundef, arg_info);

    arg_info = FreeInfo (arg_info);
    TRAVpop ();

    DBUG_RETURN (fundef);
}

/** <!--******************************************************************-->
 *
 * @fn node *IDCfinalize( node *fundef, bool all)
 *
 *  @brief finalizes the constraint system for the given fundef.
 *         Iff all is true apply finalization to the entire fundef chain
 *
 *  @param fundef: fundef to be ifinalized
 *         all:    indicator whether to be used on the entire chain
 *                 or on one fundef only
 *  @return the finalized fundef (chain)

 ***************************************************************************/

node *
IDCfinalize (node *fundef, bool all)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "IDCfinalize called on nun-fundef!");

    TRAVpush (TR_idc);

    arg_info = MakeInfo ();
    INFO_ALL (arg_info) = all;
    INFO_MODE (arg_info) = IDC_finalize;

    fundef = TRAVdo (fundef, arg_info);

    arg_info = FreeInfo (arg_info);
    TRAVpop ();

    DBUG_RETURN (fundef);
}

#undef DBUG_PREFIX
