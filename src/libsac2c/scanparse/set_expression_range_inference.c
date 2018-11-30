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
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
SERIdoInferRanges (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

#if 0
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_setwl,
                "SERIdoInferRanges called on non-Nsetwl node!");
#endif

    arg_info = MakeInfo (NULL);
    TRAVpush (TR_seri);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}


/**
 * Used to scan selections for ids found in a prior lamination.
 * Depending on the type of the selection vector, ScanId or ScanVector
 * is called.
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
        DBUG_PRINT_TAG ("SERI_ACT", "Primitive selection found; scanning for genvars..." );
        SEUTscanSelectionForShapeInfo (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), INFO_SERI_IDTABLE (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/**
 * hook to handle axis control selections. Any selection found is parsed
 * for dots and if any found replaced by the matching withloop.
 * If currently scanning for selection ids, the ids of the selection are
 * passed to ScanId/ScanVector depending on the selection vector type.
 * All other AP nodes are passed without any further action.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
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

    if (STReq (SPAP_NAME (arg_node), "sel") && (SPAP_NS (arg_node) == NULL)) {
        DBUG_PRINT_TAG ("SERI_ACT", "Spap selection found; scanning for genvars..." );
        SEUTscanSelectionForShapeInfo (SPAP_ARG1 (arg_node), SPAP_ARG2 (arg_node), INFO_SERI_IDTABLE (arg_info));
    } 

    DBUG_RETURN (arg_node);
}

/**
 * hook to handle any lamination operators. The inner expression is parsed for
 * occuriencies of ids in the lamination vector. Afterwards the lamination
 * operator is replaced by the corresponding withloop and the inner expression
 * is parsed. To distinguish between parsing for ids and normal dot
 * replacement, an entry within the info node is used.
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
SERIsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("looking at Set-Expression in line %d:", global.linenum);

    /* we do this bottom up to ensure we have a shape! Only the last
     * partition is guaranteed to produce an upper bound!
     * A reference to that upper bound is put into INFO_SERI_SHP of
     * the *surrounding* arg_info, i.e. into the *current* arg_info.
     */
    if (SETWL_NEXT (arg_node) != NULL) {
        SETWL_NEXT (arg_node) = TRAVdo (SETWL_NEXT (arg_node), arg_info);
        DBUG_ASSERT (INFO_SERI_SHP (arg_info) != NULL, "last setWL partition"
                     " did not insert shape!");
    }

    arg_info = MakeInfo (arg_info);

    /*
     * First, we check which ranges are missing
     */
    INFO_SERI_LBMISSING( arg_info) = 
        ( (SETWL_GENERATOR (arg_node) == NULL)
          || (GENERATOR_BOUND1 (SETWL_GENERATOR (arg_node)) == NULL) );
    DBUG_PRINT( " lower bound is %s!", (INFO_SERI_LBMISSING( arg_info)? "missing" : "present"));

    INFO_SERI_UBMISSING( arg_info) = 
        ( (SETWL_GENERATOR (arg_node) == NULL)
          || (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node)) == NULL) );
    DBUG_PRINT( " upper bound is %s!", (INFO_SERI_UBMISSING( arg_info)? "missing" : "present"));

    DBUG_PRINT( "adding new ids to idtable");
    INFO_SERI_IDTABLE (arg_info) = SEUTbuildIdTable (SETWL_VEC (arg_node),
                                                     INFO_SERI_IDTABLE (INFO_SERI_NEXT (arg_info)));
    DBUG_PRINT( "traversing expression...");
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);
    if (SETWL_GENERATOR (arg_node) == NULL) {
        SETWL_GENERATOR (arg_node) = TBmakeGenerator (F_noop, F_noop, NULL, NULL, NULL, NULL);
    }
    INFO_SERI_ISLASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);

    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);

    SEUTfreeIdTable (INFO_SERI_IDTABLE (arg_info));
    INFO_SERI_IDTABLE (arg_info) = NULL;
    arg_info = FreeInfo (arg_info);
    DBUG_POP();

    DBUG_RETURN (arg_node);
}


