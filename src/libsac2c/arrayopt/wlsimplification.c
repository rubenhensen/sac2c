/**
 *
 * @defgroup wlsimp With-loop simplification
 * @ingroup opt
 *
 * <pre>
 *
 * Terminology:
 *
 *   lb: Lower bound (GENERATOR_BOUND1) of a With-Loop generator.
 *
 *   ub: Upper bound (GENERATOR_BOUND2) of a With-Loop generator.
 *
 *   Zero-trip generator:  A generator for which 0 == product( ub - lb)
 *    NB: we name these generators zero-trip rather than empty here
 *        as we want them not to be confused with the one-trip generators
 *        that have empty bounds: ( [:int] <= iv < [:int] ).
 *        For those we have product( ub - lb) = product( []) == 1 !!!
 *
 *    IMPORTANT: we make use of the condition all( lb <= ub) here!!!
 *
 * This optimization does 2 things:
 *
 *   A) it eliminates zero-trip generators from With-Loops
 *      In case all generators are eliminated, the entire With-Loop
 *      is replaced by some appropriate alternative code (for details
 *      see below!).
 *
 *   B) it creates GENERATOR_GENWIDTH annotations
 *
 * The optimization ASSUMES that all With-Loops are full partitions
 * ( i.e., WLPG has been run prior to the optimization).
 * As a consequence, the absence of a default-partition indicates that appropriate
 * generator-partitions have been inserted instead!
 *
 *
 * A) handling zero-trip generators
 *
 * We use TULSisZeroTripGenerator from the tree_utils module here in order
 * to identify zero-trip generators. If identified, INFO_ZEROTRIP( arg_info)
 * is being set which signals WLSIMPpart to delete that partition.
 *
 * After all partitions  have been inspected, we inspect whether all
 * partitions including a potentially existing default-partition are
 * gone. If so, we initiate the following transformation while
 * traversing the operation parts:
 *
 * NB:  due to the prerequisite of having a full partition we know that
 *      the full index space is empty. As a consequence, we can make
 *      the following changes:
 *
 *   res = with {
 *         } genarray( shape, default);
 *
 *         ====>    res =  [:type(res)]
 *
 *       NB: type(res) is guaranteed to be AKS because, otherwise, we cannot
 *           statically decide that all partitions are in fact empty!
 *           If it was not AKS, either there would be a default partition,
 *           or partition generation would have inserted a second partition
 *           which depends on the unknown lower / upper limit!
 *           Well, in case we have a partition [0,0] -> [0,a], we can 
 *           decide that it is empty despite not knowing the exact shape!
 *           in that case, we generate more complex code:
 *         ====>    res = _reshape_VxA_ (_cat_VxV_ (shape, _shape_A_ (default)),
 *                                       []);
 *
 *   res = with {                    ===>     res = a;
 *         } modarray( a );
 *
 *   res = with {                    ===>     res = neutr;
 *         } fold( fun, neutr);
 *
 *  IN ALL CASES where we replace a With-Loop by an assignment, we have
 *  to adjust potential break / propagate operators accordingly!
 *  More precisely, we have to eliminate the LHSs for breaks and we
 *  have to create assignments for all propagates.
 *
 *
 *   B.  GENERATOR_GENWIDTH annotation creation:
 *
 *   This replaces empty GENWIDTH values by (ub -lb).
 *
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

#define DBUG_PREFIX "WLSIMP"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "shape.h"
#include "DupTree.h"
#include "constants.h"
#include "pattern_match.h"
#include "check.h"
#include "phase.h"
#include "ctinfo.h"
#include "with_loop_utilities.h"
#include "tree_utils.h"
#include "flattengenerators.h"

/**
 * INFO structure
 */
struct INFO {

    /*
     * elements for inserting GENERATOR_GENWIDTH
     */
    node *fundef;
    node *preassign;

    /*
     * elements for identifying empty generators
     */
    node *lhs;
    int num_genparts;
    bool zerotrip;
    bool replace;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)

#define INFO_LHS(n) ((n)->lhs)
#define INFO_NUM_GENPARTS(n) ((n)->num_genparts)

