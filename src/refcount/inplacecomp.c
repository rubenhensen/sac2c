/*
 *
 * $Log$
 * Revision 1.2  2005/04/12 15:53:38  ktr
 * In-Place-Computation can now cope with fold-withloops.
 *
 * Revision 1.1  2004/12/16 14:38:26  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup icp Inplace computation traversal
 * @ingroup alloc
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file inplacecomp.c
 *
 *
 */
#include "inplacecomp.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "internal_lib.h"
#include "free.h"
#include "new_types.h"

#include <string.h>

/*
 * INFO structure
 */
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
};

/*
 * INFO macros
 */
#define INFO_EMIP_FUNDEF(n) ((n)->fundef)
#define INFO_EMIP_LHS(n) ((n)->lhs)
#define INFO_EMIP_REUSELUT(n) ((n)->reuselut)
#define INFO_EMIP_PREDAVIS(n) ((n)->predavis)
#define INFO_EMIP_MEMAVIS(n) ((n)->memavis)
#define INFO_EMIP_RCAVIS(n) ((n)->rcavis)
#define INFO_EMIP_OK(n) ((n)->ok)
#define INFO_EMIP_NOUSE(n) ((n)->nouse)
#define INFO_EMIP_NOAP(n) ((n)->noap)
#define INFO_EMIP_LASTSAFE(n) ((n)->lastsafe)

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_EMIP_FUNDEF (result) = fundef;
    INFO_EMIP_LHS (result) = NULL;
    INFO_EMIP_REUSELUT (result) = NULL;
    INFO_EMIP_PREDAVIS (result) = NULL;
    INFO_EMIP_MEMAVIS (result) = NULL;
    INFO_EMIP_RCAVIS (result) = NULL;
    INFO_EMIP_OK (result) = FALSE;
    INFO_EMIP_NOUSE (result) = NULL;
    INFO_EMIP_NOAP (result) = NULL;
    INFO_EMIP_LASTSAFE (result) = NULL;

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

