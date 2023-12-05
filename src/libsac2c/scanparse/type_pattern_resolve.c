/******************************************************************************
 *
 * This file contains two traversals, RTPF and RTPE, for resolving type patterns
 * and transforming them to assignments and checks. Along with static methods,
 * that are shared between the two traversals.
 *
 * The formal rules that describe how features are converted to these
 * expressions are described in the function ResolveCase. To generate the
 * expressions themselves, we use type_pattern_constraints.
 *
 * The order in which we generate these expressions is important, as some type
 * pattern features depend on the values found by other features.
 *
 * Consider, for example, the selection function:
 *
 * int[d:shp]
 * sel (int[vlen] idx, int[vlen:_,d:shp] array)
 * {
 *     ...
 * }
 *
 * Here we see that we can only compute d after computing vlen.
 *
 * To ensure that constraints are inserted in the correct order, we also keep
 * track of the necessary dependencies along with each constraint. We store this
 * information, along with additional relevant information, in the private
 * constraint_info structure. We then implement a resolution algorithm that
 * generates constraints in order based on these dependencies. This algorithm is
 * described in the Resolve function.
 *
 * Both approaches are different enough that combining them in a single
 * traversal would result in a lot of conditionals, depending on whether we
 * are traversing a function, or external. Because of this, this file contains
 * two separate traversals, defined in type_pattern_resolve_fundefs.h, and
 * type_pattern_resolve_externals.h.
 * These two implementations, as well as the static methods for resolution,
 * have a lot of shared information that is passed back and forth in a struct
 * resolution_info. Most fields of this struct are accessed in all three steps,
 * making it infeasible to implement this struct using setter and getter
 * functions. Instead, we combine these three steps into this single file to
 * ensure that this struct can be shared, without being able to be seen by the
 * outside world.
 *
 * RTPF:
 *
 * The first traversal, RTPF, traverses functions with type patterns, and
 * generates additional functions that insert the necessary assignments and
 * checks imposed by these type patterns.
 *
 * Consider a function with type patterns:
 *
 * int[2,d:shp] foo (int[d:shp] a, int[d:shp] b) {
 *     return [a, b];
 * }
 *
 * We first need to generate a function to check whether the arguments match
 * their type patterns. If so, we return true, otherwise we return a bottom
 * type with a descriptive error message.
 *
 * inline
 * bool foo_pre (int[*] a, int[*] b) {
 *     d = dim(a);
 *     shp = shape(a);
 *
 *     pred = true;
 *     pred = dim(b) == d ? pred : _|_;
 *     pred = shape(b) == shp ? pred : _|_;
 *     return pred;
 * }
 *
 * We do the same for the returned values, which might depend on variables that
 * are defined by type patterns of arguments.
 *
 * inline
 * bool foo_post (int[*] a, int[*] b, int[+] res) {
 *     d = dim(a);
 *     shp = shape(a);
 *
 *     pred = true;
 *     pred = dim(res) >= 1 ? pred : _|_;
 *     pred = shape(res)[0] == 2 ? pred : _|_;
 *     pred = drop (1, shape(res)) == shp ? pred : _|_;
 *     return pred;
 * }
 *
 * Finally, we rename foo to foo_impl, and generate a new version of foo that
 * combines our generated functions.
 *
 * inline
 * int[+] foo (int[*] a, int[*] b) {
 *     pred = foo_pre (a, b);
 *     a, b = guard (a, b, pred);
 *     res = foo_impl (a, b);
 *     res = guard (res, pred);
 *     pred' = foo_post (a, b, res);
 *     res = guard (res, pred');
 *     return res;
 * }
 *
 * Note here that we added an additional guard using the pre-condition result,
 * after applying the implementation foo_impl. This ensures that the post-check
 * does not accidentally consume and hide an erroneous result.
 *
 * We do this by first traversing the type patterns of arguments and return
 * types, and generating the corresponding assignments and checks.
 * The approach is implemented as follows: first, apply GenerateConstraints to
 * each type pattern, to generate the constraints and dependencies for each of
 * those type patterns separately. (The formal rules of which are described in
 * the ResolveCase function.)
 * After doing so for every type pattern, for both arguments and return types,
 * we apply Resolve. Resolve fills the list of pre- and post-assignments, and
 * pre- and post-checks in the resolution_info structure.
 * Then we use type_pattern_guard to generate the functions described above,
 * after which we insert them into the program directly after the original
 * function definition.
 *
 * RTPE:
 *
 * The second traversal, RTPE, traverses external functions with type patterns,
 * and generates additional functions that insert the necessary assignments and
 * checks imposed by these type patterns. It is slightly different to the logic
 * described in type_pattern_resolve_fundefs.
 *
 * Consider an external function with type patterns:
 *
 * external
 * int[2,d:shp] foo (int[d:shp] a, int[d:shp] b);
 *
 * The pre-check, post-check, and wrapper functions are generated in the same
 * way as for type_pattern_resolve_fundefs. But since external functions do not
 * have a body, we do not insert any of the generated assignments into it.
 * Instead we only change the name from foo to foo_impl. If the user has not
 * provided a linkname pragma, it is expected that the linkname is the same as
 * the original function name. Since we changed it to foo_impl, we must ensure
 * to set the linkname to foo if no linkname has been given.
 *
 * external
 * int[+] foo_impl (int[*] a, int[*] b);
 *   #pragma linkname "foo"
 *
 * Note that external functions are in MODULE_FUNDECS, whereas we insert the
 * generated functions into MODULE_FUNS.
 *
 ******************************************************************************/
#include "convert.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "LookUpTable.h"
#include "memory.h"
#include "new_types.h"
#include "print.h"
#include "str.h"
#include "str_buffer.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "type_pattern_constraints.h"
#include "type_pattern_guard.h"

#define DBUG_PREFIX "RTP"
#include "debug.h"

