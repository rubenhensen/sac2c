/*
 *
 * $Log$
 * Revision 1.1  2004/07/27 14:10:35  khf
 * Initial revision
 *
 *
 *
 */

/**
 *
 * @file adjustaccumulation.c
 *
 *  In this traversal all right hand sides of accu operation and fold
 *  operations in Withloops are replaced by the corresponding return values
 *  of the fold operators.
 *
 *  Ex.:
 *    A,B,C = with(iv)
 *             gen:{ a,b  = accu( iv);
 *                   val1 = ...;
 *                   val2 = ...;
 *                   val3 = ...;
 *                   res1 = op1( a, val1);
 *                   res2 = op2( b, val2);
 *                 }: res1, res2, val3
 *            fold( op1, n1)
 *            fold( op2, n2)
 *            genarray( shp);
 *
 *  is transformed into
 *    A,B,C = with(iv)
 *             gen:{ A,B  = accu();
 *                   val1 = ...;
 *                   val2 = ...;
 *                   val3 = ...;
 *                   A    = op1( A, val1);
 *                   B    = op2( B, val2);
 *                 }: A, B, val3
 *            fold( op1, n1)
 *            fold( op2, n2)
 *            genarray( shp);
 *
 *  The index vector in the accu operation is no longer needed and will
 *  be removed.
 *
 */
#define NEW_INFO

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "LookUpTable.h"
#include "print.h"
#include "adjustaccumulation.h"

/**
 * INFO structure
 */
struct INFO {
    LUT_t lut;
    ids *lhs;
    ids *lhs_wl;
    node *withop;
};

