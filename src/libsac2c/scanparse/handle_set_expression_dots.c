#include "handle_set_expression_dots.h"
#include "set_expression_utils.h"
#include "traverse.h"

#define DBUG_PREFIX "HSED"
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
#include "pattern_match.h"

#include <strings.h>

/**
 * @file handle_set_expression_dots.c
 *
 * description:
 *   This module implements the desugaring of dots in tensor comprehensions
 *   (also referred to as "set expressions"). A detailed high-level
 *   description can be found in the desugaring document (bnf/desugaring.tex) in 
 *   >>>    gitolite@sac-home.org:docs.git     <<<
 *
 *   In essence, it elides single dots and triple dots within the index
 *   variable descriptions of all partitions. Here, we look at an example
 *   of both cases:
 *
 *   1) Single Dot:
 *
 *   { ...;
 *     [ i, ., j, ., .] -> expr | lb <= [i,j] < ub;
 *     ...; }
 *
 *   is translated into:
 *
 *   { ...;
 *     [ i, t1, j, t2, t3] -> expr[[t1, t2, t3]] 
 *         | [lb[[0]], 0, lb[[1]], 0, 0] <= [i,t1,j,t2,t3]
 *           < [ub[[0]], shape (SUBST(i,0, SUBST(j,0,expr)))[0],
 *              ub[[1]], shape (SUBST(i,0, SUBST(j,0,expr)))[1],
 *                       shape (SUBST(i,0, SUBST(j,0,expr)))[2]];
 *     ...; }
 *
 *   Note here, that the expression SUBST(i,0, SUBST(j,0,expr))
 *   potentially contains an out-of bounds access due to the cell
 *   shape having an empty component. The Tensor Comprehension paper
 *   at IFL explains how this *should* be avoided using the inference
 *   of the specialisation oracle. In practice, this is (a) very rare
 *   and (b) our optimiser often elides the code  anyways due to
 *   structural constant folding.... it should be changed at some
 *   point though....!
 *
 *   2) Triple Dots (can occur max once! per index description):
 *   
 *   { ...;
 *     [ i, ..., j] -> expr | lb <= [i,j] < ub;
 *     ...; }
 *
 *   is translated into:
 *
 *   { ...;
 *     tv -> { i = tv[0];
 *             iv = drop (1, drop (-1, tv));
 *             j = tv[shape(tv)-1];
 *           } : expr[[]++iv++[]]           // <- empties here for s-dots
 *         | [lb[[0]]]
 *            ++ genarray ([dim (SUBST(i,0, SUBST(j,0,expr))) - 0], 0)  // <- subtraction here for s-dots
 *            ++ [lb[[1]]]
 *           <= [i,t1,j,t2,t3]
 *           <  [ub[[0]]]
 *               ++ drop (0, drop (-0, shape (SUBST(i,0, SUBST(j,0,expr))))) // <- drops here for the s-dots
 *               ++ [ub[[1]]];
 *     ...; }
 *
 *   Again, we have the potential out-of-bounds problem with
 *   SUBST(i,0, SUBST(j,0,expr)) as explained above!
 *
 * implementation:
 *    needs to be described here ....
 */

/**
 * set this to defined in order to create explanatory ids. use this only
 * for debugging as it might create very long identifier names.
 */
#define HSE_USE_EXPLANATORY_NAMES

/**
 * arg_info in this file:
 * IDS:    N_exprs list of lhs identifiers of the setwl.
 * LENTD:  an expression that computes the length of the triple-dot
 *         match (potentially at runtime)
 */

/* INFO structure */
struct INFO {
    bool hd;
    node *didxs;
    node *zexpr;
    size_t lm;
    size_t ln;
    size_t rm;
    size_t rn;
    size_t k;
    struct INFO *next;
};