#include "type_pattern_resolve_externals.h"
#include "type_pattern_resolve_fundefs.h"

/******************************************************************************
 *
 * @struct constraint_info
 *
 * @brief Given a feature id of a type pattern, this structure holds a single
 * constraint that is imposed on that feature, along with other relevant
 * information about this constraint.
 *
 * @param CI_ID The name of the feature.
 * @param CI_ERROR The error message if this constraint fails.
 * @param CI_CONSTRAINT The boolean expression that checks this feature.
 * @param CI_DEPENDENCIES The dependencies (variables) that need to be known
 *        before this constraint can be checked.
 * @param CI_ISARGUMENT Whether this feature is part of an argument.
 * @param CI_ISDEFINED Whether the function has an argument with the same name
 *        as this feature, meaning that this feature is always determined by
 *        the value of that argument.
 * @param CI_ISCONSTANT Usually the constraint returns a value, that we compare
 *        to the feature 'id'. But sometimes the result is a boolean, such as
 *        5 == dim(id). In this case we dont want to generate id == 5 == dim(id)
 *        so we set is_constant to true to avoid this.
 * @param CI_ISSCALAR Whether the result of this check is a scalar,
 *        e.g. dim(id), or a vector, e.g. shape(id).
 * @param CI_ISDIMCHECK Before checking any other constraints, we need to ensure
 *        that the dimensionalities of all arguments is large enough. If this
 *        constraint is such a dimensionality check, we set this value to true.
 * @param CI_ISELIDED During resolution (Resolve) we generate an assignment or
 *        check for this constraint. Once this is done, we set this value to
 *        true to avoid generating multiple assignments and checks.
 *
 ******************************************************************************/
typedef struct CONSTRAINT_INFO {
    char *id;
    str_buf *error;
    node *constraint;
    node *dependencies;
    bool is_argument;
    bool is_defined;
    bool is_constant;
    bool is_scalar;
    bool is_dim_check;
    bool is_elided;
} constraint_info;

#define CI_ID(n) ((n)->id)
#define CI_ERROR(n) ((n)->error)
#define CI_CONSTRAINT(n) ((n)->constraint)
#define CI_DEPENDENCIES(n) ((n)->dependencies)
#define CI_ISARGUMENT(n) ((n)->is_argument)
#define CI_ISDEFINED(n) ((n)->is_defined)
#define CI_ISCONSTANT(n) ((n)->is_constant)
#define CI_ISSCALAR(n) ((n)->is_scalar)
#define CI_ISDIMCHECK(n) ((n)->is_dim_check)
#define CI_ISELIDED(n) ((n)->is_elided)

/******************************************************************************
 *
 * @struct resolution_info
 *
 * @brief This structure keeps track of the information that we require and
 * generate during type pattern resolution. Since type patterns of different
 * arguments in a function can be intertwined, we require this structure
 * for the resolution of the entire function, updating it as we go.
 *
 * @param RI_FUNDEF The name of the function we are resolving.
 * @param RI_PRED The unique name of the predicate we are using,
 *        e.g. _rtp_12_pred.
 * @param RI_ISARGUMENT Whether the type pattern we are currently resolving is
 *        a type pattern of an argument.
 * @param RI_ENV A mapping from features to their collected constraints.
 * @param RI_EXIST All identifiers that have remaining constraints.
 *        Initially these are all identifiers that occur only
 *        in the features of the function signature.
 * @param RI_DEFINED All identifiers that have thus far been defined.
 *        Initially, these are only the arguments themselves.
 * @param RI_PRECHECKS The checks that should go in foo_pre.
 * @param RI_POSTCHECKS The checks that should go in foo_post.
 * @param RI_PREASSIGNS The assignments that should go in foo_pre.
 * @param RI_POSTASSIGNS The assignments that should go in foo_post.
 * @param RI_RETURNIDS Unique identifiers corresponding to the N_rets of the
 *        function. E.g. if the function has 3 return values, we generate
 *        _rtp_12_ret, _rtp_13_ret, _rtp_14_ret.
 *
 ******************************************************************************/
typedef struct RESOLUTION_INFO {
    node *fundef;
    char *pred;
    bool is_argument;
    lut_t *env;
    node *exist;
    node *defined;
    node *pre_checks;
    node *post_checks;
    node *pre_assigns;
    node *post_assigns;
    node *return_ids;
} resolution_info;

#define RI_FUNDEF(n) ((n)->fundef)
#define RI_PRED(n) ((n)->pred)
#define RI_ISARGUMENT(n) ((n)->is_argument)
#define RI_ENV(n) ((n)->env)
#define RI_EXIST(n) ((n)->exist)
#define RI_DEFINED(n) ((n)->defined)
#define RI_PRECHECKS(n) ((n)->pre_checks)
#define RI_POSTCHECKS(n) ((n)->post_checks)
#define RI_PREASSIGNS(n) ((n)->pre_assigns)
#define RI_POSTASSIGNS(n) ((n)->post_assigns)
#define RI_RETURNIDS(n) ((n)->return_ids)

/******************************************************************************
 *
 * @fn resolution_info *MakeResolutionInfo (void)
 *
 * @brief Initialises a new resolution info.
 *
 ******************************************************************************/
static resolution_info *
MakeResolutionInfo (void)
{
    resolution_info *ri;

    DBUG_ENTER ();

    ri = (resolution_info *)MEMmalloc (sizeof (resolution_info));

    RI_FUNDEF (ri) = NULL;
    RI_PRED (ri) = NULL;
    RI_ISARGUMENT (ri) = TRUE;
    RI_ENV (ri) = LUTgenerateLut();
    RI_EXIST (ri) = NULL;
    RI_DEFINED (ri) = NULL;
    RI_PREASSIGNS (ri) = NULL;
    RI_POSTASSIGNS (ri) = NULL;
    RI_PRECHECKS (ri) = NULL;
    RI_POSTCHECKS (ri) = NULL;
    RI_RETURNIDS (ri) = NULL;

    DBUG_RETURN (ri);
}

