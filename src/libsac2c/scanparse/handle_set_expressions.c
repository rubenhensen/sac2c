#include "handle_set_expressions.h"
#include "set_expression_utils.h"
#include "traverse.h"

#define DBUG_PREFIX "HSE"
#include "debug.h"

#include "print.h"
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
#include "pattern_match.h"
#include "compare_tree.h"

#include <strings.h>

/**
 * @file handle_set_expressions.c
 *
 *
 * This phase is finally transforming all tensor comprehension
 * (aka set expressions) into proper WLs. It assumes that
 *  1) all dot-expressions have been replaced (HSED phase), and
 *  2) that all multi operator TCs have been identified and all
 *     SETWL_EXPR expressions have been standardised into an N_exprs
 *     chain of N_spid nodes potentially preceded by an N_assign
 *     chain in SETWL_ASSIGNS (MOSE phase).
 *
 * This phase then translates a TC of the form:
 *
 *  {  idxs1 -> let
 *                 v1_1, ..., v1_m = expr1;
 *              in (v1_1, ..., v1_m)
 *     | lb1 <= idxs1 < ub1;
 *   ...
 *     idxsn -> let
 *                 vn_1, ..., vn_m = exprn;
 *              in (vn_1, ..., vn_m)
 *     | lbn <= idxsn < ubn; }
 *
 *
 *   into:
 *
 *
 *   vn_1, ..., vn_n = SUBST(i, 0, SUBST(iv, 0*ubn, exprn));
 *
 *  with {
 *    (lb1 <= idxs1 < ub1) {
 *            v1_1, ..., v1_m = expr1;
 *        } : (v1_1, ..., v1_m);
 *  ...
 *    (lbn <= idxsn < ubn) {
 *            vn_1, ..., vn_m = exprn;
 *        } : (vn_1, ..., vn_m);
 *  } : (genarray (ub, with { } : genarray (shape(vn_1), zero (vn_1))),
 *       ...,
 *       genarray (ub, with { } : genarray (shape(vn_n), zero (vn_n))));
 *
 * where i and iv stand as representatives for scalar or vector indices
 * within idxsn.
 *
 * We "optimise" 2 special cases:
 * 
 * 1) if we have a last partition which
 *    a) is full (ie ranges from 0*ubn to ubn without steps / width), and
 *    b) has an exprn that does not refer to idxsn or any of its components
 *    THEN we omit the last partition as the default of the generator will do.
 * 2) if we have a last partition which
 *    a) is full (ie ranges from 0*ubn to ubn without steps / width), and
 *    b) has an exprn that is identical to a[idxsn],
 *    THEN we create a withop of the form:
 *       modarray(a)
 *    Note here, that this cannot happen for multi-operator TCs as we cannot 
 *    have multiple expressions within the expression part of the TC!
 *
 *
 *
 * Implementation:
 * ---------------
 *   Most of the implementation is rather straight forward. We traverse 
 *   through the AST mainly looking for N_setwl nodes. However, since we
 *   have to lift expressions out of TCs but are so early that we are 
 *   neither SSA nor flattened, this requires some special attention.
 *   In essence, we achieve this by INFO_HSE_LASTASSIGN. We carefully
 *   try to make sure this always points the N_assign node whose 
 *   ASSIGN_STMT we are currently traversing. When we come back
 *   to the N_assign, we replace it by whatever INFO_HSE_LASTASSIGN
 *   points to. As we traverse bottom-up through N_assign chains
 *   this approach actually works.
 *   There are a few exceptions, where we have to make sure things
 *   are lifted into the correct places. 
 *   1) within TCs, we have SETWL_ASSIGN and SETWL_EXPR. Anything 
 *      we lift from SETWL_EXPR actually needs to go to the end of 
 *      the N_assign chain in SETWL_ASSIGN. Therefore, we set
 *      INFO_HSE_LASTASSIGN to NULL before traversing SETWL_EXPR
 *      and append INFO_HSE_LASTASSIGN to SETWL_ASSIGN thereafter.
 *  2) we have exactly the same situation when dealing with N_code
 *     blocks.
 *  3) we also have a rather special situation when traversing N_genera
 *     nodes. As we are going to lift the upper bound of the last
 *     partition out of the current TC, we need to make sure that 
 *     all TCs within the upper bound that require a lifting by
 *     themselves are being place BEFORE those of the main TC.
 *     An example where this matters is:
 *         a,b = { [.,i,...] -> { [...,j] -> foo(i,j)
 *                                | [j] < {[i] -> 5 |[i] <[1] } }
 *                 | [i] < [10] };
 *    To achieve this, we use a similar trick as in cases 1) and 2):
 *    we set INFO_HSE_LASTASSIGN to NULL prior to traversing an
 *    generator and if something is being lifted during that 
 *    traversal, we store it in pot_generator_lifts (local var 
 *    in HSEsetwl) and perform the actual injection of the lifted
 *    assignments at the very end.
 *
 * F
 */

