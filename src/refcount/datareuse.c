/*
 *
 * $Log$
 * Revision 1.3  2004/11/15 12:28:16  ktr
 * insignificant change
 *
 * Revision 1.2  2004/11/09 19:39:37  ktr
 * ongoing implementation
 *
 * Revision 1.1  2004/11/02 14:27:05  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup dro Data reuse optimization
 * @ingroup rcp
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file datareuse.c
 *
 *
 */
#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    ids *lhs;
    LUT_t reuselut;
    LUT_t renamelut;
};

/*
 * INFO macros
 */
#define INFO_EMDR_FUNDEF(n) (n->fundef)
#define INFO_EMDR_LHS(n) (n->lhs)
#define INFO_EMDR_REUSELUT(n) (n->reuselut)
#define INFO_EMDR_RENAMELUT(n) (n->renamelut)

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_EMDR_FUNDEF (result) = fundef;
    INFO_EMDR_LHS (result) = NULL;
    INFO_EMDR_REUSELUT (result) = NULL;
    INFO_EMDR_RENAMELUT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRDataReuse( node *syntax_tree)
 *
 * @brief starting point of DataReuseOptimization
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMDRDataReuse (node *syntax_tree)
{
    DBUG_ENTER ("EMDRDataReuse");

    DBUG_PRINT ("EMDR", ("Data reuse optimization..."));

    act_tab = emdr_tab;

    syntax_tree = Trav (syntax_tree, NULL);

    DBUG_PRINT ("EMDR", ("Data reuse optimization complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Data reuse optimization traversal (emdr_tab)
 *
 * prefix: EMDR
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMDRap( node *arg_node, info *arg_info)
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
EMDRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /*
     * CONDFUNs are traversed in order of appearance
     */
    if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRassign( node *arg_node, info *arg_info)
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
EMDRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRassign");

    /*
     * Top-down traversal
     */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRcode( node *arg_node, info *arg_info)
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
EMDRcode (node *arg_node, info *arg_info)
{
    node *exprs;

    DBUG_ENTER ("EMDRcode");

    /*
     * Traverse into CBLOCK in order to apply datareuse in nested with-loops
     */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /*
     * rename CEXPRS if applicable
     */
    NCODE_CEXPRS (arg_node) = Trav (NCODE_CEXPRS (arg_node), arg_info);

    /*
     * The great moment:
     * check whether CEXPRS perform INPLACE-COPY-OPERATIONS
     */
    exprs = NCODE_CEXPRS (arg_node);
    while (exprs != NULL) {
        node *id = NULL;
        node *iv = NULL;
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

                valavis = ID_AVIS (val);

                if ((AVIS_SSAASSIGN (valavis) != NULL)
                    && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (valavis))) == N_prf)
                    && (PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (valavis))) == F_fill)) {
                    node *sel = PRF_ARG1 (ASSIGN_RHS (AVIS_SSAASSIGN (valavis)));

                    /*
                     * Pattern:
                     *
                     * a = fill( B[iv], a');
                     * r = wl_assign( a, A', iv);
                     *
                     * where A' is known to be a reuse of B
                     */
                    if ((NODE_TYPE (sel) == N_prf) && (PRF_PRF (sel) == F_sel)) {
                        node *vec = PRF_ARG1 (sel);
                        node *arr = PRF_ARG2 (sel);

                        if ((ID_VARDEC (iv) == ID_VARDEC (vec))
                            && (SearchInLUT_PP (INFO_EMDR_REUSELUT (arg_info),
                                                ID_VARDEC (mem))
                                == ID_VARDEC (arr))) {
                            inplace = TRUE;
                        }
                    }

                    /*
                     * Pattern:
                     *
                     *     SAC_ND_USE_GENVAR_OFFSET( idx, A);
                     * a = fill( idx_sel( idx, B), a');
                     * r = wl_assign( a, A', iv);
                     *
                     * where A' is known to be a reuse of B
                     */
                    if ((NODE_TYPE (sel) == N_prf) && (PRF_PRF (sel) == F_idx_sel)) {
                        node *idx = PRF_ARG1 (sel);
                        node *arr = PRF_ARG2 (sel);

                        node *idxavis = ID_AVIS (idx);
                        if (AVIS_SSAASSIGN (idxavis) != NULL) {
                            node *lastass = NULL;
                            node *idxass = AVIS_SSAASSIGN (idxavis);
                            while (idxass != NULL) {
                                node *icm = ASSIGN_INSTR (idxass);
                                if ((NODE_TYPE (icm) == N_icm)
                                    && (!strcmp (ICM_NAME (icm), "ND_USE_GENVAR_OFFSET"))
                                    && (ID_VARDEC (idx) == ID_VARDEC (ICM_ARG1 (icm)))
                                    && (SearchInLUT_PP (INFO_EMDR_REUSELUT (arg_info),
                                                        ID_VARDEC (mem))
                                        == ID_VARDEC (arr))) {
                                    DBUG_ASSERT (lastass != NULL,
                                                 "Write a better SSADCR, Man!");
                                    ASSIGN_NEXT (lastass)
                                      = FreeNode (ASSIGN_NEXT (lastass));
                                    inplace = TRUE;
                                    break;
                                }
                                lastass = idxass;
                                idxass = ASSIGN_NEXT (idxass);
                            }
                        }
                    }
                }
            }
        }

        if (inplace) {
            node *avis;
            ids *lhs;

            DBUG_PRINT ("EMDR", ("Inplace copy situation recognized!"));

            /*
             * Create a variable for new cexpr
             */
            FUNDEF_VARDEC (INFO_EMDR_FUNDEF (arg_info))
              = MakeVardec (TmpVar (), DupOneTypes (VARDEC_TYPE (ID_VARDEC (id))),
                            FUNDEF_VARDEC (INFO_EMDR_FUNDEF (arg_info)));

            avis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMDR_FUNDEF (arg_info)));
            AVIS_TYPE (avis) = TYCopyType (AVIS_TYPE (ID_AVIS (id)));

            /*
             * Create noop
             * a = noop( iv);
             */
            lhs = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                           ST_regular);
            IDS_AVIS (lhs) = avis;
            IDS_VARDEC (lhs) = AVIS_VARDECORARG (avis);

            NCODE_CBLOCK_INSTR (arg_node)
              = MakeAssign (MakeLet (MakePrf1 (F_noop, DupNode (iv)), lhs),
                            NCODE_CBLOCK_INSTR (arg_node));

            AVIS_SSAASSIGN (IDS_AVIS (lhs)) = NCODE_CBLOCK_INSTR (arg_node);

            EXPRS_EXPR (exprs) = FreeNode (EXPRS_EXPR (exprs));
            EXPRS_EXPR (exprs) = MakeIdFromIds (DupOneIds (lhs));
        }

        exprs = EXPRS_NEXT (exprs);
    }

    /*
     * Traverse next code
     */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRfundef( node *arg_node, info *arg_info)
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
EMDRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRfundef");

    /*
     * CONDFUNs may only be traversed from AP-nodes
     */
    if ((!FUNDEF_IS_CONDFUN (arg_node)) || (arg_info != NULL)) {

        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info = MakeInfo (arg_node);

            INFO_EMDR_REUSELUT (info) = GenerateLUT ();
            INFO_EMDR_RENAMELUT (info) = GenerateLUT ();

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

            INFO_EMDR_REUSELUT (info) = RemoveLUT (INFO_EMDR_REUSELUT (info));
            INFO_EMDR_RENAMELUT (info) = RemoveLUT (INFO_EMDR_RENAMELUT (info));

            info = FreeInfo (info);
        }
    }

    /*
     * Traverse next fundef
     */
    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRid( node *arg_node, info *arg_info)
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
EMDRid (node *arg_node, info *arg_info)
{
    node *vardec;

    DBUG_ENTER ("EMDRid");

    vardec = SearchInLUT_PP (INFO_EMDR_RENAMELUT (arg_info), ID_VARDEC (arg_node));

    if (vardec != ID_VARDEC (arg_node)) {
        arg_node = FreeNode (arg_node);

        arg_node = MakeId (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular);

        ID_VARDEC (arg_node) = vardec;
        ID_AVIS (arg_node) = VARDEC_AVIS (vardec);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRlet( node *arg_node, info *arg_info)
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
EMDRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRlet");

    INFO_EMDR_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMDRprf( node *arg_node, info *arg_info)
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
EMDRprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMDRprf");

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
    }

    switch (PRF_PRF (arg_node)) {
    case F_reuse:
        /*
         * b = reuse( a);
         *
         * Insert (b, a) into REUSELUT
         */
        InsertIntoLUT_P (INFO_EMDR_REUSELUT (arg_info),
                         IDS_VARDEC (INFO_EMDR_LHS (arg_info)),
                         ID_VARDEC (PRF_ARG1 (arg_node)));
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
                 *   Insert ( c, a) into RENAMELUT
                 */
                if (SearchInLUT_PP (INFO_EMDR_REUSELUT (arg_info),
                                    ID_VARDEC (PRF_ARG2 (arg_node)))
                    == ID_VARDEC (PRF_ARG1 (prf))) {

                    InsertIntoLUT_P (INFO_EMDR_RENAMELUT (arg_info),
                                     IDS_VARDEC (INFO_EMDR_LHS (arg_info)),
                                     ID_VARDEC (PRF_ARG1 (prf)));
                }
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