/* access macros */
#define INFO_HSED_HAS_DOTS(n) ((n)->hd)
#define INFO_HSED_DIDXS(n) ((n)->didxs)
#define INFO_HSED_ZEXPR(n) ((n)->zexpr)
#define INFO_HSED_LM(n) ((n)->lm)
#define INFO_HSED_LN(n) ((n)->ln)
#define INFO_HSED_RM(n) ((n)->rm)
#define INFO_HSED_RN(n) ((n)->rn)
#define INFO_HSED_K(n) ((n)->k)
#define INFO_HSED_NEXT(n) ((n)->next)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (info *old)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HSED_HAS_DOTS (result) = FALSE;
    INFO_HSED_DIDXS (result) = NULL;
    INFO_HSED_ZEXPR (result) = NULL;
    INFO_HSED_LM (result) = 0;
    INFO_HSED_LN (result) = 0;
    INFO_HSED_RM (result) = 0;
    INFO_HSED_RN (result) = 0;
    INFO_HSED_K (result) = 0;
    INFO_HSED_NEXT (result) = old;

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *arg_node)
{
    info *next;

    DBUG_ENTER ();

    next = INFO_HSED_NEXT (arg_node);
    arg_node = MEMfree (arg_node);

    DBUG_RETURN (next);
}

/**
 * builds an id with a free name by calling TmpVarName. If
 * HSE_USE_EXPLANATORY_NAMES is set, name is appended to the new id,
 * Use this feature only for debugging, as it might create very long
 * identifier names.
 *
 * @param name explanatory name of the identifier
 * @return a new created unique id node
 */
static node *
MakeTmpId (char *name)
{
    node *result;

    DBUG_ENTER ();

#ifdef HSE_USE_EXPLANATORY_NAMES
    result = TBmakeSpid (NULL, TRAVtmpVarName (name));
#else
    result = TBmakeSpid (NULL, TRAVtmpVar ());
#endif

    DBUG_RETURN (result);
}

/*******************************************************************************
 * The following functions are needed for generating inputs for 
 * MergeIn as they are needed for this traversal.
 *
 * CreateDotVarChain --- creates a chain of tmp vars (needed for nidxs)
 * StripDots         --- creates a chain by copying all non-dot entries from a
 *                       template (needed for nidxs)
 * CreateConstChain  --- creates a chain of 0s/1s (needed for nlb,nstep,nwidth)
 * CreateSelChain    --- creates a chain of selections into an expresion (nub)
 */
/** <!--********************************************************************-->
 * creates an N_exprs chain containing n N_spid temp vars.
 *
 * @param n number of var to create
 * @return EXPRS chain of N_spid nodes
 *****************************************************************************/
static node *
CreateDotVarChain (size_t n)
{
    node *res = NULL;
    DBUG_ENTER ();

    while (n>0) {
        res = TBmakeExprs (MakeTmpId ("sdot"), res);
        n--;
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * duplicates an N_exprs chain of N_spid nodes ommitting any N_dot nodes found
 * in between the N_spid nodes.
 *
 * @param idxs EXPRS chain of N_spid/N-dot nodes
 * @return EXPRS chain of N_spid nodes
 *****************************************************************************/
static node *
StripDots (node *didxs)
{
    node *res = NULL;
    DBUG_ENTER ();
    if (didxs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (didxs)) == N_spid) {
            res = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (didxs)),
                               StripDots (EXPRS_NEXT (didxs)));
        } else {
            res = StripDots (EXPRS_NEXT (didxs));
        }
    }

    DBUG_RETURN (res);
}


/** <!--********************************************************************-->
 * creates an N_exprs chain of N_num(val) nodes.
 *
 * @param n number of vals needed.
 * @param val value to ba used.
 * @return EXPRS chain of N_num nodes
 *****************************************************************************/
static node *
CreateConstChain (size_t n, int val)
{
    node *res = NULL;
    DBUG_ENTER ();

    while (n>0) {
        res = TBmakeExprs (TBmakeNum (val), res);
        n--;
    }
    DBUG_RETURN (res);
}


/** <!--********************************************************************-->
 * creates an N_exprs chain of selections. It selects nleft elements 
 * from the left of expr and nright elements from the right side.
 *
 * @param nleft number of elements from the left
 * @param nright number of elements from the right
 * @param expr expression that the values are to be taken from
 * @return EXPRS chain of selections
 *****************************************************************************/
