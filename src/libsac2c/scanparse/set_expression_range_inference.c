#include "set_expression_range_inference.h"
#include "set_expression_utils.h"
#include "traverse.h"

#define DBUG_PREFIX "SERI"
#include "debug.h"

#include "free.h"
#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "new_types.h"
#include "globals.h"
#include "tree_compound.h"
#include "print.h"

#include <strings.h>

/**
 * @file set_expression_range_inference.c
 *
 */


/**
 * arg_info in this file:
 * LBMISSING : boolean to indicate the presence of the lower bound
 * UBMISSING : boolean to indicate the presence of the upper bound
 * ISLASTPART: boolean to indicate whether we are operating on the last
 *             partition
 * IDTABLE   : the abstract data type in SEUT that collects, maintains
 *             and computes bounds from the shape information found
 * NEXT      : needed for stacking info nodes
 */

/* INFO structure */
struct INFO {
    bool lbmissing;
    bool ubmissing;
    bool islastpart;
    idtable *idtable;
    struct INFO *next;
};

/* access macros */
#define INFO_SERI_LBMISSING(n) ((n)->lbmissing)
#define INFO_SERI_UBMISSING(n) ((n)->ubmissing)
#define INFO_SERI_ISLASTPART(n) ((n)->islastpart)
#define INFO_SERI_IDTABLE(n) ((n)->idtable)
#define INFO_SERI_NEXT(n) ((n)->next)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (info *oldinfo)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SERI_LBMISSING (result) = FALSE;
    INFO_SERI_UBMISSING (result) = FALSE;
    INFO_SERI_ISLASTPART (result) = FALSE;
    INFO_SERI_IDTABLE (result) = NULL;
    INFO_SERI_NEXT (result) = oldinfo;

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *arg_info)
{
    info *next;

    DBUG_ENTER ();

    next = INFO_SERI_NEXT (arg_info);
    arg_info = MEMfree (arg_info);

    DBUG_RETURN (next);
}

/**
 * hook to start the range inference for set expressions.
 *
 * @param arg_node current AST
 * @result transformed AST with explicit lower and upper bounds
 */
node *
SERIdoInferRanges (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    arg_info = MakeInfo (NULL);
    TRAVpush (TR_seri);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}


/**
 * Used to scan selections for shape info
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
SERIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Check for nested info and deal with ste-expessions inside!
     * Since the scanning potentially takes copies of subexpressions
     * we need to deal with inner set expression first!
     */

    arg_node = TRAVcont (arg_node, arg_info);

    if (PRF_PRF (arg_node) == F_sel_VxA) {
        DBUG_PRINT_TAG ("SERI_SEUT", "Primitive selection found; scanning for genvars..." );
        SEUTscanSelectionForShapeInfo (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), INFO_SERI_IDTABLE (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/**
 * Used to scan selections for shape info
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
SERIspap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Check for nested info and deal with ste-expessions inside!
     * Since the scanning potentially takes copies of subexpressions
     * we need to deal with inner set expression first!
     */

    SPAP_ARGS (arg_node) = TRAVopt (SPAP_ARGS (arg_node), arg_info);

    if (STReq (SPAP_NAME (arg_node), "sel") && (SPAP_NS (arg_node) == NULL)
        && (TCcountExprs (SPAP_ARGS (arg_node)) == 2)) {
        DBUG_PRINT_TAG ("SERI_SEUT", "Spap selection found; scanning for genvars..." );
        SEUTscanSelectionForShapeInfo (SPAP_ARG1 (arg_node), SPAP_ARG2 (arg_node), INFO_SERI_IDTABLE (arg_info));
    } 

    DBUG_RETURN (arg_node);
}

/**
 * hook for with loops
 * This hook is only needed to ensure N_generator traversals will see
 * an arg_info where INFO_LBMISSING and friends are FALSE if the
 * generator lives in a WL rather than a setWL!
 * The relevant case is where a WL lives inside a setWL!
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
SERIwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);

    arg_info = MakeInfo (arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * hook for set expressions
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
SERIsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);

    arg_info = MakeInfo (arg_info);

    /*
     * First, we check which ranges are missing
     */
    INFO_SERI_LBMISSING (arg_info) = 
        ( (SETWL_GENERATOR (arg_node) == NULL)
          || (GENERATOR_BOUND1 (SETWL_GENERATOR (arg_node)) == NULL) );
    DBUG_PRINT (" lower bound is %s!", (INFO_SERI_LBMISSING (arg_info)? "missing" : "present"));

    INFO_SERI_UBMISSING (arg_info) = 
        ( (SETWL_GENERATOR (arg_node) == NULL)
          || (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node)) == NULL) );
    DBUG_PRINT (" upper bound is %s!", (INFO_SERI_UBMISSING (arg_info)? "missing" : "present"));

    DBUG_PRINT ("adding new ids to idtable");
    INFO_SERI_IDTABLE (arg_info) = SEUTbuildIdTable (SETWL_VEC (arg_node),
                                                     INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
    DBUG_PRINT ("traversing expression...");
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);
    if (SETWL_GENERATOR (arg_node) == NULL) {
        SETWL_GENERATOR (arg_node) = TBmakeGenerator (F_noop, F_noop, NULL, NULL, NULL, NULL);
    }
    INFO_SERI_ISLASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);

    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);

    SEUTfreeIdTable (INFO_SERI_IDTABLE (arg_info));
    INFO_SERI_IDTABLE (arg_info) = NULL;
    arg_info = FreeInfo (arg_info);

    SETWL_NEXT (arg_node) = TRAVopt (SETWL_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}