/* INFO structure */
struct INFO {
    bool outside;
    bool lastpart;
    bool genref;
    bool fullpart;
    node *vec;
    node *copy_from;
    node *assigns;
    node *part;
    node *code;
    node *withop;
    struct INFO *next;
};

/* access macros */
#define INFO_HSE_OUTSIDE(n) ((n)->outside)
#define INFO_HSE_LASTPART(n) ((n)->lastpart)
#define INFO_HSE_FULLPART(n) ((n)->fullpart)
#define INFO_HSE_GENREF(n) ((n)->genref)
#define INFO_HSE_VEC(n) ((n)->vec)
#define INFO_HSE_COPY_FROM(n) ((n)->copy_from)
#define INFO_HSE_LASTASSIGN(n) ((n)->assigns)
#define INFO_HSE_PART(n) ((n)->part)
#define INFO_HSE_CODE(n) ((n)->code)
#define INFO_HSE_WITHOP(n) ((n)->withop)
#define INFO_HSE_NEXT(n) ((n)->next)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (info * previous)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HSE_OUTSIDE (result) = TRUE;
    INFO_HSE_LASTPART (result) = FALSE;
    INFO_HSE_FULLPART (result) = FALSE;
    INFO_HSE_GENREF (result) = FALSE;
    INFO_HSE_VEC (result) = NULL;
    INFO_HSE_COPY_FROM (result) = NULL;
    INFO_HSE_LASTASSIGN (result) = NULL;
    INFO_HSE_PART (result) = NULL;
    INFO_HSE_CODE (result) = NULL;
    INFO_HSE_WITHOP (result) = NULL;
    INFO_HSE_NEXT (result) = previous;

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
    info *old;
    DBUG_ENTER ();

    old = INFO_HSE_NEXT (arg_info);
    arg_info = MEMfree (arg_info);

    DBUG_RETURN (old);
}


/**
 * builds a withloop generating an array of the same shape
 * as the shape of "expr" whose elements are all "zero( expr)" .
 * It is assumed that expr does NOT contain any references to
 * the generator variables!
 *
 * @param expr AST copy of the expression
 */
static node *
BuildDefault (node *expr)
{
    node *shape, *def;
    DBUG_ENTER ();

    DBUG_PRINT ("Building Default Element WL from expression:");
    DBUG_EXECUTE (PRTdoPrintFile (stderr, expr));

    shape = TCmakePrf1( F_shape_A, expr);
    def = TCmakeSpap1 (NSgetNamespace (global.preludename),
                       STRcpy ("zero"),
                       DUPdoDupTree (expr));
    DBUG_RETURN (SEUTbuildSimpleWl (shape, def));
}


/**
 * recursive helper function for converting an exprs-chain of N_spid
 * nodes into an N_spids-chain.
 *
 * @param exprs to be converted
 */
static node *
Exprs2Spids (node *exprs)
{
    node *res;
    if (exprs == NULL) {
        res = NULL;
    } else {
        res =  TBmakeSpids (STRcpy (SPID_NAME (EXPRS_EXPR (exprs))),
                            Exprs2Spids (EXPRS_NEXT (exprs)));
    }
    return res;
}

/**
 * converts the vector of a set-WL into an N_withid
 *
 * @param N_spid or N_exprs containing the idxvec or idxs
 * @return newly created N_withid node
 */
static node *
Idxs2Withid (node *idxs)
{
    node *result = NULL;

    DBUG_ENTER ();

    if (NODE_TYPE (idxs) == N_spid) {
        result = TBmakeWithid (TBmakeSpids (STRcpy (SPID_NAME (idxs)),
                                            NULL),
                               NULL);
    } else if (NODE_TYPE (idxs) == N_exprs) {
        result = TBmakeWithid (NULL,
                               Exprs2Spids (idxs));
    } else {
        CTIerror (LINE_TO_LOC (global.linenum), "illegal LHS in set expression!");
    }

    DBUG_RETURN (result);
}