static node *
CreateSelChain (size_t nleft, size_t nright, node* expr, size_t pos)
{
    node *res = NULL;
    DBUG_ENTER ();

    if (nleft > 0) {
        res = TBmakeExprs (TCmakePrf2 (F_sel_VxA,
                                       TCcreateIntVector (1,(int)pos,1),
                                       DUPdoDupTree (expr)),
                           CreateSelChain (nleft-1, nright, expr, pos+1));
    } else if (nright > 0) {
        res = TBmakeExprs (TCmakePrf2 (F_sel_VxA,
                                       TCmakePrf2 (F_sub_VxS,
                                                   TCmakePrf1 (
                                                       F_shape_A,
                                                       DUPdoDupTree (expr)),
                                                   TBmakeNum ((int)nright)),
                                       DUPdoDupTree (expr)),
                           CreateSelChain (nleft, nright-1, expr, pos));
    }
    DBUG_RETURN (res);
}


/** <!--********************************************************************-->
 * Simple construction function. Given an expression expr and an integer 
 * number n, it generates the following AST:
 *
 *      [ dim (expr) - n]
 *
 * @param nleft number of elements from the left
 * @param nright number of elements from the right
 * @param expr expression that the values are to be taken from
 * @return EXPRS chain of selections
 *****************************************************************************/
static node *
CreateTdotShape (node *expr, size_t n)
{
    node *res = NULL;
    DBUG_ENTER ();

    res = TCmakeIntVector (
            TBmakeExprs ( TCmakePrf2 ( F_sub_SxS,
                                       TCmakePrf1 ( F_dim_A,
                                                    DUPdoDupTree (expr)),
                                       TBmakeNum ((int)n)),
                          NULL));

    DBUG_RETURN (res);
}


/** <!--********************************************************************-->
 * ensures the result will be an n-element vector as N_array node or an N_dot.
 *     <expr>       is turned into        [<expr>[0], ...<expr>[n-1]]
 * Consumes its argument!
 *
 * @param vector value to be used.
 * @param n length of the vector
 * @return potentially modified vector
 *****************************************************************************/
