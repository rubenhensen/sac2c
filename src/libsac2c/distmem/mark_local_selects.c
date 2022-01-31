/*
 * Performs the DMMLS and DMGS optimizations:
 * DMMLS: Marks array read accesses that are known to be local for the distributed memory.
 * DMGS: Checks in every iteration of a modarray-with-loop if all accesses to the source
 * array are local. If that is the case, we do not need separate checks per access.
 *
 *
 * Why these optimizations?
 *
 * By generating different code we can avoid the runtime-check whether a read
 * is local or remote. Since read accesses occur frequently,
 * the frequent checks have a bad influence on the overall performance.
 * Note: We do not need these optimizations for write accesses because they are known to
 * be always local ("owner computes" principle).
 *
 *
 * How do we recognize local reads for DMMLS?
 *
 * Due to the "owner computes" principle, we know that elements of distributed
 * arrays that are written in with-loops are always local to the compute node.
 * Therefore, if in a modarray-with-loop we read an array element from the
 * "old" array and the first dimension of the index is equal to the first dimension
 * of the index we are currently writing to, we know that the read is local too.
 *
 * Note that arrays are always distributed along their first dimension.
 * This is why it is sufficient that only the first dimension of the index of a read
 * access matches the first dimension of the current write-index to know that the read is
 * local too.
 *
 *
 * What is the idea behind DMGS?
 *
 * In many cases DMMLS cannot decide that a read is local. However, for many applications
 * all reads to the source array are to indices that are close to each other. Think of
 * image convolution as an example. The DMGS optimization checks for every part of a
 * modarray-with-loopf if, after DMMLS was applied, there is a certain number (depends on
 * the -dmgs_min_selects and -dmgs_max_selects options) of reads left that could not be
 * identified as local reads.
 *
 * If that is the case, it determines the smallest and biggest offset of all reads. We
 * then insert a single check at the beginning of the with-loop-part that checks whether
 * all reads are local and copy the remainder of the with-loop-part. In one of the copies
 * we replace all selects to the source array by local selects. If, at runtime, the check
 * shows that all reads are local, we can execute the branch with local selects and do not
 * have to execute the check whether an array element is local for every select.
 *
 *    if (first_sel_offs >= local_from
 *         && last_sel_offs < local_to_excl) {
 *      // All accessed indices of the source array are local.
 *      // Code block with all local reads.
 *    } else {
 *      // Not all of the accessed indices are local.
 *      // Code block with normal reads.
 *    }
 *
 *
 * How do we mark local reads?
 *
 * Read accesses to arrays can be calls to the primitive function _sel_VxA_() or calls to
 * the sel() function that is (amongst others) defined in the SAC prelude and in the
 * ArrayBasics module of the stdlib.
 *
 * At the stage in the compilation process where we apply this optimization, the sel()
 * function applications are not inlined yet. Since parsing the definition of the sel()
 * function would be way harder than examining the call to sel() we chose to apply this
 * optimization at this point before the calls to sel() get inlined.
 *
 * If we find local reads that are calls to the primitive function _sel_VxA_() we set the
 * flag PRF_DISTMEMISLOCALREAD that triggers the generation of different code later on.
 *
 * If we find local reads that are calls to the sel() function, we replace them by calls
 * to the new SAC prelude functions _selVxADistmemLocal()/_selSxADistmemLocal(). The only
 * difference to the normal sel() implementations is that these functions use the
 * _sel_VxA_distmem_local_() primitive function rather than _sel_VxA_(). At the parsing
 * stage, _sel_VxA_distmem_local_() is replaced by _sel_VxA_() with the
 * PRF_DISTMEMISLOCALREAD flag set.
 *
 *
 * In what cases do we apply this optimization?
 *
 * To ease implementation we only apply this optimization to with-loops with a
 * single (modarray) operator.
 * Also, the optimizations assume a simple source code layout. If, there is, for instance,
 * nested with-loops it won't work. There is no reason for that other than that there was
 * not enough time to come up with a more general implementation.
 *
 *
 * How to show debug output for this optimization?
 * -# d,DMMLS
 *
 *
 * How to show the intermediate output after this optimization?
 * -bptc:dmmls
 *
 *
 * By default, both DMMLS and DMGS optimizations are activated when the distributed memory
 * backend is used. If it does produce bugs, you can switch DMMLS off by specifying: -no
 * dmmls The DMGS optimization can be switched off by specifying: -no dmgs
 *
 * There is two more options that determine whether DMGS is applied:
 * -dmgs_min_selects and -dmgs_max_selects
 */

#include "mark_local_selects.h"

#include "str.h"
#include "math_utils.h"
#include "traverse.h"
#include "memory.h"
#include "tree_basic.h"
#include "new_types.h"
#include "DupTree.h"
#include "shape.h"
#include "tree_compound.h"
#include "namespaces.h"
#include "globals.h"
#include "ctinfo.h"

#define DBUG_PREFIX "DMMLS"
#include "debug.h"

/* If global.dmgs_max_selects has this value, it is unbounded. */
#define DMGS_MAX_SELECTS_UNBOUNDED 0

