/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup icp Inplace Computation
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file inplacecomp.c
 *
 * Prefix: IPC
 *
 *****************************************************************************/
#include "inplacecomp.h"

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
#include "type_utils.h"
#include "shape.h"

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

    bool ok;
    node *nouse;
    node *noap;
    node *lastsafe;
    bool changed;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_REUSELUT(n) ((n)->reuselut)
#define INFO_PREDAVIS(n) ((n)->predavis)
#define INFO_MEMAVIS(n) ((n)->memavis)
#define INFO_RCAVIS(n) ((n)->rcavis)
#define INFO_OK(n) ((n)->ok)
#define INFO_NOUSE(n) ((n)->nouse)
#define INFO_NOAP(n) ((n)->noap)
#define INFO_LASTSAFE(n) ((n)->lastsafe)
#define INFO_CHANGED(n) ((n)->changed)

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
    INFO_OK (result) = FALSE;
    INFO_NOUSE (result) = NULL;
    INFO_NOAP (result) = NULL;
    INFO_LASTSAFE (result) = NULL;
    INFO_CHANGED (result) = FALSE;

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
 * @fn node *EMIPdoInplaceComputation( node *syntax_tree)
 *
 * @brief starting point of Inplace Computation traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMIPdoInplaceComputation (node *syntax_tree)
{
    DBUG_ENTER ("EMIPdoInplaceComputation");

    DBUG_PRINT ("EMIP", ("Inplace Computation  optimization..."));

    TRAVpush (TR_emip);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("EMIP", ("Inplace Computation optimization complete."));

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *copyOrArray(node *arg_node)
 *
 * @brief Get id in first place of prf
 *
 *****************************************************************************/

static node *
copyOrArray (node *val)
{
    DBUG_ENTER ("copyOrArray");

    if (NODE_TYPE (val) == N_prf) {
        DBUG_ASSERT ((PRF_PRF (val) == F_copy), "Expected copy prf");
        val = PRF_ARG1 (val);
    } else if (NODE_TYPE (val) == N_array) {
        while (NODE_TYPE (val) == N_array) {
            DBUG_ASSERT ((NODE_TYPE (ARRAY_AELEMS (val)) == N_exprs), "Broken ast?");
            val = EXPRS_EXPR (ARRAY_AELEMS (val));
        }
    } else {
        DBUG_ASSERT (FALSE, "Unexpected node");
    }

    DBUG_ASSERT (NODE_TYPE (val) == N_id, "Unexpected node expected an N_id");

    DBUG_RETURN (val);
}

/** <!--********************************************************************-->
 *
 * @fn node *HandleBlock(node *arg_node)
 *
 * @brief The main part of this traversal
 *        Are rets performing in-place computation?
 *
 *****************************************************************************/
static node *
HandleBlock (node *block, node *rets, info *arg_info)
{
    DBUG_ENTER ("HandleBlock");

    while (rets != NULL) {
        node *cid;
        node *wlass;
        node *rhs;
        node *mem;
        node *val;
        node *cval;
        node *memass;
        node *memop;
        node *avis;
        bool isinblock;
        node *assigns;

        cid = EXPRS_EXPR (rets);
        wlass = AVIS_SSAASSIGN (ID_AVIS (cid));

        if (wlass != NULL) {
            rhs = ASSIGN_RHS (wlass);

            if ((NODE_TYPE (rhs) == N_prf) && (PRF_PRF (rhs) == F_fill)
                && (((NODE_TYPE (PRF_ARG1 (rhs)) == N_prf)
                     && (PRF_PRF (PRF_ARG1 (rhs)) == F_copy))
                    || (NODE_TYPE (PRF_ARG1 (rhs)) == N_array))) {
                /*
                 * Search for suballoc situation
                 *
                 *   a  = ...
                 *   m' = suballoc( A, iv);
                 *   m  = fill( copy( a), m');
                 * }: m
                 *
                 * or (as appears in with3 loops:
                 *
                 *   a  = ...
                 *   m' = suballoc( A, iv);
                 *   m  = fill( [ a], m');
                 */
                val = PRF_ARG1 (rhs);
                mem = PRF_ARG2 (rhs);
                cval = copyOrArray (val);
                avis = ID_AVIS (cval);
                memass = AVIS_SSAASSIGN (ID_AVIS (mem));
                memop = LET_EXPR (ASSIGN_INSTR (memass));

                /*
                 * a must be assigned inside the current block in order to
                 * move suballoc in front of a.
                 */
                isinblock = FALSE;
                if (AVIS_SSAASSIGN (avis) != NULL) {
                    assigns = BLOCK_INSTR (block);
                    while (assigns != NULL) {
                        if (assigns == AVIS_SSAASSIGN (avis)) {
                            isinblock = TRUE;
                            break;
                        }
                        assigns = ASSIGN_NEXT (assigns);
                    }
                }

                if ((isinblock) && (PRF_PRF (memop) == F_suballoc)) {
                    /*
                     * Situation recognized, find highest position for suballoc
                     */
                    node *def = AVIS_SSAASSIGN (ID_AVIS (cval));
                    INFO_LASTSAFE (arg_info) = NULL;
                    INFO_NOUSE (arg_info) = LUTsearchInLutPp (INFO_REUSELUT (arg_info),
                                                              ID_AVIS (PRF_ARG1 (memop)));
                    if (INFO_NOUSE (arg_info) == ID_AVIS (PRF_ARG1 (memop))) {
                        INFO_NOUSE (arg_info) = NULL;
                    }
                    INFO_NOAP (arg_info) = NULL;

                    /*
                     * BETWEEN def and LASTSAFE:
                     *
                     * NOUSE must not be used at all!!!
                     * NOAP must not be used in function applications
                     */
                    INFO_OK (arg_info) = TRUE;

                    while (INFO_OK (arg_info)) {
                        TRAVpush (TR_emiph);
                        ASSIGN_NEXT (def) = TRAVdo (ASSIGN_NEXT (def), arg_info);
                        TRAVpop ();

                        if (INFO_OK (arg_info)) {
                            node *defrhs = ASSIGN_RHS (def);
                            node *withop, *ids;
                            switch (NODE_TYPE (defrhs)) {
                            case N_prf:
                                if (PRF_PRF (defrhs) == F_fill) {
                                    node *memass
                                      = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (defrhs)));
                                    node *memop = ASSIGN_RHS (memass);
                                    if ((PRF_PRF (memop) == F_alloc)
                                        || (PRF_PRF (memop) == F_reuse)
                                        || (PRF_PRF (memop) == F_alloc_or_reuse)) {
                                        INFO_LASTSAFE (arg_info) = memass;
                                        if (PRF_PRF (memop) == F_reuse) {
                                            avis = ID_AVIS (PRF_ARG1 (memop));
                                            def = AVIS_SSAASSIGN (
                                              ID_AVIS (PRF_ARG1 (memop)));
                                            INFO_NOAP (arg_info)
                                              = ID_AVIS (PRF_ARG1 (memop));
                                        } else {
                                            INFO_OK (arg_info) = FALSE;
                                        }
                                    } else {
                                        INFO_OK (arg_info) = FALSE;
                                    }
                                } else {
                                    INFO_OK (arg_info) = FALSE;
                                }
                                break;

                            case N_with:
                            case N_with2:
                            case N_with3:
                                withop = WITH_OR_WITH2_OR_WITH3_WITHOP (defrhs);
                                ids = ASSIGN_LHS (def);
                                while (IDS_AVIS (ids) != avis) {
                                    ids = IDS_NEXT (ids);
                                    withop = WITHOP_NEXT (withop);
                                }
                                if ((NODE_TYPE (withop) == N_genarray)
                                    || (NODE_TYPE (withop) == N_modarray)) {
                                    node *memass
                                      = AVIS_SSAASSIGN (ID_AVIS (WITHOP_MEM (withop)));
                                    node *memop = ASSIGN_RHS (memass);
                                    if ((PRF_PRF (memop) == F_alloc)
                                        || (PRF_PRF (memop) == F_reuse)
                                        || (PRF_PRF (memop) == F_alloc_or_reuse)) {
                                        INFO_LASTSAFE (arg_info) = memass;
                                        if (PRF_PRF (memop) == F_reuse) {
                                            avis = ID_AVIS (PRF_ARG1 (memop));
                                            def = AVIS_SSAASSIGN (
                                              ID_AVIS (PRF_ARG1 (memop)));
                                            INFO_NOAP (arg_info)
                                              = ID_AVIS (PRF_ARG1 (memop));
                                        } else {
                                            INFO_OK (arg_info) = FALSE;
                                        }
                                    } else {
                                        INFO_OK (arg_info) = FALSE;
                                    }
                                } else {
                                    INFO_OK (arg_info) = FALSE;
                                }
                                break;

                            default:
                                INFO_OK (arg_info) = FALSE;
                                break;
                            }
                        }
                    }

                    if (INFO_LASTSAFE (arg_info) != NULL) {
                        node *n;
                        ntype *type = NULL;
                        /*
                         * Replace some alloc or reuse or alloc_or_reuse with
                         * suballoc
                         */
                        ASSIGN_RHS (INFO_LASTSAFE (arg_info))
                          = FREEdoFreeNode (ASSIGN_RHS (INFO_LASTSAFE (arg_info)));
                        ASSIGN_RHS (INFO_LASTSAFE (arg_info)) = DUPdoDupNode (memop);

                        /*
                         * Are we suballocing a scaler?
                         * If so make it a [1] array.
                         */
                        type
                          = AVIS_TYPE (IDS_AVIS (ASSIGN_LHS (INFO_LASTSAFE (arg_info))));
                        if (TUisScalar (type)) {
                            type = TYmakeAKS (type, SHcreateShape (1, 1));
                            AVIS_TYPE (IDS_AVIS (ASSIGN_LHS (INFO_LASTSAFE (arg_info))))
                              = type;
                        }

                        /*
                         * Replace CEXPR
                         */
                        EXPRS_EXPR (rets) = FREEdoFreeNode (EXPRS_EXPR (rets));
                        EXPRS_EXPR (rets) = DUPdoDupNode (cval);

                        /*
                         * Remove old suballoc/fill(copy) combination
                         */
                        n = BLOCK_INSTR (block);
                        while (ASSIGN_NEXT (n) != memass) {
                            n = ASSIGN_NEXT (n);
                        }
                        ASSIGN_NEXT (n) = FREEdoFreeNode (ASSIGN_NEXT (n));
                        ASSIGN_NEXT (n) = FREEdoFreeNode (ASSIGN_NEXT (n));
                        INFO_CHANGED (arg_info) = TRUE;
                    }
                }
                break;
            }
        }
        rets = EXPRS_NEXT (rets);
    }

    DBUG_RETURN (NULL);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *EMIPap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPap");

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
 * @fn node *EMIPcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPcond (node *arg_node, info *arg_info)
{
    lut_t *oldlut;

    DBUG_ENTER ("EMIPcond");

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
 * @fn node *EMIPcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPcode");

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    HandleBlock (CODE_CBLOCK (arg_node), CODE_CEXPRS (arg_node), arg_info);

    /*
     * Traverse next code
     */
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPrange( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPrange");

    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    INFO_CHANGED (arg_info) = TRUE;
    while (INFO_CHANGED (arg_info)) {
        INFO_CHANGED (arg_info) = FALSE;
        HandleBlock (RANGE_BODY (arg_node), RANGE_RESULTS (arg_node), arg_info);
    }

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPfundef");

    /*
     * CONDFUNs may only be traversed from AP-nodes
     */
    if ((!FUNDEF_ISCONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info = MakeInfo (arg_node);

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
 * @fn node *EMIPlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPprf");

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

    case F_fill:
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_prf) {
            /*
             * c = fill( PRF, b);
             */
            node *prf = PRF_ARG1 (arg_node);
            switch (PRF_PRF (prf)) {

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

/******************************************************************************
 *
 * @name IPC helper traversal
 *
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMIPHap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPHap (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("EMIPHap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);

        tmp = INFO_NOUSE (arg_info);
        INFO_NOUSE (arg_info) = INFO_NOAP (arg_info);
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        INFO_NOUSE (arg_info) = tmp;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPHassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPHassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPHaassign");

    if (arg_node != INFO_LASTSAFE (arg_info)) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPHid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
EMIPHid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPHid");

    if (ID_AVIS (arg_node) == INFO_NOUSE (arg_info)) {
        INFO_OK (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Inplace Computation -->
 *****************************************************************************/