static node *
Scalarize (node *vector, size_t n)
{
    node *res;
    DBUG_ENTER ();
    if ((NODE_TYPE (vector) == N_array)|| (NODE_TYPE (vector) == N_dot)) {
        res = vector;
    } else {
        res = TCmakeIntVector (CreateSelChain (n, 0, vector, 0));
        vector = FREEdoFreeTree (vector);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * This function generates the splitting of the index var which is needed for
 * the resolution of ...-symbols within the generator. It generates an
 * N_assign chain of the form
 *
 * exprl_0 = tmp[pos];
 * ...
 * exprl_m = tmp[pos+m];
 * tvar = drop( m+1, drop (-(n+1), tmp));
 * exprr_0 = tmp[shape(tmp)-(n+1)];
 * ...
 * exprr_n = tmp[shape(tmp)-1];
 *
 * @param exprl EXPRS chain of N_spid nodes before the ...-entry
 * @param tvar N_spid node of the name of the ...-equivalent vector
 * @param exprr EXPRS chain of N_spid nodes after the ...-entry
 * @param tmp N_spid node of the name of the new generator variable
 * @param pos running position, initially to be called with 0
 * @return EXPRS chain of N_spid nodes
 *****************************************************************************/
static node *
MakeTdotAssigns (node *exprl, node *tvar, node *exprr, node *tmp, size_t pos)
{
    node *assign = NULL;
    size_t pos2;
    DBUG_ENTER ();
    DBUG_ASSERT (((exprl == NULL) || (NODE_TYPE (exprl) == N_exprs)),
                 "N_exprs chain expected for exprl");
    DBUG_ASSERT (((tvar == NULL) || (NODE_TYPE (tvar) == N_spid)),
                 "N_spid expected for tvar");
    DBUG_ASSERT (((exprr == NULL) || (NODE_TYPE (exprr) == N_exprs)),
                 "N_exprs chain expected for exprr");
    if (exprl != NULL) {
        /*
         * construct      exprl = tmp[pos];
         */
        assign = TBmakeAssign (
                   TBmakeLet (
                     TBmakeSpids (STRcpy (SPID_NAME (EXPRS_EXPR (exprl))),
                                  NULL),
                     TCmakePrf2 (F_sel_VxA,
                                 TCcreateIntVector (1,(int)pos,1),
                                 DUPdoDupTree (tmp))),
                   MakeTdotAssigns (EXPRS_NEXT (exprl), tvar, exprr, tmp, pos+1));
    } else if (tvar != NULL) {
        /*
         * construct     tvar = drop( pos, drop( -len(exprr), tmp));
         */
        pos2 = TCcountExprs (exprr);
        assign = TBmakeAssign (
                   TBmakeLet (
                     TBmakeSpids (STRcpy (SPID_NAME (tvar)),
                                  NULL),
                     TCmakePrf2 (F_drop_SxV,
                                 TBmakeNum ((int)pos),
                                 TCmakePrf2 (F_drop_SxV,
                                             TBmakeNum (-(int)pos2),
                                             DUPdoDupTree (tmp)))),
                   MakeTdotAssigns (exprl, NULL, exprr, tmp, pos2));
    } else if (exprr != NULL) {
        /*
         * construct     exprr = tmp[shape(tmp)-pos];
         */
        assign = TBmakeAssign (
                   TBmakeLet (
                     TBmakeSpids (STRcpy (SPID_NAME (EXPRS_EXPR (exprr))),
                                  NULL),
                     TCmakePrf2 (F_sel_VxA,
                                 TCmakePrf2 (F_sub_VxS,
                                             TCmakePrf1 (F_shape_A,
                                                         DUPdoDupTree (tmp)),
                                             TBmakeNum ((int)pos)),
                                 DUPdoDupTree (tmp))),
                   MakeTdotAssigns (exprl, tvar, EXPRS_NEXT (exprr), tmp, pos-1));
    }
    DBUG_RETURN (assign);
}


/** <!--********************************************************************-->
 * wraps an N_exprs chain either in an N_array (in case no triple-dot)
 * or splits the N_exprs chain at position pos_tdot into left and right 
 * and creates a * concatenation (left ++ (tdot_vec ++ right))
 * NB: this function consumes all its arguments!
 *
 * @param exprs chain
 * @param num_tdot 0/1 value indicating whether triple-dots need to be injected
 * @param pos_tdot indicates after how many exprs tdot_vec is to be injected
 * @param tdot_vec vector to be injected at the triple dot position
 * @return expression that represents the vector
 *****************************************************************************/
static node *
Exprs2expr (node *exprs, size_t num_tdot, size_t pos_tdot, node *tdot_vec)
{
    node *vec;
    node *left, *right;
    DBUG_ENTER ();
    if (num_tdot == 0) {
        vec = TCmakeIntVector (exprs);
    } else {
        // insert t_dot_vec as concatenation of arrays
        if (pos_tdot == 0) {
            right = exprs;
            exprs = NULL;
        } else {
            left = TCgetNthExprs (pos_tdot-1, exprs);
            right = EXPRS_NEXT (left);
            EXPRS_NEXT (left) = NULL;
        }
        left = TCmakeIntVector (exprs);
        right = TCmakeIntVector (right);
        vec = TCmakePrf2 (F_cat_VxV,
                          left,
                          TCmakePrf2 (F_cat_VxV,
                                      tdot_vec,
                                      right));
    }
    DBUG_RETURN (vec);
}


/** <!--********************************************************************-->
 * analyses the setWL vector and sets the following arg_info values:
 * bool INFO_HAS_DOTS (arg_info)
 * bool INFO_DIDXS (arg_info)
 * bool INFO_LM (arg_info)               [only set if INFO_HAS_DOTS is TRUE]
 * bool INFO_RM (arg_info)               [only set if INFO_HAS_DOTS is TRUE]
 * bool INFO_LN (arg_info)               [only set if INFO_HAS_DOTS is TRUE]
 * bool INFO_RM (arg_info)               [only set if INFO_HAS_DOTS is TRUE]
 * bool INFO_K (arg_info)                [only set if INFO_HAS_DOTS is TRUE]
 *
 * @param vec the vector of the setWL to be analysed
 * @return the modified arg_info
 *****************************************************************************/
static info *
AnalyseVec (node *vec, info *arg_info)
{
    bool before = TRUE;
    DBUG_ENTER ();

    INFO_HSED_HAS_DOTS (arg_info) = FALSE;
    INFO_HSED_DIDXS (arg_info) = vec;
    if (NODE_TYPE (vec) == N_exprs) {
        while (vec != NULL) {
            if (NODE_TYPE (EXPRS_EXPR (vec)) == N_spid) {
                if (before) {
                    INFO_HSED_LM (arg_info) ++;
                } else {
                    INFO_HSED_RM (arg_info) ++;
                }
            } else {
                DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (vec)) == N_dot),
                             "illegal set-WL vector!");
                INFO_HSED_HAS_DOTS (arg_info) = TRUE;
                if (DOT_NUM (EXPRS_EXPR (vec)) == 1) {
                    if (before) {
                        INFO_HSED_LN (arg_info) ++;
                    } else {
                        INFO_HSED_RN (arg_info) ++;
                    }
                } else {
                    if (before) {
                        INFO_HSED_K (arg_info) = 1;
                        before = FALSE;
                    } else {
                        CTIerrorLine (global.linenum,
                                      "only one \"...\" per set-expression"
                                      " admissable");
                    }
                }
            }
            vec = EXPRS_NEXT (vec);
        }
    }
    DBUG_PRINT ("Vec analyses yields: lm = %zu, ln = %zu | k = %zu | rm = %zu, rn = %zu\n",
                INFO_HSED_LM (arg_info), INFO_HSED_LN (arg_info), INFO_HSED_K (arg_info),
                INFO_HSED_RM (arg_info), INFO_HSED_RN (arg_info));

    DBUG_RETURN (arg_info);
}