/******************************************************************************
 *
 * Inplace Computation optimization traversal (emip_tab)
 *
 * prefix: EMIP
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMIPap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
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

        if (strstr (FUNDEF_NAME (AP_FUNDEF (arg_node)), "ReuseCond") != NULL) {
            /*
             * Transform predavis, memavis and rcavis before traversing REUSECOND
             */
            node *funargs, *apargs;
            funargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
            apargs = AP_ARGS (arg_node);

            while (apargs != NULL) {

                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_EMIP_PREDAVIS (arg_info)) {
                    INFO_EMIP_PREDAVIS (arg_info) = ARG_AVIS (funargs);
                }
                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_EMIP_MEMAVIS (arg_info)) {
                    INFO_EMIP_MEMAVIS (arg_info) = ARG_AVIS (funargs);
                }
                if (ID_AVIS (EXPRS_EXPR (apargs)) == INFO_EMIP_RCAVIS (arg_info)) {
                    INFO_EMIP_RCAVIS (arg_info) = ARG_AVIS (funargs);
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
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMIPcond (node *arg_node, info *arg_info)
{
    lut_t *oldlut;

    DBUG_ENTER ("EMIPcond");

    oldlut = INFO_EMIP_REUSELUT (arg_info);
    INFO_EMIP_REUSELUT (arg_info) = LUTduplicateLut (oldlut);

    if ((NODE_TYPE (COND_COND (arg_node)) == N_id)
        && (ID_AVIS (COND_COND (arg_node)) == INFO_EMIP_PREDAVIS (arg_info))) {
        /*
         * b = reuse( a);
         *
         * Insert (memavis, rcavis) into REUSELUT
         */
        LUTinsertIntoLutP (INFO_EMIP_REUSELUT (arg_info), INFO_EMIP_MEMAVIS (arg_info),
                           INFO_EMIP_RCAVIS (arg_info));
    }
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_EMIP_REUSELUT (arg_info) = LUTremoveLut (INFO_EMIP_REUSELUT (arg_info));
    INFO_EMIP_REUSELUT (arg_info) = oldlut;

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMIPcode (node *arg_node, info *arg_info)
{
    node *cexprs;

    DBUG_ENTER ("EMIPcode");

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
    cexprs = CODE_CEXPRS (arg_node);
    while (cexprs != NULL) {
        node *cid = EXPRS_EXPR (cexprs);
        node *wlass = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (cid))));
        node *mem;
        node *val;
        node *cval;
        node *memass;
        node *memop;
        node *avis;
        bool isinblock;
        node *assigns;

        if ((NODE_TYPE (wlass) == N_prf) && (PRF_PRF (wlass) == F_fill)
            && (NODE_TYPE (PRF_ARG1 (wlass)) == N_prf)
            && (PRF_PRF (PRF_ARG1 (wlass)) == F_copy)) {
            /*
             * Search for suballoc situation
             *
             *   a  = ...
             *   m' = suballoc( A, iv);
             *   m  = fill( copy( a), m');
             * }: m
             */
            val = PRF_ARG1 (wlass);
            mem = PRF_ARG2 (wlass);
            cval = PRF_ARG1 (val);
            avis = ID_AVIS (cval);
            memass = AVIS_SSAASSIGN (ID_AVIS (mem));
            memop = LET_EXPR (ASSIGN_INSTR (memass));

            /*
             * a must be assigned inside the current block in order to
             * move suballoc in front of a.
             */
            isinblock = FALSE;
            if (AVIS_SSAASSIGN (avis) != NULL) {
                assigns = BLOCK_INSTR (CODE_CBLOCK (arg_node));
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
                INFO_EMIP_LASTSAFE (arg_info) = NULL;
                INFO_EMIP_NOUSE (arg_info)
                  = LUTsearchInLutPp (INFO_EMIP_REUSELUT (arg_info),
                                      ID_AVIS (PRF_ARG1 (memop)));
                if (INFO_EMIP_NOUSE (arg_info) == ID_AVIS (PRF_ARG1 (memop))) {
                    INFO_EMIP_NOUSE (arg_info) = NULL;
                }
                INFO_EMIP_NOAP (arg_info) = NULL;

                /*
                 * BETWEEN def and LASTSAFE:
                 *
                 * NOUSE must not be used at all!!!
                 * NOAP must not be used in function applications
                 */
                INFO_EMIP_OK (arg_info) = TRUE;

                while (INFO_EMIP_OK (arg_info)) {
                    TRAVpush (TR_emiph);
                    ASSIGN_NEXT (def) = TRAVdo (ASSIGN_NEXT (def), arg_info);
                    TRAVpop ();

                    if (INFO_EMIP_OK (arg_info)) {
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
                                    INFO_EMIP_LASTSAFE (arg_info) = memass;
                                    if (PRF_PRF (memop) == F_reuse) {
                                        avis = ID_AVIS (PRF_ARG1 (memop));
                                        def = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (memop)));
                                        INFO_EMIP_NOAP (arg_info)
                                          = ID_AVIS (PRF_ARG1 (memop));
                                    } else {
                                        INFO_EMIP_OK (arg_info) = FALSE;
                                    }
                                } else {
                                    INFO_EMIP_OK (arg_info) = FALSE;
                                }
                            } else {
                                INFO_EMIP_OK (arg_info) = FALSE;
                            }
                            break;

                        case N_with:
                        case N_with2:
                            withop = WITH_OR_WITH2_WITHOP (defrhs);
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
                                    INFO_EMIP_LASTSAFE (arg_info) = memass;
                                    if (PRF_PRF (memop) == F_reuse) {
                                        avis = ID_AVIS (PRF_ARG1 (memop));
                                        def = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (memop)));
                                        INFO_EMIP_NOAP (arg_info)
                                          = ID_AVIS (PRF_ARG1 (memop));
                                    } else {
                                        INFO_EMIP_OK (arg_info) = FALSE;
                                    }
                                } else {
                                    INFO_EMIP_OK (arg_info) = FALSE;
                                }
                            } else {
                                INFO_EMIP_OK (arg_info) = FALSE;
                            }
                            break;

                        default:
                            INFO_EMIP_OK (arg_info) = FALSE;
                            break;
                        }
                    }
                }

                if (INFO_EMIP_LASTSAFE (arg_info) != NULL) {
                    node *n;
                    /*
                     * Replace some alloc or reuse or alloc_or_reuse with
                     * suballoc
                     */
                    ASSIGN_RHS (INFO_EMIP_LASTSAFE (arg_info))
                      = FREEdoFreeNode (ASSIGN_RHS (INFO_EMIP_LASTSAFE (arg_info)));
                    ASSIGN_RHS (INFO_EMIP_LASTSAFE (arg_info)) = DUPdoDupNode (memop);

                    /*
                     * Replace CEXPR
                     */
                    EXPRS_EXPR (cexprs) = FREEdoFreeNode (EXPRS_EXPR (cexprs));
                    EXPRS_EXPR (cexprs) = DUPdoDupNode (cval);

                    /*
                     * Remove old suballoc/fill(copy) combination
                     */
                    n = BLOCK_INSTR (CODE_CBLOCK (arg_node));
                    while (ASSIGN_NEXT (n) != memass) {
                        n = ASSIGN_NEXT (n);
                    }
                    ASSIGN_NEXT (n) = FREEdoFreeNode (ASSIGN_NEXT (n));
                    ASSIGN_NEXT (n) = FREEdoFreeNode (ASSIGN_NEXT (n));
                }
            }
            break;
        }

        cexprs = EXPRS_NEXT (cexprs);
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
 * @fn node *EMIPfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
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
                INFO_EMIP_PREDAVIS (info) = INFO_EMIP_PREDAVIS (arg_info);
                INFO_EMIP_MEMAVIS (info) = INFO_EMIP_MEMAVIS (arg_info);
                INFO_EMIP_RCAVIS (info) = INFO_EMIP_RCAVIS (arg_info);
            }

            INFO_EMIP_REUSELUT (info) = LUTgenerateLut ();

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            INFO_EMIP_REUSELUT (info) = LUTremoveLut (INFO_EMIP_REUSELUT (info));

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
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMIPlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPlet");

    INFO_EMIP_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
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
        LUTinsertIntoLutP (INFO_EMIP_REUSELUT (arg_info),
                           IDS_AVIS (INFO_EMIP_LHS (arg_info)),
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
                INFO_EMIP_PREDAVIS (arg_info) = IDS_AVIS (INFO_EMIP_LHS (arg_info));
                INFO_EMIP_MEMAVIS (arg_info) = ID_AVIS (PRF_ARG1 (prf));
                INFO_EMIP_RCAVIS (arg_info) = ID_AVIS (PRF_ARG2 (prf));
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

/******************************************************************************
 *
 * Inplace Computation optimization helper traversal (emiph_tab)
 *
 * prefix: EMIPH
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMIPHap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMIPHap (node *arg_node, info *arg_info)
{
    node *tmp;

    DBUG_ENTER ("EMIPHap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);

        tmp = INFO_EMIP_NOUSE (arg_info);
        INFO_EMIP_NOUSE (arg_info) = INFO_EMIP_NOAP (arg_info);
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        INFO_EMIP_NOUSE (arg_info) = tmp;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMIPHassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMIPHassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPHaassign");

    if (arg_node != INFO_EMIP_LASTSAFE (arg_info)) {
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
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
EMIPHid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMIPHid");

    if (ID_AVIS (arg_node) == INFO_EMIP_NOUSE (arg_info)) {
        INFO_EMIP_OK (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/* @} */
