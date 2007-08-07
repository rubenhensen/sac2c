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
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REUSELUT(n) ((n)->reuselut)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_MEMAVIS(n) ((n)->memavis)
#define INFO_RCAVIS(n) ((n)->rcavis)

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
        node *iv = NULL;
        node *idx = NULL;
        bool inplace = FALSE;

        id = EXPRS_EXPR (exprs);

        if (AVIS_SSAASSIGN (ID_AVIS (id)) != NULL) {
            node *wlass = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (id)));

            if ((NODE_TYPE (wlass) == N_prf) && (PRF_PRF (wlass) == F_wl_assign)) {
                node *val;
                node *mem;
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
                                         TCmakePrf1 (F_noop, DUPdoDupNode (iv))),
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
