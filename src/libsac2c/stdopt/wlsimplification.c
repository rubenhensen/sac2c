/**
 *
 * $Id$
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
 *   Empty generator:  A generator for which 0 == product( ub - lb)
 *    Perhaps we should denote these as zero-trip generators,
 *    since the term "empty generator" is ambiguous.
 *    NB. we make use of the condition all( lb <= ub) here!!!
 *
 *   One-trip generator: A generator for which 1 == product( ub - lb).
 *    NB. The generator  ( [:int] <= iv < [:int] ) is a one-trip
 *        generator.
 *
 * This optimization does 3 things:
 *
 *   A) it eliminates empty generators from With-Loops
 *      In case all generators are eliminated, the entire With-Loop
 *      is being replaced by some appropriate alternative code (for details
 *      see below!).
 *
 *   B) it creates GENERATOR_GENWIDTH annotations
 *
 *   C) it eliminates one-trip With-Loops.
 *
 * The optimization ASSUMES that all With-Loops are full partitions
 * ( i.e., WLPG has been run prior to the optimization),
 * AND that WLFS has not yet been run!
 * As a consequence, all With-Loops need to be of the form:
 *    with {
 *      [ (.... ) : expr; ] +
 *      [ default: expr; ]
 *    }  ( genarray(...) | modarray(...) | fold(...) [ break ] )
 *       [ propagate(...) ] *
 *
 * Note here, that the absence of a default-partition indicates that appropriate
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
 *
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
 *                                  [:basetype])
 *
 *   res = with {                    ===>     res = a;
 *         } modarray( a );
 *
 *   res = with {                    ===>     res = neutr;
 *         } fold( fun, neutr);
 *         FIXME: This does not work as of 2009-05-31, because of
 *         the possible presence of a non-empty CODE_CBLOCK,
 *         as occurs here:
 *
 *          lb = [:int];
 *          ub = [:int];
 *          x = with {
 *                (lb <= iv < ub) {
 *                  q = 6;
 *                  q2 = 8;
 *                  q3 = _add_SxS_(q, q2);
 *                } : q3;
 *              } : fold( -, 42  );
 *
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
 *
 *
 *   B.  GENERATOR_GENWIDTH annotation creation:
 *
 *   This replaces empty GENWIDTH values by (ub -lb).
 *
 *   C. Elimination of one-trip With-Loops:
 *      There are three cases handled here. In each case,
 *      the With-Loop body is executed exactly once.
 *
 *      1. genarray:
 *
 *         res = with {
 *                 (lb <= iv < ub) : {expr0; expr1;... exprn;}
 *                   : exprn;
 *               } : genarray( shp, def);
 *
 *         This is transformed into:
 *
 *         {expr0; expr1; ...exprn;}
 *         res =  exprn;
 *
 *      2. modarray:
 *
 *         res = with {
 *                 (lb <= iv < ub) : {expr0; expr1;... exprn;}
 *                   : exprn;
 *               } : modarray( arr);
 *
 *
 *         This is transformed into:
 *
 *         {expr0; expr1; ...exprn;}
 *         res =  exprn;
 *
 *      3. fold: Broken. See above FIXME.
 *
 *         res = with {
 *                 (lb <= iv < ub) : {expr0; expr1;... exprn;}
 *                   : exprn;
 *               } : fold( op, neut);
 *
 *         This is transformed into:
 *
 *         {expr0; expr1; ...exprn;}
 *         res =  op( neut, exprn);
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
#include "check.h"

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
    node *with;   /* Needed as long as the [] problem is not ironed out */
    node *withid; /* partition's WITH_WITHID value. If -ssaiv,
                     this will have to be reworked. */
    node *bound1; /* GENERATOR_BOUND1 */
    node *cexprs; /* partition's CODE_CEXPRS value */
    node *cblock; /* partition's CODE_CBLOCK value */
    int num_genparts;
    bool emptypart;
    bool onetrip; /* Entire WL is empty - [:int] <= iv < [:int] */
    bool replace;
    bool onefundef;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)

#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITH(n) ((n)->with)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_BOUND1(n) ((n)->bound1)
#define INFO_CEXPRS(n) ((n)->cexprs)
#define INFO_CBLOCK(n) ((n)->cblock)
#define INFO_NUM_GENPARTS(n) ((n)->num_genparts)

