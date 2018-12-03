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
    node *ids;
    node *lentd;
};

/* access macros */
#define INFO_HSED_IDS(n) ((n)->ids)
#define INFO_HSED_LENTD(n) ((n)->lentd)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HSED_IDS (result) = NULL;
    INFO_HSED_LENTD (result) = NULL;

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
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
 * checks for any occurrences of a dot symbol within a set notation
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of dots found
 *****************************************************************************/

/** <!--********************************************************************-->
 * counts the number of triple dots contained in
 * selection vector.
 *
 * @param ids EXPRS node containing ids
 * @return number of triple dots found
 *****************************************************************************/

/** <!--********************************************************************-->
 * traverses spids vector and replaces dots by new spids and creates an 
 * N_exprs chain with these spids as return value
 *
 * @param exprs EXPR node containing ids
 * @return N_exprs chain of new spids
 *****************************************************************************/

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
RecMergeIn (node *didxs, node *e_vals, node *s_vals)
{
    node *res = NULL;
    DBUG_ENTER ();

    if (didxs != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (didxs)) != N_dot) {
            res = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (e_vals)),
                               RecMergeIn (EXPRS_NEXT (didxs),
                                           EXPRS_NEXT (e_vals),
                                           s_vals));
        } else {
            DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (didxs)) == N_spid),
                         "neither N_dot nor N_spid in didxs of MergeIn");
            if (DOT_NUM (EXPRS_EXPR (didxs)) == 1) {
                res = TBmakeExprs (DUPdoDupTree (EXPRS_EXPR (s_vals)),
                                   RecMergeIn (EXPRS_NEXT (didxs),
                                               e_vals,
                                               EXPRS_NEXT (s_vals)));
            } else {
                // skip triple dot here!
                res = RecMergeIn (EXPRS_NEXT (didxs),
                                  e_vals,
                                  s_vals);
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
    node *left,*right;
    int m,n,k;

    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (didxs) == N_exprs,
                "exprs-chain of Spid nodes expected as didxs param to MergeIn");
    DBUG_ASSERT ((e_vals == NULL)
                 || (NODE_TYPE (e_vals) == N_exprs)
                 || (NODE_TYPE (e_vals) == N_dot),
                "potentially empty exprs-chain of expression nodes or N_dot "
                "expected as e_val param to MergeIn");
    DBUG_ASSERT (NODE_TYPE (s_vals) == N_exprs,
                "exprs-chain of expression nodes expected as s_val param "
                "to MergeIn");
    DBUG_ASSERT ((t_val == NULL) || (NODE_TYPE (t_val) == N_array),
                "t_val is expected to be either NULL or to be an N_array node");

    if ((e_vals != NULL)
        && (NODE_TYPE (e_vals) == N_dot)
        && (DOT_NUM (e_vals) == 1)) {
        res = TBmakeDot (1);
    } else {
        m = TCcountExprs (e_vals);
        n = TCcountExprs (s_vals);
        k = (t_val == NULL ? 0 : TCcountExprs (ARRAY_AELEMS (t_val)));
        DBUG_ASSERT (TCcountExprs (didxs) == m+n+k,
                     "length of dotted generator variables expected to match the"
                     "number of expressions to be merged, ie. we should have"
                     "%d = %d+%d+%d", TCcountExprs (didxs), m, n, k);
        res = RecMergeIn( didxs, e_vals, s_vals);
        if (k==1) {
            // insert t_val as concatenation of arrays
            left = TCgetNthExprs (m-1, res);
            right = EXPRS_NEXT (left);
            EXPRS_NEXT (left) = NULL;
            left = TCmakeIntVector (res);
            right = TCmakeIntVector (right);
            res = TCmakePrf2 (F_cat_VxV,
                              left,
                              TCmakePrf2 (F_cat_VxV,
                                          t_val,
                                          right));
        } 
    }
    
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
HSEDsetwl (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#if 0
    DBUG_PRINT ("looking at Set-Expression in line %zu:", global.linenum);
    /* Bottom up traversal! */
    DBUG_PRINT ("handling body ... ");
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_PRINT( "body done ... ");
    /* Now, we look at this setwl! */
    ids = SETWL_VEC (arg_node);
    num_sd = CountSingleDots (ids);
    num_td = CountTripleDots (ids);

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

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
