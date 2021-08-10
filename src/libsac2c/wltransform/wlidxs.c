#include "tree_basic.h"

#define DBUG_PREFIX "WLIDX"
#include "debug.h"

#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "DupTree.h"
#include "free.h"
#include "wlidxs.h"

/*
 * INFO structure
 */
struct INFO {
    node *topblock;
    node *lhs;
    node *withop;
    node *withid;
};

/*
 * INFO macros
 */
#define INFO_TOPBLOCK(n) ((n)->topblock)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHOP(n) ((n)->withop)
#define INFO_WITHID(n) ((n)->withid)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TOPBLOCK (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_WITHID (result) = NULL;

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
 *
 * @defgroup wlidx WLIDX
 * @ingroup compile
 *
 * @{
 */

/**
 *
 * @file wlidxs.c
 *
 *  This traversal annotates with-loops with declarations of the offset
 *  variables that will be used to iterate over the newly created arrays.
 *  This index can be exploited by index vector elimination to replace
 *  selections S[expr(iv)] with idx_sel operations if S is known to have the
 *  same shape as the array generated by the with-loop using iv.
 *
 *  Example:
 *  A,B,C = with ( iv)
 *            ( lb <= iv <= ub): expr1, expr2, expr3;
 *          genarray( shp, 0),
 *          fold( op, n),
 *          modarray( C');
 *
 *  will be extended to
 *
 *  A,B,C = with ( iv)
 *            ( lb <= iv (IDXS:wlidx_A,wlidx_C) <= ub): expr1, expr2, expr3;
 *          genarray( shp, 0), // IDX: wlidx_A
 *          fold( op, n),
 *          modarray( C');     // IDX: wlidx_C
 *
 *  Remarks:
 *   - Due to with-loop fusion, offset variables can be shared if two or
 *     or more result arrays are known to have equal shapes.
 *     In the above example, this would mean we only need wlidx_A if
 *     shape(A) == shape(C).
 *   - Fold operators are not augmented with an offset as they do not induce
 *     a canonical iteration order. However, if fold with-loops have been
 *     fused with genarray/modarray with-loops it is likely their selections
 *     can use one of the existing offsets.
 *
 */

/**
 *
 * @name Some utility functions:
 *
 * @{
 */

/*@}*/

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for WLIDX:
 *
 * @{
 ****************************************************************************/
/** <!--*******************************************************************-->
 *
 * @fn node *WLIDXfundef( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
WLIDXfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *WLIDXlet( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
WLIDXlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *WLIDXwith( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
WLIDXwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITHID (arg_info) = NULL;
    INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *WLIDXwithid( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
WLIDXwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_WITHID (arg_info) == NULL) {
        node *withop, *ids, *idsidx;

        withop = INFO_WITHOP (arg_info);
        ids = INFO_LHS (arg_info);
        idsidx = NULL;

        while (ids != NULL) {
            if ((NODE_TYPE (withop) == N_genarray)
                || (NODE_TYPE (withop) == N_modarray)) {
                node *avis;
                node *idxs, *ids2;

                /*
                 * Try to reuse one of the already existing index variables.
                 * To do this, we need pointers to the wlidx's and their corresponding
                 * lhs variables (as these do hold the result types).
                 * We hold these in 'idxs' and in 'ids2', respectively.
                 * Originally, ids2 was simply using INFO_LHS. However, that is not 
                 * possible as issue 2270 demonstrates! The problem is that idxs can be shorter
                 * than INFO_LHS in case of re-uses! We can only guarantee a 1-1 correspondence
                 * between the two if we create a new ids-chain for ids2 that leaves out
                 * duplicates.
                 */
                DBUG_PRINT ("trying to identify suitable index variable for lhs \"%s\"",
                            IDS_NAME (ids));
                avis = NULL;
                idxs = WITHID_IDXS (arg_node);
                ids2 = idsidx;

                while ((ids2 != NULL) && (avis == NULL)) {
                    ntype *t1, *t2;

                    t1 = IDS_NTYPE (ids);
                    t2 = IDS_NTYPE (ids2);

                    if (TUshapeKnown (t1) && TUshapeKnown (t2)
                        && SHcompareShapes (TYgetShape (t1), TYgetShape (t2))) {
                        DBUG_PRINT ("   re-using idx variable of lhs \"%s\": \"%s\"",
                                    IDS_NAME (ids2), IDS_NAME (idxs));
                        avis = IDS_AVIS (idxs);
                    } else {
                        DBUG_PRINT ("   skipping idx variable of lhs \"%s\": \"%s\"",
                                    IDS_NAME (ids2), IDS_NAME (idxs));
                    }
                    idxs = IDS_NEXT (idxs);
                    ids2 = IDS_NEXT (ids2);
                }

                /*
                 * If no matching offset variable was found, build a new one
                 */
                if (avis == NULL) {
                    node *vardec;

                    avis = TBmakeAvis (TRAVtmpVarName (IDS_NAME (ids)),
                                       TYmakeAKS (TYmakeSimpleType (T_int),
                                                  SHmakeShape (0)));
                    DBUG_PRINT ("   creating new idx variable \"%s\"", AVIS_NAME (avis));

                    vardec
                      = TBmakeVardec (avis, BLOCK_VARDECS (INFO_TOPBLOCK (arg_info)));
                    BLOCK_VARDECS (INFO_TOPBLOCK (arg_info)) = vardec;

                    WITHID_IDXS (arg_node)
                      = TCappendIds (WITHID_IDXS (arg_node), TBmakeIds (avis, NULL));

                    idsidx = TCappendIds (idsidx, TBmakeIds (IDS_AVIS (ids), NULL));
                }

                if (NODE_TYPE (withop) == N_genarray) {
                    GENARRAY_IDX (withop) = avis;
                } else {
                    MODARRAY_IDX (withop) = avis;
                }
            }

            withop = WITHOP_NEXT (withop);
            ids = IDS_NEXT (ids);
        }

        INFO_WITHID (arg_info) = arg_node;
        if (idsidx != NULL) {
            idsidx = FREEdoFreeTree (idsidx);
        }
    } else {
        WITHID_IDXS (arg_node) = DUPdoDupTree (WITHID_IDXS (INFO_WITHID (arg_info)));
    }
    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *WLIDXdoAnnotateWithloopIdxs( node *arg_node)
 *
 ****************************************************************************/
node *
WLIDXdoAnnotateWithloopIdxs (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_wlidx);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/*@}*/
/*@}*/

#undef DBUG_PREFIX