/*
 * INFO structure
 */
struct INFO {
    /* Are we currently traversing a modarray-withloop for which we apply this
     * optimization? */
    bool traversing_modarray_with;
    /* Are we currently traversing the parts of a modarray-withloop for
     * which we apply this optimization? */
    bool traversing_modarray_with_parts;
    /* The source array of the modarray-withloop. */
    node *modarray_target;
    /* Scalarized with-loop index */
    node *modarray_withid_ids;
    /* Vector with-loop index */
    node *modarray_withid_vec;
    /* Set of expressions that appear within the curly braces
     * that (may) follow a WL-generator specification */
    node *modarray_part_cblock;
    /* Keep track of the number of local selects that we found. */
    int local_selects;
    /* Number of selects from the source array of the modarray with-loop
     * per iteration in a part. */
    int selects_from_modarray_source;
    /* Keep track of vardecs we have to append to the vardecs of the function. */
    node *vardec;
    /* Number of selects transformed by the 2nd part. */
    int boundary_selects;
};

/*
 * INFO macros
 */
#define INFO_TRAVERSING_MODARRAY_WITH(n) ((n)->traversing_modarray_with)
#define INFO_TRAVERSING_MODARRAY_WITH_PARTS(n) ((n)->traversing_modarray_with_parts)
#define INFO_MODARRAY_TARGET(n) ((n)->modarray_target)
#define INFO_MODARRAY_WITHID_IDS(n) ((n)->modarray_withid_ids)
#define INFO_MODARRAY_WITHID_VEC(n) ((n)->modarray_withid_vec)
#define INFO_MODARRAY_PART_CBLOCK(n) ((n)->modarray_part_cblock)
#define INFO_LOCAL_SELECTS(n) ((n)->local_selects)
#define INFO_SELECTS_FROM_MODARRAY_SOURCE(n) ((n)->selects_from_modarray_source)
#define INFO_VARDEC(n) ((n)->vardec)
#define INFO_BOUNDARY_SELECTS(n) ((n)->boundary_selects)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TRAVERSING_MODARRAY_WITH (result) = FALSE;
    INFO_TRAVERSING_MODARRAY_WITH_PARTS (result) = FALSE;
    INFO_MODARRAY_TARGET (result) = NULL;
    INFO_MODARRAY_WITHID_IDS (result) = NULL;
    INFO_MODARRAY_WITHID_VEC (result) = NULL;
    INFO_MODARRAY_PART_CBLOCK (result) = NULL;
    INFO_LOCAL_SELECTS (result) = 0;
    INFO_SELECTS_FROM_MODARRAY_SOURCE (result) = 0;
    INFO_VARDEC (result) = NULL;
    INFO_BOUNDARY_SELECTS (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Helper functions
 */

/*
 * Returns the right-hand side of the last let to the variable var in the with-loop-part
 * INFO_MODARRAY_PART_CBLOCK( arg_info) or NULL if none is found.
 */
static node *
GetLastAssignment (node *var, info *arg_info)
{
    /* Search the assignments of the with-loop-part for the latest value of the variable.
     */
    node *cblock_expr = BLOCK_ASSIGNS (INFO_MODARRAY_PART_CBLOCK (arg_info));
    /* Saves the last assignment to the variable. */
    node *latest_let_expr = NULL;

    /* For each assignment ... */
    while (cblock_expr != NULL) {
        node *stmt = ASSIGN_STMT (cblock_expr);

        if (NODE_TYPE (stmt) == N_let) {
            /* The statement is a let. */

            if (IDS_AVIS (LET_IDS (stmt)) == ID_AVIS (var)) {
                DBUG_PRINT ("Found last let to variable %s.", AVIS_NAME (ID_AVIS (var)));

                latest_let_expr = LET_EXPR (stmt);
            }
        }

        /* Continue to next assignment. */
        cblock_expr = ASSIGN_NEXT (cblock_expr);
    }

    return latest_let_expr;
}

/*
 * Examines a variable that contains a selection index.
 * Returns whether the first dimension of the index is equal to
 * the first dimension of the with-loop index.
 */
static bool
CheckIfVarIsFirstDimOfWLIdx (node *var, info *arg_info)
{
    /* Saves the last assignment to the variable. */
    node *latest_let_expr = GetLastAssignment (var, arg_info);

    if (latest_let_expr != NULL && NODE_TYPE (latest_let_expr) == N_array) {
        /* The variable is assigned a constant array. */
        DBUG_PRINT ("The let is to a constant array.");

        if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (latest_let_expr))) == N_id) {
            DBUG_PRINT ("The first element of the array is a variable.");

            if (INFO_MODARRAY_WITHID_IDS (arg_info) != NULL
                && ID_AVIS (EXPRS_EXPR (ARRAY_AELEMS (latest_let_expr)))
                     == IDS_AVIS (INFO_MODARRAY_WITHID_IDS (arg_info))) {
                DBUG_PRINT ("!!! The variable is the first dimension of the with-loop "
                            "index. We have found a local read access !!!");
                INFO_LOCAL_SELECTS (arg_info)++;
                return TRUE;
            }
        }
    }

    return FALSE;
}