/** <!--********************************************************************-->
 * helper function to recursively implememt the merging of e_vals and s_vals.
 * triple-dots are skipped here as they do not match the recursion scheme.
 *
 * @param didxs index-vector from set-WL
 * @param e_vals standard values
 * @param s_vals single-dot replacements
 * @return expr-list of merged results
 *****************************************************************************/
static node *
RecMergeIn (node *didxs, node *e_vals, node *s_vals, size_t pos, size_t *tdot_pos)
{
    node *res = NULL;
    DBUG_ENTER ();

    if (didxs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (didxs)) != N_dot) {
            res = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (e_vals)),
                               RecMergeIn (EXPRS_NEXT (didxs),
                                           EXPRS_NEXT (e_vals),
                                           s_vals,
                                           pos+1, tdot_pos));
        } else {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (didxs)) == N_dot),
                         "neither N_dot nor N_spid in didxs of MergeIn");
            if (DOT_NUM (EXPRS_EXPR (didxs)) == 1) {
                res = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (s_vals)),
                                   RecMergeIn (EXPRS_NEXT (didxs),
                                               e_vals,
                                               EXPRS_NEXT (s_vals),
                                               pos+1, tdot_pos));
            } else {
                // skip triple dot here but memo pos in tdot_pos!
                *tdot_pos = pos;
                res = RecMergeIn (EXPRS_NEXT (didxs),
                                  e_vals,
                                  s_vals,
                                  pos+1, tdot_pos);
            }
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 * Implements the MergeIn function described in sacdoc/bnf/desugaring.tex
 * It takes the index-vector that contains N_dot symbols as a blueprint
 * to merge the entries from 3 lists: one for existing values e_vals,
 * one for single-dot replacements s_vals, and one for the value that
 * needs to replace a potentially contained triple-dot.
 * NB: this function only inspects all its arguments but does NOT
 *     consume them!
 *
 * @param didxs index-vector from set-WL
 * @param e_vals standard values
 * @param s_vals single-dot replacements
 * @param t_val triple-dot replacement
 * @return merged list of copies
 *****************************************************************************/
