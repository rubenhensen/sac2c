/**
 *
 * $Id$
 *
 * @defgroup wlsimp With-loop simplification
 * @ingroup opt
 *
 * <pre>
 * </pre>
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

/**
 * INFO structure
 */
struct INFO {
    bool onefundef;

    /*
     * elements for identifying empty generators
     */
    bool emptypart;
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

    if (!INFO_ONEFUNDEF (arg_info)) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
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

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

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
    DBUG_ENTER ("WLSIMPwith");

    if (!(TUshapeKnown (IDS_NTYPE (WITH_VEC (arg_node)))
          && (SHgetUnrLen (TYgetShape (IDS_NTYPE (WITH_VEC (arg_node)))) == 0))) {

        INFO_WITH (arg_info) = arg_node;
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        if (WITH_PART (arg_node) == NULL) {
            node *new_node = NULL;
            switch (NODE_TYPE (WITH_WITHOP (arg_node))) {
            case N_genarray:
                /*
                 * TODO: FIX ME!
                 *
                 * Genarray with-loops without oarts should be replaced with
                 * reshape( cat( shp, shape(def)), (:basetype)[])
                 *
                 * This is currently not possible as the basetype of [] cannot be
                 * preserved in the NTC.
                 */
                DBUG_ASSERT ((FALSE), "This must never happen. See comment!");
                break;

            case N_modarray:
                new_node = DUPdoDupNode (MODARRAY_ARRAY (WITH_WITHOP (arg_node)));
                break;

            case N_fold:
                new_node = DUPdoDupNode (FOLD_NEUTRAL (WITH_WITHOP (arg_node)));
                break;

            default:
                DBUG_ASSERT (FALSE, "Illegal withop!");
                break;
            }

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = new_node;
        }
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
    node *preass;

    DBUG_ENTER ("WLSIMPcode");

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    if (CODE_USED (arg_node) == 0) {
        arg_node = FREEdoFreeNode (arg_node);
    } else {
        if (CODE_CBLOCK (arg_node) != NULL) {
            preass = INFO_PREASSIGN (arg_info);
            INFO_PREASSIGN (arg_info) = NULL;
            CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
            INFO_PREASSIGN (arg_info) = preass;
        }
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

    if (PART_NEXT (arg_node) != NULL) {
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
    }

    INFO_EMPTYPART (arg_info) = FALSE;

    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (INFO_EMPTYPART (arg_info)) {
        /*
         * TODO: FIX ME
         *
         * Do not delete last part of genarray with-loop. See comment above!
         */
        if (!((NODE_TYPE (WITH_WITHOP (INFO_WITH (arg_info))) == N_genarray)
              && (WITH_PART (INFO_WITH (arg_info)) == arg_node)
              && (PART_NEXT (arg_node) == NULL))) {
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSIMPgenerator( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPgenerator (node *arg_node, info *arg_info)
{
    node *lb, *ub, *width;
    constant *cnst;

    DBUG_ENTER ("WLSIMPgenerator");

    /**
     * Remove empty generators
     *
     * First, we check the lower and upper bounds
     */
    lb = GENERATOR_BOUND1 (arg_node);
    ub = GENERATOR_BOUND2 (arg_node);

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
