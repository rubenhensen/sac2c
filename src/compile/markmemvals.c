/*
 *
 * $Log$
 * Revision 1.6  2004/07/31 21:29:15  ktr
 * moved treatment of F_fill, F_accu und F_suballoc into seperate functions.
 *
 * Revision 1.5  2004/07/28 12:26:24  khf
 * F_accu will be removed in compile.c
 *
 * Revision 1.4  2004/07/28 11:29:53  khf
 * insert N_empty node if BLOCK_INSTR is empty after removal
 * of F_accu
 *
 * Revision 1.3  2004/07/28 09:08:19  khf
 * support for ExplicitAccumulate added
 *
 * Revision 1.2  2004/07/22 14:13:50  ktr
 * a = fill( b, c) is now converted into c = b
 * which appears to be easier to compile.
 *
 * Revision 1.1  2004/07/21 16:52:24  ktr
 * Initial revision
 *
 *
 */

/**
 *
 * @file markmemvals.c
 *
 * 1. In this traversal all fill operations and unnecessary MemVal-variables
 *    are removed by means of a substitution traversal.
 *
 *    Ex.:
 *      c = alloc();
 *      a = fill( ..., c)
 *
 *    is transformed into
 *      c = alloc();
 *      c = ...;
 *
 *    All subsequent references to a are renamed into references to c.
 *
 *    As genarray-withloops are nothing but fancy fill-operations after
 *    the explicit alloc-operations are inserted, their result is treated
 *    exactly like the fill operation's result.
 *
 * 2. If ExplicitAccumulate was applied:
 *    All subsequent references to left hand side of accu operations
 *    in Withloops are renamed into references to the corresponding
 *    return values of the fold operators.
 *    The corresponding cexpr is also be renamed.
 *
 *
 *    Ex.:
 *      A,B,C = with(iv)
 *               gen:{ a,b   = accu( iv);
 *                     val1  = ...;
 *                     val2  = ...;
 *                     val3  = ...;
 *                     emal1 = alloc(...);
 *                     emal1 = op1( a, val1);
 *                     emal2 = alloc(...);
 *                     emal2 = op2( b, val2);
 *                   }: emal1, emal2, val3
 *              fold( op1, n1)
 *              fold( op2, n2)
 *              genarray( shp);
 *
 *    is transformed into
 *      A,B,C = with(iv)
 *               gen:{ A,B   = accu( iv);
 *                     val1  = ...;
 *                     val2  = ...;
 *                     val3  = ...;
 *                     emal1 = alloc(...);
 *                     emal1 = op1( A, val1);
 *                     emal2 = alloc(...);
 *                     emal2 = op2( B, val2);
 *                     A     = emal1;
 *                     B     = emal2;
 *                   }: A, B, val3
 *              fold( op1, n1)
 *              fold( op2, n2)
 *              genarray( shp);
 *
 *
 */

#define NEW_INFO

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "LookUpTable.h"
#include "print.h"
#include "DupTree.h"
#include "free.h"

/**
 * INFO structure
 */
struct INFO {
    LUT_t lut;
    ids *lhs;
    ids *lhs_wl;
    node *withop;
};

