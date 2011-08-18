/*
 *
 * $Id$
 */

/**
 *
 * @file insert_domain_constraints.c
 *
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
    int level;
    int code;
};

#define INFO_ALL(n) ((n)->all)
#define INFO_MODE(n) ((n)->mode)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_POSTASSIGN(n) ((n)->post)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_CODE(n) ((n)->code)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_ALL (result) = FALSE;
    INFO_MODE (result) = IDC_init;
    INFO_COUNTER (result) = 0;
    INFO_POSTASSIGN (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_CODE (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

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
    AVIS_SUBSTLVL (old_avis) = INFO_LEVEL (arg_info);
    AVIS_SUBSTCD (old_avis) = INFO_CODE (arg_info);

    DBUG_RETURN (avis);
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
        expr = TCmakePrf2 (F_type_constraint, TBmakeType (AVIS_CONSTRTYPE (avis)),
                           TBmakeId (avis));
        expr = TRAVdo (expr, arg_info);
        arg_info = BuildPrfConstraint (AVIS_CONSTRVAR (avis), expr, arg_info);
        AVIS_CONSTRVAR (avis) = NULL;
        AVIS_CONSTRTYPE (avis) = NULL;
    }

    if (AVIS_CONSTRSET (avis) != NULL) {
        constraint = AVIS_CONSTRSET (avis);
        AVIS_CONSTRSET (avis) = CONSTRAINT_NEXT (constraint);
        CONSTRAINT_NEXT (constraint) = NULL;

        arg_info = HandleConstraints (avis, arg_info);

        CONSTRAINT_EXPR (constraint) = TRAVdo (CONSTRAINT_EXPR (constraint), arg_info);
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

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
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
    BLOCK_ASSIGNS (arg_node) = TRAVdo (BLOCK_ASSIGNS (arg_node), arg_info);

    if (post_assign != NULL) {
        if (NODE_TYPE (BLOCK_ASSIGNS (arg_node)) == N_empty) {
            BLOCK_ASSIGNS (arg_node) = FREEdoFreeNode (BLOCK_ASSIGNS (arg_node));
            BLOCK_ASSIGNS (arg_node) = post_assign;
        } else {
            BLOCK_ASSIGNS (arg_node)
              = TCappendAssign (post_assign, BLOCK_ASSIGNS (arg_node));
        }
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
 * @fn node *IDCwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * part needs to be traversed BEFORE code so that the N_ids of
     * the generator variable obtain the right AVIS_POS, i.e., one
     * that is smaller than any variable defined in the code.
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    INFO_LEVEL (arg_info)++;
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    INFO_LEVEL (arg_info)--;
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *IDCwithid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
IDCwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /**
     * for proper renaming of upper and lower bounds, we need to make sure
     * the INFO_LEVEL in the parts need to reflect the outer level.
     * However, the withid(s) should be traversed with the inner level!!
     * see bug 417 for details.
     */
    INFO_LEVEL (arg_info)++;
    arg_node = TRAVcont (arg_node, arg_info);
    INFO_LEVEL (arg_info)--;

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
        INFO_CODE (arg_info)++;
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        INFO_CODE (arg_info)--;
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

    while ((AVIS_SUBST (ID_AVIS (arg_node)) != NULL)
           && (AVIS_SUBSTLVL (ID_AVIS (arg_node)) <= INFO_LEVEL (arg_info))
           && (AVIS_SUBSTCD (ID_AVIS (arg_node)) == INFO_CODE (arg_info))) {
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
            /**
             * do NOT increment counter here as all args are tagged 1!
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
    char *tmp_str;
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
        CTIwarnLine (NODE_LINE (expr), "illegal requirement ignored!");
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