static node *
MergeIn (node *didxs, node *e_vals, node *s_vals, node *t_val)
{
    node *res = NULL;
    size_t m,n,k;
    size_t t_pos;

    DBUG_ENTER ();
    DBUG_PRINT_TAG ("HSED_MERGE", "MergeIn called on:");
    DBUG_EXECUTE_TAG ("HSED_MERGE", (PRTdoPrintFile (stderr, didxs),
                                    (e_vals != NULL? PRTdoPrintFile (stderr, e_vals):0),
                                    (s_vals != NULL? PRTdoPrintFile (stderr, s_vals):0),
                                    (t_val != NULL? PRTdoPrintFile (stderr, t_val):0)));
    DBUG_ASSERT (NODE_TYPE (didxs) == N_exprs,
                "exprs-chain of Spid nodes expected as didxs param to MergeIn");
    DBUG_ASSERT ((e_vals == NULL)
                 || (NODE_TYPE (e_vals) == N_exprs)
                 || (NODE_TYPE (e_vals) == N_dot),
                "potentially empty exprs-chain of expression nodes or N_dot "
                "expected as e_val param to MergeIn");
    DBUG_ASSERT ((s_vals == NULL) || (NODE_TYPE (s_vals) == N_exprs),
                "exprs-chain of expression nodes expected as s_val param "
                "to MergeIn");

    if ((e_vals != NULL)
        && (NODE_TYPE (e_vals) == N_dot)
        && (DOT_NUM (e_vals) == 1)) {
        res = TBmakeDot (1);
    } else {
        m = TCcountExprs (e_vals);
        n = TCcountExprs (s_vals);
        k = (t_val == NULL ? 0 : 1);
        DBUG_ASSERT (TCcountExprs (didxs) == m+n+k,
                     "length of dotted generator variables expected to match the"
                     " number of expressions to be merged, ie. we should have"
                     " %zu = %zu+%zu+%zu", TCcountExprs (didxs), m, n, k);
        res = RecMergeIn( didxs, e_vals, s_vals, 0, &t_pos);
        res = Exprs2expr (res, k, t_pos, DUPdoDupTree (t_val));
    }
    
    DBUG_PRINT_TAG ("HSED_MERGE", "MergeIn result:");
    DBUG_EXECUTE_TAG ("HSED_MERGE", PRTdoPrintFile (stderr, res));
    DBUG_RETURN (res);
}




