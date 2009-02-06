/**
 *
 * $Id$
 *
 * @defgroup wlsimp With-loop simplification
 * @ingroup opt
 *
 * <pre>
 * This optimization does 2 things:
 *   A) it eliminates empty generators from With-Loops
 *      In case all generators are eliminated, the entire With-Loop
 *      is being replaced by some appropriate alternative code (for details
 *      see below!).
 *
 *   B) it creates GENERATOR_GENWIDTH annotations
 *
 * It ASSUMES that all With-Loops are full partitions, i.e., WLPG
 * has been run prior to it AND that WLFS has not yet been run!
 * As a consequence, all With-Loops need to be of the form:
 *    with {
 *      [ (.... ) : expr; ] +
 *      [ default: expr; ]
 *    }  ( genarray(...) | modarray(...) | fold(...) [ break ] )
 *       [ propagate(...) ] *
 *
 * Note here, that the absense of a default-partition indicates that appropriate
 * generator-partitions have been inserted instead!
 * Note also, that we have a maximum of one genarray/modarray/fold operators!
 *
 *
 *
 * A) handling empty generator-partitions:
 *
 * The criteria for emptiness (as implemented in WLSIMPgenerator) are:
 *
 *  1) (  a <= iv <  a)    where a::int[n]  with n>0  !
 *  2) ( lb <= iv < ub)    where lb::int[n]{vec1}, ub::int[n]{vec2}
 *                               and vec1 >= vec2
 *  3) ( [l1, ..., ln] <= iv < [u1, ..., un])
 *                         where exists n>0 so that:
 *                               ln and un are the same variable
 *                               or ln >= un
 *  4) ( lb <= iv <= ub width a)
 *                         where a::int[n]{vec} and vec contains a 0
 *  5) ( lb <= iv <= ub width [v1,...,vn])
 *                         where exists i such that vi == 0
 *
 * In case one of the above criteria holds, INFO_EMPTYPART( arg_info)
 * is being set which signals WLSIMPpart to delete that partition.
 *
 * After all partitions  have been inspected, we may find 3 different
 * situations (see WLSIMPwith):
 *  1) at least one generator-partition still exists => we are done
 *
 *  2) all generator-partitions are gone
 *     AND there is no default partition:
 *
 *      due to the prerequisite of having a full partition we know that
 *      the full index space is empty. As a consequence, we can make
 *      the following changes:
 *
 *   res = with {
 *         } genarray( shape, default);
 *
 *         ====>    res =  reshape( _cat_VxV_( shape), shape( default),
                                    [:basetype])
 *
 *   res = with {                    ===>     res = a;
 *         } modarray( a );
 *
 *   res = with {                    ===>     res = neutr;
 *         } fold( fun, neutr);
 *
 *
 *  3) all generator-partitions are gone
 *     BUT there is a default partition:
 *
 *      as there is a default partition, we know that we are dealing with
 *      a degenerate genarray-WL or modarray-WL, which we can treat as
 *      follows:
 *
 *   res = with {
 *           default( iv) : default;
 *         } genarray( shape, default);
 *
 *         ====>    res = with {
 *                          ( 0 * shape <= iv < shape) : default;
 *                        } genarray( shape, default);
 *
 *   res = with {                    ===>     res = a;
 *           default( iv) : a[iv];
 *         } modarray( a );
 *
 *      Note here, that we cannot have a fold-WL here, as these never
 *      obtain default partitions!!
 *
 *  IN ALL CASES where we replace a With-Loop by an assignment, we have
 *  to adjust potential break / propagate operators accordingly!
 *  More precisely, we have to eliminate the LHSs for breaks and we
 *  have to create assignments for all propagates.
 * </pre>
 *
 * @{
 */

/**
 *
 * @file wlsimplification.c
 *
 *
 */
#include "wlsimplification.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "print.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "shape.h"
#include "DupTree.h"
#include "constants.h"
#include "globals.h"
#include "pattern_match.h"