/**
 * hook to insert the inferred shape knowledge, or, in the lack thereof, 
 * dots UNLESS we are the last partition in which case the lack of bounds
 * will raise an error.
 * This constitutes the meat of this traversal. A formal specification
 * can be found in the desugaring document within the sacdoc/bnf directory!
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
 */
node *
SERIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("traversing generator...");
    if (INFO_SERI_UBMISSING (arg_info)) {
        if (SEUTshapeInfoComplete (INFO_SERI_IDTABLE (arg_info))) {
            /* We now insert the information we found.
             * We distinguish 2 cases of original set-expressions here:
             * 1) { iv -> expr | lb rel iv ; ...} where lbi != "."
             *    => { iv -> expr | iv < take(shape(lb),shape_info) ; ... }
             * 2) { iv -> expr ; ... } 
             *    or { iv -> expr | . <= iv; ... }
             *    or { [i_0, ..., i_n] | ...; ...}
             *    => { didxs -> expr | idxs < shape_info ; ... }
             */
            if (SEUTisVector (INFO_SERI_IDTABLE (arg_info))
                 && !INFO_SERI_LBMISSING (arg_info)
                 && (NODE_TYPE (GENERATOR_BOUND1 (arg_node)) != N_dot)) {
                // case 1)
                DBUG_PRINT ("Adding suitable prefix of inferred upper bound");
                GENERATOR_BOUND2 (arg_node)
                    = TCmakePrf2 (F_take_SxV,
                                  TCmakePrf2 (F_sel_VxA,
                                              TCcreateIntVector (1, 0, 0),
                                              TCmakePrf1 (F_shape_A,
                                                          DUPdoDupTree (GENERATOR_BOUND1 (arg_node)))),
                                  SEUTgenShape (INFO_SERI_IDTABLE (arg_info)));
                GENERATOR_OP2 (arg_node) = F_wl_lt;
            } else {
                // case 2)
                DBUG_PRINT ("Adding inferred upper bound");
                GENERATOR_BOUND2 (arg_node) = SEUTgenShape (INFO_SERI_IDTABLE (arg_info));
                GENERATOR_OP2 (arg_node) = F_wl_lt;
            }
        } else {
            if (INFO_SERI_ISLASTPART (arg_info)) {
                CTIerror (NODE_LOCATION (arg_node),
                          "Unable to infer upper bound for final partition of"
                          " set expression; please specify an upper bound.");
            } else {
                /*
                 * The shiny new "adjustable length bound-dot" (see HWLD for details)
                 * allows us to insert "."  in all cases here :-)
                 * The rules SERI-None and SERI-Lower in the desugaring doc both
                 * use dot here:-)
                 *
                 * { didxs -> expr ; ...}
                 *    =>   { didxs -> expr | idxs <= . ; ...}
                 *
                 */
                CTInote ("Unable to infer upper bound for a partition of"
                         " a set expression; using \".\" instead.");
                GENERATOR_BOUND2 (arg_node) = TBmakeDot (1);
                GENERATOR_OP2 (arg_node) = F_wl_le;
            }
        }
        
    } else {
        DBUG_PRINT ("Traversing user specified upper bound");
        GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    }

    if (INFO_SERI_LBMISSING (arg_info)) {
        /*
         * We know, that the upper bound now does exist PROVIDED
         * we did not have an error!
         * Hence, we distinguish 2 cases:
         * 1) { didxs -> expr | idxs <= .}
         *    =>   { didxs -> expr | . <= idxs <= .}
         *
         * 2) { didxs -> expr | idxs <= ub }
         *    =>  { didxs -> expr | 0*ub <= idxs <= ub }
         */
        if (GENERATOR_BOUND2 (arg_node) != NULL) {
            if (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_dot) {
                GENERATOR_BOUND1 (arg_node) = TBmakeDot (1);
            } else {
                 GENERATOR_BOUND1 (arg_node) = TCmakePrf2 (F_mul_SxV,
                                                           TBmakeNum (0),
                                                           DUPdoDupTree (GENERATOR_BOUND2 (arg_node)));
            }
            GENERATOR_OP1 (arg_node) = F_wl_le;
        }
        
    } else {
        DBUG_PRINT ("Traversing user specified lower bound");
        GENERATOR_BOUND1 (arg_node) = TRAVdo (GENERATOR_BOUND1 (arg_node), arg_info);
    }

    if (GENERATOR_STEP (arg_node) != NULL) {
        DBUG_PRINT ("traversing step expression");
        GENERATOR_STEP (arg_node) = TRAVdo (GENERATOR_STEP (arg_node), arg_info);
    }

    if (GENERATOR_WIDTH (arg_node) != NULL) {
        DBUG_PRINT ("traversing width expression");
        GENERATOR_WIDTH (arg_node) = TRAVdo (GENERATOR_WIDTH (arg_node), arg_info);
    }

    DBUG_PRINT ("traversing generator done; final state:");
    DBUG_EXECUTE (PRTdoPrintFile (stderr, arg_node););

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