/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HSEdoEliminateSetExpressions (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo (NULL);

    TRAVpush (TR_hse);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

/**
 * This is a local function to find out whether an expression
 * "s_expr" denotes the shape of another expression "expr".
 * It is needed to identify that a copy expression is used
 * across the ENTIRE shape of the array it is copied from.
 * We look for two pattern here.
 *
 * Pattern 1 (implemented in pat1) is 
 *    s_expr = _shape_ (expr)
 *
 * Pattern 2 is more intricate. here, we look for 
 *    s_expr = [ _sel_VxA_ ([0], _shape_ (expr)),
 *               _sel_VxA_ ([1], _shape_ (expr))...]
 *
 * Note for pattern 2, that we do not check how many components the array has.
 * This implies that we consider a prefix of the shape acceptable!
 */
static bool
IsShapeOf (node *s_expr, node *expr)
{
    bool result = FALSE;
    pattern *pat1, *pat2;
    node *val, *exprs;
    int pos;
    int one = 1;
    DBUG_ENTER ();

    DBUG_PRINT ("checking whether upper bound matches shape of copy expression");
    DBUG_PRINT ("upper bound is:");
    DBUG_EXECUTE (PRTdoPrintFile (stderr, s_expr));
    DBUG_PRINT ("array copied from is:");
    DBUG_EXECUTE (PRTdoPrintFile (stderr, expr));

    pat1 = PMprf (1, PMAisPrf (F_shape_A),
                  1, PMany( 1, PMAgetNode( &val), 0));
    pat2 = PMprf (1, PMAisPrf (F_sel_VxA),
                  2, PMarray ( 1, PMAhasLen (&one),
                               1, PMint( 1, PMAisIVal (&pos))),
                     pat1);

    if (PMmatchExact (pat1, s_expr)) {
        result =  (CMPTdoCompareTree (expr, val) == CMPT_EQ);
    } else if (NODE_TYPE (s_expr) == N_array) {
        exprs = ARRAY_AELEMS (s_expr);
        pos = 0;
        result = TRUE;
        while ((exprs != NULL) && (result == TRUE)) {
            result = (PMmatchExact (pat2, EXPRS_EXPR (exprs))
                      && (CMPTdoCompareTree (expr, val) == CMPT_EQ));
            exprs = EXPRS_NEXT (exprs);
            pos++;
        }
    }

    DBUG_PRINT ("upper bound does %s match shape of copy expression",
                (result ? "" : "NOT"));

    pat2 = PMfree (pat2);

    DBUG_RETURN (result);
}

/**
 * This function checks whether the generator indices "idxs" are 
 * used as indices in a given selection "array_expr"["idx_expr"].
 * We look for the following pattern:
 * Pattern 1:  idx_expr == idxs  (index vector case)
 * Pattern 2:  [idx_expr] == idxs (overloaded selection sel::int x int[+] -> int[*])
 * Pattern 3:  idx_expr == [i1,...,in] = idxs (scalar case)
 */
static node *
CheckCopy (node *idxs, node *idx_expr, node *array_expr)
{
    node * result = NULL;
    DBUG_ENTER ();

    if (!SEUTcontainsIdxs (array_expr, idxs)) {

        if ((NODE_TYPE (idxs) == N_spid) && (NODE_TYPE (idx_expr) == N_spid)
            && STReq (SPID_NAME (idxs), SPID_NAME (idx_expr))) {
            // pattern 1:
            result = array_expr;
        } else if (NODE_TYPE (idxs) == N_exprs) {
            if (NODE_TYPE (idx_expr) == N_spid) {
                if ((EXPRS_NEXT (idxs) == NULL)
                    && STReq (SPID_NAME (EXPRS_EXPR (idxs)),
                              SPID_NAME (idx_expr))) {
                    // pattern 2:
                    result = array_expr;
                }
            } else if (NODE_TYPE (idx_expr) == N_array) {
                // pattern 3:
                idx_expr = ARRAY_AELEMS (idx_expr);
                result = array_expr;
                while ((idxs != NULL) && (idx_expr != NULL)) {
                    if ((NODE_TYPE (EXPRS_EXPR( idx_expr)) == N_spid)
                        && STReq (SPID_NAME (EXPRS_EXPR (idxs)),
                                  SPID_NAME ( EXPRS_EXPR (idx_expr)))) {
                        idxs = EXPRS_NEXT (idxs);
                        idx_expr = EXPRS_NEXT (idx_expr);
                    } else {
                        result = NULL;
                        idxs = NULL;
                    }
                }
                if ((idxs != NULL) || (idx_expr != NULL)) {
                    result = NULL;
                }
            }
        }
    }

    DBUG_RETURN (result);
}

/**
 * 
 * checks whether the given expression "expr" is a selection
 * of the form  <arr-expr>[ iv ] or
 *              <arr-expr>[ i_1, ..., i_n]
 * where iv/i_x are identical to the indices given in "idxs".
 *
 * If found, and "idxs" are not free in <arr-expr>, <arr-expr>
 * is returned; otherwise, NULL is returned.
 *
 * @param assigns the body assignments
 * @param exprs the body term(s) to be analysed
 * @param idxs the index vector (spid, or exprs(spid))
 * @return <arr-expr> or NULL
 */
static node *
IsCopyBody (node *assigns, node *exprs, node *idxs)
{
    node *result = NULL;
    node *expr;
    char *var;
    DBUG_ENTER ();

    if ((exprs != NULL) && (TCcountExprs (exprs) == 1)) {
        expr = EXPRS_EXPR (exprs);
        DBUG_ASSERT (NODE_TYPE (expr) == N_spid,
                     "non identifier in SETWL_EXPR found!");
        var = SPID_NAME (expr);
        if ((ASSIGN_NEXT (assigns) == NULL) 
            && (NODE_TYPE (ASSIGN_STMT( assigns)) == N_let)
            && STReq (var, SPIDS_NAME (ASSIGN_LHS( assigns)))) {
            expr = ASSIGN_RHS( assigns);
            if ((NODE_TYPE (expr) == N_spap)
                && STReq (SPID_NAME (SPAP_ID (expr)), "sel")) {
                result = CheckCopy (idxs,
                                    EXPRS_EXPR1 (SPAP_ARGS (expr)),
                                    EXPRS_EXPR2 (SPAP_ARGS (expr)));
            } else if ((NODE_TYPE (expr) == N_prf)
                       && (PRF_PRF (expr) == F_sel_VxA)) {
                result = CheckCopy (idxs,
                                    EXPRS_EXPR1 (PRF_ARGS (expr)),
                                    EXPRS_EXPR2 (PRF_ARGS (expr)));
            }
        }
    }

    DBUG_RETURN (result);
}



static node *
BuildWithop (node *setwl, node *expr, info *arg_info)
{
    node *def, *shape;
    node *res;

    DBUG_ENTER ();

    if (INFO_HSE_FULLPART (arg_info) && !INFO_HSE_GENREF (arg_info)) {
        def = DUPdoDupTree (expr);
    } else {
        shape = TCmakePrf1( F_shape_A, DUPdoDupTree (expr));
        def = TCmakeSpap1 (NSgetNamespace (global.preludename),
                           STRcpy ("zero"),
                           DUPdoDupTree (expr));
        def = SEUTbuildSimpleWl (shape, def);
    }
    res = TBmakeGenarray
              (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (setwl))),
               def);

    DBUG_RETURN (res);
}

