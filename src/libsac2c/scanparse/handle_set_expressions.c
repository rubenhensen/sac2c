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
 */

/* INFO structure */
struct INFO {
    bool outside;
    bool lastpart;
    bool genref;
    bool fullpart;
    node *vec;
    node *copy_from;
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

static node*
ExtracIdsFromAssigns (node *assign)
{
    node *res = NULL;
    DBUG_ENTER ();

    if (assign != NULL) {
        if (STRprefix ("_", SPIDS_NAME (ASSIGN_LHS (assign)))) {
            res = ExtracIdsFromAssigns (ASSIGN_NEXT (assign));
        } else {
            res = TBmakeExprs (
                    TBmakeSpid ( NULL, STRcpy (SPIDS_NAME (ASSIGN_LHS (assign)))),
                    ExtracIdsFromAssigns (ASSIGN_NEXT (assign)));
              
        }
    }

    DBUG_RETURN (res);
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
 * @param expr the body term to be analysed
 * @param idxs the index vector (spid, or exprs(spid))
 * @return <arr-expr> or NULL
 */
static node *
IsCopyBody (node *expr, node *idxs)
{
    node *result = NULL;
    DBUG_ENTER ();

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

    DBUG_RETURN (result);
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
 * For details see the HSE description in sacdoc/bnf/desugaring.tex.
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSEsetwl (node *arg_node, info *arg_info)
{
    node *code, *part, *subst, *let_vars;
    info *oldinfo = NULL;
    DBUG_ENTER ();

    if (INFO_HSE_OUTSIDE (arg_info)) {
        DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);
        oldinfo = arg_info;
        arg_info = MakeInfo (oldinfo);
    } else {
        DBUG_PRINT ("next partition of Set-Expression in line %zu:", global.linenum);
        INFO_HSE_OUTSIDE (arg_info) = TRUE;
    }

    INFO_HSE_LASTPART (arg_info) = (SETWL_NEXT (arg_node) == NULL);
    INFO_HSE_VEC (arg_info) = SETWL_VEC (arg_node);

    /* traverse the generator (may contain setwls!) 
     * This traversal infers INFO_HSE_FULLPART which is only valid
     * in case INFO_HSE_LASTPART is TRUE!
     */
    DBUG_PRINT ("traversing generator");
    SETWL_GENERATOR (arg_node) = TRAVdo (SETWL_GENERATOR (arg_node), arg_info);
    DBUG_PRINT ("generator is %s",
                (INFO_HSE_FULLPART (arg_info) ?
                 "full if this is the last partition." :
                 "not full."));

    /* traverse the expression (may contain setwls!)
     * This traversal also infers INFO_HSE_GENREF and INFO_HSE_COPY_FROM
     * in case INFO_HSE_LASTPART is TRUE!
     */
    DBUG_PRINT ("traversing expression");
    SETWL_EXPR (arg_node) = TRAVdo (SETWL_EXPR (arg_node), arg_info);
    INFO_HSE_GENREF (arg_info) = SEUTcontainsIdxs (SETWL_EXPR (arg_node),
                                                   SETWL_VEC (arg_node));
    DBUG_PRINT( "generator variable %s in expression!",
                (INFO_HSE_GENREF (arg_info)? "found" : "not found"));
    INFO_HSE_COPY_FROM (arg_info) = IsCopyBody (SETWL_EXPR (arg_node),
                                                SETWL_VEC (arg_node));
    if ((INFO_HSE_COPY_FROM (arg_info) != NULL)
        && !IsShapeOf (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node)),
                       INFO_HSE_COPY_FROM (arg_info))) {
        INFO_HSE_COPY_FROM (arg_info) = NULL;
    }
    DBUG_PRINT( "expression is %sa copy partition!",
                (INFO_HSE_COPY_FROM (arg_info) == NULL ?  "NOT " : ""));
    
    /* We build the WLs bottom up! */
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
                           TBmakeExprs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                        NULL));
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
        } else if (INFO_HSE_FULLPART (arg_info) && !INFO_HSE_GENREF (arg_info)) {
            /*
             * As there is no reference to the generator and the partition is full,
             * we use the expression as default.
             * If the partition is NOT full, we have to generate a default default
             * (see next case :-)
             */
            INFO_HSE_WITHOP (arg_info)
              = TBmakeGenarray (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                                DUPdoDupTree (SETWL_EXPR (arg_node)) );
        } else {
            if (SETWL_ASSIGNS (arg_node) != NULL) {
                /*
                 * super special case! We know, that we had a ... in the original.
                 * Unfortunately, our zero'ing approach used in all other cases
                 * does not work here because we do not have a proper "let" with
                 * SETWL_ASSIGNS. As a consequence, all variables defined there
                 * are relatvely free in SETWL_EXPR (arg_node). Consequently,
                 * we cannot lift them out! Neither for the shape nor for the
                 * default!
                 * However, as we have ..., we do know that the cell shape is scalar!
                 * So all we really need is the base type, ie. a suitable argument to 
                 * the function zero.
                 * One might think that choosing SPAP_ARG2 (SETWL_EXPR (arg_node))
                 * would work, as this avoids referring to all variables defined
                 * in SETWL_ASSIGNS that stand for dot symbols in SETWL_VEC.
                 * Unfortunately, the original non-dot symbols can still appear.
                 * Since they are defined in SETWL_ASSIGNS as well, we need
                 * to identify these and then replace there occurrances in
                 * PRF_ARG2 (SETWL_EXPR (arg_node)) by zeros.
                 */
                let_vars = ExtracIdsFromAssigns (SETWL_ASSIGNS (arg_node));
                INFO_HSE_WITHOP (arg_info)
                   = TBmakeGenarray
                       (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                       TCmakeSpap1 (NSgetNamespace (global.preludename),
                                    STRcpy ("zero"),
                                    SEUTsubstituteIdxs (
                                      DUPdoDupTree (
                                        SPAP_ARG2 (SETWL_EXPR (arg_node))),
                                      let_vars,
                                      TBmakeNum (0))));
                let_vars = (let_vars != NULL ? FREEdoFreeTree (let_vars) : NULL);
            } else {
                if (NODE_TYPE (SETWL_VEC (arg_node)) == N_spid) {
                    subst = TCmakePrf2 (F_mul_SxV,
                                        TBmakeNum (0),
                                        DUPdoDupTree (
                                          GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))));
                } else {
                    subst = TBmakeNum (0);
                }
                INFO_HSE_WITHOP (arg_info)
                    = TBmakeGenarray
                        (DUPdoDupTree (GENERATOR_BOUND2 (SETWL_GENERATOR (arg_node))),
                         BuildDefault (SEUTsubstituteIdxs (DUPdoDupTree (SETWL_EXPR (arg_node)),
                                                           SETWL_VEC (arg_node),
                                                           subst)));
                subst = FREEdoFreeTree (subst);
            }
        }
        DBUG_PRINT ("withop inserted:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, INFO_HSE_WITHOP (arg_info)));
    }

    arg_node = FREEdoFreeTree (arg_node);

    if (oldinfo != NULL) {
        arg_node = TBmakeWith (INFO_HSE_PART( arg_info), 
                               INFO_HSE_CODE( arg_info),
                               INFO_HSE_WITHOP( arg_info));
        arg_info = FreeInfo( arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
