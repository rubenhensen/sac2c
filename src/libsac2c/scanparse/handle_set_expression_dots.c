#include "handle_set_expression_dots.h"
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

#include <strings.h>

/**
 * @file handle_set_expression_dots.c
 *
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
    int lm;
    int ln;
    int rm;
    int rn;
    int k;
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


/** <!--********************************************************************-->
 * creates an N_exprs chain containing n N_spid temp vars.
 *
 * @param n number of var to create
 * @return EXPRS chain of N_spid nodes
 *****************************************************************************/
static node *
CreateDotVarChain (int n)
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
            res = TBmakeExprs (DUPdoDupNode (EXPRS_EXPR (didxs)),
                               StripDots (EXPRS_NEXT (didxs)));
        } else {
            res = StripDots (EXPRS_NEXT (didxs));
        }
    }

    DBUG_RETURN (res);
}



/** <!--********************************************************************-->
 * wraps an N_exprs chain either in an N_array (in case no triple-dot)
 * or splits the N_exprs chain at position pos_tdot into left and right 
 * and creates a * concatenation (left ++ (tdot_vec ++ right))
 *
 * @param exprs chain
 * @param num_tdot 0/1 value indicating whether triple-dots need to be injected
 * @param pos_tdot indicates after how many exprs tdot_vec is to be injected
 * @param tdot_vec vector to be injected at the triple dot position
 * @return expression that represents the vector
 *****************************************************************************/
static node *
Exprs2expr (node *exprs, int num_tdot, int pos_tdot, node *tdot_vec)
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
    DBUG_PRINT ("Vec analyses yields: lm = %d, ln = %d | k = %d | rm = %d, rn = %d\n",
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
RecMergeIn (node *didxs, node *e_vals, node *s_vals, int pos, int *tdot_pos)
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
    int m,n,k;
    int t_pos;

    DBUG_ENTER ();
    DBUG_PRINT_TAG ("HSED_MERGE", "MergeIn called on:");
    DBUG_EXECUTE_TAG ("HSED_MERGE", PRTdoPrintFile (stderr, didxs),
                                    (e_vals != NULL? PRTdoPrintFile (stderr, e_vals):0),
                                    (s_vals != NULL? PRTdoPrintFile (stderr, s_vals):0),
                                    (t_val != NULL? PRTdoPrintFile (stderr, t_val):0));
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
                     "number of expressions to be merged, ie. we should have"
                     "%d = %d+%d+%d", TCcountExprs (didxs), m, n, k);
        res = RecMergeIn( didxs, e_vals, s_vals, 0, &t_pos);
        res = Exprs2expr (res, k, t_pos, t_val);
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
 *
 * @param arg_node current node of the ast
 * @param arg_info info node
 * @return transformed AST
 */
