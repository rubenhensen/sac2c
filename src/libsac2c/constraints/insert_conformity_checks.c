#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "insert_domain_constraints.h"
#include "LookUpTable.h"
#include "memory.h"
#include "new_types.h"
#include "shape.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"

#define DBUG_PREFIX "ICC"
#include "debug.h"

#include "insert_conformity_checks.h"

typedef node *(*iccfun_p) (node *, node *);

/******************************************************************************
 *
 * @struct INFO
 *
 * @param INFO_PREASSIGNS The pre-guard functions that must be inserted.
 * With-loops might be nested, so we might get multiple guards.
 * @param INFO_POSTASSIGNS The post-guard functions that must be inserted.
 * With-loops might be nested, so we might get multiple guards.
 * @param INFO_VARDECS N_vardec chain of the current block.
 * @param INFO_LHS LHS identifiers of the current let.
 * @param INFO_WLIDS N_exprs chain of identifiers in the with-loop that have
 * at least one conformity check, and should be guarded by a pre-gaurd. This
 * chain directly becomes (part of) the actual arguments chain of the generated
 * guard function, and thus we populate this chain with copies of the
 * encountered identifiers!
 * @param INFO_WLIDSUBST A lookup-table that holds the mapping from the original
 * with-loop identifier AVISes, to the guarded AVISes returned by the guard
 * function. All occurences of these original variables should be updated to
 * their guarded variants, but within this with-loop only.
 * @param INFO_WLPREDS N_exprs chain of identifiers for the predicates that
 * were generated for the current with-loop.
 * @param INFO_GENERATOR The current with-loop generator.
 * @param INFO_WITHOPS The current with-loop ops.
 * @param INFO_CEXPRS The current with-loop cexprs.
 *
 ******************************************************************************/
struct INFO {
    node *preassigns;
    node *postassigns;
    node *vardecs;
    node *lhs;
    node *wlids;
    lut_t *wlidsubst;
    node *wlpreds;
    node *generator;
    node *withops;
    node *cexprs;
};

