/*
 *
 * $Id$
 */

/**
 *
 * @file insert_domain_constraints.c
 *
 */

#include "dbug.h"

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
};

#define INFO_ALL(n) ((n)->all)
#define INFO_MODE(n) ((n)->mode)
#define INFO_COUNTER(n) ((n)->counter)
#define INFO_POSTASSIGN(n) ((n)->post)
#define INFO_VARDECS(n) ((n)->vardecs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ALL (result) = FALSE;
    INFO_MODE (result) = IDC_init;
    INFO_COUNTER (result) = 0;
    INFO_POSTASSIGN (result) = NULL;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

node *
FindAvisOfLastDefinition (node *exprs)
{
    node *avis, *last_avis = NULL;
    node *expr;

    DBUG_ENTER ("FindAvisOfLastDefinition");

    while (exprs != NULL) {
        expr = EXPRS_EXPR (exprs);
        if (NODE_TYPE (expr) == N_id) {
            avis = ID_AVIS (expr);
            DBUG_ASSERT (ASSIGN_POS (AVIS_SSAASSIGN (avis)) > 0,
                         "IDCaddConstraint used before IDCinit()!");
            if ((last_avis == NULL)
                || (ASSIGN_POS (AVIS_SSAASSIGN (last_avis))
                    < ASSIGN_POS (AVIS_SSAASSIGN (avis)))) {
                last_avis = avis;
            }
        }

        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (last_avis);
}

node *
CreateNewVarAndInitiateRenaming (node *id, info *arg_info)
{
    node *old_avis, *avis;
    DBUG_ENTER ("CreateNewVarAndInitiateRenaming");
    old_avis = ID_AVIS (id);
    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (old_avis)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    AVIS_SUBST (old_avis) = avis;

    DBUG_RETURN (avis);
}

node *
DupIdExprsWithoutDuplicates (node *exprs)
{
    node *args;
    bool found;
    node *tmp, *avis;

    DBUG_ENTER ("DupIdExprsWithoutDuplicates");

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

node *
BuildDataFlowHook (node *ids, node *expr, info *arg_info)
{
    node *exprs, *assign, *avis;
    int i;

    DBUG_ENTER ("BuildDataFlowHook");

    exprs = PRF_ARGS (expr);

    if (PRF_PRF (expr) == F_type_constraint) {
        exprs = EXPRS_NEXT (exprs);
    }

    assign = TBmakeAssign (NULL, NULL);

    for (i = 0; i < ndf_rets[PRF_PRF (expr)]; i++) {
        avis = CreateNewVarAndInitiateRenaming (EXPRS_EXPR (exprs), arg_info);
        ids = TBmakeIds (avis, ids);
        AVIS_SSAASSIGN (avis) = assign;
        exprs = EXPRS_NEXT (exprs);
    }

    ASSIGN_INSTR (assign) = TBmakeLet (ids, expr);

    /**
     * assign needs to be put at the very end due to potential data
     * dependencies!
     */
    INFO_POSTASSIGN (arg_info) = TCappendAssign (INFO_POSTASSIGN (arg_info), assign);

    DBUG_RETURN (assign);
}

info *
BuildPrfConstraint (node *pavis, node *expr, info *arg_info)
{
    node *assign;

    DBUG_ENTER ("BuildPrfConstraint");

    INFO_VARDECS (arg_info) = TBmakeVardec (pavis, INFO_VARDECS (arg_info));
    assign = BuildDataFlowHook (TBmakeIds (pavis, NULL), expr, arg_info);
    AVIS_SSAASSIGN (pavis) = assign;

    DBUG_RETURN (arg_info);
}

info *
BuildUdfConstraint (node *pavis, node *expr, info *arg_info)
{
    node *assign;

    DBUG_ENTER ("BuildUdfConstraint");

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (pavis, NULL), expr), NULL);
    AVIS_SSAASSIGN (pavis) = assign;

    INFO_POSTASSIGN (arg_info) = TCappendAssign (INFO_POSTASSIGN (arg_info), assign);

    expr = TBmakePrf (F_guard, DupIdExprsWithoutDuplicates (AP_ARGS (expr)));
    assign = BuildDataFlowHook (NULL, expr, arg_info);

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
    DBUG_ENTER ("IDCfundef");

    INFO_COUNTER (arg_info) = 1;

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_ALL (arg_info) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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

    DBUG_ENTER ("IDCassign");

    switch (INFO_MODE (arg_info)) {
    case IDC_init:
        ASSIGN_POS (arg_node) = INFO_COUNTER (arg_info);
        INFO_COUNTER (arg_info)++;

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        break;
    case IDC_finalize:
        ASSIGN_POS (arg_node) = 0;

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        break;
    default:
        break;
    }
    post_assign = INFO_POSTASSIGN (arg_info);
    INFO_POSTASSIGN (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (post_assign != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (post_assign, ASSIGN_NEXT (arg_node));
    }

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
    node *expr, *avis, *constraint;

    DBUG_ENTER ("IDCids");

    if (INFO_MODE (arg_info) == IDC_insert) {

        avis = IDS_AVIS (arg_node);
        if (AVIS_CONSTRTYPE (avis) != NULL) {
            expr = TCmakePrf2 (F_type_constraint, TBmakeType (AVIS_CONSTRTYPE (avis)),
                               TBmakeId (avis));
            expr = TRAVdo (expr, arg_info);
            arg_info = BuildPrfConstraint (AVIS_CONSTRVAR (avis), expr, arg_info);
        }

        while (AVIS_CONSTRSET (avis) != NULL) {
            constraint = AVIS_CONSTRSET (avis);
            CONSTRAINT_EXPR (constraint)
              = TRAVdo (CONSTRAINT_EXPR (constraint), arg_info);

            if (NODE_TYPE (CONSTRAINT_EXPR (constraint)) == N_prf) {
                arg_info = BuildPrfConstraint (CONSTRAINT_PREDAVIS (constraint),
                                               CONSTRAINT_EXPR (constraint), arg_info);
            } else {
                arg_info = BuildUdfConstraint (CONSTRAINT_PREDAVIS (constraint),
                                               CONSTRAINT_EXPR (constraint), arg_info);
            }
            AVIS_CONSTRSET (avis) = CONSTRAINT_NEXT (constraint);
            constraint = FREEdoFreeNode (constraint);
        }

        if (IDS_NEXT (arg_node) != NULL) {
            IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
        }
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
    DBUG_ENTER ("IDCid");

    if (AVIS_SUBST (ID_AVIS (arg_node)) != NULL) {
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
    DBUG_ENTER ("IDCavis");

    if ((INFO_MODE (arg_info) == IDC_init) || (INFO_MODE (arg_info) == IDC_finalize)) {
        AVIS_SUBST (arg_node) = NULL;
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

    DBUG_ENTER ("IDCinitialize");

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
 *  @return N_avis node of the predicate
 *
 ***************************************************************************/

node *
IDCaddTypeConstraint (ntype *type, node *avis)
{
    node *res;
    ntype *act_type;
#ifndef DBUG_OFF
    char *tmp_str;
#endif

    DBUG_ENTER ("IDCaddTypeConstraint");

    DBUG_EXECUTE ("NTC", tmp_str = TYtype2String (type, FALSE, 0););
    DBUG_PRINT ("IDC",
                ("type constraint requested: %s for %s", tmp_str, AVIS_NAME (avis)));
    DBUG_EXECUTE ("IDC", tmp_str = MEMfree (tmp_str););

    act_type = AVIS_TYPE (avis);
    if (TYleTypes (act_type, type)) {
        DBUG_PRINT ("IDC", ("inferred type is precise enough"));
    } else {
        if (AVIS_CONSTRTYPE (avis) != NULL) {
            if (TYleTypes (AVIS_CONSTRTYPE (avis), type)) {
                DBUG_PRINT ("IDC", ("strong enough constraint exists already"));
            } else {
                AVIS_CONSTRTYPE (avis) = TYfreeType (AVIS_CONSTRTYPE (avis));
                AVIS_CONSTRTYPE (avis) = type;
                /**
                 * for getting half-decent error-msgs, we copy the pos info
                 * into the  avis from where we will spread it upon code
                 * generation
                 */
                res = AVIS_CONSTRVAR (avis);
                NODE_LINE (res) = NODE_LINE (avis);
                NODE_FILE (res) = NODE_FILE (avis);
                DBUG_PRINT ("IDC", ("replacing existing constraint"));
            }
        } else {
            AVIS_CONSTRTYPE (avis) = type;
            res = TBmakeAvis (TRAVtmpVar (),
                              TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
            AVIS_CONSTRVAR (avis) = res;
            /**
             * for getting half-decent error-msgs, we copy the pos info
             * into the  avis from where we will spread it upon code
             * generation
             */
            NODE_LINE (res) = NODE_LINE (avis);
            NODE_FILE (res) = NODE_FILE (avis);
            DBUG_PRINT ("IDC", ("constraint added"));
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

    DBUG_ENTER ("IDCaddPrfConstraint");

    DBUG_ASSERT ((NODE_TYPE (expr) == N_prf) || (NODE_TYPE (expr) == N_ap),
                 "illegal expr in IDCaddFunConstraint");

    DBUG_PRINT ("IDC", ("constraint requested: %s",
                        (NODE_TYPE (expr) == N_prf ? "prf" : "udf")));

    args = AP_OR_PRF_ARGS (expr);
    avis = FindAvisOfLastDefinition (args);

    if (avis == NULL) {
        CTIwarnLine (NODE_LINE (expr), "illegal requirement ignored!");
        res = NULL;
    } else {
        res = TBmakeAvis (TRAVtmpVar (),
                          TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
        AVIS_CONSTRSET (avis) = TBmakeConstraint (res, expr, AVIS_CONSTRSET (avis));
        /**
         * for getting half-decent error-msgs, we copy the pos info
         * into the  avis from where we will spread it upon code
         * generation
         */
        NODE_LINE (res) = NODE_LINE (expr);
        NODE_FILE (res) = NODE_FILE (expr);
        DBUG_PRINT ("IDC", ("constraint added to %s", AVIS_NAME (avis)));
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

    DBUG_ENTER ("IDCinsertConstraints");

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

    DBUG_ENTER ("IDCfinalize");

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