node *
HSEDgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
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

    DBUG_PRINT ("processing generator...");
    SETWL_GENERATOR (arg_node) = TRAVopt (SETWL_GENERATOR (arg_node), arg_info);

    if (INFO_HSED_HAS_DOTS (arg_info)) {
        DBUG_PRINT( "adjusting expr ... ");
        sdot_exprs = CreateDotVarChain ( INFO_HSED_LN (arg_info)
                                         + INFO_HSED_RN (arg_info));
        tdot_idx = (INFO_HSED_K (arg_info) == 1 ?
                    TBmakeExprs (MakeTmpId ("tmpv"), NULL)
                    : NULL);
        sdot_idxs = Exprs2expr (sdot_exprs,
                                INFO_HSED_K (arg_info),
                                INFO_HSED_LN (arg_info),
                                tdot_idx);
        DBUG_PRINT ("selection index is:");
        DBUG_EXECUTE (PRTdoPrintFile (stderr, sdot_idxs));
        SETWL_EXPR (arg_node) = TCmakePrf2 (F_sel_VxA,
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
        if (INFO_HSED_K (arg_info) == 1) {
            // some voodoo needs to happen here :-)
        } else {
            SETWL_VEC (arg_node) = FREEdoFreeTree (SETWL_VEC (arg_node));
            idxs = FREEdoFreeTree (idxs);
            //NB: sdot_idxs is absorbed by the selection! no freeing here!
            //NB: tdot_idx is absorbed by MergeIn! no freeing here!
            SETWL_VEC (arg_node) = nidxs;
        }
    }
#if 0
    if (num_td == 1) {
        DBUG_PRINT (" triple dot with %d single dots found!", num_sd);
        spid = MakeTmpId( "td_vec");

        /*
         * traverse all ids before the "..." and return an array
         * of selections into "td_vec" according to their position
         */
        array1 = TCmakeIntVector
                     (HandleSingleDotsBeforeTriple (ids, spid, 0));

        /* find the "..." */
        td_pos = 0;
        td_ids = ids;
        while (( NODE_TYPE (EXPRS_EXPR (td_ids)) != N_dot)
               || (DOT_NUM (EXPRS_EXPR (td_ids)) == 1) ) {
            td_ids = EXPRS_NEXT (td_ids);
            td_pos++;
        }

       /*
        * Now traverse all ids behind the "..." and return an array
        * of selections into "td_vec" according to their position and
        * the number of dimensions (len_td) matched by the triple dots.
        * We have      len_td = dim( expr'') - num_sd
        * where expr'' is expr with all references to ids replaced
        * by scalar 0. NB: is this really safe to do?
        */
       expr_pp = ReplaceIdsZero (DUPdoDupTree (SETWL_EXPR (arg_node)), ids);
       len_td = TCmakePrf2 (F_sub_SxS,
                            TCmakePrf1 (F_dim_A, expr_pp),
                            TBmakeNum( num_sd));
       array2 = TCmakeIntVector
                    (HandleSingleDotsBehindTriple
                        (EXPRS_NEXT (td_ids), spid, td_pos, len_td));
        
       /*
        * Finally, we modify the original expression expr into expr' to use selections
        * into spid rather than scalar ids and we insert the overall selection:
        *     expr'[ _cat_VxV_( _cat_VxV_( array1,
        *                                  drop( td_pos+1-len_ids, drop( td_pos, spid))),
        *                                  array2) ]
        */
       expr_p = ReplaceIdsSel (SETWL_EXPR (arg_node), ids, len_td);
       index = TCmakePrf2
                   (F_cat_VxV,
                   TCmakePrf2
                       (F_cat_VxV,
                       array1,
                       TCmakePrf2
                           (F_drop_SxV,
                           TBmakeNum (td_pos+1-TCcountExprs(ids)),
                           TCmakePrf2
                               (F_drop_SxV,
                               TBmakeNum (td_pos),
                               spid)) ),
                   array2);
       SETWL_EXPR (arg_node) = TCmakeSpap2 (NULL, strdup ("sel"),
                                           index,
                                           SETWL_EXPR (arg_node));
       /*
        * and we replace the ids by spid:
        */
       SETWL_VEC (arg_node) = FREEdoFreeTree (SETWL_VEC (arg_node));
       SETWL_VEC (arg_node) = spid;
   
    } else if (num_td > 1) {
      CTIerrorLine( global.linenum,
                    " triple-dot notation used more than once in array comprehension;");

    } else if (num_sd >0) {
      /* We have at least one single dot but no triple dots! */
      DBUG_PRINT (" no triple dots but %d single dots found!", num_sd);

      /*
       * traverse the ids, replace dots with vars and create 
       * array of those variables.
       */

      array = HandleSingleDots( ids);
      
      SETWL_EXPR (arg_node) = TCmakeSpap2 (NULL, strdup ("sel"),
                                           TCmakeIntVector (array),
                                           SETWL_EXPR (arg_node));

    }  else {
      DBUG_PRINT (" no dots found!");
    }
#endif
    arg_info = FreeInfo (arg_info); //pop arg_info!

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