#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WLIDS(n) ((n)->wlids)
#define INFO_WLIDSUBST(n) ((n)->wlidsubst)
#define INFO_WLPREDS(n) ((n)->wlpreds)
#define INFO_GENERATOR(n) ((n)->generator)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_CEXPRS(n) ((n)->cexprs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WLIDS (result) = NULL;
    INFO_WLIDSUBST (result) = LUTgenerateLut ();
    INFO_WLPREDS (result) = NULL;
    INFO_GENERATOR (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_CEXPRS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_WLIDSUBST (info) = LUTremoveLut (INFO_WLIDSUBST (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn node *MakeGuard (node *lhs, node *args, node *types, node *preds,
 *                      const char *msg)
 *
 * @brief Creates a new guard primitive function and assignment from the given
 * lhs, and the given arguments (the guarded arguments, their types, and the
 * predicates that must hold).
 *
 * @note SSA assigns of the lhs are updated accordingly.
 *
 ******************************************************************************/
static node *
MakeGuard (node *lhs, node *args, node *types, node *preds, const char *msg)
{
    node *prf, *assign;

    DBUG_ENTER ();

    // Append types and predicates to actual arguments
    args = TCappendExprs (args, TCappendExprs (types, preds));

    // Create guard
    prf = TBmakePrf (F_guard, args);
    PRF_NUMVARIABLERETS (prf) = TCcountIds (lhs);
    PRF_CONTEXTSTRING (prf) = STRcpy (msg);
    assign = TBmakeAssign (TBmakeLet (lhs, prf), NULL);

    // Update SSA assigns
    while (lhs != NULL) {
        AVIS_SSAASSIGN (IDS_AVIS (lhs)) = assign;
        lhs = IDS_NEXT (lhs);
    }

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * @fn node *EmitPreGuard (node **prf_args, node **vardecs, node *preds,
 *                         lut_t *substitutions, const char *msg)
 *
 * @brief Creates a new pre-guard assignment, given input arguments to a
 * primitive function, and a chain of predicates that must hold. These
 * arguments, as well as the vardecs chain, are updated accordingly.
 *
 * @example Given a primitive function
 *   y1, .., ym = _prf_ (x1, .., xn);
 * We generate
 *   icc1, .., iccn = _guard_ (x1, .., xn, type1, .., typem, pred1, .., predo);
 *   y1, .., ym = _prf_ (icc1, .., iccn);
 *
 * @note The prf_args might contain multiple identifiers with the same AVIS.
 * If an identifier is encountered with an AVIS that was already seen
 * previously, that second identifier not added to the guard's arguments.
 *
 * @return The generated guard assignment.
 *
 ******************************************************************************/
static node *
EmitPreGuard (node **prf_args /* inout */, node **vardecs /* inout */,
              node *preds, lut_t *substitutions, const char *msg)
{
    node *avis;
    node *orig_args, *args, *prev_arg;
    node *lhs = NULL, *typ, *typs = NULL;
    node *assign;

    DBUG_ENTER ();

    DBUG_PRINT ("Emitting pre-guard");

    orig_args = args = *prf_args;
    *prf_args = NULL;
    prev_arg = NULL;

    while (args != NULL) {
        avis = TBmakeAvis (TRAVtmpVar (),
                           TYcopyType (ID_NTYPE (EXPRS_EXPR (args))));

        /**
         * Because we need to be able to update all occurrences of guarded
         * variables in with-loops, prf_args might contain multiple identifiers
         * with the same AVIS. If we encounter such a case, we can remove that
         * identifiers from the guard's actual arguments at this point.
         */
        if (LUTsearchInLutP (substitutions, ID_AVIS (EXPRS_EXPR (args))) != NULL) {
            DBUG_PRINT ("Substitution for %s already exists, "
                        "removing it from the guard arguments",
                        ID_NAME (EXPRS_EXPR (args)));

            /**
             * Remove this argument from the guard's actual arguments. Because
             * we populate the prf_args with copies of the original arguments,
             * we are required to free these arguments at this point.
             */
            args = FREEdoFreeNode (args);
            if (prev_arg == NULL) {
                orig_args = args;
            } else {
                EXPRS_NEXT (prev_arg) = args;
            }

            continue;
        }

        DBUG_PRINT ("Setting substitution %s -> %s",
                    ID_NAME (EXPRS_EXPR (args)), AVIS_NAME (avis));
        substitutions = LUTinsertIntoLutP (substitutions,
                                           ID_AVIS (EXPRS_EXPR (args)),
                                           avis);

        // Add avis to declarations
        *vardecs = TBmakeVardec (avis, *vardecs);

        // Update arguments
        *prf_args = TCappendExprs (*prf_args, TBmakeExprs (TBmakeId (avis), NULL));

        // Convert avis to required nodes
        lhs = TCappendIds (lhs, TBmakeIds (avis, NULL));
        typ = TBmakeType (TYcopyType (AVIS_TYPE (avis)));
        typs = TCappendExprs (typs, TBmakeExprs (typ, NULL));

        prev_arg = args;
        args = EXPRS_NEXT (args);
    }

    assign = MakeGuard (lhs, orig_args, typs, preds, msg);

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * @fn node *EmitPostGuard (node **prf_lhs, node **vardecs, node *preds,
 *                          const char *msg)
 *
 * @brief Creates a new post-guard assignment, given a resulting lhs from a
 * primitive function, and a chain of predicates that must hold. This lhs, as
 * well as the vardecs chain, are updated accordingly.
 *
 * @example Given a primitive function
 *   y1, .., ym = _prf_ (x1, .., xn);
 * We generate
 *   icc1, .., iccm = _prf_ (x1, .., xn);
 *   y1, .., yn = _guard_ (icc1, .., iccm, type1, .., typem, pred1, .., predo);
 *
 * @return The generated guard assignment.
 *
 ******************************************************************************/
static node *
EmitPostGuard (node **prf_lhs /* inout */, node **vardecs /* inout */,
               node *preds, const char *msg)
{
    node *avis;
    node *orig_lhs, *lhs;
    node *args = NULL, *typ, *typs = NULL;
    node *assign;

    DBUG_ENTER ();

    DBUG_PRINT ("Emitting post-guard");

    orig_lhs = lhs = *prf_lhs;
    *prf_lhs = NULL;

    while (lhs != NULL) {
        avis = TBmakeAvis (TRAVtmpVar (),
                           TYcopyType (AVIS_TYPE (IDS_AVIS (lhs))));

        // Add avis to declarations
        *vardecs = TBmakeVardec (avis, *vardecs);

        // Update lhs
        *prf_lhs = TCappendIds (*prf_lhs, TBmakeIds (avis, NULL));

        // Convert avis to required nodes
        args = TCappendExprs (args, TBmakeExprs (TBmakeId (avis), NULL));
        typ = TBmakeType (TYcopyType (AVIS_TYPE (avis)));
        typs = TCappendExprs (typs, TBmakeExprs (typ, NULL));

        lhs = IDS_NEXT (lhs);
    }

    assign = MakeGuard (orig_lhs, args, typs, preds, msg);

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * @fn ntype *ArgEncodingToTypeConstraint (prf fun, unsigned int argno,
 *                                         ntype *scalartype)
 *
 * @brief Returns a type constraint for the given prf argument and scalartype.
 *
 * @param fun The primitive function.
 * @param argno Number of argument.
 * @param scalartype The scalartype for the constraint.
 *
 * @returns The constraint type or NULL if none needed.
 *
 ******************************************************************************/
static ntype *
ArgEncodingToTypeConstraint (prf fun, unsigned int argno, ntype *scalartype)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    switch (PRF_ARGENCODING (fun, argno)) {
    case PA_S:
        res = TYmakeAKS (TYcopyType (scalartype), SHmakeShape (0));
        break;

    case PA_V:
        res = TYmakeAKD (TYcopyType (scalartype), 1, SHmakeShape (0));
        break;

    /**
     * Nothing to be done here.
     */
    case PA_A:
    case PA_x:
        break;

    default:
        DBUG_UNREACHABLE ("Unknown arg encoding found");
        break;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *EmitConstraint (node *ids, node *constraint)
 *
 * @brief Emits the given constraint and appends the corresponding guard
 * expression, if any, to the Exprs chain ids.
 *
 * @param ids Chain of N_exprs node.
 * @param constraint The constraint (N_ap or N_prf).
 *
 * @returns The amended N_exprs chain.
 *
 ******************************************************************************/
static node *
EmitConstraint (node *ids, node *constraint)
{
    node *avis;

    DBUG_ENTER ();

#ifndef DBUG_OFF
    if (NODE_TYPE (constraint) == N_prf) {
        DBUG_PRINT ("Emitting %s", PRF_NAME (PRF_PRF (constraint)));
    } else {
        DBUG_PRINT ("Emitting fun constraint");
    }
#endif

    avis = IDCaddFunConstraint (constraint);
    if (avis != NULL) {
        ids = TBmakeExprs (TBmakeId (avis), ids);
    }

    DBUG_RETURN (ids);
}

static node *
EmitTypeConstraint (node *ids, node *arg, ntype *constraint)
{
    node *avis;

    DBUG_ENTER ();

    if (NODE_TYPE (arg) == N_id) {
        avis = IDCaddTypeConstraint (constraint, ID_AVIS (arg));
        if (avis != NULL) {
            ids = TBmakeExprs (TBmakeId (avis), ids);
        }
    }

    DBUG_RETURN (ids);
}

static node *
ICCnone (node *ids, node *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (ids);
}

static node *
ICCsameShape (node *ids, node *args)
{
    node *prf;

    DBUG_ENTER ();

    prf = TBmakePrf (F_same_shape_AxA, DUPdoDupTree (args));
    ids = EmitConstraint (ids, prf);

    DBUG_RETURN (ids);
}

static node *
ICCreshape (node *ids, node *args)
{
    node *prf;

    DBUG_ENTER ();

    prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (EXPRS_EXPR (args)));
    ids = EmitConstraint (ids, prf);

    prf = TBmakePrf (F_prod_matches_prod_shape_VxA, DUPdoDupTree (args));
    ids = EmitConstraint (ids, prf);

    DBUG_RETURN (ids);
}

static node *
ICCsel (node *ids, node *args)
{
    node *prf;

    DBUG_ENTER ();

    prf = TBmakePrf (F_shape_matches_dim_VxA, DUPdoDupTree (args));
    ids = EmitConstraint (ids, prf);

    prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (EXPRS_EXPR (args)));
    ids = EmitConstraint (ids, prf);

    prf = TBmakePrf (F_val_lt_shape_VxA, DUPdoDupTree (args));
    ids = EmitConstraint (ids, prf);

    DBUG_RETURN (ids);
}

static node *
ICCsimd_sel (node *ids, node *args)
{
    node *res;

    DBUG_ENTER ();

    /**
     * Throw away the first arg, and do the same as for a standard selection.
     * The first argument should be just a number which might be checked
     * elsewhere.
     */
    args = EXPRS_NEXT (args);
    res = ICCsel (ids, args);

    DBUG_RETURN (res);
}

static node *
ICCprfModarray (node *ids, node *args)
{
    node *prf;

    DBUG_ENTER ();

    prf = TCmakePrf2 (F_shape_matches_dim_VxA,
                      DUPdoDupTree (EXPRS_EXPR2 (args)),
                      DUPdoDupTree (EXPRS_EXPR (args)));
    ids = EmitConstraint (ids, prf);

    prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (EXPRS_EXPR2 (args)));
    ids = EmitConstraint (ids, prf);

    prf = TCmakePrf2 (F_val_lt_shape_VxA,
                      DUPdoDupTree (EXPRS_EXPR2 (args)),
                      DUPdoDupTree (EXPRS_EXPR (args)));
    ids = EmitConstraint (ids, prf);

    DBUG_RETURN (ids);
}

static node *
ICCvalMatchLen (node *ids, node *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (ids);
}

static iccfun_p iccfuns[] = {
#define PRFicc_fun(icc_fun) icc_fun
#include "prf_info.mac"
};

/******************************************************************************
 *
 * @fn node *ICCdoInsertConformityChecks (node *arg_node)
 *
 ******************************************************************************/
node *
ICCdoInsertConformityChecks (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_icc);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCfundef (node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain and inserts INFO_VARDECS into
 * the fundefs vardec chain. We do not need to traverse fundefs that are marked
 * as guard functions, since conformity checks have already been inserted into
 * these functions by type pattern analysis and resolution.
 *
 ******************************************************************************/
node *
ICCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("Traversing %s", FUNDEF_NAME (arg_node));

        arg_node = IDCinitialize (arg_node, FALSE);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (INFO_VARDECS (arg_info) != NULL) {
            FUNDEF_VARDECS (arg_node) =
                TCappendVardec (FUNDEF_VARDECS (arg_node),
                                INFO_VARDECS (arg_info));
            INFO_VARDECS (arg_info) = NULL;
        }

        arg_node = IDCinsertConstraints (arg_node, FALSE);
        arg_node = IDCfinalize (arg_node, FALSE);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCblock (node *arg_node, info *arg_info)
 *
 * @brief Stacks the INFO_LHS chain.
 *
 ******************************************************************************/
node *
ICCblock (node *arg_node, info *arg_info)
{
    node *lhs;

    DBUG_ENTER ();

    lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    INFO_LHS (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCassign (node *arg_node, info *arg_info)
 *
 * @brief Inserts guards around the current assign, but does not traverse the
 * inserted assigns.
 *
 ******************************************************************************/
node *
ICCassign (node *arg_node, info *arg_info)
{
    node *preassigns, *postassigns;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    INFO_WLIDSUBST (arg_info) = LUTremoveContentLut (INFO_WLIDSUBST (arg_info));

    /**
     * We have to store the pre- and post-assign here, as we do not want to
     * emit domain checks on the domain checks we have just created!
     */
    preassigns = INFO_PREASSIGNS (arg_info);
    postassigns = INFO_POSTASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;
    INFO_POSTASSIGNS (arg_info) = NULL;

    // Traverse next assign before adding pre- and post-assigns
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    // Append post-assigns after this assign
    if (postassigns != NULL) {
        DBUG_PRINT ("Inserting post-assign");
        postassigns = TCappendAssign (postassigns, ASSIGN_NEXT (arg_node));
        ASSIGN_NEXT (arg_node) = postassigns;
    }

    // Prepend pre-assigns before this assign
    if (preassigns != NULL) {
        DBUG_PRINT ("Inserting pre-assign");
        preassigns = TCappendAssign (preassigns, arg_node);
        arg_node = preassigns;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICClet (node *arg_node, info *arg_info)
 *
 * @brief Sets INFO_LHS before traversing the LET_EXPR and inserts the
 * (potentially) modified INFO_LHS back into the tree.
 *
 ******************************************************************************/
node *
ICClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    LET_IDS (arg_node) = INFO_LHS (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCprf (node *arg_node, info *arg_info)
 *
 * @brief Emits type and prf constraints for the given prf.
 *
 ******************************************************************************/
node *
ICCprf (node *arg_node, info *arg_info)
{
    node *args, *expr, *cids = NULL;
    ntype *scalar_type, *constraint_type;
    unsigned int arg_cnt;
    node *guard;

    DBUG_ENTER ();

    args = PRF_ARGS (arg_node);
    arg_cnt = 0;

    DBUG_PRINT ("Traversing prf %s", PRF_NAME (PRF_PRF (arg_node)));

    while (args != NULL) {
        expr = EXPRS_EXPR (args);

        // We only act on N_id arguments with proper array types (non-bottoms)
        if (NODE_TYPE (expr) == N_id && TYisArray (ID_NTYPE (expr))) {
            scalar_type = TYgetScalar (ID_NTYPE (expr));
            constraint_type = ArgEncodingToTypeConstraint (PRF_PRF (arg_node),
                                                           arg_cnt,
                                                           scalar_type);

            if (constraint_type != NULL) {
                DBUG_PRINT ("Emitting type constraint");
                cids = EmitTypeConstraint (cids, expr, constraint_type);
                constraint_type = TYfreeType (constraint_type);
            }
        }

        args = EXPRS_NEXT (args);
        arg_cnt++;
    }

    // Now handle non-type constraints
    if (iccfuns[PRF_PRF (arg_node)] != NULL) {
        cids = iccfuns[PRF_PRF (arg_node)](cids, PRF_ARGS (arg_node));
    }

    // If we have collected any constraints, we emit a guard
    if (cids != NULL) {
        char *msg = STRcatn (3, "Primitive function ",
                                global.prf_name[PRF_PRF (arg_node)],
                                " guard failed");

        guard = EmitPreGuard (&PRF_ARGS (arg_node),
                              &INFO_VARDECS (arg_info),
                              DUPdoDupTree (cids),
                              INFO_WLIDSUBST (arg_info),
                              msg);
        ASSIGN_NEXT (guard) = INFO_PREASSIGNS (arg_info);
        INFO_PREASSIGNS (arg_info) = guard;

        guard = EmitPostGuard (&INFO_LHS (arg_info),
                               &INFO_VARDECS (arg_info),
                               cids,
                               msg);
        ASSIGN_NEXT (guard) = INFO_POSTASSIGNS (arg_info);
        INFO_POSTASSIGNS (arg_info) = guard;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ANONreplaceWLId (node *arg_node, info *arg_info)
 *
 * @brief Check the identifiers occurring in the current with-loop, and
 * replace them by their substitution if there exist one in the LUT.
 *
 ******************************************************************************/
static node *
ANONreplaceWLId (node *arg_node, info *arg_info)
{
    void **lut_pointer;
    node *substitution;

    DBUG_ENTER ();

    lut_pointer = LUTsearchInLutP (INFO_WLIDSUBST (arg_info),
                                   ID_AVIS (arg_node));

    if (lut_pointer != NULL) {
        substitution = (node *)(*lut_pointer);
        DBUG_PRINT ("Substituting with-loop id %s with %s",
                    ID_NAME (arg_node), AVIS_NAME (substitution));
        ID_AVIS (arg_node) = substitution;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCwith (node *arg_node, info *arg_info)
 *
 * @brief Starts multiple traversals of the withloop.
 *
 ******************************************************************************/
node *
ICCwith (node *arg_node, info *arg_info)
{
    node *wlids, *wlpreds;
    node *guard;

    DBUG_ENTER ();

    DBUG_PRINT ("Traversing with-loop");

    // With-loops might be nested, so store previous values
    wlids = INFO_WLIDS (arg_info);
    wlpreds = INFO_WLPREDS (arg_info);
    INFO_WLIDS (arg_info) = NULL;
    INFO_WLPREDS (arg_info) = NULL;

    /**
     * Generate constraints for parts/generators/cexprs, including those
     * that depend on withops.
     */
    if (WITH_PART (arg_node) != NULL) {
        INFO_WITHOPS (arg_info) = WITH_WITHOP (arg_node);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        INFO_WITHOPS (arg_info) = NULL;
    }

    // Now the self contained constraints in withops
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    // Now go on with the code
    if (WITH_CODE (arg_node) != NULL) {
        INFO_WITHOPS (arg_info) = WITH_WITHOP (arg_node);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_WITHOPS (arg_info) = NULL;
    }

    // Insert guards
    if (INFO_WLPREDS (arg_info) != NULL) {
        DBUG_ASSERT (INFO_WLIDS (arg_info) != NULL,
                     "With-loop predicates have been generated, "
                     "but no corresponding identifiers are given");

        guard = EmitPreGuard (&INFO_WLIDS (arg_info),
                              &INFO_VARDECS (arg_info),
                              DUPdoDupTree (INFO_WLPREDS (arg_info)),
                              INFO_WLIDSUBST (arg_info),
                              "With-loop guard failed");
        ASSIGN_NEXT (guard) = INFO_PREASSIGNS (arg_info);
        INFO_PREASSIGNS (arg_info) = guard;

        DBUG_PRINT ("Updating with-loop identifiers");
        DBUG_EXECUTE_TAG (DBUG_PREFIX "_LUT",
                          LUTprintLut (stderr, INFO_WLIDSUBST (arg_info)));

        /**
         * The guard function returns new values that we should use in the
         * with-loop instead. Check all identifiers occurring in the with-loop,
         * and replace them by their substitutions given by the LUT, if any.
         * This approach is required instead of simply setting AVIS_SUBST, as
         * this can also update occurences of these variables occurring before
         * the guard function.
         */
        TRAVpushAnonymous ((anontrav_t[]){{N_id, &ANONreplaceWLId},
                                          {(nodetype)0, NULL}},
                           &TRAVsons);
        arg_node = TRAVcont (arg_node, arg_info);
        TRAVpop ();

        guard = EmitPostGuard (&INFO_LHS (arg_info),
                               &INFO_VARDECS (arg_info),
                               INFO_WLPREDS (arg_info),
                               "With-loop guard failed");
        ASSIGN_NEXT (guard) = INFO_POSTASSIGNS (arg_info);
        INFO_POSTASSIGNS (arg_info) = guard;
    }

    INFO_WLIDS (arg_info) = wlids;
    INFO_WLPREDS (arg_info) = wlpreds;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCgenerator (node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for the generator, Starts a traversal of
 * INFO_WITHOPS with INFO_GENERATOR set to the current node to insert
 * generator/operator constraints.
 *
 ******************************************************************************/
node *
ICCgenerator (node *arg_node, info *arg_info)
{
    ntype *constraint_type;
    node *bound1, *bound2, *width, *step;
    node *prf;

    DBUG_ENTER ();

    // Ensure all are int[.]
    constraint_type = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));

    bound1 = GENERATOR_BOUND1 (arg_node);
    bound2 = GENERATOR_BOUND2 (arg_node);
    width = GENERATOR_WIDTH (arg_node);
    step = GENERATOR_STEP (arg_node);

    INFO_WLPREDS (arg_info) =
        EmitTypeConstraint (INFO_WLPREDS (arg_info), bound1, constraint_type);
    INFO_WLPREDS (arg_info) =
        EmitTypeConstraint (INFO_WLPREDS (arg_info), bound2, constraint_type);

    if (width != NULL) {
        INFO_WLPREDS (arg_info) =
            EmitTypeConstraint (INFO_WLPREDS (arg_info), width, constraint_type);
    }

    if (step != NULL) {
        INFO_WLPREDS (arg_info) =
            EmitTypeConstraint (INFO_WLPREDS (arg_info), step, constraint_type);
    }

    constraint_type = TYfreeType (constraint_type);

    // Ensure bounds are non negative
    prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (bound1));
    INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);
    INFO_WLIDS (arg_info) = TBmakeExprs (DUPdoDupNode (bound1), INFO_WLIDS (arg_info));

    prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (bound2));
    INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);
    INFO_WLIDS (arg_info) = TBmakeExprs (DUPdoDupNode (bound2), INFO_WLIDS (arg_info));

    if (width != NULL) {
        prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (width));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);
        INFO_WLIDS (arg_info) = TBmakeExprs (DUPdoDupNode (width), INFO_WLIDS (arg_info));
    }

    if (step != NULL) {
        prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (step));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);
        INFO_WLIDS (arg_info) = TBmakeExprs (DUPdoDupNode (step), INFO_WLIDS (arg_info));
    }

    // Now generate generate/withop constraints
    INFO_GENERATOR (arg_info) = arg_node;
    INFO_WITHOPS (arg_info) = TRAVopt (INFO_WITHOPS (arg_info), arg_info);
    INFO_GENERATOR (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCcode (node *arg_node, info *arg_info)
 *
 * @brief Traverses the code and inserts constraints for the cexprs by
 * traversing INFO_WITHOPS with INFO_CEXPRS set.
 *
 ******************************************************************************/
node *
ICCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    // Emit withop/cexprs constraints
    if (INFO_WITHOPS (arg_info) != NULL) {
        INFO_CEXPRS (arg_info) = CODE_CEXPRS (arg_node);
        INFO_WITHOPS (arg_info) = TRAVdo (INFO_WITHOPS (arg_info), arg_info);
        DBUG_ASSERT (INFO_CEXPRS (arg_info) == NULL,
                     "Not all cexprs handled by withops");
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCgenarray (node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for genarray. For different modes see ICCcode
 * and ICCgenerator.
 *
 ******************************************************************************/
node *
ICCgenarray (node *arg_node, info *arg_info)
{
    ntype *constraint_type;
    node *shape, *def, *bound1, *bound2;
    node *prf;

    DBUG_ENTER ();

    shape = GENARRAY_SHAPE (arg_node);
    def = GENARRAY_DEFAULT (arg_node);

    if (INFO_GENERATOR (arg_info) != NULL) {
        bound1 = GENERATOR_BOUND1 (INFO_GENERATOR (arg_info));
        bound2 = GENERATOR_BOUND2 (INFO_GENERATOR (arg_info));

        // Emit generator-dependent constraints
        prf = TCmakePrf2 (F_same_shape_AxA, DUPdoDupTree (bound1), DUPdoDupTree (shape));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);

        prf = TCmakePrf2 (F_val_le_val_VxV, DUPdoDupTree (bound1), DUPdoDupTree (shape));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);

        prf = TCmakePrf2 (F_same_shape_AxA, DUPdoDupTree (bound2), DUPdoDupTree (shape));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);

        prf = TCmakePrf2 (F_val_le_val_VxV, DUPdoDupTree (bound2), DUPdoDupTree (shape));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        // Emit cexpr constraints iff we have a default value
        if (def != NULL) {
            DBUG_PRINT ("Emitting F_same_shape_AxA CEXPR-constraint");

            /**
             * We simply ignore the returned avis, as we do not propagate the
             * boolean guard out of the withloop.
             */
            prf = TCmakePrf2 (F_same_shape_AxA,
                              DUPdoDupTree (EXPRS_EXPR (INFO_CEXPRS (arg_info))),
                              DUPdoDupTree (def));
            IDCaddFunConstraint (prf);
            INFO_WLIDS (arg_info) = TBmakeExprs (DUPdoDupNode (def), INFO_WLIDS (arg_info));
        }

        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        // Emit withop-local constraints
        constraint_type = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));
        INFO_WLPREDS (arg_info) =
            EmitTypeConstraint (INFO_WLPREDS (arg_info), shape, constraint_type);
        constraint_type = TYfreeType (constraint_type);

        prf = TCmakePrf1 (F_non_neg_val_V, DUPdoDupTree (shape));
        INFO_WLPREDS (arg_info) = EmitConstraint (INFO_WLPREDS (arg_info), prf);
        INFO_WLIDS (arg_info) = TBmakeExprs (DUPdoDupNode (shape), INFO_WLIDS (arg_info));
    }

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCmodarray (node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for modarray. For different modes see ICCcode
 * and ICCgenerator.
 *
 ******************************************************************************/
node *
ICCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        // Emit generator dependent constraints
        /* TODO */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        // Emit cexpr constraints
        /* TODO */
        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        // Emit withop-local constraints
        /* TODO */
    }

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCfold (node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for fold. For different modes see ICCcode
 * and ICCgenerator.
 *
 ******************************************************************************/
node *
ICCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        // Emit generator dependent constraints
        /* TODO */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        // Emit cexpr constraints
        /* TODO */
        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        // Emit withop-local constraints
        /* TODO */
    }

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCbreak (node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for foldfix. For different modes see ICCcode
 * and ICCgenerator.
 *
 ******************************************************************************/
node *
ICCbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        // Emit generator dependent constraints
        /* TODO */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        // Emit cexpr constraints
        /* TODO */
        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        // Emit withop-local constraints
        /* TODO */
    }

    BREAK_NEXT (arg_node) = TRAVopt (BREAK_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *ICCpropagate (node *arg_node, info *arg_info)
 *
 * @brief Inserts constraints for propagate. For different modes see ICCcode
 * and ICCgenerator.
 *
 ******************************************************************************/
node *
ICCpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_GENERATOR (arg_info) != NULL) {
        // Emit generator dependent constraints
        /* TODO */
    } else if (INFO_CEXPRS (arg_info) != NULL) {
        // Emit cexpr constraints
        /* TODO */
        INFO_CEXPRS (arg_info) = EXPRS_NEXT (INFO_CEXPRS (arg_info));
    } else {
        // Emit withop-local constraints
    }

    PROPAGATE_NEXT (arg_node) = TRAVopt (PROPAGATE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
