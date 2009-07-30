/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup dro Data Reuse Optimization
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file datareuse.c
 *
 * Prefix: EMDR
 *
 *****************************************************************************/
#include "datareuse.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *lhs;
    lut_t *reuselut;
    node *predavis;
    node *memavis;
    node *rcavis;
    node *iv;
    node *ivids;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REUSELUT(n) ((n)->reuselut)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_MEMAVIS(n) ((n)->memavis)
#define INFO_RCAVIS(n) ((n)->rcavis)
#define INFO_IV(n) ((n)->iv)
#define INFO_IVIDS(n) ((n)->ivids)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_LHS (result) = NULL;
    INFO_REUSELUT (result) = NULL;
    INFO_PREDAVIS (result) = NULL;
    INFO_MEMAVIS (result) = NULL;
    INFO_RCAVIS (result) = NULL;
    INFO_IV (result) = NULL;
    INFO_IVIDS (result) = NULL;

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
 * @fn node *EMDRdoDataReuse( node *syntax_tree)
 *
 * @brief starting point of DataReuseOptimization
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMDRdoDataReuse (node *syntax_tree)
{
    DBUG_ENTER ("EMDRdoDataReuse");

    TRAVpush (TR_emdr);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EMDRap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRap");

    /*
     * CONDFUNs are traversed in order of appearance
     */
    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {

        if (STRsub ("ReuseCond", FUNDEF_NAME (AP_FUNDEF (arg_node)))) {
            /*
             * Transform predavis, memavis and rcavis before traversing REUSECOND
             */
            node *funargs, *apargs;
            funargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            apargs = AP_ARGS (arg_node);

            while (apargs != NULL) {

                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_PREDAVIS (arg_info)) {
                    INFO_PREDAVIS (arg_info) = ARG_AVIS (funargs);
                }
                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_MEMAVIS (arg_info)) {
                    INFO_MEMAVIS (arg_info) = ARG_AVIS (funargs);
                }
                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_RCAVIS (arg_info)) {
                    INFO_RCAVIS (arg_info) = ARG_AVIS (funargs);
                }

                apargs = EXPRS_NEXT (apargs);
                funargs = ARG_NEXT (funargs);
            }
        }
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRassign");

    /*
     * Top-down traversal
     */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRcond (node *arg_node, info *arg_info)
{
    lut_t *oldlut;

    DBUG_ENTER ("EMDRcond");

    oldlut = INFO_REUSELUT (arg_info);
    INFO_REUSELUT (arg_info) = LUTduplicateLut (oldlut);

    if ((NODE_TYPE (COND_COND (arg_node)) == N_id)
        && (ID_AVIS (COND_COND (arg_node)) == INFO_PREDAVIS (arg_info))) {
        /*
         * b = reuse( a);
         *
         * Insert (memavis, rcavis) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), INFO_MEMAVIS (arg_info),
                           INFO_RCAVIS (arg_info));
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_REUSELUT (arg_info) = LUTremoveLut (INFO_REUSELUT (arg_info));
    INFO_REUSELUT (arg_info) = oldlut;

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRwithid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRwithid");

    INFO_IV (arg_info) = WITHID_VEC (arg_node);
    INFO_IVIDS (arg_info) = WITHID_IDS (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRwith (node *arg_node, info *arg_info)
{
    node *oldivs, *oldiv;

    DBUG_ENTER ("EMDRwith");

    oldiv = INFO_IV (arg_info);
    oldivs = INFO_IVIDS (arg_info);

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    INFO_IVIDS (arg_info) = oldivs;
    INFO_IV (arg_info) = oldiv;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRwith2 (node *arg_node, info *arg_info)
{
    node *oldivs, *oldiv;

    DBUG_ENTER ("EMDRwith2");

    oldiv = INFO_IV (arg_info);
    oldivs = INFO_IVIDS (arg_info);

    WITH2_WITHID (arg_node) = TRAVopt (WITH2_WITHID (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);

    INFO_IVIDS (arg_info) = oldivs;
    INFO_IV (arg_info) = oldiv;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRcode (node *arg_node, info *arg_info)
{
    node *exprs;
    pattern *pat;

    DBUG_ENTER ("EMDRcode");

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    /*
     * The great moment:
     * check whether CEXPRS perform INPLACE-COPY-OPERATIONS
     */
    exprs = CODE_CEXPRS (arg_node);
    while (exprs != NULL) {
        node *id = NULL;
        node *idx = NULL;
        bool inplace = FALSE;

        id = EXPRS_EXPR (exprs);

        if (AVIS_SSAASSIGN (ID_AVIS (id)) != NULL) {
            node *wlass = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (id)));
            node *aexprs = NULL;
            node *iv = NULL;
            node *mem = NULL;

            if ((NODE_TYPE (wlass) == N_prf) && (PRF_PRF (wlass) == F_wl_assign)) {
                node *val;
                node *valavis;

                val = PRF_ARG1 (wlass);
                mem = PRF_ARG2 (wlass);
                iv = PRF_ARG3 (wlass);
                idx = PRF_ARG4 (wlass);

                valavis = ID_AVIS (val);

                if ((AVIS_SSAASSIGN (valavis) != NULL)
                    && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (valavis))) == N_prf)
                    && (PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (valavis))) == F_fill)) {
                    node *sel = PRF_ARG1 (ASSIGN_RHS (AVIS_SSAASSIGN (valavis)));

                    /*
                     * Pattern:
                     *
                     * a = fill( B[iv], a');
                     * r = wl_assign( a, A', iv, idx);
                     *
                     * where A' is known to be a reuse of B
                     */
                    if ((NODE_TYPE (sel) == N_prf) && (PRF_PRF (sel) == F_sel_VxA)) {
                        node *vec = PRF_ARG1 (sel);
                        node *arr = PRF_ARG2 (sel);

                        if ((ID_AVIS (iv) == ID_AVIS (vec))
                            && (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                                == ID_AVIS (arr))) {
                            inplace = TRUE;
                        }
                    }

                    /*
                     * Pattern:
                     *
                     * a = fill( idx_sel( idx, B), a');
                     * r = wl_assign( a, A', iv, idx);
                     *
                     * where A' is known to be a reuse of B
                     */
                    if ((NODE_TYPE (sel) == N_prf) && (PRF_PRF (sel) == F_idx_sel)) {
                        node *selidx = PRF_ARG1 (sel);
                        node *arr = PRF_ARG2 (sel);

                        if ((ID_AVIS (idx) == ID_AVIS (selidx))
                            && (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                                == ID_AVIS (arr))) {
                            inplace = TRUE;
                        }
                    }
                }
            }

            /*
             * Pattern:
             *
             * a' = suballoc( A', _)
             * r = fill( [v1, ..., vn], a')
             *
             * where A' is known to be a reuse of B
             * and the vi are defined as
             *
             * vi = fill( idx_sel( idxi, B), vi');
             * idxi = idxs2offset( _, iv1, ..., ivn, i)
             *
             * the shape (arg 1) is by construction the
             * shape of A and B
             */
            pat = PMprf (1, PMAisPrf (F_fill), 2,
                         PMarray (0, 1, PMskip (1, PMAgetNode (&aexprs))),
                         PMprf (1, PMAisPrf (F_suballoc), 2,
                                PMvar (1, PMAgetNode (&mem), 0), PMskip (0)));
            if (PMmatchFlat (pat, wlass)) {
                node *expr;
                node *arr = NULL;
                bool iscopy = TRUE;
                int pos = 0;

                DBUG_PRINT ("EMDR", ("vector copy: potential candiate found."));

                while ((aexprs != NULL) && iscopy) {
                    expr = EXPRS_EXPR (aexprs);

                    if (!PMO (PMOvar (
                          &arr,
                          PMOany (
                            NULL,
                            PMOnumVal (
                              pos,
                              PMOpartExprs (
                                INFO_IVIDS (arg_info),
                                PMOany (NULL,
                                        PMOprf (F_idxs2offset,
                                                PMOprf (F_fill,
                                                        PMOprf (F_idx_sel,
                                                                PMOprf (F_fill,
                                                                        expr))))))))))) {
                        iscopy = FALSE;
#ifndef DBUG_OFF
                    } else {
                        DBUG_PRINT ("EMDR", ("vector copy: element %d fits.", pos));
#endif
                    }

                    aexprs = EXPRS_NEXT (aexprs);
                    pos++;
                }

#ifndef DBUG_OFF
                if (iscopy) {
                    DBUG_PRINT ("EMDR", ("vector copy expression found"));
                }
#endif
                if (iscopy
                    && (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                        == ID_AVIS (arr))) {
                    DBUG_PRINT ("EMDR", ("vector copy: reuse identified."));

                    inplace = TRUE;
                }
            }

            /*
             * Pattern:
             * mem = suballoc( A, _);
             * r = with2/with
             *       ( _ <= ivi=[ii1, ..., iim] < _) : r_inner;
             *     genarray( _, _, mem)
             *
             * with only 1 partition (i.e., one full partition)
             * which selects is defined as
             *
             * r_inner = sel( iv ++ ivi, B)
             *
             * - or -
             *
             * r_inner = sel( [i1, ..., in, ii1, ..., iim], B)
             *
             * and A is a reuse of B
             *
             * The sel operations have been processed by vect2offset!
             */
            if ((NODE_TYPE (wlass) == N_with) || (NODE_TYPE (wlass) == N_with2)) {
                node *withop, *wlids, *wliv, *code;
                bool iscopy = FALSE;

                withop = WITH_OR_WITH2_WITHOP (wlass);
                wlids = WITH_OR_WITH2_IDS (wlass);
                wliv = WITH_OR_WITH2_VEC (wlass);
                code = WITH_OR_WITH2_CODE (wlass);

                if ((NODE_TYPE (withop) == N_genarray) && (GENARRAY_NEXT (withop) == NULL)
                    && (CODE_NEXT (code) == NULL)) {
                    node *cexpr = EXPRS_EXPR (CODE_CEXPRS (code));
                    node *offset = NULL;
                    node *arr = NULL;
                    node *mem = NULL;

                    DBUG_PRINT ("EMDR", ("wl copy: potential candiate found."));

                    if (PMO (PMOvar (&arr, PMOvar (&offset,
                                                   PMOprf (F_idx_sel,
                                                           PMOprf (F_fill,
                                                                   PMOprf (F_wl_assign,
                                                                           cexpr))))))) {
                        node *cat_arg1 = NULL, *cat_arg2 = NULL;

                        /*
                         * offset can be defined as an idxs2offset or a vect2offset
                         * the shape does not matter, as we select from a reuse
                         * candidate which has the same shape.
                         */
                        if ((wlids != NULL) && (INFO_IVIDS (arg_info) != NULL)
                            && PMO (
                                 PMOexprs (&wlids,
                                           PMOpartExprs (
                                             INFO_IVIDS (arg_info),
                                             PMOany (NULL, PMOprf (F_idxs2offset,
                                                                   PMOprf (F_fill,
                                                                           offset))))))) {

                            DBUG_PRINT ("EMDR", ("wl copy: inner sel is scalar copy."));
                            iscopy = TRUE;
                        }

                        /*
                         * or it can be a concatenation of the two ivs
                         */
                        else if (
                          PMO (PMOvar (
                            &cat_arg2,
                            PMOvar (
                              &cat_arg2,
                              PMOprf (F_cat_VxV,
                                      PMOprf (F_fill,
                                              PMOany (NULL,
                                                      PMOprf (F_vect2offset,
                                                              PMOprf (F_fill,
                                                                      offset))))))))) {
                            /*
                             * arg_1/2 can be iv or [ivids]
                             */
                            if ((((INFO_IV (arg_info) != NULL)
                                  && PMO (PMOvar (&INFO_IV (arg_info), cat_arg1)))
                                 || ((INFO_IVIDS (arg_info) != NULL)
                                     && PMO (PMOexprs (&INFO_IVIDS (arg_info),
                                                       PMOarray (NULL, NULL, cat_arg1)))))
                                && (((wliv != NULL) && PMO (PMOvar (&wliv, cat_arg2)))
                                    || ((wlids != NULL)
                                        && (PMO (
                                             PMOexprs (&wlids, PMOarray (NULL, NULL,
                                                                         cat_arg2))))))) {

                                DBUG_PRINT ("EMDR",
                                            ("wl copy: inner sel is vector copy."));
                                iscopy = TRUE;
                            }
                        }
                    }

                    if (iscopy
                        && PMO (PMOvar (&mem, PMOprf (F_suballoc, GENARRAY_MEM (withop))))
                        && (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                            == ID_AVIS (arr))) {
                        DBUG_PRINT ("EMDR", ("wl copy: reuse identified."));

                        inplace = TRUE;
                    }
                }
            }
        }

        if (inplace) {
            node *avis;

            DBUG_PRINT ("EMDR", ("Inplace copy situation recognized!"));

            /*
             * Create a variable for new cexpr
             */
            avis = TBmakeAvis (TRAVtmpVar (), TYeliminateAKV (AVIS_TYPE (ID_AVIS (id))));

            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

            /*
             * Create noop
             * a = noop( iv);
             */
            CODE_CBLOCK_INSTR (arg_node)
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                         TCmakePrf1 (F_noop,
                                                     DUPdoDupNode (INFO_IV (arg_info)))),
                              CODE_CBLOCK_INSTR (arg_node));

            AVIS_SSAASSIGN (avis) = CODE_CBLOCK_INSTR (arg_node);

            EXPRS_EXPR (exprs) = FREEdoFreeNode (EXPRS_EXPR (exprs));
            EXPRS_EXPR (exprs) = TBmakeId (avis);
        }

        exprs = EXPRS_NEXT (exprs);
    }

    /*
     * Traverse next code
     */
    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRfundef");

    DBUG_PRINT ("EMDR", ("Traversing function %s", FUNDEF_NAME (arg_node)));

    /*
     * CONDFUNs may only be traversed from AP-nodes
     */
    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;

            DBUG_PRINT ("EMDR", ("Traversing function body %s", FUNDEF_NAME (arg_node)));
            info = MakeInfo (arg_node);

            if (arg_info != NULL) {
                INFO_PREDAVIS (info) = INFO_PREDAVIS (arg_info);
                INFO_MEMAVIS (info) = INFO_MEMAVIS (arg_info);
                INFO_RCAVIS (info) = INFO_RCAVIS (arg_info);
            }

            INFO_REUSELUT (info) = LUTgenerateLut ();

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            INFO_REUSELUT (info) = LUTremoveLut (INFO_REUSELUT (info));

            info = FreeInfo (info);
        }
    }

    /*
     * Traverse next fundef
     */
    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRprf");

    switch (PRF_PRF (arg_node)) {
    case F_reuse:
        /*
         * b = reuse( a);
         *
         * Insert (b, a) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (PRF_ARG1 (arg_node)));
        break;

    case F_reshape_VxA:
        /*
         * b = reshape( dim, shp, a);
         *
         * Insert (b, a) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (PRF_ARG3 (arg_node)));
        break;

    case F_fill:
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_prf) {
            /*
             * c = fill( PRF, b);
             */
            node *prf = PRF_ARG1 (arg_node);
            switch (PRF_PRF (prf)) {
            case F_copy:
                /*
                 * c = fill( copy( a), b);
                 *
                 * If ( b, a) is in REUSELUT:
                 *   convert copy( a) into noop( a)
                 */
                if (LUTsearchInLutPp (INFO_REUSELUT (arg_info),
                                      ID_AVIS (PRF_ARG2 (arg_node)))
                    == ID_AVIS (PRF_ARG1 (prf))) {
                    DBUG_PRINT ("EMDR", ("Inplace copy situation recognized!"));
                    PRF_PRF (prf) = F_noop;
                }
                break;

            case F_isreused:
                /*
                 * c = fill( isreused( mem, rc), c')
                 *
                 * put ( c, mem, rc) into ( predavis, memavis, rcavis)
                 */
                INFO_PREDAVIS (arg_info) = IDS_AVIS (INFO_LHS (arg_info));
                INFO_MEMAVIS (arg_info) = ID_AVIS (PRF_ARG1 (prf));
                INFO_RCAVIS (arg_info) = ID_AVIS (PRF_ARG2 (prf));
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Data Reuse Optimisation -->
 *****************************************************************************/
