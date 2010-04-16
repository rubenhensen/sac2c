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
    bool remove_with3;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REUSELUT(n) ((n)->reuselut)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_MEMAVIS(n) ((n)->memavis)
#define INFO_RCAVIS(n) ((n)->rcavis)
#define INFO_IV(n) ((n)->iv)
#define INFO_IVIDS(n) ((n)->ivids)
#define INFO_REMOVE_WITH3(n) ((n)->remove_with3)

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
    INFO_REMOVE_WITH3 (result) = FALSE;

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

static node *
ATravWith3 (node *arg_node, info *arg_info)
{
    bool stack;
    DBUG_ENTER ("ATravWith3");

    stack = INFO_REMOVE_WITH3 (arg_info);
    INFO_REMOVE_WITH3 (arg_info) = FALSE;

    arg_node = TRAVcont (arg_node, arg_info);

    if ((TCcountRanges (WITH3_RANGES (arg_node)) == 1)
        && (TCcountWithops (WITH3_OPERATIONS (arg_node)) == 1)
        && (INFO_REMOVE_WITH3 (arg_info))) {
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = TBmakePrf (F_noop, NULL);
    }

    INFO_REMOVE_WITH3 (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

static node *
ATravRange (node *arg_node, info *arg_info)
{
    node *assign;
    DBUG_ENTER ("ATravRange");

    arg_node = TRAVcont (arg_node, arg_info);

    assign
      = ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (RANGE_RESULTS (arg_node)))));

    if ((NODE_TYPE (LET_EXPR (assign)) == N_prf)
        && (PRF_PRF (LET_EXPR (assign)) == F_noop)) {
        INFO_REMOVE_WITH3 (arg_info) = TRUE;
    }

#if 0
  if ( NODE_TYPE( LET_EXPR( assign)) == N_with3){
    node *assign2 = ASSIGN_INSTR( AVIS_SSAASSIGN( ID_AVIS( EXPRS_EXPR( RANGE_RESULTS( WITH3_RANGES( LET_EXPR( assign)))))));
    if ( ( NODE_TYPE( LET_EXPR( assign2)) == N_prf) &&
         ( PRF_PRF( LET_EXPR( assign2)) == F_noop)){
      INFO_REMOVE_WITH3( arg_info) = TRUE;
    }
  }
#endif

    DBUG_RETURN (arg_node);
}

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
    anontrav_t cnw_trav[3] = {{N_with3, &ATravWith3}, {N_range, &ATravRange}, {0, NULL}};
    DBUG_ENTER ("EMDRdoDataReuse");

    TRAVpush (TR_emdr);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    TRAVpushAnonymous (cnw_trav, &TRAVsons);

    syntax_tree = TRAVopt (syntax_tree, MakeInfo (NULL));

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

    WITH_WITHID (arg_node) = TRAVopt (WITH_WITHID (arg_node), arg_info);
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
 * @fn node *EMDRrange( node *arg_node, info *arg_info)
 *
 * @brief Stack IV and IVIDS
 *
 *****************************************************************************/
node *
EMDRwith3 (node *arg_node, info *arg_info)
{
    node *oldivs, *oldiv;

    DBUG_ENTER ("EMDRwith3");

    oldiv = INFO_IV (arg_info);
    oldivs = INFO_IVIDS (arg_info);

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    INFO_IVIDS (arg_info) = oldivs;
    INFO_IV (arg_info) = oldiv;

    DBUG_RETURN (arg_node);
}

#if 0
static
node *GetAddAvis( node *id){
  node *res = NULL;

  DBUG_ENTER( "GetAddAvis");

  if ( NODE_TYPE( id) == N_id){
    res = TBmakeIds( ID_AVIS( id), res);
  } else {
    if ( ( NODE_TYPE( arg_node) == N_prf) &&
       ( PRF_PRF( arg_node) == F_fill) &&
       ( NODE_TYPE( PRF_ARG1( arg_node)) == N_prf) &&
       ( PRF_PRF( PRF_ARG1( arg_node)) == F_add)){
      res = 
        TCappendIds( res,
                     GetAddAvis( PRF_ARG1( PRF_PRF( PRF_ARG1( arg_node)))));
      res = 
        TCappendIds( res, 
                     GetAddAvis( PRF_ARG2( PRF_PRF( PRF_ARG1( arg_node)))));
    } else {
      res = TBmakeIds( NULL, res);
    }
  }

  DBUG_RETURN( res);
}
static
bool ValidIds( node *ids){
  bool res = FALSE;
  DBUG_ENTER( "ValidIds");

  res = IDS_AVIS( ids) != NULL;

  if ( IDS_NEXT( ids) != NULL){
    res &&= ValidIds( ids);
  }

  DBUG_RETURN( res);
}