#define INFO_AACC_LUT(n) (n->lut)
#define INFO_AACC_LHS(n) (n->lhs)
#define INFO_AACC_LHS_WL(n) (n->lhs_wl)
#define INFO_AACC_WITHOP(n) (n->withop)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_AACC_LUT (result) = GenerateLUT ();
    INFO_AACC_LHS (result) = NULL;
    INFO_AACC_LHS_WL (result) = NULL;
    INFO_AACC_WITHOP (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_AACC_LUT (info) = RemoveLUT (INFO_AACC_LUT (info));
    info = Free (info);

    DBUG_RETURN (info);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn AACCdo
 *
 *  @brief Traverses the loop body, the condition and the skip-block
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
AACCdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AACCdo");

    if (DO_BODY (arg_node) != NULL) {
        DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
    }

    if (DO_COND (arg_node) != NULL) {
        DO_COND (arg_node) = Trav (DO_COND (arg_node), arg_info);
    }

    if (DO_SKIP (arg_node) != NULL) {
        DO_SKIP (arg_node) = Trav (DO_SKIP (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACCfundef
 *
 *  @brief Traverses a FUNDEF's body and clears the LUT afterwards.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return the given fundef with substituted identifiers.
 *
 ***************************************************************************/
node *
AACCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AACCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("AACC", PrintNode (arg_node););
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        DBUG_EXECUTE ("AACC", PrintNode (arg_node););
    }

    INFO_AACC_LUT (arg_info) = RemoveContentLUT (INFO_AACC_LUT (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACCid
 *
 *  @brief Substitutes the current reference with a reference from the
 *         LUT if possible
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return potentially, a new N_id node
 *
 ***************************************************************************/
node *
AACCid (node *arg_node, info *arg_info)
{
    char *newname;

    DBUG_ENTER ("AACCid");

    newname = SearchInLUT_SS (INFO_AACC_LUT (arg_info), ID_NAME (arg_node));

    if (newname != ID_NAME (arg_node)) {
        ID_NAME (arg_node) = Free (ID_NAME (arg_node));
        ID_NAME (arg_node) = StringCopy (newname);
        ID_VARDEC (arg_node)
          = SearchInLUT_PP (INFO_AACC_LUT (arg_info), ID_VARDEC (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACClet
 *
 *  @brief Traverses right hand side and substitutes left hand side
 *         identifiers. If a substitution is made, the old vardec is marked
 *         ST_artificial as the new left hand side.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return modified let node.
 *
 ***************************************************************************/
node *
AACClet (node *arg_node, info *arg_info)
{
    ids *i;
    char *newname;

    DBUG_ENTER ("AACClet");

    INFO_AACC_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    i = LET_IDS (arg_node);

    while (i != NULL) {
        newname = SearchInLUT_SS (INFO_AACC_LUT (arg_info), IDS_NAME (i));

        if (newname != IDS_NAME (i)) {
            /*
             * Mark the old lhs vardec ST_artificial such that it
             * will be removed by precompile
             */
            VARDEC_STATUS (IDS_VARDEC (i)) = ST_artificial;

            IDS_NAME (i) = Free (IDS_NAME (i));
            IDS_NAME (i) = StringCopy (newname);
            IDS_VARDEC (i) = SearchInLUT_PP (INFO_AACC_LUT (arg_info), IDS_VARDEC (i));
        }

        i = IDS_NEXT (i);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACCprf
 *
 *  @brief Adds the LHS and the corresponding LHS of current WL into LUT
 *         if this prf is a F_accu.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
AACCprf (node *arg_node, info *arg_info)
{
    node *withop;
    ids *ids_assign, *ids_wl;

    DBUG_ENTER ("AACCprf");

    if (PRF_PRF (arg_node) == F_accu) {
        /* A,B = with(iv)
         *        gen:{
         *             a = accu( iv);
         *                   ...
         *             }...
         *            fold( op1, n1)
         *            ...
         *
         *     rename: a -> A
         */

        ids_assign = INFO_AACC_LHS (arg_info);
        ids_wl = INFO_AACC_LHS_WL (arg_info);
        withop = INFO_AACC_WITHOP (arg_info);

        DBUG_ASSERT ((withop != NULL), "F_accu without withloop");

        while (withop != NULL) {
            if (NWITHOP_IS_FOLD (withop)) {
                DBUG_ASSERT ((ids_wl != NULL), "ids of wl is missing");
                DBUG_ASSERT ((ids_assign != NULL), "ids of assign is missing");

                InsertIntoLUT_S (INFO_AACC_LUT (arg_info), IDS_NAME (ids_assign),
                                 IDS_NAME (ids_wl));

                InsertIntoLUT_P (INFO_AACC_LUT (arg_info), IDS_VARDEC (ids_assign),
                                 IDS_VARDEC (ids_wl));

                ids_assign = IDS_NEXT (ids_assign);
            }
            ids_wl = IDS_NEXT (ids_wl);
            withop = NWITHOP_NEXT (withop);
        }

        /*
         * remove index vector
         *
         * accu(iv) -> accu()
         */
        PRF_ARGS (arg_node) = FreeNode (PRF_ARGS (arg_node));
    } else {
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACCwith
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
AACCwith (node *arg_node, info *arg_info)
{
    node *withop;
    ids *lhs;

    DBUG_ENTER ("AACCwith");

    /* stack lhs and withop of surrounding WL */
    lhs = INFO_AACC_LHS_WL (arg_info);
    withop = INFO_AACC_WITHOP (arg_info);

    INFO_AACC_LHS_WL (arg_info) = INFO_AACC_LHS (arg_info);
    INFO_AACC_WITHOP (arg_info) = NWITH_WITHOP (arg_node);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    INFO_AACC_WITHOP (arg_info) = withop;
    INFO_AACC_LHS_WL (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACCwith2
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
AACCwith2 (node *arg_node, info *arg_info)
{
    node *withop;
    ids *lhs;

    DBUG_ENTER ("AACCwith2");

    /* stack lhs and withop of surrounding WL */
    lhs = INFO_AACC_LHS_WL (arg_info);
    withop = INFO_AACC_WITHOP (arg_info);

    INFO_AACC_LHS_WL (arg_info) = INFO_AACC_LHS (arg_info);
    INFO_AACC_WITHOP (arg_info) = NWITH2_WITHOP (arg_node);

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    INFO_AACC_WITHOP (arg_info) = withop;
    INFO_AACC_LHS_WL (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn AACCcode
 *
 *  @brief Adds the cexprs into LUT if the corresponding WL operations
 *         are fold.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
AACCcode (node *arg_node, info *arg_info)
{
    node *cexprs, *withop, *cexpr;
    ids *_ids;

    DBUG_ENTER ("AACCcode");

    /* A,B = with(iv)
     *        gen:{...}:res1,res2
     *            fold( op1, n1)
     *            ...
     *
     *     rename: res1 -> A
     */

    _ids = INFO_AACC_LHS_WL (arg_info);
    cexprs = NCODE_CEXPRS (arg_node);
    withop = INFO_AACC_WITHOP (arg_info);

    while (withop != NULL) {
        if (NWITHOP_IS_FOLD (withop)) {
            DBUG_ASSERT ((_ids != NULL), "ids is missing");
            cexpr = EXPRS_EXPR (cexprs);
            DBUG_ASSERT ((cexpr != NULL), "CEXPR is missing");
            DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "CEXPR is not a N_id");

            InsertIntoLUT_S (INFO_AACC_LUT (arg_info), VARDEC_NAME (ID_VARDEC (cexpr)),
                             IDS_NAME (_ids));

            InsertIntoLUT_P (INFO_AACC_LUT (arg_info), ID_VARDEC (cexpr),
                             IDS_VARDEC (_ids));
        }

        _ids = IDS_NEXT (_ids);
        cexprs = EXPRS_NEXT (cexprs);
        withop = NWITHOP_NEXT (withop);
    }

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (NCODE_CEXPRS (arg_node) != NULL) {
        NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn AdjustAccumulation
 *
 *  @brief Starting function of AdjustAccumulation traversal
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree
 *
 ***************************************************************************/
node *
AdjustAccumulation (node *syntax_tree)
{
    funtab *tmp_tab;
    info *info;

    DBUG_ENTER ("AdjustAccumulation");

    info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = aacc_tab;

    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);
    act_tab = tmp_tab;

    DBUG_RETURN (syntax_tree);
}