/*
 * Checks whether a select (either a call to the _sel_VxA_ primitive function or a call to
 * the sel() function) has the source array of the modarray-with-loop as a target and
 * whether the first argument (the selection index) is a variable.
 */
static bool
CheckIfSelectToSourceArray (node *first_arg, node *second_arg, info *arg_info)
{
    if (NODE_TYPE (second_arg) == N_id
        && ID_AVIS (second_arg) == INFO_MODARRAY_TARGET (arg_info)) {
        DBUG_PRINT ("Found select with source array of modarray with-loop as target.");

        if (NODE_TYPE (first_arg) == N_id && AVIS_DECL (ID_AVIS (first_arg)) != NULL) {
            /* The first argument is a local variable. */
            DBUG_PRINT ("The first argument is the local variable %s.",
                        AVIS_NAME (ID_AVIS (first_arg)));
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Checks whether a select (either a call to the _sel_VxA_ primitive function or a call to
 * the sel() function) is local by examining the first argument (the selection index) if
 * the selection index is a scalar.
 */
static bool
CheckIfSelectIsLocalAndIndexScalar (node *first_arg, info *arg_info)
{
    if (INFO_MODARRAY_WITHID_IDS (arg_info) != NULL
        && ID_AVIS (first_arg) == IDS_AVIS (INFO_MODARRAY_WITHID_IDS (arg_info))) {
        DBUG_PRINT ("!!! The first argument is equal to the first dimension of the "
                    "with-loop index. We have found a local read access !!!");
        return TRUE;
    }

    return FALSE;
}

/*
 * Checks whether a select (either a call to the _sel_VxA_ primitive function or a call to
 * the sel() function) is local by examining the first argument (the selection index) if
 * the selection index is a vector.
 */
static bool
CheckIfSelectIsLocalAndIndexVector (node *first_arg, info *arg_info)
{
    if (INFO_MODARRAY_WITHID_VEC (arg_info) != NULL
        && ID_AVIS (first_arg) == IDS_AVIS (INFO_MODARRAY_WITHID_VEC (arg_info))) {
        DBUG_PRINT ("!!! The first argument is equal to the with-loop index. We have "
                    "found a local read access !!!");
        INFO_LOCAL_SELECTS (arg_info)++;
        return TRUE;
    } else {
        return CheckIfVarIsFirstDimOfWLIdx (first_arg, arg_info);
    }
}

/*
 * Returns the SPAP node if the STMT is a sel()
 * in the correct namespace.
 * Returns NULL otherwise.
 */
static node *
GetSpapIfStmtIsSel (node *assign)
{
    if (NODE_TYPE (ASSIGN_STMT (assign)) == N_let
        && NODE_TYPE (LET_EXPR (ASSIGN_STMT (assign))) == N_spap) {
        node *spap = LET_EXPR (ASSIGN_STMT (assign));
        node *spid = SPAP_ID (spap);
        namespace_t *ns = SPID_NS (spid);
        DBUG_PRINT ("Found spap: name = %s, ns = %s, module = %s", SPID_NAME (spid),
                    NSgetName (ns), NSgetModule (ns));
        if (STReq (SPID_NAME (spid), "sel")) {
            /* Check the namespace. sel() may be redefined elsewhere. */
            if (STReq (NSgetName (ns), "Array") || STReq (NSgetName (ns), "ArrayBasics")
                || STReq (NSgetName (ns), "ArrayArith")
                || STReq (NSgetName (ns), "MathArray")
                || STReq (NSgetName (ns), NSgetName (NSgetNamespace (global.preludename)))
                || STReq (NSgetName (ns), NSgetName (NSgetRootNamespace ()))) {

                DBUG_PRINT ("Found sel() in correct namespace.");
                return spap;
            }
        }
    }

    return NULL;
}

/*
 * Traversal functions
 */

/* with-loop */
node *
DMMLSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    bool is_single_modarray_for_opt = FALSE;

    /* Check if this is a with-loop with a single operator and that single operator
     * is modarray. */
    if (NODE_TYPE (WITH_WITHOP (arg_node)) == N_modarray
        && MODARRAY_NEXT (WITH_WITHOP (arg_node)) == NULL) {
        DBUG_PRINT ("Found a single-operator modarray-with-loop.");

        /* Check if the source array is a variable. */
        if (NODE_TYPE (MODARRAY_ARRAY (WITH_WITHOP (arg_node))) == N_id) {
            DBUG_PRINT ("Source array is a variable: %s",
                        AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (WITH_WITHOP (arg_node)))));
            is_single_modarray_for_opt = TRUE;
        } else {
            DBUG_PRINT ("Source array is not a variable");
        }
    }

    if (is_single_modarray_for_opt) {
        /*
         * This is a with-loop with a single modarray-operator
         * and the source array is a variable.
         * We apply the optimization to this with-loop.
         */
        info *new_info = MakeInfo ();
        INFO_TRAVERSING_MODARRAY_WITH (new_info) = TRUE;
        INFO_TRAVERSING_MODARRAY_WITH_PARTS (new_info) = TRUE;
        INFO_MODARRAY_TARGET (new_info)
          = ID_AVIS (MODARRAY_ARRAY (WITH_WITHOP (arg_node)));
        INFO_VARDEC (new_info) = NULL;
        DBUG_PRINT ("BEGIN traversing modarray with-loop to apply optimization.");

        /* Traverse parts. */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), new_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDEC (new_info) != NULL) {
            INFO_VARDEC (arg_info)
              = TCappendVardec (INFO_VARDEC (new_info), INFO_VARDEC (arg_info));
            INFO_VARDEC (new_info) = NULL;
        }

        DBUG_PRINT ("END traversing modarray with-loop to apply optimization.");
        INFO_LOCAL_SELECTS (arg_info) += INFO_LOCAL_SELECTS (new_info);
        INFO_BOUNDARY_SELECTS (arg_info) += INFO_BOUNDARY_SELECTS (new_info);
        new_info = FreeInfo (new_info);
    } else {
        /*
         * This is a different kind of with-loop. Do not apply the optimization.
         * Traverse the parts anyway as the with-loop may be contained within
         * a modarray with-loop for which we optimize or it may contain
         * a modarray with-loop that we optimize.
         */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Checks whether the spap is the arithmetic function name
 * from the correct namespace.
 */
static bool
isArithFromCorrectNamespace (node *spid, namespace_t *ns, char *name)
{
    if (STReq (SPID_NAME (spid), name)) {
        /* Check the namespace. name() may be redefined elsewhere. */
        if (STReq (NSgetName (ns), "Array") || STReq (NSgetName (ns), "ArrayArith")) {

            DBUG_PRINT ("Found %s() in correct namespace.", name);
            return TRUE;
        }
    }

    return FALSE;
}

/* with-loop part */
node *
DMMLSpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    bool traversing_modarray_with_parts = INFO_TRAVERSING_MODARRAY_WITH_PARTS (arg_info);

    /* Traverse next part if present. */
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    /* We are done traversing the parts of the last-found modarray-with-loop. */
    INFO_TRAVERSING_MODARRAY_WITH_PARTS (arg_info) = FALSE;

    if (traversing_modarray_with_parts) {
        /* We are applying the optimization. Save the with-loop indices. */

        INFO_MODARRAY_WITHID_IDS (arg_info) = WITHID_IDS (PART_WITHID (arg_node));
        if (INFO_MODARRAY_WITHID_IDS (arg_info) != NULL) {
            DBUG_PRINT ("Found scalarized withid, first dimension: %s",
                        AVIS_NAME (IDS_AVIS (INFO_MODARRAY_WITHID_IDS (arg_info))));
        }

        INFO_MODARRAY_WITHID_VEC (arg_info) = WITHID_VEC (PART_WITHID (arg_node));
        if (INFO_MODARRAY_WITHID_VEC (arg_info) != NULL) {
            DBUG_PRINT ("Found vector withid: %s",
                        AVIS_NAME (IDS_AVIS (INFO_MODARRAY_WITHID_VEC (arg_info))));
        }
    }

    INFO_MODARRAY_PART_CBLOCK (arg_info) = CODE_CBLOCK (PART_CODE (arg_node));

    /* Traverse code of part. */
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    if (INFO_TRAVERSING_MODARRAY_WITH (arg_info)) {
        INFO_SELECTS_FROM_MODARRAY_SOURCE (arg_info) = 0;

        /* Manually search for SPAPs. For some reason the son-traversal never reaches it.
         */
        node *assign = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (arg_node)));

        /* For all assignments within the curly braces that (may) follow a WL generator
         * specification ... */
        while (assign != NULL) {
            node *spap = GetSpapIfStmtIsSel (assign);

            if (spap != NULL) {
                /* The assignment is a LET and the right-hand side is a sel(). */
                node *spid = SPAP_ID (spap);

                /* The first argument of sel is the index. */
                node *first_arg = EXPRS_EXPR (SPAP_ARGS (spap));

                /* The second argument of sel is the array from which we select. */
                node *second_arg = EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (spap)));

                if (CheckIfSelectToSourceArray (first_arg, second_arg, arg_info)) {
                    /* We found a select with the modarray source array as target. */
                    INFO_SELECTS_FROM_MODARRAY_SOURCE (arg_info)++;

                    if (CheckIfSelectIsLocalAndIndexScalar (first_arg, arg_info)) {
                        INFO_LOCAL_SELECTS (arg_info)++;
                        DBUG_PRINT (
                          "Found local sel(). Replacing by _selSxADistmemLocal.");
                        SPID_NAME (spid) = STRcpy ("_selSxADistmemLocal");
                        SPID_NS (spid)
                          = NSdupNamespace (NSgetNamespace (global.preludename));
                    } else if (CheckIfSelectIsLocalAndIndexVector (first_arg, arg_info)) {
                        DBUG_PRINT (
                          "Found local sel(). Replacing by _selVxADistmemLocal.");
                        SPID_NAME (spid) = STRcpy ("_selVxADistmemLocal");
                        SPID_NS (spid)
                          = NSdupNamespace (NSgetNamespace (global.preludename));
                    }
                }
            }
            assign = ASSIGN_NEXT (assign);
        }

        /*
         *  DMGS optimization: Insert a single runtime check that checks
         *  if all accesses are local.
         */
        DBUG_PRINT ("### DMGS optimization ###");

        if (global.optimize.dodmgs
            && (INFO_SELECTS_FROM_MODARRAY_SOURCE (arg_info)
                - INFO_LOCAL_SELECTS (arg_info))
                 >= global.dmgs_min_selects
            && ((INFO_SELECTS_FROM_MODARRAY_SOURCE (arg_info)
                 - INFO_LOCAL_SELECTS (arg_info))
                  <= global.dmgs_max_selects
                || global.dmgs_max_selects == DMGS_MAX_SELECTS_UNBOUNDED)) {
            /*
             * The DMGS optimization is enabled
             * AND there is at least dmgs_min_selects selects that may be applicable to
             * DMGS optimization AND there is at most dmgs_max_selects selects that may be
             * applicable to the DMGS optimization OR there is no maximum. Continue with
             * the DMGS optimization.
             */
            DBUG_PRINT ("!!! There is %d reads from the source array that have not been "
                        "marked as local yet. Continuing with DMGS optimization.",
                        INFO_SELECTS_FROM_MODARRAY_SOURCE (arg_info)
                          - INFO_LOCAL_SELECTS (arg_info));

            node *thenBlock = CODE_CBLOCK (PART_CODE (arg_node));
            node *elseBlock = DUPdoDupTree (thenBlock);

            /* Manually search for SPAPs. For some reason the son-traversal never reaches
             * it. */
            assign = BLOCK_ASSIGNS (thenBlock);
            int max_neg_offset = 0;
            int max_pos_offset = 0;

            /* For all assignments within the curly braces that (may) follow a WL
             * generator specification ... */
            while (assign != NULL) {
                node *spap = GetSpapIfStmtIsSel (assign);

                if (spap != NULL) {
                    /* The assignment is a LET and the right-hand side is a sel(). */
                    node *spid = SPAP_ID (spap);

                    /* The first argument of sel is the index. */
                    node *first_arg = EXPRS_EXPR (SPAP_ARGS (spap));

                    /* The second argument of sel is the array from which we select. */
                    node *second_arg = EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (spap)));

                    if (CheckIfSelectToSourceArray (first_arg, second_arg, arg_info)) {
                        /* We found a select with the modarray source array as target. */

                        node *index_def = GetLastAssignment (first_arg, arg_info);

                        if (index_def != NULL && NODE_TYPE (index_def) == N_array) {
                            /* The variable is assigned a constant array. */

                            node *first_index = EXPRS_EXPR (ARRAY_AELEMS (index_def));
                            if (NODE_TYPE (first_index) == N_id) {
                                DBUG_PRINT ("Index is a constant array. The first index "
                                            "element is the variable %s.",
                                            AVIS_NAME (ID_AVIS (first_index)));

                                node *calc_spap
                                  = GetLastAssignment (first_index, arg_info);

                                if (NODE_TYPE (calc_spap) == N_spap) {
                                    /* The first element of the index is calculated by a
                                     * spap. */
                                    node *calc_spid = SPAP_ID (calc_spap);
                                    namespace_t *calc_ns = SPID_NS (calc_spid);

                                    /* The first argument of the calculation. */
                                    node *calc_first_arg
                                      = EXPRS_EXPR (SPAP_ARGS (calc_spap));

                                    /* The second argument of the calculation. */
                                    node *calc_second_arg
                                      = EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (calc_spap)));

                                    DBUG_PRINT (
                                      "%s is calculated by spap: name = %s, ns = %s, "
                                      "module = %s, first arg = %s, second arg = %s",
                                      AVIS_NAME (ID_AVIS (first_index)),
                                      SPID_NAME (calc_spid), NSgetName (calc_ns),
                                      NSgetModule (calc_ns), NODE_TEXT (calc_first_arg),
                                      NODE_TEXT (calc_second_arg));

                                    bool makeLocal = FALSE;
                                    if (isArithFromCorrectNamespace (calc_spid, calc_ns,
                                                                     "+")) {
                                        if (CheckIfSelectIsLocalAndIndexScalar (
                                              calc_first_arg, arg_info)) {
                                            node *num
                                              = GetLastAssignment (calc_second_arg,
                                                                   arg_info);
                                            if (NODE_TYPE (num) != N_num) {
                                                continue;
                                            }
                                            DBUG_PRINT ("!!! The first argument of the + "
                                                        "is the with-loop index. The "
                                                        "other is %d. Making sel() "
                                                        "local.",
                                                        NUM_VAL (num));
                                            max_pos_offset
                                              = MATHmax (max_pos_offset, NUM_VAL (num));
                                            makeLocal = TRUE;
                                        } else if (CheckIfSelectIsLocalAndIndexScalar (
                                                     calc_second_arg, arg_info)) {
                                            node *num = GetLastAssignment (calc_first_arg,
                                                                           arg_info);
                                            if (NODE_TYPE (num) != N_num) {
                                                continue;
                                            }
                                            DBUG_PRINT ("!!! The second argument of the "
                                                        "+ is the with-loop index. The "
                                                        "other is %d. Making sel() "
                                                        "local.",
                                                        NUM_VAL (num));
                                            max_pos_offset
                                              = MATHmax (max_pos_offset, NUM_VAL (num));
                                            makeLocal = TRUE;
                                        }
                                    } else if (isArithFromCorrectNamespace (calc_spid,
                                                                            calc_ns,
                                                                            "-")) {
                                        node *num
                                          = GetLastAssignment (calc_second_arg, arg_info);
                                        if (NODE_TYPE (num) != N_num) {
                                            continue;
                                        }
                                        DBUG_PRINT ("!!! The first argument of the - is "
                                                    "the with-loop index. The other is "
                                                    "%d. Making sel() local.",
                                                    NUM_VAL (num));
                                        max_neg_offset
                                          = MATHmax (max_neg_offset, NUM_VAL (num));
                                        makeLocal = TRUE;
                                    }

                                    if (makeLocal) {
                                        SPID_NAME (spid) = STRcpy ("_selVxADistmemLocal");
                                        SPID_NS (spid) = NSdupNamespace (
                                          NSgetNamespace (global.preludename));
                                        INFO_BOUNDARY_SELECTS (arg_info)++;
                                    }
                                }
                            }
                        }
                    }
                }
                assign = ASSIGN_NEXT (assign);
            }

            if (INFO_BOUNDARY_SELECTS (arg_info) >= global.dmgs_min_selects
                && (INFO_BOUNDARY_SELECTS (arg_info) <= global.dmgs_max_selects
                    || global.dmgs_max_selects == DMGS_MAX_SELECTS_UNBOUNDED)) {
                /*
                 * We have found at least dmgs_min_selects selects that are applicable to
                 * the DMGS optimization AND we have found at most dmgs_max_selects
                 * selects that are applicable to the DMGS optimization OR there is no
                 * maximum. Continue with the DMGS optimization.
                 */
                DBUG_PRINT ("!!! Found %d reads that are applicable to the DMGS "
                            "optimization. Continuing.",
                            INFO_BOUNDARY_SELECTS (arg_info));

                DBUG_PRINT ("!!! Max. offsets: + %d, - %d", max_pos_offset,
                            max_neg_offset);

                /*
                 *    Generate the following condition:
                 *    if (first_sel_offs >= local_from
                 *         && last_sel_offs < local_to_excl) {
                 *      // All accessed indices of the source array are local.
                 *      // Code block with all local reads.
                 *    } else {
                 *      // Not all of the accessed indices are local.
                 *      // Code block with normal reads.
                 *    }
                 */

                /* Determine which indices are local. */

                /* src_arr_shape = _shape_A( src_arr); */
                node *srcArrShapeAvis
                  = TBmakeAvis (TRAVtmpVarName ("src_arr_shape"),
                                TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (srcArrShapeAvis, INFO_VARDEC (arg_info));
                node *srcArrShapeLet
                  = TBmakeLet (TBmakeIds (srcArrShapeAvis, NULL),
                               TBmakePrf (F_shape_A,
                                          TBmakeExprs (TBmakeId (
                                                         INFO_MODARRAY_TARGET (arg_info)),
                                                       NULL)));

                /* local_count = _localCount_A( src_arr); */
                node *localCountAvis
                  = TBmakeAvis (TRAVtmpVarName ("local_count"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (localCountAvis, INFO_VARDEC (arg_info));
                node *localCountLet
                  = TBmakeLet (TBmakeIds (localCountAvis, NULL),
                               TBmakePrf (F_localCount_A,
                                          TBmakeExprs (TBmakeId (
                                                         INFO_MODARRAY_TARGET (arg_info)),
                                                       NULL)));

                /* local_from = _localFrom_A( src_arr); */
                node *localFromAvis
                  = TBmakeAvis (TRAVtmpVarName ("local_from"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (localFromAvis, INFO_VARDEC (arg_info));
                node *localFromLet
                  = TBmakeLet (TBmakeIds (localFromAvis, NULL),
                               TBmakePrf (F_localFrom_A,
                                          TBmakeExprs (TBmakeId (
                                                         INFO_MODARRAY_TARGET (arg_info)),
                                                       NULL)));

                /* local_to_excl = local_from + local_count; */
                node *localToExclAvis
                  = TBmakeAvis (TRAVtmpVarName ("local_to_excl"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (localToExclAvis, INFO_VARDEC (arg_info));
                node *localToExclLet
                  = TBmakeLet (TBmakeIds (localToExclAvis, NULL),
                               TBmakePrf (F_add_SxS,
                                          TBmakeExprs (TBmakeId (localFromAvis),
                                                       TBmakeExprs (TBmakeId (
                                                                      localCountAvis),
                                                                    NULL))));

                /* Check that the first index is local. */

                /* max_neg_offs = max_neg_offset; */
                node *maxNegOffsAvis
                  = TBmakeAvis (TRAVtmpVarName ("max_neg_offs"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (maxNegOffsAvis, INFO_VARDEC (arg_info));
                node *maxNegOffsLet = TBmakeLet (TBmakeIds (maxNegOffsAvis, NULL),
                                                 TBmakeNumint (max_neg_offset));

                /* first_sel_idx = i - max_neg_offs; */
                node *firstSelIdxAvis
                  = TBmakeAvis (TRAVtmpVarName ("first_sel_idx"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (firstSelIdxAvis, INFO_VARDEC (arg_info));
                node *firstSelIdxLet
                  = TBmakeLet (TBmakeIds (firstSelIdxAvis, NULL),
                               TBmakePrf (F_sub_SxS,
                                          TBmakeExprs (TBmakeId (IDS_AVIS (
                                                         INFO_MODARRAY_WITHID_IDS (
                                                           arg_info))),
                                                       TBmakeExprs (TBmakeId (
                                                                      maxNegOffsAvis),
                                                                    NULL))));

                /* first_sel_vec = [ first_sel_idx ]; */
                node *firstSelVecAvis = TBmakeAvis (TRAVtmpVarName ("first_sel_vec"),
                                                    TYmakeAKS (TYmakeSimpleType (T_int),
                                                               SHcreateShape (1, 1)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (firstSelVecAvis, INFO_VARDEC (arg_info));
                node *firstSelVecLet
                  = TBmakeLet (TBmakeIds (firstSelVecAvis, NULL),
                               TCmakeIntVector (
                                 TBmakeExprs (TBmakeId (firstSelIdxAvis), NULL)));

                /* first_sel_offs = _vect2offset( src_arr_shape, first_sel_vec); */
                node *firstSelOffsAvis
                  = TBmakeAvis (TRAVtmpVarName ("first_sel_offs"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (firstSelOffsAvis, INFO_VARDEC (arg_info));
                node *firstSelOffsLet
                  = TBmakeLet (TBmakeIds (firstSelOffsAvis, NULL),
                               TBmakePrf (F_vect2offset,
                                          TBmakeExprs (TBmakeId (srcArrShapeAvis),
                                                       TBmakeExprs (TBmakeId (
                                                                      firstSelVecAvis),
                                                                    NULL))));

                /* cond_first = first_sel_offs >= local_from; */
                node *condFirstAvis
                  = TBmakeAvis (TRAVtmpVarName ("cond_first"),
                                TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (condFirstAvis, INFO_VARDEC (arg_info));
                node *condFirstLet
                  = TBmakeLet (TBmakeIds (condFirstAvis, NULL),
                               TBmakePrf (F_ge_SxS,
                                          TBmakeExprs (TBmakeId (firstSelOffsAvis),
                                                       TBmakeExprs (TBmakeId (
                                                                      localFromAvis),
                                                                    NULL))));
                /* Check that the last index is local. */

                /* max_pos_offs = max_pos_offset; */
                node *maxPosOffsAvis
                  = TBmakeAvis (TRAVtmpVarName ("max_pos_offs"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (maxPosOffsAvis, INFO_VARDEC (arg_info));
                node *maxPosOffsLet = TBmakeLet (TBmakeIds (maxPosOffsAvis, NULL),
                                                 TBmakeNumint (max_pos_offset));

                /* last_sel_idx = i + max_pos_offs; */
                node *lastSelIdxAvis
                  = TBmakeAvis (TRAVtmpVarName ("last_sel_idx"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (lastSelIdxAvis, INFO_VARDEC (arg_info));
                node *lastSelIdxLet
                  = TBmakeLet (TBmakeIds (lastSelIdxAvis, NULL),
                               TBmakePrf (F_add_SxS,
                                          TBmakeExprs (TBmakeId (IDS_AVIS (
                                                         INFO_MODARRAY_WITHID_IDS (
                                                           arg_info))),
                                                       TBmakeExprs (TBmakeId (
                                                                      maxPosOffsAvis),
                                                                    NULL))));

                /* last_sel_vec = [ last_sel_idx ]; */
                node *lastSelVecAvis = TBmakeAvis (TRAVtmpVarName ("last_sel_vec"),
                                                   TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (1, 1)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (lastSelVecAvis, INFO_VARDEC (arg_info));
                node *lastSelVecLet
                  = TBmakeLet (TBmakeIds (lastSelVecAvis, NULL),
                               TCmakeIntVector (
                                 TBmakeExprs (TBmakeId (lastSelIdxAvis), NULL)));

                /* last_sel_offs = _vect2offset_( src_arr_shape, last_sel_vec); */
                node *lastSelOffsAvis
                  = TBmakeAvis (TRAVtmpVarName ("last_sel_offs"),
                                TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (lastSelOffsAvis, INFO_VARDEC (arg_info));
                node *lastSelOffsLet
                  = TBmakeLet (TBmakeIds (lastSelOffsAvis, NULL),
                               TBmakePrf (F_vect2offset,
                                          TBmakeExprs (TBmakeId (srcArrShapeAvis),
                                                       TBmakeExprs (TBmakeId (
                                                                      lastSelVecAvis),
                                                                    NULL))));

                /* cond_last = last_sel_offs < local_to_excl; */
                node *condLastAvis
                  = TBmakeAvis (TRAVtmpVarName ("cond_last"),
                                TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (condLastAvis, INFO_VARDEC (arg_info));
                node *condLastLet
                  = TBmakeLet (TBmakeIds (condLastAvis, NULL),
                               TBmakePrf (F_lt_SxS,
                                          TBmakeExprs (TBmakeId (lastSelOffsAvis),
                                                       TBmakeExprs (TBmakeId (
                                                                      localToExclAvis),
                                                                    NULL))));

                /* Check that the first and the last index are local. */
                /* arr_all_sel_local = cond_first && cond_last; */
                node *conditionAvis
                  = TBmakeAvis (TRAVtmpVarName ("arr_all_sel_local"),
                                TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0)));
                INFO_VARDEC (arg_info)
                  = TBmakeVardec (conditionAvis, INFO_VARDEC (arg_info));
                node *conditionLet
                  = TBmakeLet (TBmakeIds (conditionAvis, NULL),
                               TBmakePrf (F_and_SxS,
                                          TBmakeExprs (TBmakeId (condFirstAvis),
                                                       TBmakeExprs (TBmakeId (
                                                                      condLastAvis),
                                                                    NULL))));

                node *cond = TBmakeCond (TBmakeId (conditionAvis), thenBlock, elseBlock);

                /* Determine which indices are local. */
                node *optAssigns
                  = TBmakeAssign (
                    srcArrShapeLet,
                    TBmakeAssign (
                      localCountLet,
                      TBmakeAssign (
                        localFromLet,
                        TBmakeAssign (
                          localToExclLet,
                          /* Check that the first index is local. */
                          TBmakeAssign (
                            maxNegOffsLet,
                            TBmakeAssign (
                              firstSelIdxLet,
                              TBmakeAssign (
                                firstSelVecLet,
                                TBmakeAssign (
                                  firstSelOffsLet,
                                  TBmakeAssign (
                                    condFirstLet,
                                    /* Check that the last index is local. */
                                    TBmakeAssign (
                                      maxPosOffsLet,
                                      TBmakeAssign (
                                        lastSelIdxLet,
                                        TBmakeAssign (
                                          lastSelVecLet,
                                          TBmakeAssign (
                                            lastSelOffsLet,
                                            TBmakeAssign (
                                              condLastLet,
                                              /* Check that the first and the last index
                                                 are local. */
                                              TBmakeAssign (
                                                conditionLet,
                                                TBmakeAssign (cond, NULL))))))))))))))));
                node *optBlock = TBmakeBlock (optAssigns, NULL);
                CODE_CBLOCK (PART_CODE (arg_node)) = optBlock;
            } else {
                /* We do not apply the optimization. No need to generate two code blocks.
                 */
                elseBlock = MEMfree (elseBlock);
            }
        }

        DBUG_PRINT ("### Done with DMGS optimization ###");
    }

    DBUG_RETURN (arg_node);
}

/* Function definition */
node *
DMMLSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDEC (arg_info) = NULL;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDEC (arg_info) != NULL) {
            FUNDEF_VARDECS (arg_node)
              = TCappendVardec (INFO_VARDEC (arg_info), FUNDEF_VARDECS (arg_node));
            INFO_VARDEC (arg_info) = NULL;
        }
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/* Primitive function application */
node *
DMMLSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Found primitive function: %s", global.prf_name[PRF_PRF (arg_node)]);

    /* We only care about F_sel_VxA. */
    if (INFO_TRAVERSING_MODARRAY_WITH (arg_info) && PRF_PRF (arg_node) == F_sel_VxA) {
        /* The first argument of F_sel_VxA is the index. */
        node *first_arg = EXPRS_EXPR (PRF_ARGS (arg_node));

        /* The second argument of F_sel_VxA is the array from which we select. */
        node *second_arg = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (arg_node)));

        if (CheckIfSelectToSourceArray (first_arg, second_arg, arg_info)) {
            PRF_DISTMEMISLOCALREAD (arg_node)
              = CheckIfSelectIsLocalAndIndexVector (first_arg, arg_info);
        }
    }

    /* In any case, traverse the arguments of the primitive function application. */
    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
DMMLSdoMarkLocalSelects (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_dmmls);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);
    CTInote ("Found %d local selects (DMMLS optimization).", INFO_LOCAL_SELECTS (info));
    CTInote ("Found %d possibly local selects (DMGS optimization).",
             INFO_BOUNDARY_SELECTS (info));
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