#define INFO_MMV_LUT(n) (n->lut)
#define INFO_MMV_LHS(n) (n->lhs)
#define INFO_MMV_LHS_WL(n) (n->lhs_wl)
#define INFO_MMV_WITHOP(n) (n->withop)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_MMV_LUT (result) = GenerateLUT ();
    INFO_MMV_LHS (result) = NULL;
    INFO_MMV_LHS_WL (result) = NULL;
    INFO_MMV_WITHOP (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_MMV_LUT (info) = RemoveLUT (INFO_MMV_LUT (info));
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
 * @fn MMVdo
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
MMVdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVdo");

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
 * @fn MMVfundef
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
MMVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_EXECUTE ("MMV", PrintNode (arg_node););

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        DBUG_EXECUTE ("MMV", PrintNode (arg_node););
    }

    INFO_MMV_LUT (arg_info) = RemoveContentLUT (INFO_MMV_LUT (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVid
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
MMVid (node *arg_node, info *arg_info)
{
    char *newname;

    DBUG_ENTER ("MMVid");

    newname = SearchInLUT_SS (INFO_MMV_LUT (arg_info), ID_NAME (arg_node));

    if (newname != ID_NAME (arg_node)) {
        ID_NAME (arg_node) = Free (ID_NAME (arg_node));
        ID_NAME (arg_node) = StringCopy (newname);
        ID_VARDEC (arg_node)
          = SearchInLUT_PP (INFO_MMV_LUT (arg_info), ID_VARDEC (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVlet
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
MMVlet (node *arg_node, info *arg_info)
{
    ids *i;
    char *newname;

    DBUG_ENTER ("MMVlet");

    INFO_MMV_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    i = LET_IDS (arg_node);

    while (i != NULL) {
        newname = SearchInLUT_SS (INFO_MMV_LUT (arg_info), IDS_NAME (i));

        if (newname != IDS_NAME (i)) {
            /*
             * Mark the old lhs vardec ST_artificial such that it
             * will be removed by precompile
             */
            VARDEC_STATUS (IDS_VARDEC (i)) = ST_artificial;

            IDS_NAME (i) = Free (IDS_NAME (i));
            IDS_NAME (i) = StringCopy (newname);
            IDS_VARDEC (i) = SearchInLUT_PP (INFO_MMV_LUT (arg_info), IDS_VARDEC (i));
        }

        i = IDS_NEXT (i);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfFill
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfFill (node *arg_node, info *arg_info)
{
    node *temp;

    DBUG_ENTER ("MMVprfFill");

    /*
     * a = fill( ..., b')
     *
     * rename: b' -> b
     */
    PRF_ARG2 (arg_node) = Trav (PRF_ARG2 (arg_node), arg_info);

    /*
     * a = fill( ..., b)
     *
     * rename: a -> b
     */
    InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                     VARDEC_NAME (ID_VARDEC (PRF_ARG2 (arg_node))));

    InsertIntoLUT_P (INFO_MMV_LUT (arg_info), IDS_VARDEC (INFO_MMV_LHS (arg_info)),
                     ID_VARDEC (PRF_ARG2 (arg_node)));

    /*
     * eliminate the fill operation
     *
     * b = ...;
     */
    temp = PRF_ARG1 (arg_node);
    PRF_ARG1 (arg_node) = NULL;
    arg_node = FreeTree (arg_node);
    arg_node = temp;

    /*
     * Traverse the new rhs
     */
    if (arg_node != NULL) {
        arg_node = Trav (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfAccu
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfAccu (node *arg_node, info *arg_info)
{
    node *withop;
    ids *ids_assign, *ids_wl;

    DBUG_ENTER ("MMVprfAccu");

    /* A,B = with(iv)
     *        gen:{
     *             a = accu( iv);
     *             ...
     *             }...
     *            fold( op1, n1)
     *            ...
     *
     *     rename: a -> A
     */

    ids_assign = INFO_MMV_LHS (arg_info);
    ids_wl = INFO_MMV_LHS_WL (arg_info);
    withop = INFO_MMV_WITHOP (arg_info);

    DBUG_ASSERT ((withop != NULL), "F_accu without withloop");

    while (withop != NULL) {
        if (NWITHOP_IS_FOLD (withop)) {
            DBUG_ASSERT ((ids_wl != NULL), "ids of wl is missing");
            DBUG_ASSERT ((ids_assign != NULL), "ids of assign is missing");

            InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (ids_assign),
                             IDS_NAME (ids_wl));

            InsertIntoLUT_P (INFO_MMV_LUT (arg_info), IDS_VARDEC (ids_assign),
                             IDS_VARDEC (ids_wl));

            ids_assign = IDS_NEXT (ids_assign);
        }
        ids_wl = IDS_NEXT (ids_wl);
        withop = NWITHOP_NEXT (withop);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfSuballoc
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfSuballoc (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVprfSuballoc");

    /*
     * a = suballoc( A, iv)
     *
     * 1. remove iv
     * => a = suballoc( A);
     */
    PRF_EXPRS2 (arg_node) = FreeTree (PRF_EXPRS2 (arg_node));

    /*
     * rename RHS
     */
    PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

    /*
     *
     */
    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprf
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         prf is a F_fill.
 *         Adds the LHS and the corresponding LHS of current WL into LUT
 *         if this prf is a F_accu.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVprf");

    switch (PRF_PRF (arg_node)) {
    case F_fill:
        arg_node = MMVprfFill (arg_node, arg_info);
        break;

    case F_accu:
        arg_node = MMVprfAccu (arg_node, arg_info);
        break;

    case F_suballoc:
        arg_node = MMVprfSuballoc (arg_node, arg_info);
        break;

    default:
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith
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
MMVwith (node *arg_node, info *arg_info)
{
    node *withop;
    ids *lhs;

    DBUG_ENTER ("MMVwith");

    /* stack lhs and withop of surrounding WL */
    lhs = INFO_MMV_LHS_WL (arg_info);
    withop = INFO_MMV_WITHOP (arg_info);

    INFO_MMV_LHS_WL (arg_info) = INFO_MMV_LHS (arg_info);

    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    INFO_MMV_WITHOP (arg_info) = withop;
    INFO_MMV_LHS_WL (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith2
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
MMVwith2 (node *arg_node, info *arg_info)
{
    node *withop;
    ids *lhs;

    DBUG_ENTER ("MMVwith2");

    /* stack lhs and withop of surrounding WL */
    lhs = INFO_MMV_LHS_WL (arg_info);
    withop = INFO_MMV_WITHOP (arg_info);

    INFO_MMV_LHS_WL (arg_info) = INFO_MMV_LHS (arg_info);
    INFO_MMV_WITHOP (arg_info) = NULL;

    if (NWITH2_WITHOP (arg_node) != NULL) {
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    if (NWITH2_SEGS (arg_node) != NULL) {
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    }

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    INFO_MMV_WITHOP (arg_info) = withop;
    INFO_MMV_LHS_WL (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwithop
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVwithop");

    if (INFO_MMV_WITHOP (arg_info) == NULL) {
        INFO_MMV_WITHOP (arg_info) = arg_node;
    }

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        if (NWITHOP_SHAPE (arg_node) != NULL) {
            NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        }
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        if (NWITHOP_MEM (arg_node) != NULL) {
            InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                             ID_NAME (NWITHOP_MEM (arg_node)));

            InsertIntoLUT_P (INFO_MMV_LUT (arg_info),
                             IDS_VARDEC (INFO_MMV_LHS (arg_info)),
                             ID_VARDEC (NWITHOP_MEM (arg_node)));
        }

        break;

    case WO_modarray:
        if (NWITHOP_ARRAY (arg_node) != NULL) {
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        }
        if (NWITHOP_MEM (arg_node) != NULL) {
            InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                             ID_NAME (NWITHOP_MEM (arg_node)));

            InsertIntoLUT_P (INFO_MMV_LUT (arg_info),
                             IDS_VARDEC (INFO_MMV_LHS (arg_info)),
                             ID_VARDEC (NWITHOP_MEM (arg_node)));
        }
        break;

    case WO_foldprf:
    case WO_foldfun:
        if (NWITHOP_NEUTRAL (arg_node) != NULL) {
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;

    case WO_unknown:
        DBUG_ASSERT ((0), "Unknown withop type found!");
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        INFO_MMV_LHS (arg_info) = IDS_NEXT (INFO_MMV_LHS (arg_info));
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVcode
 *
 *  @brief Substitutes a CEXPR reference with the corresponding reference
 *         on LHS of current withloop if the corresponding WL operation
 *         is fold.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVcode (node *arg_node, info *arg_info)
{
    node *cexprs, *withop, *cexpr, *nassign;
    ids *_ids;

    DBUG_ENTER ("AACCcode");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (NCODE_CEXPRS (arg_node) != NULL) {
        NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);
    }

    if (eacc) {

        /* A,B = with(iv)
         *        gen:{res1 = ...;
         *             res2 = ...;
         *            }:res1,res2
         *       fold( op1, n1)
         *            ...
         *
         *     rename cexpr: res1 -> A
         *     add assign  : A = res1
         *
         */

        _ids = INFO_MMV_LHS_WL (arg_info);
        cexprs = NCODE_CEXPRS (arg_node);
        withop = INFO_MMV_WITHOP (arg_info);
        nassign = NULL;

        while (withop != NULL) {
            if (NWITHOP_IS_FOLD (withop)) {
                DBUG_ASSERT ((_ids != NULL), "ids is missing");
                cexpr = EXPRS_EXPR (cexprs);
                DBUG_ASSERT ((cexpr != NULL), "CEXPR is missing");
                DBUG_ASSERT ((NODE_TYPE (cexpr) == N_id), "CEXPR is not a N_id");

                if (IDS_VARDEC (_ids) != ID_VARDEC (cexpr)) {
                    nassign
                      = MakeAssign (MakeLet (DupNode (cexpr), DupOneIds (_ids)), nassign);
                    BLOCK_INSTR (NCODE_CBLOCK (arg_node))
                      = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)), nassign);

                    EXPRS_EXPR (cexprs) = FreeNode (EXPRS_EXPR (cexprs));
                    EXPRS_EXPR (cexprs) = DupIds_Id (_ids);
                }
            }

            _ids = IDS_NEXT (_ids);
            cexprs = EXPRS_NEXT (cexprs);
            withop = NWITHOP_NEXT (withop);
        }
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
 * @fn MarkMemVals
 *
 *  @brief Starting function of MarkMemVals traversal
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree
 *
 ***************************************************************************/
node *
MarkMemVals (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MarkMemVals");

    info = MakeInfo ();

    act_tab = mmv_tab;

    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