/******************************************************************************
 *
 * @fn resolution_info *ClearResolutionInfo (resolution_info *ri)
 *
 * @brief Set the values of this resolution info back to their default values,
 * ready to be used for resolution of another function.
 *
 ******************************************************************************/
static resolution_info *
ClearResolutionInfo (resolution_info *ri)
{
    DBUG_ENTER ();

    RI_FUNDEF (ri) = NULL;
    RI_PRED (ri) = MEMfree (RI_PRED (ri));
    RI_ENV (ri) = LUTremoveContentLut (RI_ENV (ri));
    RI_EXIST (ri) = FREEoptFreeTree (RI_EXIST (ri));
    RI_DEFINED (ri) = FREEoptFreeTree (RI_DEFINED (ri));
    RI_PREASSIGNS (ri) = NULL;
    RI_POSTASSIGNS (ri) = NULL;
    RI_PRECHECKS (ri) = NULL;
    RI_POSTCHECKS (ri) = NULL;
    RI_RETURNIDS (ri) = FREEoptFreeTree (RI_RETURNIDS (ri));

    DBUG_RETURN (ri);
}

/******************************************************************************
 *
 * @struct info
 *
 * @brief Info structure, shared by both RTPF and RTPE.
 *
 * @param INFO_RI the accumulated resolution info.
 * @param INFO_NEWFUNDEFS the functions generated by type patterns, to be
 *        inserted into the MODULE_FUNS node.
 *
 ******************************************************************************/
struct INFO {
    resolution_info *ri;
    node *new_fundefs;
};

#define INFO_RESOLUTION(n) ((n)->ri)
#define INFO_NEWFUNDEFS(n) ((n)->new_fundefs)

static info *
MakeInfo (void)
{
    info *res;

    DBUG_ENTER ();

    res = (info *)MEMmalloc (sizeof (info));

    INFO_RESOLUTION (res) = MakeResolutionInfo ();
    INFO_NEWFUNDEFS (res) = NULL;

    DBUG_RETURN (res);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_RESOLUTION (info) = MEMfree (INFO_RESOLUTION (info));
    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn node *AddSpidToEnv (char *v, char *id, char *error, node *constraint,
 *                         node *dependencies, resolution_info *ri,
 *                         bool is_scalar)
 *
 * @brief Given an argument 'v' with a type pattern, and a feature 'id' of that
 * type pattern. This method adds the dependencies and constraint for this
 * feature to the environment of 'v', along with additional information that
 * we will require in Resolve.
 *
 ******************************************************************************/
static void
AddSpidToEnv (char *v, char *id, char *error, node *constraint,
              node *dependencies, resolution_info *ri, bool is_scalar)
{
    constraint_info *ci;

    DBUG_ENTER ();

    if (TPCtryAddSpid (&RI_EXIST (ri), id)) {
        // Add id to exists if it was not yet in the LUT
        DBUG_PRINT ("added `%s' to exist of %s",
                    id, FUNDEF_NAME (RI_FUNDEF (ri)));
    } else {
        // Get the name of the argument we are guarding against
        void **ptr = LUTsearchInLutS (RI_ENV (ri), id);
        DBUG_ASSERT (ptr != NULL,
                     "`%s' is in exist but not in env of %s",
                     id, FUNDEF_NAME (RI_FUNDEF (ri)));

        ci = (constraint_info *)(*ptr);
        v = CI_ISARGUMENT (ci) ? CI_ID (ci) : "return";
    }

    // Create a new constraint info, to insert into the env
    ci = (constraint_info *)MEMmalloc (sizeof (constraint_info));
    CI_ID (ci) = STRcpy (v);
    CI_ERROR (ci) = SBUFcreate (strlen (error) + 1);
    CI_ERROR (ci) = SBUFprint (CI_ERROR (ci), error);
    CI_CONSTRAINT (ci) = constraint;
    CI_DEPENDENCIES (ci) = dependencies;
    CI_ISARGUMENT (ci) = RI_ISARGUMENT (ri);
    CI_ISDEFINED (ci) = TCspidsContains (RI_DEFINED (ri), id);
    CI_ISCONSTANT (ci) = STReq (v, id);
    CI_ISSCALAR (ci) = is_scalar;
    CI_ISDIMCHECK (ci) = FALSE;
    CI_ISELIDED (ci) = FALSE;

    // Insert the constraint info into the env
    RI_ENV (ri) = LUTinsertIntoLutS (RI_ENV (ri), id, ci);

    DBUG_EXECUTE ({ char *tmp = CVspids2String (dependencies);
                    DBUG_PRINT ("constraint for feature `%s' of %s "
                                "with dependencies: %s",
                                id, v, tmp);
                    tmp = MEMfree (tmp); });

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn bool ResolveCase (char *v, node *pattern, node *feature, int fdim,
 *                       node *vdim, node **dependencies, resolution_info *ri)
 *
 * @brief This function generates type pattern constraints for a single feature
 * and collects them in the env lut for later use. Here we use the fdim and vdim
 * we computed in type_pattern_analyse.
 *
 * The implemented algorithms looks like this:
 *
 * 1) GenTPC (type[<num>, …rest] v, dim, dep, env)
 *     = GenTPC (type[rest] v, dim+1, dep,
 *         env + { v: (dep, <num> == shape(v)[dim]) })
 *
 * 2) GenTPC (type[., …rest] v, dim, dep, env)
 *     = GenTPC (type[rest] v, dim+1, dep, env)
 *
 * 3) GenTPC (type[+, …rest] v, dim, dep, env)
 *     = GenTPCrhs (type[rest] v, dim(v)-1, {}, env)
 *
 * 4) GenTPC (type[*, …rest] v, dim, dep, env)
 *     = GenTPCrhs (type[rest] v, dim(v)-1, {}, env)
 *
 * 5) GenTPC (type[id, …rest] v, dim, dep, env)
 *     = GenTPC (type[rest] v, dim+1, dep,
 *         env + { id: (dep, shape(v)[dim]) })
 *
 * 6) GenTPC (type[<num>:shp, …rest] v, dim, dep, env)
 *     = GenTPC (type[rest] v, dim+num, dep,
 *         env + { shp: (dep, take(num, drop(dim, shape(v)))) })
 *
 * 7) GenTPC (type[id:shp, …rest] v, dim, dep, env)
 *     = GenTPC (type[rest] v, dim+id, dep+{id},
 *         env + { id: (dep+vdots\id, dim(v) - fdots - sum(vdots\id)) }
 *             + { shp: (dep+{id}, take(id, drop(dim, shape(v)))) })
 *
 * We keep track of the current dimension in two variables: fdim for the
 * statically known index into the shape, and vdim for all N_spids that define a
 * variable number of dimensions. By adding these we get the current index into
 * the shape vector of the argument. We store the current list of dependencies
 * as an N_spids chain, which we update as we go along.
 *
 ******************************************************************************/