static node *
BuildWithops (node *setwl, node *expr, info *arg_info)
{
    node * res = NULL;
    node * rest = NULL;

    DBUG_ENTER ();

    if (expr != NULL) {
        DBUG_ASSERT ((NODE_TYPE (expr) == N_exprs),
                     "MOSE should have converted all SETWL_EXPR into N_exprs!");
        rest = BuildWithops (setwl, EXPRS_NEXT (expr), arg_info);
        res = BuildWithop (setwl, EXPRS_EXPR (expr), arg_info);
        L_WITHOP_NEXT (res, rest);
    }
    
    DBUG_RETURN (res);
}




node *
HSEassign (node *arg_node, info *arg_info)
{
    node * old_assign;
    DBUG_ENTER ();

    old_assign = INFO_HSE_LASTASSIGN (arg_info);

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_HSE_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    arg_node = INFO_HSE_LASTASSIGN (arg_info);

    INFO_HSE_LASTASSIGN (arg_info) = old_assign;
    
    DBUG_RETURN (arg_node);
}

node *
HSEcode (node *arg_node, info *arg_info)
{
    node *mem_lastassign;
    node *expr_assign;

    DBUG_ENTER ();

    mem_lastassign = INFO_HSE_LASTASSIGN (arg_info);

    INFO_HSE_LASTASSIGN (arg_info) = NULL;
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);

    expr_assign = INFO_HSE_LASTASSIGN (arg_info);
    INFO_HSE_LASTASSIGN (arg_info) = mem_lastassign;

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    if (expr_assign != NULL) {
        if (CODE_CBLOCK (arg_node) == NULL) {
            CODE_CBLOCK (arg_node) = TBmakeBlock (expr_assign, NULL);
        } else {
            BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
                = TCappendAssign ( BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)),
                                   expr_assign);
        }
    }

    // we do not traverse the next code block as we have a one-to-one relation
    // between parts and codes and we take one pair at a time.

    DBUG_RETURN (arg_node);
}