#define INFO_EMPTYPART(n) ((n)->emptypart)
#define INFO_ONETRIP(n) ((n)->onetrip)
#define INFO_REPLACE(n) ((n)->replace)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WITH (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_BOUND1 (result) = NULL;
    INFO_CEXPRS (result) = NULL;
    INFO_CBLOCK (result) = NULL;
    INFO_NUM_GENPARTS (result) = 0;
    INFO_EMPTYPART (result) = FALSE;
    INFO_ONETRIP (result) = FALSE;
    INFO_REPLACE (result) = FALSE;
    INFO_ONEFUNDEF (result) = TRUE;

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
    bool old_onefundef;
    DBUG_ENTER ("WLSIMPfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

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
 *  We look into the N_with only if the WITH_VEC is [:int].
 *
 *****************************************************************************/
node *
WLSIMPwith (node *arg_node, info *arg_info)
{
    node *preass;
    node *info_lhs;
    info *old_info;

    DBUG_ENTER ("WLSIMPwith");

    DBUG_PRINT ("WLSIMP",
                ("examining With-Loop: %s in line %d",
                 AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), NODE_LINE (arg_node)));

    old_info = arg_info;
    arg_info = MakeInfo ();

    INFO_ONEFUNDEF (arg_info) = INFO_ONEFUNDEF (old_info);
    INFO_FUNDEF (arg_info) = INFO_FUNDEF (old_info);
    INFO_WITH (arg_info) = arg_node;
    INFO_LHS (arg_info) = INFO_LHS (old_info);

    /* Do inner-most WLs first */
    info_lhs = INFO_LHS (arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    INFO_LHS (arg_info) = info_lhs;

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

    /* Propagate these to WLSIMPassign */
    INFO_LHS (old_info) = INFO_LHS (arg_info);
    INFO_REPLACE (old_info) = INFO_REPLACE (arg_info);
    INFO_PREASSIGN (old_info) = INFO_PREASSIGN (arg_info);
    arg_info = FreeInfo (arg_info);

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

    if (!INFO_ONETRIP (arg_info)) {
        if (NULL != GENARRAY_DEFAULT (arg_node)) {
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
             * Genarray with-loops without parts should be replaced with
             * reshape( cat( shp, shape(def)), (:basetype)[])
             *
             * This is currently not possible as the basetype of [] cannot be
             * preserved in the NTC.
             */
            DBUG_ASSERT (FALSE, "killed all gens of genarrayWL!");
        }
    }

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                 DUPdoDupNode (INFO_CEXPRS (arg_info))),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info))) = INFO_PREASSIGN (arg_info);
    INFO_REPLACE (arg_info) = TRUE;

    if (NULL != INFO_CBLOCK (arg_info)) {
        INFO_PREASSIGN (arg_info)
          = TCappendAssign (INFO_CBLOCK (arg_info), INFO_PREASSIGN (arg_info));
        INFO_CBLOCK (arg_info) = NULL;
    }

    /* For a single-trip WL, set the WITHID_VEC to the generator lower bound */
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (INFO_WITHID (arg_info), INFO_BOUND1 (arg_info)),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (IDS_AVIS (INFO_WITHID (arg_info))) = INFO_PREASSIGN (arg_info);

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
 *
 *****************************************************************************/
node *
WLSIMPmodarray (node *arg_node, info *arg_info)
{
    node *rhs;

    DBUG_ENTER ("WLSIMPmodarray");

    rhs = INFO_ONETRIP (arg_info) ? INFO_CEXPRS (arg_info) : MODARRAY_ARRAY (arg_node);

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)), DUPdoDupNode (rhs)),
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
 * For one-trip WLs, we need to produce a function to perform
 * one trip through the reduction.
 *
 *  This does not work, due to _accu(iv) stuff. FIXME
 *
 *****************************************************************************/
node *
WLSIMPfold (node *arg_node, info *arg_info)
{
    node *rhs;

    DBUG_ENTER ("WLSIMPfold");

    if (FALSE && INFO_ONETRIP (arg_info)) { /* FIXME. see above */

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                     TCmakeAp2 (DUPdoDupNode (FOLD_FUNDEF (arg_node)),
                                                DUPdoDupNode (FOLD_NEUTRAL (arg_node)),
                                                DUPdoDupNode (INFO_CEXPRS (arg_info)))),
                          INFO_PREASSIGN (arg_info));

    } else {

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LHS (arg_info)),
                                     DUPdoDupNode (rhs)),
                          INFO_PREASSIGN (arg_info));
    }

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
 *
 *****************************************************************************/
node *
WLSIMPpart (node *arg_node, info *arg_info)
{
    bool b1;
    bool b2;

    DBUG_ENTER ("WLSIMPpart");

    INFO_NUM_GENPARTS (arg_info) = INFO_NUM_GENPARTS (arg_info) + 1;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    INFO_ONETRIP (arg_info) = FALSE;
    INFO_EMPTYPART (arg_info) = FALSE;

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (INFO_EMPTYPART (arg_info) || INFO_ONETRIP (arg_info)) {

        /* Save the code block info for later, as it'll be gone by the
         * time we look into the WITH_OP */
        INFO_CEXPRS (arg_info)
          = DUPdoDupTree (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (arg_node))));
        INFO_WITHID (arg_info) = DUPdoDupTree (WITHID_VEC (PART_WITHID (arg_node)));
        INFO_BOUND1 (arg_info)
          = DUPdoDupTree (GENERATOR_BOUND1 (PART_GENERATOR (arg_node)));

        /*
         * Delete last part of genarray with-loop only if a
         * default cell exists.
         */
        b1 = (N_genarray == NODE_TYPE (WITH_WITHOP (INFO_WITH (arg_info))))
             && ((1 < INFO_NUM_GENPARTS (arg_info))
                 || (NULL != GENARRAY_DEFAULT (WITH_WITHOP (INFO_WITH (arg_info)))));

        b2 = INFO_ONETRIP (arg_info) &&
             /*FIXME - fold loops don't collapse properly */
             (N_fold != NODE_TYPE (WITH_WITHOP (INFO_WITH (arg_info))));

        if (b1 || b2) {
            DBUG_PRINT ("WLSIMP", ("eliminating generator in %s",
                                   AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)))));

            if (N_assign
                == NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node))))) {
                INFO_CBLOCK (arg_info)
                  = DUPdoDupTree (BLOCK_INSTR (CODE_CBLOCK (PART_CODE (arg_node))));
            }

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

        /* [:int] <= iv < [:int] executes the loop once. */
        if ((NULL == lb) && (NULL == ub)) {
            INFO_ONETRIP (arg_info) = TRUE;
            INFO_EMPTYPART (arg_info) = TRUE;
        }

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