/**
 * INFO structure
 */
struct INFO {
    bool onefundef;

    /*
     * elements for identifying empty generators
     */
    bool emptypart;
    bool default_exists;
    int num_genparts;
    node *lhs;
    bool replacement;
    node *with; /* Needed as long as the [] problem is not ironed out */

    /*
     * elements for inserting GENRATOR_GENWIDTH
     */
    node *fundef;
    node *preassign;
};

/**
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_EMPTYPART(n) ((n)->emptypart)
#define INFO_DEFAULT_EXISTS(n) ((n)->default_exists)
#define INFO_NUM_GENPARTS(n) ((n)->num_genparts)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REPLACE(n) ((n)->replacement)
#define INFO_WITH(n) ((n)->with)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = TRUE;
    INFO_EMPTYPART (result) = FALSE;
    INFO_DEFAULT_EXISTS (result) = FALSE;
    INFO_NUM_GENPARTS (result) = 0;
    INFO_REPLACE (result) = FALSE;
    INFO_WITH (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPdoWithloopSimplification( node *fundef)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
WLSIMPdoWithloopSimplification (node *fundef)
{
    info *info;

    DBUG_ENTER ("WLSIMPdoWithloopSimplification");

    info = MakeInfo ();

    TRAVpush (TR_wlsimp);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPdoWithloopSimplificationModule( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
WLSIMPdoWithloopSimplificationModule (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLSIMPdoWithloopSimplificationModule");

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = FALSE;

    TRAVpush (TR_wlsimp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    /*
     * Descend into local fundef chains regardless of whether
     * INFO_ONEFUNDEF is set.
     */
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPassign");

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_REPLACE (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REPLACE (arg_info) = FALSE;
    }
    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPwith (node *arg_node, info *arg_info)
{
    node *preass;
    DBUG_ENTER ("WLSIMPwith");

    if (!(TUshapeKnown (IDS_NTYPE (WITH_VEC (arg_node)))
          && (SHgetUnrLen (TYgetShape (IDS_NTYPE (WITH_VEC (arg_node)))) == 0))) {

        DBUG_PRINT ("WLSIMP", ("examining With-Loop in line %d", NODE_LINE (arg_node)));
        INFO_WITH (arg_info) = arg_node;

        INFO_DEFAULT_EXISTS (arg_info) = FALSE;
        INFO_NUM_GENPARTS (arg_info) = 0;
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        if (INFO_NUM_GENPARTS (arg_info) == 0) {

            WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        }

        if (!INFO_REPLACE (arg_info)) {
            preass = INFO_PREASSIGN (arg_info);
            INFO_PREASSIGN (arg_info) = NULL;
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
            INFO_PREASSIGN (arg_info) = preass;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPgenarray");

    if (INFO_DEFAULT_EXISTS (arg_info)) {
        /*
         * TODO: FIX ME!
         *
         * Here we should generate a non-default partition...
         */
        DBUG_ASSERT (FALSE, "killed all gens of genarrayWL!");
    } else {
        /*
         * TODO: FIX ME!
         *
         * Genarray with-loops without oarts should be replaced with
         * reshape( cat( shp, shape(def)), (:basetype)[])
         *
         * This is currently not possible as the basetype of [] cannot be
         * preserved in the NTC.
         */
        DBUG_ASSERT (FALSE, "killed all gens of genarrayWL!");

        INFO_REPLACE (arg_info) = TRUE;

        if (GENARRAY_NEXT (arg_node) != NULL) {
            INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPmodarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPmodarray");

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                 DUPdoDupNode (MODARRAY_ARRAY (arg_node))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info))) = INFO_PREASSIGN (arg_info);
    INFO_REPLACE (arg_info) = TRUE;

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPfold( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPfold");
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                 DUPdoDupNode (FOLD_NEUTRAL (arg_node))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info))) = INFO_PREASSIGN (arg_info);
    INFO_REPLACE (arg_info) = TRUE;

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPbreak( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPbreak");

    if (BREAK_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPpropagate( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPpropagate");

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                 DUPdoDupNode (PROPAGATE_DEFAULT (arg_node))),
                      INFO_PREASSIGN (arg_info));
    DBUG_ASSERT (IDS_NEXT (LET_IDS (ASSIGN_INSTR (INFO_PREASSIGN (arg_info)))) == NULL,
                 " DUPdoDupNode should not copy the IDS_NEXT!");
    AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info))) = INFO_PREASSIGN (arg_info);

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPcode");

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    if (CODE_USED (arg_node) == 0) {
        arg_node = FREEdoFreeNode (arg_node);
    } else {
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPpart( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPpart");

    INFO_NUM_GENPARTS (arg_info) = INFO_NUM_GENPARTS (arg_info) + 1;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    INFO_EMPTYPART (arg_info) = FALSE;

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (INFO_EMPTYPART (arg_info)) {
        /*
         * TODO: FIX ME
         *
         * Do not delete last part of genarray with-loop. The problem is the
         * potential lack of default information....
         */
        if (!((NODE_TYPE (WITH_WITHOP (INFO_WITH (arg_info))) == N_genarray)
              && (INFO_NUM_GENPARTS (arg_info) == 1))) {
            DBUG_PRINT ("WLSIMP",
                        ("eliminating generator in line %d!", NODE_LINE (arg_node)));
            arg_node = FREEdoFreeNode (arg_node);

            INFO_NUM_GENPARTS (arg_info) = INFO_NUM_GENPARTS (arg_info) - 1;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPdefault( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPdefault (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPdefault");
    INFO_DEFAULT_EXISTS (arg_info) = TRUE;
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPgenerator( node *arg_node, info *arg_info)
 *
 * @brief
 *         Generator bounds may be either N_id or N_array nodes.
 *         We attempt to find N_array nodes from the N_id nodes.
 *
 * @param WL generator
 *
 * @return modified syntax_tree.
 *
 *
 *****************************************************************************/
node *
WLSIMPgenerator (node *arg_node, info *arg_info)
{
    node *lb;
    node *ub;
    node *b = NULL;
    constant *bfs = NULL;
    node *width;
    constant *cnst;

    DBUG_ENTER ("WLSIMPgenerator");

    lb = GENERATOR_BOUND1 (arg_node);
    ub = GENERATOR_BOUND2 (arg_node);
    /*
     * If one argument is an N_id and the other is an N_array, try
     * to find an N_array node that is the predecessor of the N_id node.
     *
     */
    if ((N_id == NODE_TYPE (lb)) && (N_array == NODE_TYPE (ub))) {
        if (PM (PMarray (&bfs, &b, lb))) {
            bfs = COfreeConstant (bfs);
            lb = b;
        }
    }

    if ((N_array == NODE_TYPE (lb)) && (N_id == NODE_TYPE (ub))) {
        if (PM (PMarray (&bfs, &b, ub))) {
            bfs = COfreeConstant (bfs);
            ub = b;
        }
    }

    /**
     * Remove empty generators
     *
     * First, we check the lower and upper bounds
     */

    if ((NODE_TYPE (lb) == N_id) && (NODE_TYPE (ub) == N_id)) {

        if ((ID_AVIS (lb) == ID_AVIS (ub)) && (TUshapeKnown (ID_NTYPE (lb)))
            && (TYgetDim (ID_NTYPE (lb)) == 1)
            && (SHgetExtent (TYgetShape (ID_NTYPE (lb)), 0) > 0)) {
            INFO_EMPTYPART (arg_info) = TRUE;
        } else {
            if (TYisAKV (ID_NTYPE (lb)) && TYisAKV (ID_NTYPE (ub))) {

                constant *lt
                  = COlt (TYgetValue (ID_NTYPE (lb)), TYgetValue (ID_NTYPE (ub)));
                if (!COisTrue (lt, TRUE)) {
                    INFO_EMPTYPART (arg_info) = TRUE;
                }

                lt = COfreeConstant (lt);
            }
        }
    } else {
        DBUG_ASSERT ((NODE_TYPE (lb) == N_array) && (NODE_TYPE (ub) == N_array),
                     "Boundaries are neither only N_array nor only N_id nodes");

        lb = ARRAY_AELEMS (lb);
        ub = ARRAY_AELEMS (ub);

        while (lb != NULL) {
            node *lbelem, *ubelem;

            lbelem = EXPRS_EXPR (lb);
            ubelem = EXPRS_EXPR (ub);

            if ((NODE_TYPE (lbelem) == N_id) && (NODE_TYPE (ubelem) == N_id)
                && (ID_AVIS (lbelem) == ID_AVIS (ubelem))) {
                INFO_EMPTYPART (arg_info) = TRUE;
            } else {
                ntype *lbt, *ubt;
                lbt = NTCnewTypeCheck_Expr (lbelem);
                ubt = NTCnewTypeCheck_Expr (ubelem);

                if (TYisAKV (lbt) && TYisAKV (ubt)) {
                    constant *lt = COlt (TYgetValue (lbt), TYgetValue (ubt));
                    if (!COisTrue (lt, TRUE)) {
                        INFO_EMPTYPART (arg_info) = TRUE;
                    }

                    lt = COfreeConstant (lt);
                }

                lbt = TYfreeType (lbt);
                ubt = TYfreeType (ubt);
            }

            lb = EXPRS_NEXT (lb);
            ub = EXPRS_NEXT (ub);
        }
    }

    /**
     * Now, we check whether there exists a width vector
     * and if so, whether it contains a zero
     */
    width = GENERATOR_WIDTH (arg_node);
    if (width != NULL) {
        if (NODE_TYPE (width) == N_id) {
            if (TYisAKV (ID_NTYPE (width))
                && COisZero (TYgetValue (ID_NTYPE (width)), FALSE)) {
                INFO_EMPTYPART (arg_info) = TRUE;
            }
        } else {
            DBUG_ASSERT ((NODE_TYPE (width) == N_array),
                         "Width spec is neither N_id nor N_array");
            cnst = COaST2Constant (width);
            if (cnst != NULL) {
                if (COisZero (cnst, FALSE)) {
                    INFO_EMPTYPART (arg_info) = TRUE;
                }
                cnst = COfreeConstant (cnst);
            }
        }
    }

    /**
     * Annotate GENERATOR_GENWIDTH
     */
    if ((global.optimize.douip) && (GENERATOR_GENWIDTH (arg_node) == NULL)) {

        if ((NODE_TYPE (GENERATOR_BOUND1 (arg_node)) == N_array)
            && (NODE_TYPE (GENERATOR_BOUND2 (arg_node)) == N_array)) {
            node *ub, *lb;
            node *exprs = NULL;

            lb = ARRAY_AELEMS (GENERATOR_BOUND1 (arg_node));
            ub = ARRAY_AELEMS (GENERATOR_BOUND2 (arg_node));

            while (lb != NULL) {
                node *diffavis;
                diffavis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHmakeShape (0)));

                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (diffavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (diffavis, NULL),
                                             TCmakePrf2 (F_sub_SxS,
                                                         DUPdoDupNode (EXPRS_EXPR (ub)),
                                                         DUPdoDupNode (EXPRS_EXPR (lb)))),
                                  INFO_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (diffavis) = INFO_PREASSIGN (arg_info);

                exprs = TCappendExprs (exprs, TBmakeExprs (TBmakeId (diffavis), NULL));

                lb = EXPRS_NEXT (lb);
                ub = EXPRS_NEXT (ub);
            }

            GENERATOR_GENWIDTH (arg_node) = TCmakeIntVector (exprs);
        }
    }

    DBUG_RETURN (arg_node);
}

/* @} */
