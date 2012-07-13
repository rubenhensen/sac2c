/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup temp Unshare WL-fold index vectors
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |  y  |       |
 * follows N_ap to LaC funs                |   -----   |  n  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    |  y  |  jsa  |  04/07/12
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |     |       |
 * utilises SAA annotations                |   -----   |  n  |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    |     |       |
 * tolerates flattened Generators          |    yes    |     |       |
 * tolerates flattened operation parts     |    yes    |     |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |     |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  y  |  jsa  |  04/07/12
 * =============================================================================
 * </pre>
 *
 *  This traversal inserts the F_unshare primitive function into the N_code block of WL
 *folds. The index vector object is *always* modified in-place, without considering its
 *reference count. The F_unshare prf compiles into a dynamic if-condition in the generated
 *code that checks whether the index vector object is leaving the WL iteration body. If
 *so, it will allocate a new memory place for the index vector to 'unshare' it.
 *
 *  Example - IndexMin:
 *    inline int[+] imin(float[*] a, int[+] k1, int[+] k2)
 *    {
 *      return (a[k1] <= a[k2]) ? k1 : k2;
 *    }
 *
 *    int indexmin(float[*] a)
 *    {
 *      z = with {
 *        ( 0*shape(a) <= iv &lt; shape(a)) : iv;
 *      } : fold( imin(a), 0*shape(a));
 *      return z;
 *    }
 *
 *  The program above will compile into the following for-loop (assuming dim(a)=1):
 *  pseudo-C code:
 *    iv = malloc()...
 *    accu = malloc()...
 *    accu[0] = 0;
 *    h = malloc()...
 *
 *    for (int w = 0; w &lt; shape(a)[0]; ) {
 *      iv[0] = w;        // in-place destructive modification
 *
 *      if (a[accu[0]] <= a[iv[0]]) {
 *        inc_rc( accu);
 *        h = accu;
 *      } else {
 *        inc_rc( iv);
 *        h = iv;
 *      }
 *
 *      dec_rc( accu);
 *      accu = h;
 *      w = w + 1;
 *    }
 *
 *  The above code will cause memory sharing of the iv and the accu objects (at some
 *point). But the iv variable will continue to be modified in-place at the beggining of
 *the for-loop, hence leading to an incorrect result.
 *
 *  To fix it the F_unshare is inserted in this traversal.
 *
 *  This is the original code after the accu phase but before UFIV:
 *
 *      z = with {
 *        ( 0*shape(a) <= iv &lt; shape(a)) {
 *          acc = _accu_( iv);
 *          inc_rc( iv);
 *          h = imin( a, acc, iv);
 *        } : h;
 *      } : fold( imin(a), 0*shape(a));
 *
 *  The UFIV traversal inserts the F_unshare primitive function at the end of the
 *  WL code block:
 *
 *      z = with {
 *        ( 0*shape(a) <= iv &lt; shape(a)) {
 *          acc = _accu_( iv);
 *          inc_rc( iv);
 *          h = imin( a, acc, iv);
 *          g = _unshare_( h, iv);          // inserted
 *        } : g;                            // modified
 *      } : fold( imin(a), 0*shape(a));
 *
 *  The _unshare_ function has the following operational semantics:
 *      if (h is the same memory as iv) {
 *        iv = malloc()...
 *        copy_data( from h to iv);
 *        dec_rc( d);
 *      }
 *      g = h;
 *
 *  Hence, if the iv gets shared (aliased) with h, the iv object is re-initialized with a
 *new memory place and the data is copied over. The generated C code looks like this:
 *
 *    for (int w = 0; w &lt; shape(a)[0]; ) {
 *      iv[0] = w;        // in-place destructive modification
 *
 *      if (a[accu[0]] <= a[iv[0]]) {
 *        inc_rc( accu);
 *        h = accu;
 *      } else {
 *        inc_rc( iv);
 *        h = iv;
 *      }
 *
 *      if (desc(h) == desc(iv)) {
 *        iv = malloc()...
 *        copy( iv, h);
 *        dec_rc( h);
 *      }
 *      g = h;
 *
 *      dec_rc( accu);
 *      accu = g;
 *      w = w + 1;
 *    }
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file unshare_fold_iv.c
 *
 * Prefix: UFIV
 *
 *****************************************************************************/
#include "unshare_fold_iv.h"

#define DBUG_PREFIX "UFIV"

/*
 * Other includes go here
 */
#include "debug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "DupTree.h"
#include "tree_compound.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *cblock;
    node *withid;
    node *postassign;
    node *new_vardecs;
    bool fold_found;
    node *withop;
    node *cur_withop;
};

