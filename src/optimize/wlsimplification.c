/*
 *
 * $Log$
 * Revision 1.3  2005/09/13 15:16:11  ktr
 * with-loops with empty generators are noe replaced by simpler
 * representations. Unfortunately, this is not yet possible with
 * genarray with-loops. (FIX ME!)
 *
 * Revision 1.2  2005/09/12 17:43:14  ktr
 * removed PRTdoPrintNode
 *
 * Revision 1.1  2005/09/12 13:56:13  ktr
 * Initial revision
 *
 */

/**
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
#include "internal_lib.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "type_utils.h"
#include "shape.h"
#include "DupTree.h"
#include "constants.h"

/**
 * INFO structure
 */
struct INFO {
    bool emptypart;
    node *with; /* Needed as long as the [] problem is not ironed out */
};

/**
 * INFO macros
 */
#define INFO_EMPTYPART(n) (n->emptypart)
#define INFO_WITH(n) (n->with)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_EMPTYPART (result) = FALSE;
    INFO_WITH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
 * @fn node *WLSIMPfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
WLSIMPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSIMPfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
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
    DBUG_ENTER ("WLSIMPcode");

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    if (CODE_USED (arg_node) == 0) {
        arg_node = FREEdoFreeNode (arg_node);
    } else {
        if (CODE_CBLOCK (arg_node) != NULL) {
            CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
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
              && (WITH_PART (INFO_WITH (arg_info)) == arg_node))) {
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
    node *lb, *ub;

    DBUG_ENTER ("WLSIMPgenerator");

    lb = GENERATOR_BOUND1 (arg_node);
    ub = GENERATOR_BOUND2 (arg_node);

    if ((NODE_TYPE (lb) == N_id) && (NODE_TYPE (ub) == N_id)) {

        if (ID_AVIS (lb) == ID_AVIS (ub)) {
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

    DBUG_RETURN (arg_node);
}

/* @} */