#define INFO_ZEROTRIP(n) ((n)->zerotrip)
#define INFO_REPLACE(n) ((n)->replace)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NUM_GENPARTS (result) = 0;
    INFO_ZEROTRIP (result) = FALSE;
    INFO_REPLACE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * local helper functions
 */

/** <!--********************************************************************-->
 *
 * @fn node *CreateGenwidth( node *lb_array, node *ub_array, info* arg_info)
 *
 * @brief we assume lb_array and ub_array to be N-array nodes that represent
 *        integer vectors, namely the lower and upper bound of a generator.
 *        If lb_array == [ a, b, 3] and ub_array == [7, x, 9] it creates:
 *          tmp3 = 9 - 3;
 *          tmp2 = x - b;
 *          tmp1 = 7 - a;
 *        and returns [ tmp1, tmp2, tmp3];
 *
 *        All declarations are directly inserted (using INFO_FUNDEF)
 *        and the assignments are inserted in INFO_PREASSIGN.
 *
 *****************************************************************************/
static node *
CreateGenwidth (node *lb_array, node *ub_array, info *arg_info)
{
    node *ub_exprs, *lb_exprs;
    node *exprs = NULL;
    node *diffavis;
    node *lb;
    node *ub;
    node *prfassgn;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (lb_array) == N_array, "lb should be N_array");
    DBUG_ASSERT (NODE_TYPE (ub_array) == N_array, "ub should be N_array");

    lb_exprs = ARRAY_AELEMS (lb_array);
    ub_exprs = ARRAY_AELEMS (ub_array);

    while (lb_exprs != NULL) {
        DBUG_ASSERT (ub_exprs != NULL, "upper bound shorter than lower bound!");
        diffavis = TBmakeAvis (TRAVtmpVar (),
                               TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (diffavis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        lb = FLATGexpression2Avis (DUPdoDupNode (EXPRS_EXPR (lb_exprs)),
                                   &FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                                   &INFO_PREASSIGN (arg_info),
                                   TYmakeAKS (TYmakeSimpleType (T_int),
                                              SHcreateShape (0)));

        ub = FLATGexpression2Avis (DUPdoDupNode (EXPRS_EXPR (ub_exprs)),
                                   &FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                                   &INFO_PREASSIGN (arg_info),
                                   TYmakeAKS (TYmakeSimpleType (T_int),
                                              SHcreateShape (0)));

        prfassgn = TBmakeAssign (TBmakeLet (TBmakeIds (diffavis, NULL),
                                            TCmakePrf2 (F_sub_SxS, TBmakeId (ub),
                                                        TBmakeId (lb))),
                                 NULL);
        AVIS_SSAASSIGN (diffavis) = prfassgn;
        INFO_PREASSIGN (arg_info) = TCappendAssign (INFO_PREASSIGN (arg_info), prfassgn);

        exprs = TCappendExprs (exprs, TBmakeExprs (TBmakeId (diffavis), NULL));

        lb_exprs = EXPRS_NEXT (lb_exprs);
        ub_exprs = EXPRS_NEXT (ub_exprs);
    }
    DBUG_ASSERT (ub_exprs == NULL, "upper bound longer than lower bound!");

    DBUG_RETURN (TCmakeIntVector (exprs));
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPdoWithloopSimplification( node *arg_node)
 *
 * @brief
 *
 * @return modified N_fundef or N_module node.
 *
 *****************************************************************************/
node *
WLSIMPdoWithloopSimplification (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_wlsimp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * AST traversal functions:
 */

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only functions of the module, skipping all the rest for
 *        performance reasons.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSIMPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPfundef( node *arg_node, info *arg_info)
 *
 * @brief LaC-funs: - does NOT follow N_ap at all!
 *                  - supports glf format (ie traverses all local funs when
 *                    present)
 *        sets INFO_FUNDEF while traversing the body.
 *
 *****************************************************************************/
node *
WLSIMPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPassign( node *arg_node, info *arg_info)
 *
 * @brief bottom up traversal
 *        reacts on:
 *         - INFO_REPLACE => frees the actual assignment
 *         - INFO_PREASSIGN => inserts code prior to this assignment
 *         NB: INFO_REPLACE and INFO_PREASSIGN may coexist!
 *
 *****************************************************************************/
node *
WLSIMPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_REPLACE (arg_info)) {
        DBUG_PRINT ("Freeing N_assign for %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)))));
        TUclearSsaAssign (arg_node);
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REPLACE (arg_info) = FALSE;
    }
    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        TUsetSsaAssign (arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPlet( node *arg_node, info *arg_info)
 *
 * @brief inserts INFO_LHS while traversing the rhs.
 *
 *****************************************************************************/
node *
WLSIMPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

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
    node *preass, *old_lhs;

    DBUG_ENTER ();

    DBUG_PRINT ("examining With-Loop: %s in line %zu",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), NODE_LINE (arg_node));

    INFO_NUM_GENPARTS (arg_info) = 0;
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    DBUG_PRINT ("%d parts left", INFO_NUM_GENPARTS (arg_info));

    if (INFO_NUM_GENPARTS (arg_info) == 0) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    } else {
        INFO_NUM_GENPARTS (arg_info) = 0;
    }
    /* save old lhs, because if we are traversing a N_withs, the other with-loops
     need it. The let traversal resets this to NULL afterwards anyway. */
    old_lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = NULL;

    if (!INFO_REPLACE (arg_info)) {
        preass = INFO_PREASSIGN (arg_info);
        INFO_PREASSIGN (arg_info) = NULL;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_PREASSIGN (arg_info) = preass;
    }

    INFO_LHS (arg_info) = old_lhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPgenarray( node *arg_node, info *arg_info)
 *
 * @brief  creates an assignment of the form:
 *        lhs = [:lhstype];                \
 *        where INFO_LHS == lhs            | iff lhstype is AKS!
 *        and the type of lhs is lhstype!  /
 *
 *        tmp = shp;                        \
 *        tmp2 = _shape_A_ (default);       |
 *        tmp3 = _cat_VxV_ (tmp, tmp2);     | otherwise
 *        tmp4 = [lhsscal];                 |
 *        lhs = _reshape_VxA_ (tmp3, tmp4); /
 *
 *****************************************************************************/
node *
WLSIMPgenarray (node *arg_node, info *arg_info)
{
    node *lhs, *empty;
    ntype *lhstype;
    node *avis, *avis2, *avis3, *avis4;
    node *shp, *shpdef, *cat, *rhs;

    DBUG_ENTER ();

    DBUG_PRINT ("transforming N_genarray");

    lhs = INFO_LHS (arg_info);
    lhstype = IDS_NTYPE (lhs);

    if (TUshapeKnown (lhstype)) {
        empty = TBmakeArray (TYmakeAKS (TYcopyType (TYgetScalar (lhstype)), SHmakeShape (0)),
                             SHcopyShape (TYgetShape (lhstype)), NULL);
        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhs), empty), INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (IDS_AVIS (lhs)) = INFO_PREASSIGN (arg_info);
    } else {
        avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
        avis2 = TBmakeAvis (TRAVtmpVar (), TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
        avis3 = TBmakeAvis (TRAVtmpVar (), TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0)));
        avis4 = TBmakeAvis (TRAVtmpVar (), TYmakeAKD (TYcopyType (TYgetScalar (lhstype)), 1, SHmakeShape (0)));
                            
        shp = DUPdoDupNode (GENARRAY_SHAPE (arg_node));
        shpdef = TCmakePrf1 (F_shape_A, DUPdoDupNode (GENARRAY_DEFAULT (arg_node)));
        cat = TCmakePrf2 (F_cat_VxV, TBmakeId (avis), TBmakeId (avis2));
        empty = TBmakeArray (TYmakeAKS (TYcopyType (TYgetScalar (lhstype)), SHmakeShape (0)),
                             SHcreateShape (1,0), NULL);
        rhs = TCmakePrf2 (F_reshape_VxA, TBmakeId (avis3), TBmakeId (avis4));

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhs), rhs), INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (IDS_AVIS (lhs)) = INFO_PREASSIGN (arg_info);

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis4, NULL), empty), INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (avis4) = INFO_PREASSIGN (arg_info);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis4, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis3, NULL), cat), INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (avis3) = INFO_PREASSIGN (arg_info);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis3, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis2, NULL), shpdef), INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (avis2) = INFO_PREASSIGN (arg_info);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis2, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), shp), INFO_PREASSIGN (arg_info));
        AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    }

    INFO_REPLACE (arg_info) = TRUE;

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPmodarray( node *arg_node, info *arg_info)
 *
 * @brief creates an assignment of the form:    lhs = a;
 *        where INFO_LHS == lhs
 *         and  arg_node == modarray(a)
 *
 *****************************************************************************/