/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HSEDdoEliminateSetExpressionDots (node *arg_node)
{
    DBUG_ENTER ();

    TRAVpush (TR_hsed);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

/**
 * This hook is only needed to ensure N_generator traversals will see 
 * an arg_info where INFO_HSED_HAS_DOTS (arg_info) is FALSE if the
 * generator lives in a WL rather than a setWL!
 * The relevant case is where a WL lives inside a setWL!
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSEDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MakeInfo (arg_info); //stack arg_info!

    arg_node = TRAVcont (arg_node, arg_info);

    arg_info = FreeInfo (arg_info); //pop arg_info!

    DBUG_RETURN (arg_node);
}

/**
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSEDgenerator (node *arg_node, info *arg_info)
{
    node *sdot_vals,*tdot_vals;
    node *nval, *shape_z;
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_HSED_HAS_DOTS (arg_info)) {
        DBUG_PRINT ("adjusting lower bound...");
        GENERATOR_BOUND1 (arg_node) = Scalarize (GENERATOR_BOUND1 (arg_node),
                                                 INFO_HSED_LM (arg_info)
                                                 +INFO_HSED_RM (arg_info));
        sdot_vals = CreateConstChain (INFO_HSED_LN (arg_info)
                                      +INFO_HSED_RN (arg_info), 0);
        tdot_vals = SEUTbuildSimpleWl (
                      CreateTdotShape (INFO_HSED_ZEXPR (arg_info),
                                       INFO_HSED_LN (arg_info)
                                       +INFO_HSED_RN (arg_info)),
                      TBmakeNum (0));
        nval = MergeIn (INFO_HSED_DIDXS (arg_info),
                        (NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_dot?
                         GENERATOR_BOUND1 (arg_node) :
                         ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node))),
                        sdot_vals,
                        (INFO_HSED_K (arg_info) == 0 ? NULL : tdot_vals));
        sdot_vals = (sdot_vals != NULL ? FREEdoFreeTree (sdot_vals) : NULL);
        tdot_vals = FREEdoFreeTree (tdot_vals);
        GENERATOR_BOUND1 (arg_node) = FREEdoFreeTree (GENERATOR_BOUND1 (arg_node));
        GENERATOR_BOUND1 (arg_node) = nval;
        DBUG_PRINT ("new lower bound is:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, nval));

        DBUG_PRINT ("adjusting upper bound...");
        GENERATOR_BOUND2 (arg_node) = Scalarize (GENERATOR_BOUND2 (arg_node),
                                                 INFO_HSED_LM (arg_info)
                                                 +INFO_HSED_RM (arg_info));
        shape_z = TCmakePrf1 (F_shape_A,
                              INFO_HSED_ZEXPR (arg_info));
        sdot_vals = CreateSelChain (INFO_HSED_LN (arg_info),
                                    INFO_HSED_RN (arg_info),
                                    shape_z, 0);
        tdot_vals = TCmakePrf2 (F_drop_SxV,
                                TBmakeNum ((int)INFO_HSED_LN (arg_info)),
                                TCmakePrf2 (F_drop_SxV,
                                            TBmakeNum (-(int)INFO_HSED_RN (arg_info)),
                                            shape_z));
        nval = MergeIn (INFO_HSED_DIDXS (arg_info),
                        (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_dot?
                         GENERATOR_BOUND2 (arg_node) :
                         ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node))),
                        sdot_vals, 
                        (INFO_HSED_K (arg_info) == 0 ? NULL : tdot_vals));
        sdot_vals = (sdot_vals != NULL ? FREEdoFreeTree (sdot_vals) : NULL);
        tdot_vals = FREEdoFreeTree (tdot_vals);
        GENERATOR_BOUND2 (arg_node) = FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
        GENERATOR_BOUND2 (arg_node) = nval;
        DBUG_PRINT ("new upper bound is:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, nval));

        if (GENERATOR_STEP (arg_node) != NULL) {
            DBUG_PRINT ("adjusting step value...");
            GENERATOR_STEP (arg_node) = Scalarize (GENERATOR_STEP (arg_node),
                                                   INFO_HSED_LM (arg_info)
                                                   +INFO_HSED_RM (arg_info));
            sdot_vals = CreateConstChain (INFO_HSED_LN (arg_info)
                                          +INFO_HSED_RN (arg_info), 1);
            tdot_vals = SEUTbuildSimpleWl (
                          CreateTdotShape (INFO_HSED_ZEXPR (arg_info),
                                           INFO_HSED_LN (arg_info)
                                           +INFO_HSED_RN (arg_info)),
                          TBmakeNum (1));
            nval = MergeIn (INFO_HSED_DIDXS (arg_info),
                            GENERATOR_STEP (arg_node),
                            sdot_vals, 
                            (INFO_HSED_K (arg_info) == 0 ? NULL : tdot_vals));
            sdot_vals = (sdot_vals != NULL ? FREEdoFreeTree (sdot_vals) : NULL);
            tdot_vals = FREEdoFreeTree (tdot_vals);
            GENERATOR_STEP (arg_node) = FREEdoFreeTree (GENERATOR_STEP (arg_node));
            GENERATOR_STEP (arg_node) = nval;
            DBUG_PRINT ("new step value is:");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, nval));
        }

        if (GENERATOR_WIDTH (arg_node) != NULL) {
            DBUG_PRINT ("adjusting width value...");
            GENERATOR_WIDTH (arg_node) = Scalarize (GENERATOR_WIDTH (arg_node),
                                                   INFO_HSED_LM (arg_info)
                                                   +INFO_HSED_RM (arg_info));
            sdot_vals = CreateConstChain (INFO_HSED_LN (arg_info)
                                          +INFO_HSED_RN (arg_info), 1);
            tdot_vals = SEUTbuildSimpleWl (
                          CreateTdotShape (INFO_HSED_ZEXPR (arg_info),
                                           INFO_HSED_LN (arg_info)
                                           +INFO_HSED_RN (arg_info)),
                          TBmakeNum (1));
            nval = MergeIn (INFO_HSED_DIDXS (arg_info),
                            GENERATOR_WIDTH (arg_node),
                            sdot_vals, 
                            (INFO_HSED_K (arg_info) == 0 ? NULL : tdot_vals));
            sdot_vals = (sdot_vals != NULL ? FREEdoFreeTree (sdot_vals) : NULL);
            tdot_vals = FREEdoFreeTree (tdot_vals);
            GENERATOR_WIDTH (arg_node) = FREEdoFreeTree (GENERATOR_WIDTH (arg_node));
            GENERATOR_WIDTH (arg_node) = nval;
            DBUG_PRINT ("new width value is:");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, nval));
        }
    }
    
    DBUG_RETURN (arg_node);
}

/**
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSEDsetwl (node *arg_node, info *arg_info)
{
    node *sdot_exprs, *sdot_idxs, *tdot_idx;
    node *idxs, *nidxs;
    node *exprl, *exprr, *tvar;
    node *assigns;
    bool match;
    pattern *pat;
    DBUG_ENTER ();

    arg_info = MakeInfo (arg_info); //stack arg_info!

    DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);
    /* Bottom up traversal! */
    SETWL_NEXT (arg_node) = TRAVopt (SETWL_NEXT (arg_node), arg_info);
    SETWL_EXPR (arg_node) = TRAVopt (SETWL_EXPR (arg_node), arg_info);

    /*
     * set HAS_DOTS, DIDXS, LM, RM, LN, RN, K in arg_info:
     */
    arg_info = AnalyseVec (SETWL_VEC (arg_node), arg_info);

    /*
     * set ZEXPR iff HAS_DOTS
     */
    if (INFO_HSED_HAS_DOTS (arg_info)) {
        INFO_HSED_ZEXPR (arg_info) =
          SEUTsubstituteIdxs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                              SETWL_VEC (arg_node),
                              TBmakeNum (0));
        DBUG_PRINT ("zexpr generated:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_HSED_ZEXPR (arg_info)));
    }

    DBUG_PRINT ("processing generator...");
    SETWL_GENERATOR (arg_node) = TRAVopt (SETWL_GENERATOR (arg_node), arg_info);

    if (INFO_HSED_HAS_DOTS (arg_info)) {
        DBUG_PRINT( "adjusting expr ... ");
        sdot_exprs = CreateDotVarChain ( INFO_HSED_LN (arg_info)
                                         + INFO_HSED_RN (arg_info));
        tdot_idx = (INFO_HSED_K (arg_info) == 1 ?
                    MakeTmpId ("tmpv")
                    : NULL);
        sdot_idxs = Exprs2expr (sdot_exprs,
                                INFO_HSED_K (arg_info),
                                INFO_HSED_LN (arg_info),
                                tdot_idx);
        DBUG_PRINT ("selection index is:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, sdot_idxs));
        SETWL_EXPR (arg_node) =TCmakeSpap2 (NSgetNamespace (global.preludename),
                                            STRcpy ("sel"),
                                            sdot_idxs,
                                            SETWL_EXPR (arg_node));
        /*
         * Finally, we adjust SETWL_VEC!
         */
        DBUG_PRINT_TAG ("HSED_STRIP", "stripping didxs:");
        DBUG_EXECUTE_TAG ("HSED_STRIP", PRTdoPrintFile (stderr, SETWL_VEC (arg_node)));
        idxs = StripDots (SETWL_VEC (arg_node));
        DBUG_PRINT_TAG ("HSED_STRIP", "into idxs:");
        DBUG_EXECUTE_TAG ("HSED_STRIP", PRTdoPrintFile (stderr, idxs));

        nidxs = MergeIn (SETWL_VEC (arg_node), idxs, sdot_exprs, tdot_idx);
        DBUG_PRINT ("nidxs of setWL is:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, nidxs));

        if (INFO_HSED_K (arg_info) == 1) {
            pat = PMprf (1, PMAisPrf (F_cat_VxV),
                         2, PMarray (0,
                                     1, PMskip (1, PMAgetNode (&exprl),0)),
                            PMprf (1, PMAisPrf (F_cat_VxV),
                                   2, PMany( 1, PMAgetNode( &tvar),0 ),
                                      PMarray (0,
                                               1, PMskip (1, PMAgetNode (&exprr)))));
            match = PMmatchExact (pat, nidxs);
            pat = PMfree (pat);
            DBUG_ASSERT (match, "built nidxs does not match the expacted pattern");
            SETWL_VEC (arg_node) = FREEdoFreeTree (SETWL_VEC (arg_node));
            SETWL_VEC (arg_node) = MakeTmpId ("tmp");
            assigns = MakeTdotAssigns (exprl, tvar, exprr, SETWL_VEC (arg_node), 0);
            SETWL_ASSIGNS (arg_node) = assigns;
            DBUG_PRINT ("Assignments inserted:");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, assigns));
            nidxs = FREEdoFreeTree (nidxs);
        } else {
            SETWL_VEC (arg_node) = FREEdoFreeTree (SETWL_VEC (arg_node));
            SETWL_VEC (arg_node) = ARRAY_AELEMS (nidxs);
            ARRAY_AELEMS (nidxs) = NULL;
            nidxs = FREEdoFreeNode (nidxs);
        }
        idxs = FREEdoFreeTree (idxs);
        //NB: sdot_idxs is absorbed by the selection! no freeing here!
        //NB: tdot_idx is absorbed by the Exprs2expr! no freeing here!
    }
    arg_info = FreeInfo (arg_info); //pop arg_info!

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
