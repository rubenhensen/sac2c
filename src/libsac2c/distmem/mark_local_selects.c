/*
 * Marks array read accesses that are known to be local for the distributed memory.
 *
 *
 * Why this optimization?
 *
 * By generating different code we can avoid the runtime-check whether a read
 * is local or remote. Since read accesses occur frequently,
 * the frequent checks have a bad influence on the overall performance.
 * Note: We do not need this optimization for write accesses because they are known to be
 * always local ("owner computes" principle).
 *
 *
 * How do we recognize local reads?
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
 * By default, this optimization is activated when the distributed memory backend is used.
 * If it does produce bugs, you can switch it off by specifying: -no dmmls
 *
 */

#include "mark_local_selects.h"

#include "str.h"
#include "traverse.h"
#include "memory.h"
#include "tree_basic.h"
#include "new_types.h"
#include "namespaces.h"

#define DBUG_PREFIX "DMMLS"
#include "debug.h"

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
 * Examines a variable that contains a selection index.
 * Returns whether the first dimension of the index is equal to
 * the first dimension of the with-loop index.
 */
static bool
CheckIfVarIsFirstDimOfWLIdx (node *var, info *arg_info)
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
                DBUG_PRINT ("Found let to the selection index variable.");

                latest_let_expr = LET_EXPR (stmt);
            }
        }

        /* Continue to next assignment. */
        cblock_expr = ASSIGN_NEXT (cblock_expr);
    }

    if (latest_let_expr != NULL && NODE_TYPE (latest_let_expr) == N_array) {
        /* The variable is assigned a constant array. */
        DBUG_PRINT ("The let is to a constant array.");

        if (NODE_TYPE (EXPRS_EXPR (ARRAY_AELEMS (latest_let_expr))) == N_id) {
            DBUG_PRINT ("The first element of the array is a variable.");

            if (ID_AVIS (EXPRS_EXPR (ARRAY_AELEMS (latest_let_expr)))
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
 * the sel() function) is local by examining the first argument (the selection index) and
 * the second argument (the array that is accessed) if the selection index is a scalar.
 */
static bool
CheckIfSelectIsLocalAndIndexScalar (node *first_arg, node *second_arg, info *arg_info)
{
    if (CheckIfSelectToSourceArray (first_arg, second_arg, arg_info)) {
        if (INFO_MODARRAY_WITHID_IDS (arg_info) != NULL
            && ID_AVIS (first_arg) == IDS_AVIS (INFO_MODARRAY_WITHID_IDS (arg_info))) {
            DBUG_PRINT ("!!! The first argument is equal to the first dimension of the "
                        "with-loop index. We have found a local read access !!!");
            INFO_LOCAL_SELECTS (arg_info)++;
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Checks whether a select (either a call to the _sel_VxA_ primitive function or a call to
 * the sel() function) is local by examining the first argument (the selection index) and
 * the second argument (the array that is accessed) if the selection index is a vector.
 */
static bool
CheckIfSelectIsLocalAndIndexVector (node *first_arg, node *second_arg, info *arg_info)
{
    if (CheckIfSelectToSourceArray (first_arg, second_arg, arg_info)) {
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

    return FALSE;
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
        DBUG_PRINT ("BEGIN traversing modarray with-loop to apply optimization.");

        /* Traverse parts. */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), new_info);

        DBUG_PRINT ("END traversing modarray with-loop to apply optimization.");
        INFO_LOCAL_SELECTS (arg_info) += INFO_LOCAL_SELECTS (new_info);
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

/* with-loop part */ /*
                      * This is a test for the DMMLS optimization.
                      * We detect local selects (Array::sel()) and replace them
                      * by sacprelude::_selVxADistmemLocal.
                      */
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
        /* Manually search for SPAPs. For some reason the son-traversal never reaches it.
         */
        node *assign = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (arg_node)));

        /* For all assignments within the curly braces that (may) follow a WL generator
         * specification ... */
        while (assign != NULL) {
            /* If it is a LET statement and the right-hand side is a SPAPMakeInfo. */
            if (NODE_TYPE (ASSIGN_STMT (assign)) == N_let
                && NODE_TYPE (LET_EXPR (ASSIGN_STMT (assign))) == N_spap) {
                node *spap = LET_EXPR (ASSIGN_STMT (assign));
                node *spid = SPAP_ID (spap);
                namespace_t *ns = SPID_NS (spid);
                DBUG_PRINT ("Found spap: name = %s, ns = %s, module = %s",
                            SPID_NAME (spid), NSgetName (ns), NSgetModule (ns));
                if (STReq (SPID_NAME (spid), "sel")) {
                    /* Check the namespace. sel() may be redefined elsewhere. */
                    if (STReq (NSgetName (ns), "Array")
                        || STReq (NSgetName (ns), "ArrayBasics")
                        || STReq (NSgetName (ns), "ArrayArith")
                        || STReq (NSgetName (ns), "MathArray")
                        || STReq (NSgetName (ns),
                                  NSgetName (NSgetNamespace (global.preludename)))
                        || STReq (NSgetName (ns), NSgetName (NSgetRootNamespace ()))) {

                        DBUG_PRINT ("Found sel() in correct namespace.");

                        /* The first argument of sel is the index. */
                        node *first_arg = EXPRS_EXPR (SPAP_ARGS (spap));

                        /* The second argument of sel is the array from which we select.
                         */
                        node *second_arg = EXPRS_EXPR (EXPRS_NEXT (SPAP_ARGS (spap)));

                        if (CheckIfSelectIsLocalAndIndexScalar (first_arg, second_arg,
                                                                arg_info)) {
                            DBUG_PRINT (
                              "Found local sel(). Replacing by _selSxADistmemLocal.");
                            SPID_NAME (spid) = STRcpy ("_selSxADistmemLocal");
                            SPID_NS (spid)
                              = NSdupNamespace (NSgetNamespace (global.preludename));
                        } else if (CheckIfSelectIsLocalAndIndexVector (first_arg,
                                                                       second_arg,
                                                                       arg_info)) {
                            DBUG_PRINT (
                              "Found local sel(). Replacing by _selVxADistmemLocal.");
                            SPID_NAME (spid) = STRcpy ("_selVxADistmemLocal");
                            SPID_NS (spid)
                              = NSdupNamespace (NSgetNamespace (global.preludename));
                        }
                    }
                }
            }
            assign = ASSIGN_NEXT (assign);
        }
    }

    DBUG_RETURN (arg_node);
}

/* Function application, TODO: remove */
node *
DMMLSspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("!!!!!!!!!!!Found fun ap outside: name = %s ns = %s",
                SPID_NAME (SPAP_ID (arg_node)), SPID_NS (SPAP_ID (arg_node)));

    if (INFO_TRAVERSING_MODARRAY_WITH (arg_info)) {
        DBUG_PRINT ("!!!!!!!!!!!!Found fun ap inside: name = %s ns = %s",
                    SPID_NAME (SPAP_ID (arg_node)), SPID_NS (SPAP_ID (arg_node)));
    }

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

        PRF_DISTMEMISLOCALREAD (arg_node)
          = CheckIfSelectIsLocalAndIndexVector (first_arg, second_arg, arg_info);
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
    DBUG_PRINT ("Found %d local selects.", INFO_LOCAL_SELECTS (info));
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