static int
ResolveCase (char *v, node *pattern, node *feature, int fdim,
             node **vdim, node **dependencies, resolution_info *ri)
{
    node *expr;
    char *error;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (feature) == N_num ||
                 NODE_TYPE (feature) == N_dot ||
                 NODE_TYPE (feature) == N_spid,
                 "expected N_num, N_dot, or N_spid node in type pattern");

    DBUG_EXECUTE ({ char *tmp = CVtypePatternShape2String (feature);
                    DBUG_PRINT ("feature %s", tmp);
                    tmp = MEMfree (tmp); });

    // Generate an error message for the bottom type
    error = TPCmakeFeatureError (feature, v,
                                 FUNDEF_NAME (RI_FUNDEF (ri)),
                                 RI_ISARGUMENT (ri));

    // Case 1
    if (NODE_TYPE (feature) == N_num) {
        expr = TPCmakeDimSum (v, fdim, *vdim);
        expr = TPCmakeShapeSel (v, pattern, expr);
        expr = TPCmakeNumCheck (NUM_VAL (feature), expr);
        AddSpidToEnv (v, v, error, expr,
                      DUPdoDupTree (*dependencies), ri, TRUE);
        fdim += 1;
    }
    // Case 2
    else if (NODE_TYPE (feature) == N_dot) {
        fdim += 1;
    }
    // Case 3
    else if (STReq (SPID_NAME (feature), "+")) {
        fdim += 1;
    }
    // Case 4
    else if (STReq (SPID_NAME (feature), "*")) {
        // Nothing to do
    }
    // Case 5
    else if (SPID_TDIM (feature) == NULL) {
        expr = TPCmakeDimSum (v, fdim, *vdim);
        expr = TPCmakeShapeSel (v, pattern, expr);
        AddSpidToEnv (v, SPID_NAME (feature), error, expr,
                      DUPdoDupTree (*dependencies), ri, TRUE);
        fdim += 1;
    }
    // Case 6
    else if (NODE_TYPE (SPID_TDIM (feature)) == N_num) {
        expr = TPCmakeDimSum (v, fdim, *vdim);
        expr = TPCmakeTakeDrop (v, pattern, SPID_TDIM (feature), expr);
        AddSpidToEnv (v, SPID_NAME (feature), error, expr,
                      DUPdoDupTree (*dependencies), ri, FALSE);
        fdim += NUM_VAL (SPID_TDIM (feature));
    }
    // Case 7
    else {
        char *id = SPID_NAME (SPID_TDIM (feature));
        node *id_deps = DUPdoDupTree (TYPEPATTERN_VDIM (pattern));
        TPCremoveSpid (&id_deps, id);
        id_deps = TCappendSpids (id_deps, DUPdoDupTree (*dependencies));

        expr = TPCmakeDimCalc (v, pattern, id);
        AddSpidToEnv (v, id, error, expr, id_deps, ri, TRUE);

        TPCtryAddSpid (dependencies, id);

        expr = TPCmakeDimSum (v, fdim, *vdim);
        expr = TPCmakeTakeDrop (v, pattern, SPID_TDIM (feature), expr);
        AddSpidToEnv (v, SPID_NAME (feature), error, expr,
                      DUPdoDupTree (*dependencies), ri, FALSE);

        *vdim = TCappendSpids (*vdim, TBmakeSpids (STRcpy (id), NULL));
    }

    DBUG_RETURN (fdim);
}

/******************************************************************************
 *
 * @fn void ResolveDim (char *v, node *pattern, int fdim, resolution_info *ri)
 *
 * @brief Before applying the assignments and checks inserted by type patterns,
 * we must ensure that the dimensionality of the argument is large enough to
 * avoid an out of bounds error in our generated code. If there are no variable
 * dimensions, we know exactly the expected dimensionality. Otherwise, we check
 * whether the dimensionality is at least as large as the number of fixed
 * dimensions, in which case all variable dimensions will be zero.
 *
 ******************************************************************************/