/**
 * hook to insert the inferred shape knowledge, or, in the lack thereof, 
 * dots UNLESS we are the last partition in which cazse the lack of bounds
 * will raise an error.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
 */
node *
SERIgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("SERI_ACT", "traversing generator...");
    if (INFO_SERI_UBMISSING (arg_info)) {
        if (SEUTshapeInfoComplete (INFO_SERI_IDTABLE (arg_info))) {
            /* We now insert the information we found.
             * We distinguish 2 cases of original set-expressions here:
             * 1) { iv -> expr | lb rel iv ; ...}
             *    => { iv -> expr | iv < take(shape(lb),shape_info) ; ... }
             * 2) { iv -> expr ; ... } 
             *    or { iv -> expr | . <= iv; ... }
             *    or { [i_0, ..., i_n] | ...; ...}
             *    => { idx -> expr | idx < shape_info ; ... }
             */
            if (SEUTisVector (INFO_SERI_IDTABLE (arg_info))
                 && !INFO_SERI_LBMISSING (arg_info)
                 && (NODE_TYPE (GENERATOR_BOUND1 (arg_node)) != N_dot)) {
                // case 1)
                DBUG_PRINT ("Adding suitable prefix of inferred upper bound");
                GENERATOR_BOUND2 (arg_node)
                    = TCmakePrf2 (F_take_SxV,
                                  TCmakePrf2 (F_sel_VxA,
                                              TCcreateIntVector( 1, 0, 0),
                                              TCmakePrf1 (F_shape_A,
                                                          DUPdoDupTree (GENERATOR_BOUND1 (arg_node)))),
                                  SEUTbuildWLShape (INFO_SERI_IDTABLE (arg_info)));
                GENERATOR_OP2 (arg_node) = F_wl_lt;
            } else {
                // case 2)
                DBUG_PRINT ("Adding inferred upper bound");
                GENERATOR_BOUND2 (arg_node) = SEUTbuildWLShape (INFO_SERI_IDTABLE (arg_info));
                GENERATOR_OP2 (arg_node) = F_wl_lt;
            }
        } else {
            if (INFO_SERI_ISLASTPART (arg_info)) {
                CTIerrorLoc ( NODE_LOCATION (arg_node),
                              "Unable to infer upper bound for final partition of"
                              " set expression; please specify an upper bound.");
            } else {
                /*
                 * We distinguish 3 cases of original set-expressions here:
                 * 1) { iv -> expr ; ...}   or     { iv -> expr ; . <= iv ; ...}
                 *    =>   { iv -> expr | iv <= . ; ...}
                 *
                 * 2) { iv -> expr | lb rel iv ; ... | idx < last_ub }
                 *    =>  { iv -> expr | lb rel iv < take(shape(lb), last_ub) ; ...}
                 *
                 * 3) { [i_1, ..., i_n] -> expr | ... ; ... | idx < last_ub }
                 *    => { [i_1, ..., i_n] -> expr | [i_1, ..., i_n] < take(n,last_ub) ; ...}
                 */
                if (SEUTisVector (INFO_SERI_IDTABLE (arg_info))
                    && !SEUTisDot (INFO_SERI_IDTABLE (arg_info))) {
                    if (INFO_SERI_LBMISSING (arg_info) 
                        || (NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_dot) ){
                        // case 1)
                        CTIwarn ("Unable to infer upper bound for a partition of"
                                 " a set expression; using \".\" instead.");
                        GENERATOR_BOUND2 (arg_node) = TBmakeDot (1);
                        GENERATOR_OP2 (arg_node) = F_wl_le;
                    } else {
                        // case 2)
                        CTIwarn ("Unable to infer upper bound for a partition of"
                                 " a set expression; using upper bound from last partition instead.");
                        GENERATOR_BOUND2 (arg_node)
                            = TCmakePrf2 (F_take_SxV,
                                          TCmakePrf2 (F_sel_VxA,
                                                      TCcreateIntVector( 1, 0, 0),
                                                      TCmakePrf1 (F_shape_A,
                                                                  DUPdoDupTree (GENERATOR_BOUND1 (arg_node)))),
                                          DUPdoDupTree (INFO_SERI_SHP (INFO_SERI_NEXT (arg_info))));
                        GENERATOR_OP2 (arg_node) = F_wl_lt;
                    }
                } else if (SEUTisScalar (INFO_SERI_IDTABLE (arg_info))) {
                    // case 3)
                    CTIwarn ("Unable to infer upper bound for a partition of"
                             " a set expression; using upper bound from last partition instead.");
                    GENERATOR_BOUND2 (arg_node)
                        = TCmakePrf2 (F_take_SxV,
                                      TBmakeNum (SEUTcountIds (INFO_SERI_IDTABLE (arg_info))),
                                      DUPdoDupTree (INFO_SERI_SHP (INFO_SERI_NEXT (arg_info))));
                    GENERATOR_OP2 (arg_node) = F_wl_lt;
                }
            }
        }
        
    } else {
        DBUG_PRINT ("Traversing user specified upper bound");
        GENERATOR_BOUND2 (arg_node) = TRAVdo (GENERATOR_BOUND2 (arg_node), arg_info);
    }

    if (INFO_SERI_ISLASTPART (arg_info)) {
        /* Now, we add the shape information into the "surrounding" arg_info!
         * We need to put it there as it is needed by all other partitions of the
         * current setwl as well and the current arg_info will be gone as soon as we
         * leave the last partition!
         */
        INFO_SERI_SHP (INFO_SERI_NEXT (arg_info)) = GENERATOR_BOUND2 (arg_node);
    }

    if (INFO_SERI_LBMISSING (arg_info)) {
        /*
         * We distinguish 3 cases of original set-expressions here:
         * 1) { iv -> expr }   or     { iv -> expr ; iv <= . ; ...}
         *    =>   { iv -> expr | . <= iv }
         *
         * 2) { iv -> expr | iv rel ub }
         *    =>  { iv -> expr | genarray(shape(ub), 0) <= iv rel ub}
         *
         * 3) { [i_1, ..., i_n] -> expr | ...}
         *    => { [i_1, ..., i_n] -> expr | [0,...0] <= [i_1, ..., i_n] }
         */
         if (SEUTisVector (INFO_SERI_IDTABLE (arg_info))) {
             if (INFO_SERI_UBMISSING (arg_info)
                || (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_dot) ) {
                 // case 1)
                 CTInote ("Unable to infer lower bound for a partition of"
                          " a set expression; using \".\" instead.");
                 GENERATOR_BOUND1 (arg_node) = TBmakeDot (1);
             } else {
                 // case 2)
                 CTInote ("Unable to infer lower bound for a partition of"
                          " a set expression; using upper-bound-many 0s instead.");
                 GENERATOR_BOUND1 (arg_node) = BuildWlZeros(
                                                   TCmakePrf1 (F_shape_A,
                                                      DUPdoDupTree (GENERATOR_BOUND2 (arg_node))));
             }
         } else if (SEUTisScalar (INFO_SERI_IDTABLE (arg_info))) {
             // case 3)
             CTInote ("Unable to infer lower bound for a partition of"
                      " a set expression; using a vector of 0s instead.");
             GENERATOR_BOUND1 (arg_node)
                 = TCcreateIntVector (SEUTcountIds (INFO_SERI_IDTABLE (arg_info)),0,0);
         }
         GENERATOR_OP1 (arg_node) = F_wl_le;
        
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

    DBUG_PRINT_TAG ("SERI_ACT", "traversing generator done; final state:");
    DBUG_EXECUTE (PRTdoPrintFile (stderr, arg_node););

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