/**
 * Access macros:
 *  INFO_CBLOCK                 indicates traversing inside the WL code block
 *  INFO_WITHID                 the withid node of the current WL
 *  INFO_POSTASSIGN             the chain of new assignment nodes to be later appended
 *                              to the code block.
 *  INFO_NEW_VARDECS            the chain of new vardec nodes to be later appended into
 * the fundef top-block INFO_WITHOP                 the withop node (chain) of the current
 * WL INFO_CUR_WITHOP             current withop node in the chain of the WL. This chain
 * is correlated to the chain of code block result expressions to determine which result
 * is folded.
 */
#define INFO_CBLOCK(n) ((n)->cblock)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_NEW_VARDECS(n) ((n)->new_vardecs)
#define INFO_WITHOP(n) ((n)->withop)
#define INFO_CUR_WITHOP(n) ((n)->cur_withop)

static info *
MakeInfo (void)
{
    DBUG_ENTER ();

    info *result = (info *)MEMmalloc (sizeof (info));

    INFO_CBLOCK (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_NEW_VARDECS (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_CUR_WITHOP (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *UFIVdoUnshareFoldIV( node *syntax_tree)
 *
 *****************************************************************************/
node *
UFIVdoUnshareFoldIV (node *syntax_tree)
{
    DBUG_ENTER ();

    info *info = MakeInfo ();

    DBUG_PRINT ("Starting UFIV traversal.");

    TRAVpush (TR_ufiv);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("UFIV traversal complete.");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *UnshareIV(node *outp_id, info *arg_info)
 *
 * @brief Inject the F_unshare primitive function into the data-flow.
 *
 *  The outp_id node is (one of) the result of the WL body iteration in a fold.
 *  The result is a replacement node for the outp_id in the expression chain.
 *
 *****************************************************************************/
static node *
UnshareIV (node *outp_id, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (outp_id) == N_id, "expected id");
    DBUG_PRINT ("replacing id '%s' (id:%p, avis:%p)", ID_NAME (outp_id), outp_id,
                ID_AVIS (outp_id));

    /* type of the result */
    ntype *n_tp = TYcopyType ((ntype *)AVIS_TYPE (ID_AVIS (outp_id)));

    /* new avis to replace the result */
    node *n_avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (outp_id)), n_tp);

    /* vardec for the new avis. Will be appended to the top-block vardec chain later */
    INFO_NEW_VARDECS (arg_info) = TBmakeVardec (n_avis, INFO_NEW_VARDECS (arg_info));

    /* N_ids vector of index vectors of the WL */
    node *ivec = WITHID_VEC (INFO_WITHID (arg_info));
    DBUG_ASSERT (NODE_TYPE (ivec) == N_ids, "expected ids");
    /* transform ivec:N_ids into evec:N_exprs */
    node *n_evec = TCids2Exprs (ivec);

    /* create the primitive function and the N_let. Hook it into the data-flow:
     *  <new_outp_id> = _unshare_( <outp_id>, <iv>);
     */
    node *n_unsh
      = TBmakePrf (F_unshare, TBmakeExprs (TBmakeId (ID_AVIS (outp_id)), n_evec));

    node *n_let = TBmakeLet (TBmakeIds (n_avis, NULL), n_unsh);

    node *n_assign = TBmakeAssign (n_let, INFO_POSTASSIGN (arg_info));

    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (n_avis) = n_assign;
    /* the N_assign node will be appended to the chain in CODE_CBLOCK */
    INFO_POSTASSIGN (arg_info) = n_assign;

    /* remove the original id */
    outp_id = FREEdoFreeNode (outp_id);
    /* create a new id */
    node *new_outp_id = TBmakeId (n_avis);

    DBUG_PRINT (
      "new assignment: '%s' (id:%p, avis:%p) = unshare( '%s' (id:%p, avis:%p), ...)",
      ID_NAME (new_outp_id), new_outp_id, ID_AVIS (new_outp_id),
      ID_NAME (PRF_ARG1 (n_unsh)), PRF_ARG1 (n_unsh), ID_AVIS (PRF_ARG1 (n_unsh)));

    DBUG_RETURN (new_outp_id);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *UFIVfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
UFIVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Running UFIV in function '%s'", FUNDEF_NAME (arg_node));
    DBUG_ASSERT (INFO_NEW_VARDECS (arg_info) == NULL, "some vardecs left behind!");

    /* traverse body */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /* any new vardecs created? */
    if (INFO_NEW_VARDECS (arg_info)) {
        /* append the new vardecs to the fundef's top block */
        arg_node = TCaddVardecs (arg_node, INFO_NEW_VARDECS (arg_info));
        INFO_NEW_VARDECS (arg_info) = NULL;
    }

    /* traverse local funs and next */
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UFIVmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverse only functions in the module.
 *
 *****************************************************************************/
node *
UFIVmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* traverse only the functions in the module */
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UFIVcode(node *arg_node, info *arg_info)
 *
 * @brief Traverses code's result expressions (CExpr) and appends the new assignments
 *        into the code's assignment chain.
 *
 *****************************************************************************/
node *
UFIVcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (CODE_CBLOCK (arg_node) != NULL) {
        DBUG_ASSERT (INFO_POSTASSIGN (arg_info) == NULL, "not null!");

        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

        /* reset cur_withop pointer */
        INFO_CUR_WITHOP (arg_info) = INFO_WITHOP (arg_info);
        /* mark we're in the code */
        INFO_CBLOCK (arg_info) = CODE_CBLOCK (arg_node);

        /* traverse the code  */
        CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);

        INFO_CBLOCK (arg_info) = NULL;

        /* new assignments created? */
        if (INFO_POSTASSIGN (arg_info)) {
            /* append the new assignments to the CODE_CBLOCK's chain */
            BLOCK_ASSIGNS (CODE_CBLOCK (arg_node))
              = TCappendAssign (BLOCK_ASSIGNS (CODE_CBLOCK (arg_node)),
                                INFO_POSTASSIGN (arg_info));
            INFO_POSTASSIGN (arg_info) = NULL;
        }
    }

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UFIVexprs(node *arg_node, info *arg_info)
 *
 * @brief Correlates the CODE_CEXPRS with WITH_WITHOP and unshares the expressions
 *        that belong to a fold operator.
 *
 *****************************************************************************/
node *
UFIVexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((INFO_CBLOCK (arg_info) != NULL) && (INFO_WITHID (arg_info) != NULL)
        && (INFO_WITHOP (arg_info) != NULL)) {
        /* in the WL code's cexprs chain of results */
        DBUG_ASSERT (INFO_CUR_WITHOP (arg_info) != NULL,
                     " the WithOp chain is shorter than the Code_CExprs chain!");

        /* does the current result expression (EXPRS_EXPR) belong to an N_fold
         * operator? */
        if (NODE_TYPE (INFO_CUR_WITHOP (arg_info)) == N_fold) {
            /* yes, unshare the expression (it should be a simple N_id)
             * from the index vector */
            EXPRS_EXPR (arg_node) = UnshareIV (EXPRS_EXPR (arg_node), arg_info);
        }

        if (INFO_CUR_WITHOP (arg_info) != NULL) {
            /* Advance to the next withop in the chain.
             * We correlate the withop chain with Code_CExprs chain
             * to determine which cexpr is being in fact fold'ed */
            INFO_CUR_WITHOP (arg_info) = WITHOP_NEXT (INFO_CUR_WITHOP (arg_info));
        }
    } else {
        /* ordinary expression chain */
        EXPRS_EXPR (arg_node) = TRAVopt (EXPRS_EXPR (arg_node), arg_info);
    }

    EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UFIVpart(node *arg_node, info *arg_info)
 *
 * @brief Picks up WITHID from N_part, and traverses the Code.
 *
 *  This is for the 'N_with' style of the WL.
 *
 *****************************************************************************/
node *
UFIVpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* remember PART_WITHID */
    INFO_WITHID (arg_info) = PART_WITHID (arg_node);

    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    INFO_WITHID (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UFIVwith(node *arg_node, info *arg_info)
 *
 * @brief Begins WL unsharing in with-style WLs.
 *
 *****************************************************************************/
node *
UFIVwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* stack info */
    info *ninfo = MakeInfo ();
    /* all vardecs are collected and inserted into the ast at the fundef level */
    INFO_NEW_VARDECS (ninfo) = INFO_NEW_VARDECS (arg_info);
    /* pick withop */
    INFO_WITHOP (ninfo) = WITH_WITHOP (arg_node);

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), ninfo);
    /* do NOT traverse WITH_CODE */

    /* the chain is carried over back */
    INFO_NEW_VARDECS (arg_info) = INFO_NEW_VARDECS (ninfo);

    FreeInfo (ninfo);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UFIVwith2(node *arg_node, info *arg_info)
 *
 * @brief Begins WL unsharing in with2-style WLs.
 *
 *****************************************************************************/
node *
UFIVwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* stack info */
    info *ninfo = MakeInfo ();
    /* all vardecs are collected and inserted into the ast at the fundef level */
    INFO_NEW_VARDECS (ninfo) = INFO_NEW_VARDECS (arg_info);

    /* pick withop, and withid (there is no N_part) */
    INFO_WITHID (ninfo) = WITH2_WITHID (arg_node);
    INFO_WITHOP (ninfo) = WITH2_WITHOP (arg_node);

    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), ninfo);

    /* the chain is carried over back */
    INFO_NEW_VARDECS (arg_info) = INFO_NEW_VARDECS (ninfo);

    FreeInfo (ninfo);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal UFIV -->
 *****************************************************************************/