node *
WLSIMPmodarray (node *arg_node, info *arg_info)
{
    node *lhs;

    DBUG_ENTER ();

    DBUG_PRINT ("transforming N_modarray");
    lhs = INFO_LHS (arg_info);
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhs),
                                 DUPdoDupNode (MODARRAY_ARRAY (arg_node))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (IDS_AVIS (lhs)) = INFO_PREASSIGN (arg_info);

    INFO_REPLACE (arg_info) = TRUE;

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    } else {
        DBUG_ASSERT (IDS_NEXT (lhs) == NULL, "lhs length does not match WLops");
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPfold( node *arg_node, info *arg_info)
 *
 * @ brief create assignment of the form    lhs = neutr
 *        where lhs comes from INFO_LHS
 *         and  arg_node == fold( op, neutr)
 *
 *****************************************************************************/
node *
WLSIMPfold (node *arg_node, info *arg_info)
{
    node *lhs;

    DBUG_ENTER ();

    DBUG_PRINT ("transforming N_fold");
    lhs = INFO_LHS (arg_info);

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhs),
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
 * @brief skip this one and the corresponding lhs entry!
 *
 *****************************************************************************/
node *
WLSIMPbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
 * @brief create assignment of the form    lhs = def
 *        where lhs comes from INFO_LHS
 *         and  arg_node == propagate( def)
 *
 *****************************************************************************/
node *
WLSIMPpropagate (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                 DUPdoDupNode (PROPAGATE_DEFAULT (arg_node))),
                      INFO_PREASSIGN (arg_info));
    DBUG_ASSERT (IDS_NEXT (LET_IDS (ASSIGN_STMT (INFO_PREASSIGN (arg_info)))) == NULL,
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
    DBUG_ENTER ();

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    arg_node = WLUTremoveUnusedCodes (arg_node);

    if (0 != arg_node) {
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPpart( node *arg_node, info *arg_info)
 *
 *
 *****************************************************************************/
node *
WLSIMPpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NUM_GENPARTS (arg_info) = INFO_NUM_GENPARTS (arg_info) + 1;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (INFO_ZEROTRIP (arg_info)) {
        DBUG_PRINT ("eliminating zero-trip generator");
        /**
         * The following free implicitly decrements CODE_USED.
         * We therefore, rely on a code traversal AFTER this partition
         * traversal!
         */
        arg_node = FREEdoFreeNode (arg_node);
        INFO_ZEROTRIP (arg_info) = FALSE;
        INFO_NUM_GENPARTS (arg_info) = INFO_NUM_GENPARTS (arg_info) - 1;
        global.optcounters.wlsimp_wl++;
    }

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
    node *array;
    node *width;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));

    lb = GENERATOR_BOUND1 (arg_node);
    if (PMmatchFlat (pat, lb)) { /* propagate N_array if found */
        lb = array;
    }

    ub = GENERATOR_BOUND2 (arg_node);
    if (PMmatchFlat (pat, ub)) { /* propagate N_array if found */
        ub = array;
    }

    width = GENERATOR_WIDTH (arg_node);
    if (width != NULL) {
        if (PMmatchFlat (pat, width)) { /* propagate N_array if found */
            width = array;
        }
    }

    pat = PMfree (pat);

    INFO_ZEROTRIP (arg_info) = TULSisZeroTripGenerator (lb, ub, width);

    if (global.optimize.douip && (GENERATOR_GENWIDTH (arg_node) == NULL)
        && (NODE_TYPE (lb) == N_array) && (NODE_TYPE (ub) == N_array)) {

        /**
         * This uses INFO_FUNDEF to insert vardecs AND
         * it puts some stuff in INFO_PREASSIGN which needs to go before this
         * With-Loop!
         */
        GENERATOR_GENWIDTH (arg_node) = CreateGenwidth (lb, ub, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