/**
 * traverse inner set-wls and infer INFO_HSE_FULLPART.
 *
 * FULLPART is true, iff one of the following pattern matches:
 * Pattern 1: ( . <= ? < ub)  (no step no width!)
 * Pattern 2: ( F_mul_SxV ( 0, ?) <= ? < ub) 
 * Pattern 3: ( [0,...,0] <= ? < ub) 
 *
 * In case the relational operator for the upper bound is <=, we modify
 * the upper bound into _add_VxA_(1, ub) and adjust the operator 
 * accordingly before checking for the full partition. This ensures
 * as well that the final partition will be translated correctly into 
 * a WL (see issue 2370). We have to exclude dot-symbols from this
 * transformation as that would produce illegal code. In case we 
 * meet a dot as upper bound, we only check for potential completeness
 * if the rel is <= !
 * While this also is being done in HWLD, the problem is that HWLD
 * does work on WLs only and its main purpose is to replace the 
 * dots; the standardisation of generator relations is "just a
 * by-product". In principle, we could move the entire standardisation
 * here, or even consider making it a stand-alone phase but that
 * seems like an overkill. Therefore, we only perform the adjustment
 * on the partitions of TCs here.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HSEgenerator (node *arg_node, info *arg_info)
{
    node *handle;
    node *lb;
    pattern *pat;
    int zero=0;
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);
    
    lb = GENERATOR_BOUND1 (arg_node);
    INFO_HSE_FULLPART (arg_info) = FALSE;

    pat = PMprf (1, PMAisPrf (F_mul_SxV),
                 2, PMint (1, PMAisIVal (&zero)),
                    PMany (0, 0));

    if ((GENERATOR_STEP (arg_node) == NULL)
        && (GENERATOR_WIDTH (arg_node) == NULL)
        && (GENERATOR_OP1 (arg_node) == F_wl_le)) {
        if (NODE_TYPE( lb) == N_dot) {
            // pattern 1:
            INFO_HSE_FULLPART (arg_info) = TRUE;
        } else if (PMmatchExact (pat, lb)) {
            // pattern 2:
            INFO_HSE_FULLPART (arg_info) = TRUE;
        } else if (NODE_TYPE( lb) == N_array) {
            // pattern 3:
            handle = ARRAY_AELEMS (lb);
            INFO_HSE_FULLPART (arg_info) = TRUE;
            while (handle != NULL) {
                if ((NODE_TYPE (EXPRS_EXPR (handle)) == N_num)
                    && (NUM_VAL (EXPRS_EXPR (handle)) == 0)) {
                    handle = EXPRS_NEXT (handle);
                } else {
                    INFO_HSE_FULLPART (arg_info) = FALSE;
                    handle = NULL;
                }
            }
        }
    }
    
    pat = PMfree (pat);

    if (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_dot) {
        if (GENERATOR_OP2 (arg_node) == F_wl_lt) {
            INFO_HSE_FULLPART (arg_info) = FALSE;
        }
    } else {
        if (GENERATOR_OP2 (arg_node) == F_wl_le) {
            GENERATOR_OP2 (arg_node) = F_wl_lt;
            GENERATOR_BOUND2 (arg_node)
                = TCmakePrf2 (F_add_SxV,
                              TBmakeNum (1),
                              GENERATOR_BOUND2 (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}


/**
 * This is the centrepiece of this traversal. It generates partitions 
 * bottom up and it tries to avoid a partition for the last set notation:
 * { iv -> a[iv] | . <= iv < shape(a) } turns into modarray(a), and
 * { iv -> const | . <= iv < ub } turns into genarray( up, const),
 * otherwise, { iv -> expr | lb <= iv < ub} turns into
 * with {
 *    (lb <= iv < ub) : expr;
 * } : genarray (ub, genarray (shape (expr'), zero (expr')));
 * 
 * where expr' = [expr]_{iv}^{0}
 *
 * For details see the HSE description in sacdoc/bnf/desugaring.tex
 * and the generic description at the top of this file.
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSEsetwl (node *arg_node, info *arg_info)
{
    node *code, *part, *subst, *mem_lastassign;
    node *pot_generator_lifts;
    info *oldinfo = NULL;
    DBUG_ENTER ();

    if (INFO_HSE_OUTSIDE (arg_info)) {
        DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);
        oldinfo = arg_info;
        arg_info = MakeInfo (oldinfo);
        INFO_HSE_LASTASSIGN (arg_info) = INFO_HSE_LASTASSIGN (oldinfo);
    } else {
        DBUG_PRINT ("next partition of Set-Expression in line %zu:", global.linenum);
        INFO_HSE_OUTSIDE (arg_info) = TRUE;
    }

    /**************************************************************************
     * set INFO_HSE_LASTPART and INFO_HSE_VEC:
     */
    INFO_HSE_LASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);
    INFO_HSE_VEC (arg_info) = SETWL_VEC (arg_node);

    /**************************************************************************
     * traverse the generator (may contain setwls!) 
     * This traversal infers INFO_HSE_FULLPART which is only valid
     * in case INFO_HSE_LASTPART is TRUE!
     */
    DBUG_PRINT ("traversing generator");

    mem_lastassign = INFO_HSE_LASTASSIGN (arg_info);
    INFO_HSE_LASTASSIGN (arg_info) = NULL;

    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);
    DBUG_PRINT ("generator is %s",
                (INFO_HSE_FULLPART (arg_info) ?
                 "full if this is the last partition." :
                 "not full."));

    pot_generator_lifts = INFO_HSE_LASTASSIGN (arg_info);
    INFO_HSE_LASTASSIGN (arg_info) = mem_lastassign;

    /**************************************************************************
     * traverse the expression (may contain setwls!)
     * This traversal also infers INFO_HSE_GENREF and INFO_HSE_COPY_FROM
     * in case INFO_HSE_LASTPART is TRUE!
     */
    DBUG_PRINT ("traversing expression");

    mem_lastassign = INFO_HSE_LASTASSIGN (arg_info);
    INFO_HSE_LASTASSIGN (arg_info) = NULL;

    SETWL_EXPR (arg_node) = TRAVopt (SETWL_EXPR (arg_node), arg_info);
    SETWL_ASSIGNS (arg_node) = TRAVopt (SETWL_ASSIGNS (arg_node), arg_info);

    if (INFO_HSE_LASTASSIGN (arg_info) != NULL) {
        SETWL_ASSIGNS (arg_node) = INFO_HSE_LASTASSIGN (arg_info);
    }
    INFO_HSE_LASTASSIGN (arg_info) = mem_lastassign;


    /**************************************************************************
     * set INFO_HSE_GENREF, INFO_HSE_COPY_FROM, and INFO_HSE_LASTPART:
     */
    INFO_HSE_GENREF (arg_info) = SEUTcontainsIdxs (SETWL_EXPR (arg_node),
                                                   SETWL_VEC (arg_node));
    if (SETWL_ASSIGNS (arg_node) != NULL) {
        INFO_HSE_GENREF (arg_info) = INFO_HSE_GENREF (arg_info)
                                     || SEUTcontainsIdxs (SETWL_ASSIGNS (arg_node),
                                                          SETWL_VEC (arg_node));
    }

    DBUG_PRINT( "generator variable %s in expression!",
                (INFO_HSE_GENREF (arg_info)? "found" : "not found"));
    INFO_HSE_COPY_FROM (arg_info) = IsCopyBody (SETWL_ASSIGNS (arg_node),
                                                SETWL_EXPR (arg_node),
                                                SETWL_VEC (arg_node));
    if ((INFO_HSE_COPY_FROM (arg_info) != NULL)
        && !IsShapeOf (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node)),
                       INFO_HSE_COPY_FROM (arg_info))) {
        INFO_HSE_COPY_FROM (arg_info) = NULL;
    }
    DBUG_PRINT( "expression is %sa copy partition!",
                (INFO_HSE_COPY_FROM (arg_info) == NULL ?  "NOT " : ""));
    

    /**************************************************************************
     * Finally, we build the WLs. We do so bottom up!
     */
    if (SETWL_NEXT (arg_node) != NULL) {
        INFO_HSE_OUTSIDE (arg_info) = FALSE;
        SETWL_NEXT (arg_node) = TRAVdo (SETWL_NEXT (arg_node), arg_info);
        INFO_HSE_LASTPART (arg_info) = FALSE;
    }

    /* Now, we add a new partition to arg_info, iff we are 
     *   not the last partition
     *   or, in case we are the last part, we have an incomplete partition,
     *   or, in case we are the last part and complete, we refer to the generator variable(s)
     *   and do so in a way different from a full (!) copying expression (ie a[iv])
     */
    if (!INFO_HSE_LASTPART (arg_info) || !INFO_HSE_FULLPART (arg_info)
         || (INFO_HSE_GENREF (arg_info) && (INFO_HSE_COPY_FROM (arg_info) == NULL))) {
        DBUG_PRINT ("building code and partition");

        code = TBmakeCode (((SETWL_ASSIGNS (arg_node) == NULL) ?
                            MAKE_EMPTY_BLOCK () :
                            TBmakeBlock (DUPdoDupTree (SETWL_ASSIGNS (arg_node)), NULL)),
                           DUPdoDupTree (SETWL_EXPR (arg_node)));
        CODE_USED (code)++;
        CODE_NEXT (code) = INFO_HSE_CODE (arg_info);
        INFO_HSE_CODE (arg_info) = code;

        part = TBmakePart (code,
                           Idxs2Withid (SETWL_VEC (arg_node)),
                           DUPdoDupTree (SETWL_GENERATOR (arg_node)));

        PART_NEXT (part) = INFO_HSE_PART (arg_info);
        INFO_HSE_PART (arg_info) = part;
        DBUG_PRINT ("partition inserted:");
        DBUG_EXECUTE (PRTdoPrintFile(stderr, part));
    }

    /* if we are the last partition, we generate the operator part now: */
    if (INFO_HSE_LASTPART (arg_info)) {
        DBUG_PRINT ("building withop");
        if (INFO_HSE_FULLPART (arg_info)
            && (INFO_HSE_COPY_FROM (arg_info) != NULL)) {
            /*
             * we can generate a modarray WL
             * (the last partition has not been generated!)
             */
            INFO_HSE_WITHOP (arg_info)
              = TBmakeModarray (DUPdoDupTree (INFO_HSE_COPY_FROM (arg_info)));
        } else {
            if (SETWL_EXPR (arg_node) != NULL) {
                if (NODE_TYPE (SETWL_VEC (arg_node)) == N_spid) {
                    subst = TCmakePrf2 (F_mul_SxV,
                                        TBmakeNum (0),
                                        DUPdoDupTree (
                                            GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))));
                } else {
                    subst = TBmakeNum (0);
                }

                INFO_HSE_LASTASSIGN (arg_info)
                    = TCappendAssign (SEUTsubstituteIdxs (DUPdoDupTree (SETWL_ASSIGNS (arg_node)),
                                                          SETWL_VEC (arg_node),
                                                          subst),
                                      INFO_HSE_LASTASSIGN (arg_info));
                DBUG_PRINT ("Injecting default expression assignment:");
                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, INFO_HSE_LASTASSIGN (arg_info)));
                subst = FREEdoFreeTree (subst);
            }

            INFO_HSE_WITHOP (arg_info)
                = BuildWithops (arg_node, SETWL_EXPR (arg_node), arg_info);

        }
        DBUG_PRINT ("withop inserted:");
        DBUG_EXECUTE ( if (INFO_HSE_WITHOP (arg_info) != NULL) {
                           PRTdoPrintFile (stderr, INFO_HSE_WITHOP (arg_info));
                       } else {
                           DBUG_PRINT ("void");
                       });
    }
    INFO_HSE_LASTASSIGN (arg_info)
        = TCappendAssign (pot_generator_lifts, INFO_HSE_LASTASSIGN (arg_info));

    arg_node = FREEdoFreeTree (arg_node);

    if (oldinfo != NULL) {
        arg_node = TBmakeWith (INFO_HSE_PART( arg_info), 
                               INFO_HSE_CODE( arg_info),
                               INFO_HSE_WITHOP( arg_info));
        INFO_HSE_LASTASSIGN (oldinfo) = INFO_HSE_LASTASSIGN (arg_info);
        arg_info = FreeInfo( arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