static
bool IsWith3Indexs( node *ids){
  bool res;
  DBUG_ENTER( "IsWith3Indexs");
  
  res = AVIS_ISTHREADINDEX( 

  if ( IDS_NEXT( ids) != NULL){
    res &&= IsWith3Indexs( IDS_NEXT( ids));
  }
  
  DBUG_RETURN( res);
}
#endif

static node *
HandleCodeBlock (node *exprs, node *assigns, info *arg_info)
{
    pattern *pat;

    DBUG_ENTER ("HandleCodeBlock");

    while (exprs != NULL) {
        node *id = NULL;
        node *idx = NULL;
        bool inplace = FALSE;
        bool with3inplace = FALSE;

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
                    /*
                     * Pattern2:
                     *
                     * a = fill( idx_sel( idx2, B), a');
                     * r = wl_assign( a, A', [idx], idx);
                     *
                     * where A' is known to be a reuse of B
                     *
                     * should check
                     * idx2 and idx come from with3 index
                     * idx eqiv idx2
                     */
                    if ((NODE_TYPE (sel) == N_prf) && (PRF_PRF (sel) == F_idx_sel)) {
                        node *selidx = PRF_ARG1 (sel);
                        node *arr = PRF_ARG2 (sel);

                        if (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                            == ID_AVIS (arr)) {
                            if (ID_AVIS (idx) == ID_AVIS (selidx)) {
                                inplace = TRUE;
                            }

                            if (global.backend == BE_mutc) { /* fix this!!!!! */
                                inplace = with3inplace = TRUE;
                            }
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
             * with3{
             *  ( . <= i < .) : _noop_(...);
             * } ...
             */
            if (NODE_TYPE (wlass) == N_with3) {
                if ((TCcountRanges (WITH3_RANGES (wlass)) == 1)
                    && (TCcountWithops (WITH3_OPERATIONS (wlass)) == 1)
                    && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                          ID_AVIS (EXPRS_EXPR (RANGE_RESULTS (WITH3_RANGES (wlass))))))))
                        == N_prf)
                    && (PRF_PRF (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (
                          ID_AVIS (EXPRS_EXPR (RANGE_RESULTS (WITH3_RANGES (wlass))))))))
                        == F_noop)) {
                    inplace = with3inplace = TRUE;
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
            assigns
              = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                         TCmakePrf1 (F_noop, DUPdoDupNode (
                                                               with3inplace
                                                                 ? INFO_IVIDS (arg_info)
                                                                 : INFO_IV (arg_info)))),
                              assigns);

            AVIS_SSAASSIGN (avis) = assigns;

            EXPRS_EXPR (exprs) = FREEdoFreeNode (EXPRS_EXPR (exprs));
            EXPRS_EXPR (exprs) = TBmakeId (avis);
        }

        exprs = EXPRS_NEXT (exprs);
    }
    DBUG_RETURN (assigns);
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

    DBUG_ENTER ("EMDRcode");

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /*
     * The great moment:
     * check whether CEXPRS perform INPLACE-COPY-OPERATIONS
     */
    exprs = CODE_CEXPRS (arg_node);
    CODE_CBLOCK_INSTR (arg_node)
      = HandleCodeBlock (exprs, CODE_CBLOCK_INSTR (arg_node), arg_info);
    /*
     * Traverse next code
     */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRrange( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRrange (node *arg_node, info *arg_info)
{
    node *exprs;

    DBUG_ENTER ("EMDRrange");

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    /*
     * The great moment:
     * check whether CEXPRS perform INPLACE-COPY-OPERATIONS
     */
    INFO_IVIDS (arg_info) = RANGE_INDEX (arg_node);
    exprs = RANGE_RESULTS (arg_node);
    BLOCK_INSTR (RANGE_BODY (arg_node))
      = HandleCodeBlock (exprs, BLOCK_INSTR (RANGE_BODY (arg_node)), arg_info);
    /*
     * Traverse next code
     */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

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

static node *
FollowLut (node *pos, lut_t *lut)
{
    node *next = pos;
    DBUG_ENTER ("FollowLut");

    while (((next = LUTsearchInLutPp (lut, pos)) != NULL) && (pos != next))
        pos = next;

    DBUG_RETURN (pos);
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
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), INFO_LHS (arg_info),
                           ID_AVIS (PRF_ARG1 (arg_node)));
        break;

    case F_resize:
        /*
         * b = resize( dim, shp, a);
         *
         * Insert (b, a) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (PRF_ARG3 (arg_node)));
        break;

    case F_suballoc:
        /* Prove this does not break anything!!!!!!!!! */
        if (global.backend == BE_mutc) {
            LUTinsertIntoLutP (INFO_REUSELUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                               FollowLut (ID_AVIS (PRF_ARG1 (arg_node)),
                                          INFO_REUSELUT (arg_info)));
        }
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