static void
ResolveDim (char *v, node *pattern, int fdim, resolution_info *ri)
{
    prf prf;
    node *spid, *constraint;
    constraint_info *ci;
    char *error;

    DBUG_ENTER ();

    // Generate an error message for the bottom type
    error = TPCmakeDimError (pattern, v, FUNDEF_NAME (RI_FUNDEF (ri)),
                             fdim, RI_ISARGUMENT (ri));

    // Create an expression to check whether the dim is large enough
    spid = TBmakeSpid (NULL, STRcpy (v));
    constraint = TPCmakePrimitive (pattern, F_dim_A, "dim", spid);
    prf = TYPEPATTERN_HASVDIM (pattern) ? F_ge_SxS : F_eq_SxS;
    constraint = TCmakePrf2 (prf, constraint, TBmakeNum (fdim));

    // Create a new constraint info, to insert into the env
    ci = (constraint_info *)MEMmalloc (sizeof (constraint_info));
    CI_ID (ci) = STRcpy (v);
    CI_ERROR (ci) = SBUFcreate (strlen (error) + 1);
    CI_ERROR (ci) = SBUFprint (CI_ERROR (ci), error);
    CI_CONSTRAINT (ci) = constraint;
    CI_DEPENDENCIES (ci) = NULL;
    CI_ISARGUMENT (ci) = RI_ISARGUMENT (ri);
    CI_ISCONSTANT (ci) = TRUE;
    CI_ISSCALAR (ci) = TRUE;
    CI_ISDIMCHECK (ci) = TRUE;
    CI_ISELIDED (ci) = FALSE;

    // Insert the constraint info into the env
    RI_ENV (ri) = LUTinsertIntoLutS (RI_ENV (ri), v, ci);
    DBUG_PRINT ("constraint for `%s' dimensionality", v);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn void GenerateConstraints (char *v, node *pattern, resolution_info *ri)
 *
 * @brief Generate constraints for the type patterns of a single argument, by
 * repeatedly applying ResolveCase to each pattern. This results in an updated
 * environment, containing the newly generated constraints for the given
 * type pattern.
 *
 ******************************************************************************/
static void
GenerateConstraints (char *v, node *pattern, resolution_info *ri)
{
    node *exprs, *dependencies = NULL;
    node *vdim = NULL;
    int fdim = 0;

    DBUG_ENTER ();

    DBUG_EXECUTE ({ char *tmp = CVtypePattern2String (pattern);
                    DBUG_PRINT ("generating constraints for %s %s", tmp, v);
                    tmp = MEMfree (tmp); });

    exprs = TYPEPATTERN_SHAPE (pattern);
    while (exprs != NULL) {
        // Resolve each feature of the type pattern
        fdim = ResolveCase (v, pattern, EXPRS_EXPR (exprs),
                            fdim, &vdim, &dependencies, ri);
        exprs = EXPRS_NEXT (exprs);
    }

    // Resolve the dimensionality of the argument
    ResolveDim (v, pattern, fdim, ri);

    dependencies = FREEoptFreeTree (dependencies);
    vdim = FREEoptFreeTree (vdim);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *MakeFeatureGuard (char *id, char *pred, constraint_info *ci)
 *
 * @brief Given a feature, and one of its constraints, we generate a check
 * to insert into the code that either returns the predicate, or a bottom
 * type with a descriptive error message if the check failed.
 *
 * @example pred = id == expr() ? pred : _|_("error message");
 *
 ******************************************************************************/
static node *
MakeFeatureGuard (char *id, char *pred, constraint_info *ci)
{
    node *constraint, *type, *res;
    char *error;

    DBUG_ENTER ();

    constraint = DUPdoDupTree (CI_CONSTRAINT (ci));

    if (CI_ISCONSTANT (ci)) {
        // The constraint already returns a boolean, nothing to do
    } else if (CI_ISSCALAR (ci)) {
        // The constraint returns a number, compare it to the feature
        node *spid = TBmakeSpid (NULL, STRcpy (id));
        constraint = TCmakePrf2 (F_eq_SxS, spid, constraint);
    } else {
        // The constraint returns a vector, compare it to the feature
        node *spid = TBmakeSpid (NULL, STRcpy (id));
        constraint = TCmakePrf2 (F_eq_VxV, spid, constraint);
        constraint = TCmakePrf1 (F_all_V, constraint);
    }

    // Generate a bottom type with a descriptive error message
    if (CI_ISDIMCHECK (ci)) {
        CI_ERROR (ci) = SBUFprint (CI_ERROR (ci), ".\n");
    } else if (CI_ISDEFINED (ci)) {
        CI_ERROR (ci) = SBUFprintf (CI_ERROR (ci),
                                    " does not match argument %s.\n",
                                    id);
    } else {
        CI_ERROR (ci) = SBUFprintf (CI_ERROR (ci),
                                    " does not match feature `%s' in %s.\n",
                                    id, CI_ID (ci));
    }

    error = SBUF2strAndFree (&CI_ERROR (ci));
    type = TBmakeType (TYmakeBottomType (STRcpy (error)));
    TYPE_ISGUARD (type) = TRUE;

    // Generate an expression: pred = constraint() ? pred : _|_
    res = TCmakePrf1 (F_type_error, type);
    res = TBmakeFuncond (constraint, TBmakeSpid (NULL, STRcpy (pred)), res);
    res = TBmakeLet (TBmakeSpids (STRcpy (pred), NULL), res);
    res = TBmakeAssign (res, NULL);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn void CreateDimCheck (char *id, constraint_info *ci, resolution_info *ri)
 *
 * @brief Generate and insert a constraint for a dimensionality check, which
 * should always occur before any other checks. Thus we PREPEND this constraint
 * to the list of conditions.
 *
 ******************************************************************************/
static void
CreateDimCheck (char *id, constraint_info *ci, resolution_info *ri)
{
    node *res;

    DBUG_ENTER ();

    DBUG_PRINT ("creating a dim check for %s", id);
    res = MakeFeatureGuard (id, RI_PRED (ri), ci);

    if (CI_ISARGUMENT (ci)) {
        RI_PRECHECKS (ri) = TCappendAssign (res, RI_PRECHECKS (ri));
    } else {
        RI_POSTCHECKS (ri) = TCappendAssign (res, RI_POSTCHECKS (ri));
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn void CreateCheck (char *id, constraint_info *ci, resolution_info *ri)
 *
 * @brief There are no more dependencies for the given identifier, and it has
 * not been previously defined. Generate and insert a constraint for this check,
 * which should always occur after any dimensionality checks. Thus we APPEND
 * this constraint to the list of conditions.
 *
 ******************************************************************************/
static void
CreateCheck (char *id, constraint_info *ci, resolution_info *ri)
{
    node *res;

    DBUG_ENTER ();

    DBUG_PRINT ("creating a check for %s", id);
    res = MakeFeatureGuard (id, RI_PRED (ri), ci);

    if (CI_ISARGUMENT (ci)) {
        RI_PRECHECKS (ri) = TCappendAssign (RI_PRECHECKS (ri), res);
    } else {
        RI_POSTCHECKS (ri) = TCappendAssign (RI_POSTCHECKS (ri), res);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn void CreateAssign (char *id, constraint_info *ci, resolution_info *ri)
 *
 * @brief There are no more dependencies for the given identifier, and it has
 * not yet been defined. Generate and insert an assignment for this feature.
 *
 ******************************************************************************/
static void
CreateAssign (char *id, constraint_info *ci, resolution_info *ri)
{
    node *res;

    DBUG_ENTER ();

    DBUG_PRINT ("creating an assignment for %s", id);
    TPCtryAddSpid (&RI_DEFINED (ri), id);

    res = DUPdoDupTree (CI_CONSTRAINT (ci));
    res = TBmakeLet (TBmakeSpids (STRcpy (id), NULL), res);
    res = TBmakeAssign (res, NULL);

    if (CI_ISARGUMENT (ci)) {
        RI_PREASSIGNS (ri) = TCappendAssign (RI_PREASSIGNS (ri),
                                             DUPdoDupNode (res));
    }

    /**
     * Type pattern defined in arguments might also be used in the type pattern
     * of the return value. So always add the assign to the return assigns.
     */
    RI_POSTASSIGNS (ri) = TCappendAssign (RI_POSTASSIGNS (ri), res);

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *Resolve (resolution_info *ri)
 *
 * @brief Given an environment, filled by GenerateConstraints, we try to
 * resolve all constraints from the environment using the following algorithm:
 *
 * for each id that exists with Env(id) = (dep, constraint);
 *     if (dep \ defined == {}) {
 *         if (id in defined) {
 *             generate check (constraint);
 *         } else {
 *             generate id = constraint;
 *             defined += {id};
 *         }
 *         elide this entry from env;
 *     }
 * repeat until no more deletions in env happen.
 *
 * if env is empty: ok
 * otherwise: report unresolvable constraints
 *
 ******************************************************************************/
static void
Resolve (resolution_info *ri)
{
    bool changed = TRUE;

    DBUG_ENTER ();

    DBUG_EXECUTE ({ char *tmp = CVspids2String (RI_EXIST (ri));
                    DBUG_PRINT ("resolving exists: %s", tmp);
                    tmp = MEMfree (tmp); });

    while (changed) {
        node *exist = RI_EXIST (ri);
        node *prev_exist = NULL;
        changed = FALSE;

        while (exist != NULL) {
            bool remove_exist = TRUE;
            char *id = SPIDS_NAME (exist);
            void **lut_pointer = LUTsearchInLutS (RI_ENV (ri), id);

            DBUG_PRINT ("resolving `%s'", id);
            DBUG_ASSERT (lut_pointer != NULL,
                         "%s is in exist but not in env", id);

            while (lut_pointer != NULL) {
                constraint_info *ci = (constraint_info *)(*lut_pointer);

                if (CI_ISELIDED (ci)) {
                    // This constraint has already been resolved
                    lut_pointer = LUTsearchInLutNextS ();
                    continue;
                }

                if (TPChasMissing (CI_DEPENDENCIES (ci), RI_DEFINED (ri))) {
                    DBUG_PRINT ("there are missing dependencies for `%s'", id);
                    lut_pointer = LUTsearchInLutNextS ();
                    remove_exist = FALSE;
                    continue;
                }

                // Generate a check or assignment for this constraint
                if (CI_ISDIMCHECK (ci)) {
                    CreateDimCheck (id, ci, ri);
                } else if (TCspidsContains (RI_DEFINED (ri), id)) {
                    CreateCheck (id, ci, ri);
                } else {
                    CreateAssign (id, ci, ri);
                }

                // Check the next constraint of this feature
                lut_pointer = LUTsearchInLutNextS ();
                CI_ISELIDED (ci) = TRUE;
                changed = TRUE;
            }

            /**
             * Remove this feature from exist if all of its constraints have
             * been resolved, otherwise keep it and move on to the next
             * feature.
             */
            if (remove_exist) {
                DBUG_PRINT ("removing %s from exist", id);
                exist = FREEdoFreeNode (exist);
                if (prev_exist == NULL) {
                    RI_EXIST (ri) = exist;
                } else {
                    SPIDS_NEXT (prev_exist) = exist;
                }
            } else {
                DBUG_PRINT ("%s has remaining dependencies", id);
                prev_exist = exist;
                exist = SPIDS_NEXT (exist);
            }
        }
    }

    if (RI_EXIST (ri) != NULL) {
        CTIerror (EMPTY_LOC, "%s has remaining dependencies: %s",
                             FUNDEF_NAME (RI_FUNDEF (ri)),
                             CVspids2String (RI_EXIST (ri)));
        RI_EXIST (ri) = FREEdoFreeTree (RI_EXIST (ri));
    } else {
        DBUG_PRINT ("resolved all dependencies of %s",
                    FUNDEF_NAME (RI_FUNDEF (ri)));
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *RTPFdoResolveTypePatternFundefs (node *arg_node)
 *
 * @brief Type pattern resolution for fundefs.
 *
 ******************************************************************************/
node *
RTPFdoResolveTypePatternFundefs (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_rtpf);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPFmodule (node *arg_node, info *arg_info)
 *
 * @brief Only traverse MODULE_FUNS.
 *
 ******************************************************************************/
node *
RTPFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn void AddArgsToDefined (node *args, resolution_info *ri)
 *
 * @brief Add all given argument names to the defined set.
 *
 ******************************************************************************/
static void
AddArgsToDefined (node *args, resolution_info *ri)
{
    DBUG_ENTER ();

    while (args != NULL) {
        TPCtryAddSpid (&RI_DEFINED (ri), ARG_NAME (args));
        DBUG_PRINT ("added `%s' to defined", ARG_NAME (args));
        args = ARG_NEXT (args);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn node *RTPFfundef (node *arg_node, info *arg_info)
 *
 * @brief Generate the pre- and post-check functions and modify this function.
 * Then generate a wrapper function that combines these three functions, and
 * insert them all into the program.
 *
 ******************************************************************************/
node *
RTPFfundef (node *arg_node, info *arg_info)
{
    resolution_info *ri;
    node *pre = NULL, *post = NULL, *impl = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("----- resolving type patterns of %s -----",
                FUNDEF_NAME (arg_node));

    ri = INFO_RESOLUTION (arg_info);
    RI_FUNDEF (ri) = arg_node;
    RI_PRED (ri) = STRcpy (TRAVtmpVarName ("pred"));

    RI_ISARGUMENT (ri) = TRUE;
    // First add all arguments to defined before traversing their type patterns
    AddArgsToDefined (FUNDEF_ARGS (arg_node), ri);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    RI_ISARGUMENT (ri) = FALSE;
    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

    Resolve (ri);

    /**
     * Always traverse the body, since we have to insert the generated
     * assignments. Because then may be used in the body of this function.
     */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /**
     * Only insert guards if -ecc is enabled. If -check c is not enabled,
     * these guards are removed after the optimisation cycle.
     */
    if (global.insertconformitychecks) {
        if (RI_PRECHECKS (ri) != NULL || FUNDEF_PRECONDS (arg_node) != NULL) {
            pre = GTPmakePreCheck (arg_node,
                                   RI_PRED (ri),
                                   RI_PREASSIGNS (ri),
                                   RI_PRECHECKS (ri));
        }

        if (RI_POSTCHECKS (ri) != NULL || FUNDEF_POSTCONDS (arg_node) != NULL) {
            post = GTPmakePostCheck (arg_node,
                                     RI_PRED (ri),
                                     RI_POSTASSIGNS (ri),
                                     RI_POSTCHECKS (ri),
                                     RI_RETURNIDS (ri));
        }

        if (pre != NULL || post != NULL) {
            impl = GTPmakeImpl (arg_node);
            arg_node = GTPmodifyFundef (arg_node, impl, pre, post);
        }
    }

    FUNDEF_PRECONDS (arg_node) = FREEoptFreeTree (FUNDEF_PRECONDS (arg_node));
    FUNDEF_POSTCONDS (arg_node) = FREEoptFreeTree (FUNDEF_POSTCONDS (arg_node));
    INFO_RESOLUTION (arg_info) = ClearResolutionInfo (ri);

    /**
     * Traverse the next fundef before inserting the generated functions.
     */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    /**
     * Insert the generated function after traversing all remaining fundefs, so
     * that we can insert these functions right below their corresponding
     * original funcion definition.
     */
    if (impl != NULL) {
        FUNDEF_NEXT (impl) = arg_node;
        arg_node = impl;
    }

    if (post != NULL) {
        FUNDEF_NEXT (post) = arg_node;
        arg_node = post;
    }

    if (pre != NULL) {
        FUNDEF_NEXT (pre) = arg_node;
        arg_node = pre;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPFarg (node *arg_node, info *arg_info)
 *
 * @brief Generate the assignments and checks for the type patterns of this arg.
 *
 ******************************************************************************/
node *
RTPFarg (node *arg_node, info *arg_info)
{
    resolution_info *ri;
    node *pattern;

    DBUG_ENTER ();

    ri = INFO_RESOLUTION (arg_info);
    pattern = AVIS_TYPEPATTERN (ARG_AVIS (arg_node));

    if (pattern != NULL && TYPEPATTERN_ISTYPEPATTERN (pattern)) {
        GenerateConstraints (ARG_NAME (arg_node), pattern, ri);
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPFret (node *arg_node, info *arg_info)
 *
 * @brief Generate the assignments and checks for the type patterns of this ret.
 *
 ******************************************************************************/
node *
RTPFret (node *arg_node, info *arg_info)
{
    resolution_info *ri;
    node *pattern, *spid;
    char *name;

    DBUG_ENTER ();

    ri = INFO_RESOLUTION (arg_info);
    pattern = RET_TYPEPATTERN (arg_node);
    name = TRAVtmpVarName ("ret");

    if (pattern != NULL && TYPEPATTERN_ISTYPEPATTERN (pattern)) {
        GenerateConstraints (name, pattern, ri);
    }

    spid = TBmakeExprs (TBmakeSpid (NULL, STRcpy (name)), NULL);
    RI_RETURNIDS (ri) = TCappendExprs (RI_RETURNIDS (ri), spid);

    // Add to defined to make sure no assign is created for the return type
    TPCtryAddSpid (&RI_DEFINED (ri), name);
    DBUG_PRINT ("added `%s' to defined", name);

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPFblock (node *arg_node, info *arg_info)
 *
 * @brief Insert the generated assignments into the function body.
 *
 ******************************************************************************/
node *
RTPFblock (node *arg_node, info *arg_info)
{
    resolution_info *ri;

    DBUG_ENTER ();

    ri = INFO_RESOLUTION (arg_info);

    if (RI_PREASSIGNS (ri) != NULL) {
        BLOCK_ASSIGNS (arg_node) =
            TCappendAssign (DUPdoDupTree (RI_PREASSIGNS (ri)),
                            BLOCK_ASSIGNS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPEdoResolveTypePatternExternals (node *arg_node)
 *
 * @brief Type pattern resolution for externals.
 *
 ******************************************************************************/
node *
RTPEdoResolveTypePatternExternals (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_rtpe);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPEmodule (node *arg_node, info *arg_info)
 *
 * @brief Only traverse MODULE_FUNDECS. This results in multiple pre-check,
 * post-check, and wrapper functions that we need to add to the MODULE_FUNS.
 *
 ******************************************************************************/
node *
RTPEmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NEWFUNDEFS (arg_info) = NULL;
    MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
    MODULE_FUNS (arg_node) = TCappendFundef (INFO_NEWFUNDEFS (arg_info),
                                             MODULE_FUNS (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPEfundef (node *arg_node, info *arg_info)
 *
 * @brief Generate the pre- and post-check functions and modify this function.
 * Then generate a wrapper function that combines these three functions.
 *
 ******************************************************************************/
node *
RTPEfundef (node *arg_node, info *arg_info)
{
    node *pre = NULL, *post = NULL, *wrapper;
    resolution_info *ri;

    DBUG_ENTER ();

    DBUG_PRINT ("----- resolving type patterns of %s -----",
                FUNDEF_NAME (arg_node));

    ri = INFO_RESOLUTION (arg_info);
    RI_FUNDEF (ri) = arg_node;
    RI_PRED (ri) = STRcpy (TRAVtmpVarName ("pred"));

    RI_ISARGUMENT (ri) = TRUE;
    // First add all arguments to defined before traversing their type patterns
    AddArgsToDefined (FUNDEF_ARGS (arg_node), ri);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    RI_ISARGUMENT (ri) = FALSE;
    FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

    Resolve (ri);

    /**
     * Only insert guards if -ecc is enabled. If -check c is not enabled,
     * these guards are removed after the optimisation cycle.
     */
    if (global.insertconformitychecks) {
        if (RI_PRECHECKS (ri) != NULL || FUNDEF_PRECONDS (arg_node) != NULL) {
            pre = GTPmakePreCheck (arg_node,
                                   RI_PRED (ri),
                                   RI_PREASSIGNS (ri),
                                   RI_PRECHECKS (ri));
            INFO_NEWFUNDEFS (arg_info) =
                TCappendFundef (INFO_NEWFUNDEFS (arg_info), pre);
        }

        if (RI_POSTCHECKS (ri) != NULL || FUNDEF_POSTCONDS (arg_node) != NULL) {
            post = GTPmakePostCheck (arg_node,
                                     RI_PRED (ri),
                                     RI_POSTASSIGNS (ri),
                                     RI_POSTCHECKS (ri),
                                     RI_RETURNIDS (ri));
            INFO_NEWFUNDEFS (arg_info) =
                TCappendFundef (INFO_NEWFUNDEFS (arg_info), post);
        }

        if (pre != NULL || post != NULL) {
            char *old_name = FUNDEF_NAME (arg_node);
            char *new_name = GTPmakeValidFundefName ("impl", old_name);
            FUNDEF_NAME (arg_node) = new_name;

            // If no linkname is given, use the original function name
            if (PRAGMA_LINKNAME (FUNDEF_PRAGMA (arg_node)) == NULL) {
                PRAGMA_LINKNAME (FUNDEF_PRAGMA (arg_node)) = STRcpy (old_name);
            }

            wrapper = TBmakeFundef (old_name,
                                    FUNDEF_NS (arg_node),
                                    DUPdoDupTree (FUNDEF_RETS (arg_node)),
                                    DUPdoDupTree (FUNDEF_ARGS (arg_node)),
                                    TBmakeBlock (NULL, NULL),
                                    NULL);
            wrapper = GTPmodifyFundef (wrapper, arg_node, pre, post);
            INFO_NEWFUNDEFS (arg_info) =
                TCappendFundef (INFO_NEWFUNDEFS (arg_info), wrapper);
        }
    }

    FUNDEF_PRECONDS (arg_node) = FREEoptFreeTree (FUNDEF_PRECONDS (arg_node));
    FUNDEF_POSTCONDS (arg_node) = FREEoptFreeTree (FUNDEF_POSTCONDS (arg_node));
    INFO_RESOLUTION (arg_info) = ClearResolutionInfo (ri);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPEarg (node *arg_node, info *arg_info)
 *
 * @brief Generate the assignments and checks for the type patterns of this arg.
 *
 ******************************************************************************/
node *
RTPEarg (node *arg_node, info *arg_info)
{
    resolution_info *ri;
    node *pattern;

    DBUG_ENTER ();

    ri = INFO_RESOLUTION (arg_info);
    pattern = AVIS_TYPEPATTERN (ARG_AVIS (arg_node));

    if (pattern != NULL && TYPEPATTERN_ISTYPEPATTERN (pattern)) {
        GenerateConstraints (ARG_NAME (arg_node), pattern, ri);
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *RTPEarg (node *arg_node, info *arg_info)
 *
 * @brief Generate the assignments and checks for the type patterns of this ret.
 *
 ******************************************************************************/
node *
RTPEret (node *arg_node, info *arg_info)
{
    resolution_info *ri;
    node *pattern, *spid;
    char *name;

    DBUG_ENTER ();

    ri = INFO_RESOLUTION (arg_info);
    pattern = RET_TYPEPATTERN (arg_node);
    name = TRAVtmpVarName ("ret");

    if (pattern != NULL && TYPEPATTERN_ISTYPEPATTERN (pattern)) {
        GenerateConstraints (name, pattern, ri);
    }

    spid = TBmakeExprs (TBmakeSpid (NULL, STRcpy (name)), NULL);
    RI_RETURNIDS (ri) = TCappendExprs (RI_RETURNIDS (ri), spid);

    // Add to defined to make sure no assign is created for the return type
    TPCtryAddSpid (&RI_DEFINED (ri), name);
    DBUG_PRINT ("added `%s' to defined", name);

    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
