/** <!--********************************************************************-->
 *
 * @defgroup dro Data Reuse Optimization
 *
 * @ingroup mm
 *
 * @{
 *
 * Documentation as provided from on high:
 *
 * "
 * BTW, I took a quick look at datareuse.c, and note that:
 *
 *      - like everything else, there is zero documentation about
 *        its purpose in life, or any outline about how it does what
 *        it purports to do.
 *
 *      - I have NO idea what the backend macros (fill, wl_assign),
 *        do, because that, too, is undocumented.
 *
 *  So, I do not intend to touch it until the above two problems are
 *  remedied. Sorry about that...
 *
 *
 * Cheap excuse my dear :-)
 * check the first pub on this page out:
 *
 * http://www.sac-home.org/index.php?
 *   p=.%2F33_Research%2F31_Publications%2F3_Basic_Compiler_Implementation
 *
 * I think I'll just check the first pub, instead.
 *
 * The above doc is Kai Trojahner's well-written and highly readable
 * thesis: "Implicit Memory Management for a Functional Array
 * Processing Language". Since you have read this far, I suggest
 * you read the above for much-needed background and terminolgy.
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

#define DBUG_PREFIX "EMDR"
#include "debug.h"

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
    lut_t *sublut;
    node *predavis;
    node *memavis;
    node *rcavis;
    node *iv;
    node *ivids;
    node *wlidx;
    node *wliirr;
    node *freeme; /* stuff that should be freed at the end of the trav */
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REUSELUT(n) ((n)->reuselut)
#define INFO_SUBLUT(n) ((n)->sublut)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_MEMAVIS(n) ((n)->memavis)
#define INFO_RCAVIS(n) ((n)->rcavis)
#define INFO_IV(n) ((n)->iv)
#define INFO_IVIDS(n) ((n)->ivids)
#define INFO_WLIDXS(n) ((n)->wlidx)
#define INFO_WLIIRR(n) ((n)->wliirr)
#define INFO_FREE_ME(n) ((n)->freeme)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_LHS (result) = NULL;
    INFO_REUSELUT (result) = NULL;
    INFO_SUBLUT (result) = NULL;
    INFO_PREDAVIS (result) = NULL;
    INFO_MEMAVIS (result) = NULL;
    INFO_RCAVIS (result) = NULL;
    INFO_IV (result) = NULL;
    INFO_IVIDS (result) = NULL;
    INFO_WLIDXS (result) = NULL;
    INFO_WLIIRR (result) = NULL;
    INFO_FREE_ME (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_FREE_ME (info) = FREEoptFreeTree(INFO_FREE_ME (info));

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 * @fn bool IsSameIndex( node *idxs_s, node *idxs_t, node *idx_s, node *idx_t)
 *
 * @brief Checks whether idx_s and idx_t belong to the same wl-operation. Both
 *        are considered to stem form the same operation in they are at the
 *        same position in the idxs_s and idxs_t chains, respectively.
 *
 * @param idxs_s N_exprs chain of global ravel indices of a with3 (IIRR)
 * @param idxs_t N_ids chain of local ravel indices of a with3 (IDXS)
 * @param idx_s  N_id of the source index (read operation)
 * @param idx_t  N_id of the target index (write operation)
 *
 * @return TRUE iff idx_t and idx_s belong to the same operation
 ******************************************************************************/
static bool
IsSameIndex (node *idxs_s, node *idxs_t, node *idx_s, node *idx_t)
{
    bool result = FALSE;

    DBUG_ENTER ();

    while (!result && (idxs_s != NULL) && (idxs_t != NULL)) {
        DBUG_PRINT ("comparing S(%s/%s) and T(%s/%s)", ID_NAME (idx_t), IDS_NAME (idxs_t),
                    ID_NAME (idx_s), ID_NAME (EXPRS_EXPR (idxs_s)));

        result = result
                 || ((ID_AVIS (idx_t) == IDS_AVIS (idxs_t))
                     && (ID_AVIS (idx_s) == ID_AVIS (EXPRS_EXPR (idxs_s))));

        idxs_s = EXPRS_NEXT (idxs_s);
        idxs_t = IDS_NEXT (idxs_t);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *FindSubAllocRoot( lut_t sublut, node *avis)
 *
 * @brief Follows the chain of mappings in sublut until the root is found.
 *        If a root is found, the corresponding avis is returned. If no
 *        mapping is found, NULL is returned.
 *
 * @param sublut suballoc mapping lut
 * @param avis   avis of suballoced memvar
 *
 * @return root memvar or NULL
 ******************************************************************************/
static node *
FindSubAllocRoot (lut_t *sublut, node *avis)
{
    node *found, *result;

    DBUG_ENTER ();

    found = (node *)LUTsearchInLutPp (sublut, avis);
    DBUG_PRINT ("checking root of %s, found %s", AVIS_NAME (avis),
                (found == NULL) ? "--" : AVIS_NAME (found));

    if (found == avis) {
        result = NULL;
    } else if (found == NULL) {
        result = avis;
    } else {
        result = FindSubAllocRoot (sublut, found);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *WithResult( node *with)
 *
 *  Find the result of the current with loop
 *
 *  In the case of a nested with3 find the result of the nested with3
 *  where nested means that the result of the with3 is a with3
 *
 *  @param with with to find result of
 *****************************************************************************/
node *
WithResult (node *with)
{
    node *result = NULL;
    DBUG_ENTER ();

    if ((NODE_TYPE (with) == N_with) || (NODE_TYPE (with) == N_with2)) {
        result = CODE_CEXPR (WITH_OR_WITH2_CODE (with));
    } else {
        pattern *pat;
        int zero = 0, two = 2;
        DBUG_ASSERT (NODE_TYPE (with) == N_with3, "WithResult called without with* node");
        pat = PMretryAny (&zero, &two, 2,
                          PMvar (1, PMAgetNode (&result), 1,
                                 PMprf (1, PMAisPrf (F_wl_assign), 1, PMskip (0))),
                          PMwith3 (0, 1,
                                   PMSrange (0, 1, PMrange (0, 1, PMlink (0, 1, &pat)))));
        PMmatchFlat (pat, RANGE_RESULTS (WITH3_RANGES (with)));
        pat = PMfree (pat);
    }
    DBUG_ASSERT (result != NULL, "Could not find result of withloop");
    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *HandleCodeBlock( node *exprs, node *assigns, info *arg_info)
 *
 * @brief Tries to detect copy operations in code blocks and replaces the
 *        corresponding cexpr by a F_noop operation.
 *
 * @param exprs    result expressions of the current code block
 * @param assigns  body of the code block
 * @param arg_info info structure
 *
 * @return updated body of the code block
 ******************************************************************************/
static node *
HandleCodeBlock (node *exprs, node *assigns, info *arg_info)
{
    pattern *pat;

    DBUG_ENTER ();

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
                     * a = fill( idx_sel( idx_s, B), a');
                     * r = wl_assign( a, A', iv, idx_t);
                     *
                     * where A' is known to be a reuse of B
                     * and
                     * idx_s = idx_t (with/with2)
                     *
                     * -or-
                     *
                     * where A' is known to be a suballoc of a reuse of B
                     * and
                     * idx_s is the ravel index of a with3 (IIRR) and
                     * idx_t is the result offset of a with3 (IDXS)
                     */
                    if ((NODE_TYPE (sel) == N_prf) && (PRF_PRF (sel) == F_idx_sel)) {
                        node *selidx = PRF_ARG1 (sel);
                        node *arr = PRF_ARG2 (sel);
                        node *submem;

                        if (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                            == ID_AVIS (arr)) {
                            if (ID_AVIS (idx) == ID_AVIS (selidx)) {
                                inplace = TRUE;
                            }

                            if ((AVIS_SSAASSIGN (ID_AVIS (selidx)) != NULL)
                                && (NODE_TYPE (
                                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (selidx))))
                                    == N_prf)
                                && (PRF_PRF (
                                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (selidx))))
                                    == F_fill)) {
                                node *idxs2offset = PRF_ARG1 (
                                  ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (selidx))));
                                if ((NODE_TYPE (idxs2offset) == N_prf)
                                    && (PRF_PRF (idxs2offset) == F_idxs2offset)) {
                                    node *withids = INFO_IVIDS (arg_info);
                                    node *prf_ids = PRF_EXPRS2 (idxs2offset);
                                    while (withids != NULL) {
                                        inplace = TRUE;
                                        if (prf_ids == NULL
                                            || ID_AVIS (EXPRS_EXPR (withids))
                                                 != ID_AVIS (EXPRS_EXPR (prf_ids))) {
                                            inplace = FALSE;
                                            break;
                                        }
                                        withids = EXPRS_NEXT (withids);
                                        prf_ids = EXPRS_NEXT (prf_ids);
                                    }
                                }
                            }
                        }

                        submem = FindSubAllocRoot (INFO_SUBLUT (arg_info), ID_AVIS (mem));
                        if ((submem != NULL)
                            && (LUTsearchInLutPp (INFO_REUSELUT (arg_info), submem)
                                == ID_AVIS (arr))) {
                            DBUG_PRINT ("found root for suballoc %s --> %s",
                                        AVIS_NAME (ID_AVIS (mem)), AVIS_NAME (submem));

                            if (IsSameIndex (INFO_WLIIRR (arg_info),
                                             INFO_WLIDXS (arg_info), selidx, idx)) {
                                inplace = TRUE;
                                DBUG_PRINT ("found with3 idx_sel copy.");
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

                DBUG_PRINT ("vector copy: potential candiate found.");

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
                        DBUG_PRINT ("vector copy: element %d fits.", pos);
#endif
                    }

                    aexprs = EXPRS_NEXT (aexprs);
                    pos++;
                }

#ifndef DBUG_OFF
                if (iscopy) {
                    DBUG_PRINT ("vector copy expression found");
                }
#endif
                if (iscopy
                    && (LUTsearchInLutPp (INFO_REUSELUT (arg_info), ID_AVIS (mem))
                        == ID_AVIS (arr))) {
                    DBUG_PRINT ("vector copy: reuse identified.");

                    inplace = TRUE;
                }
            }
            pat = PMfree (pat);

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
            if ((NODE_TYPE (wlass) == N_with) || (NODE_TYPE (wlass) == N_with2)
                || (NODE_TYPE (wlass) == N_with3)) {
                node *withop, *wlids = NULL, *wliv = NULL;
                bool oneCodeIfApplicable;
                bool iscopy = FALSE;

                withop = WITH_OR_WITH2_OR_WITH3_WITHOP (wlass);

                if ((NODE_TYPE (wlass) == N_with) || (NODE_TYPE (wlass) == N_with2)) {
                    wlids = WITH_OR_WITH2_IDS (wlass);
                    wliv = WITH_OR_WITH2_VEC (wlass);
                }

                DBUG_PRINT ("inner with-oop found...");

                oneCodeIfApplicable = (NODE_TYPE (wlass) == N_with3)
                                      || (CODE_NEXT (WITH_OR_WITH2_CODE (wlass)) == NULL);

                if ((NODE_TYPE (withop) == N_genarray) && (GENARRAY_NEXT (withop) == NULL)
                    && oneCodeIfApplicable) {
                    node *cexpr = WithResult (wlass);
                    node *offset = NULL;
                    node *arr = NULL;
                    node *mem = NULL;

#if 0
          pattern *with3fill;
          int pos=0, n=1, one=1;
          with3fill =
            PMprf( 1,
                   PMAisPrf( F_fill),
                   2,
                   PMretryAll( &pos, &n,
                               1,
                               PMarray( 1,
                                        PMAgetLen( &n),
                                        1,
                                        PMskipN( &pos, 0, 0)),
                               PMprf( 1,
                                      PMAisPrf( F_suballoc),
                                      3,
                                      PMskipN( 1, 0, 0),
                                      PMvar( 1, PMAisVar( &mem), 0),
                                      PMskipN( 1, 0, 0)));
                   // If you resurrect this code, be sure to insert a PMfree(with3fill)
#endif
                    DBUG_PRINT ("wl copy: potential candiate found.");

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

                            DBUG_PRINT ("wl copy: inner sel is scalar copy.");
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

                                DBUG_PRINT ("wl copy: inner sel is vector copy.");
                                iscopy = TRUE;
                            }
                        }
                    }

                    if (iscopy) {
                        DBUG_PRINT ("wl copy: copy operation identified.");

                        if (PMO (PMOvar (&mem,
                                         PMOprf (F_suballoc, GENARRAY_MEM (withop))))) {
                            DBUG_PRINT ("wl copy: suballoc identified.");

                            node *avis = LUTsearchInLutPp (INFO_REUSELUT (arg_info),
                                                           ID_AVIS (mem));
                            if (avis == ID_AVIS (arr)) {
                                inplace = TRUE;
                                DBUG_PRINT ("wl copy: reuse identified!");
                            } else {
                                DBUG_PRINT ("wl copy: looking up \"%s\": found \"%s\".",
                                            ID_NAME (mem),
                                            (avis != NULL ? AVIS_NAME (avis) : "--"));
                                DBUG_PRINT ("wl copy: does not match \"%s\" .",
                                            ID_NAME (arr));
                            }
                        } else {
                            DBUG_PRINT ("wl copy: no suballoc found.");
                        }
                    } else {
                        DBUG_PRINT ("wl copy: no copy operation identified.");
                    }
                }
            }
        }

        if (inplace) {
            node *avis;

            DBUG_PRINT ("Inplace copy situation recognized!");

            /*
             * Create a variable for new cexpr
             */
            avis = TBmakeAvis (TRAVtmpVar (), TYeliminateAKV (AVIS_TYPE (ID_AVIS (id))));

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            /*
             * Create noop
             * a = noop( iv);
             */
            assigns = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                               TCmakePrf1 (F_noop,
                                                           DUPdoDupNode (
                                                             (INFO_IV (arg_info) == NULL)
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
 * @}  <!-- Static helper functions -->
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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    /*
     * Top-down traversal
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    INFO_IV (arg_info) = WITHID_VEC (arg_node);
    INFO_IVIDS (arg_info) = WITHID_IDS (arg_node);
    INFO_WLIDXS (arg_info) = WITHID_IDXS (arg_node);
    INFO_WLIIRR (arg_info) = NULL;

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
    node *oldivs, *oldiv, *oldidxs, *oldiirr;

    DBUG_ENTER ();

    oldiv = INFO_IV (arg_info);
    oldivs = INFO_IVIDS (arg_info);
    oldidxs = INFO_WLIDXS (arg_info);
    oldiirr = INFO_WLIIRR (arg_info);

    WITH_WITHID (arg_node) = TRAVopt (WITH_WITHID (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);

    INFO_WLIIRR (arg_info) = oldiirr;
    INFO_WLIDXS (arg_info) = oldidxs;
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
    node *oldivs, *oldiv, *oldidxs, *oldiirr;
#ifndef DBUG_OFF
    node *lhs_ids = INFO_LHS (arg_info);
#endif

    DBUG_ENTER ();

    DBUG_PRINT ("\nTraversing with2 defining \"%s\"", IDS_NAME (lhs_ids));

    oldiv = INFO_IV (arg_info);
    oldivs = INFO_IVIDS (arg_info);
    oldidxs = INFO_WLIDXS (arg_info);
    oldiirr = INFO_WLIIRR (arg_info);

    WITH2_WITHID (arg_node) = TRAVopt (WITH2_WITHID (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);

    INFO_WLIIRR (arg_info) = oldiirr;
    INFO_WLIDXS (arg_info) = oldidxs;
    INFO_IVIDS (arg_info) = oldivs;
    INFO_IV (arg_info) = oldiv;

    DBUG_PRINT ("leaving with2 defining \"%s\"\n", IDS_NAME (lhs_ids));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRwith3( node *arg_node, info *arg_info)
 *
 * @brief Stack IV and IVIDS and remember top-level left-hand sides for
 *        genarray/modarray withloops in suballoc lut
 *
 *****************************************************************************/
node *
EMDRwith3 (node *arg_node, info *arg_info)
{
    node *oldivs, *oldiv, *oldidxs, *oldiirr;

    DBUG_ENTER ();

    oldiv = INFO_IV (arg_info);
    oldivs = INFO_IVIDS (arg_info);
    oldidxs = INFO_WLIDXS (arg_info);
    oldiirr = INFO_WLIIRR (arg_info);

    /*
     * insert all top-level result memvals into the SUBLUT as
     * these might be referenced in suballocs in levels
     * further down. I use NULL here to mark the end of the
     * suballoc chain.
     */
    if (WITH3_ISTOPLEVEL (arg_node)) {
        WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);
    }

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    INFO_WLIIRR (arg_info) = oldiirr;
    INFO_WLIDXS (arg_info) = oldidxs;
    INFO_IVIDS (arg_info) = oldivs;
    INFO_IV (arg_info) = oldiv;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("adding new suballoc root %s",
                AVIS_NAME (ID_AVIS (GENARRAY_MEM (arg_node))));
    LUTinsertIntoLutP (INFO_SUBLUT (arg_info), ID_AVIS (GENARRAY_MEM (arg_node)), NULL);

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMDRmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("adding new suballoc root %s",
                AVIS_NAME (ID_AVIS (MODARRAY_MEM (arg_node))));
    LUTinsertIntoLutP (INFO_SUBLUT (arg_info), ID_AVIS (MODARRAY_MEM (arg_node)), NULL);

    MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    DBUG_PRINT ("traversing new code block...");
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    /*
     * The great moment:
     * check whether CEXPRS perform INPLACE-COPY-OPERATIONS
     */
    DBUG_PRINT ("cheking for in-place option...");
    CODE_CBLOCK_ASSIGNS (arg_node)
      = HandleCodeBlock (CODE_CEXPRS (arg_node), CODE_CBLOCK_ASSIGNS (arg_node),
                         arg_info);
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
    DBUG_ENTER ();

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    /*
     * The great moment:
     * check whether CEXPRS perform INPLACE-COPY-OPERATIONS
     */
    INFO_IVIDS (arg_info) = TBmakeExprs (DUPdoDupTree (RANGE_INDEX (arg_node)), NULL);
    INFO_FREE_ME (arg_info) = TBmakeSet (INFO_IVIDS (arg_info), INFO_FREE_ME (arg_info));
    /* for now we don't actually know the IV */
    INFO_IV (arg_info) = NULL;
    INFO_WLIIRR (arg_info) = RANGE_IIRR (arg_node);
    INFO_WLIDXS (arg_info) = RANGE_IDXS (arg_node);

    BLOCK_ASSIGNS (RANGE_BODY (arg_node))
      = HandleCodeBlock (RANGE_RESULTS (arg_node), BLOCK_ASSIGNS (RANGE_BODY (arg_node)),
                         arg_info);

    /*
     * we can now free the IIRR and IDXS info, as it is no longer needed and
     * may otherwise keep offsets/indices alive that are not actually used
     */
    RANGE_IIRR (arg_node) = FREEoptFreeTree(RANGE_IIRR (arg_node));
    RANGE_IDXS (arg_node) = FREEoptFreeTree(RANGE_IDXS (arg_node));

    /*
     * Traverse next range
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
    DBUG_ENTER ();

    DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

    /*
     * CONDFUNs may only be traversed from AP-nodes
     */
    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;

            DBUG_PRINT ("Traversing function body %s", FUNDEF_NAME (arg_node));
            info = MakeInfo (arg_node);

            if (arg_info != NULL) {
                INFO_PREDAVIS (info) = INFO_PREDAVIS (arg_info);
                INFO_MEMAVIS (info) = INFO_MEMAVIS (arg_info);
                INFO_RCAVIS (info) = INFO_RCAVIS (arg_info);
            }

            INFO_REUSELUT (info) = LUTgenerateLut ();
            INFO_SUBLUT (info) = LUTgenerateLut ();

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            INFO_SUBLUT (info) = LUTremoveLut (INFO_SUBLUT (info));
            INFO_REUSELUT (info) = LUTremoveLut (INFO_REUSELUT (info));

            info = FreeInfo (info);
        }
    }

    /*
     * Traverse next fundef
     */
    if (arg_info == NULL) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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

    case F_resize:
        /*
         * b = resize( dim, shp, a);
         *
         * Insert (b, a) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_REUSELUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                           ID_AVIS (PRF_ARG3 (arg_node)));
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

    case F_suballoc:
        /*
         * b = suballoc( A, _)
         *
         * Insert (b, A) into SUBLUT iff A is already contained
         * in sublut!
         */
        DBUG_PRINT ("checking for existing suballoc %s",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
        if (LUTsearchInLutPp (INFO_SUBLUT (arg_info), ID_AVIS (PRF_ARG1 (arg_node)))
            != ID_AVIS (PRF_ARG1 (arg_node))) {
            DBUG_PRINT ("adding %s as new suballoc of %s.",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));

            LUTinsertIntoLutP (INFO_SUBLUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                               ID_AVIS (PRF_ARG1 (arg_node)));
        }
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
                    DBUG_PRINT ("Inplace copy situation recognized!");
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

#undef DBUG_PREFIX
