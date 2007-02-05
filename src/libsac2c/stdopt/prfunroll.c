/**
 * $Id: datareuse.c 14355 2005-10-30 10:32:28Z ktr $
 *
 * @defgroup uprf unroll primitive operations
 * @ingroup opt
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file prfunroll.c
 *
 *
 */
#include "prfunroll.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *vardec;
    node *lhs;
    node *preassign;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_VARDEC(n) ((n)->vardec)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_PREASSIGN(n) ((n)->preassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = TRUE;
    INFO_VARDEC (result) = NULL;
    INFO_LHS (result) = NULL;
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
 * @fn node *UPRFdoUnrollPRFs( node *fundef)
 *
 * @brief starting point of prf unrolling
 *
 * @param fundef
 *
 * @return fundef
 *
 *****************************************************************************/
node *
UPRFdoUnrollPRFs (node *fundef)
{
    info *info;

    DBUG_ENTER ("UPRFdoUnrollPRFs");

    info = MakeInfo ();

    TRAVpush (TR_uprf);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFdoUnrollPRFsModule( node *syntax_tree)
 *
 * @brief starting point of prf unrolling
 *
 * @param syntax_tree
 *
 * @return syntax_tree
 *
 *****************************************************************************/
node *
UPRFdoUnrollPRFsModule (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("UPRFdoUnrollPRFsModule");

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = FALSE;

    TRAVpush (TR_uprf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static bool
PRFUnrollOracle (prf p)
{
    bool res;

    DBUG_ENTER ("PRFUnrollOracle");

    switch (p) {
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:

    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:

    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:

    case F_div_AxS:
    case F_div_SxA:
    case F_div_AxA:

        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

static prf
NormalizePrf (prf p)
{
    DBUG_ENTER ("NormalizePrf");

    switch (p) {
    case F_add_AxS:
    case F_add_SxA:
    case F_add_AxA:
        p = F_add_SxS;
        break;

    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        p = F_sub_SxS;
        break;

    case F_mul_AxS:
    case F_mul_SxA:
    case F_mul_AxA:
        p = F_mul_SxS;
        break;

    case F_div_AxS:
    case F_div_SxA:
    case F_div_AxA:
        p = F_div_SxS;
        break;

    default:
        DBUG_ASSERT ((FALSE), "Illegal prf!");
        break;
    }

    DBUG_RETURN (p);
}

static bool
FirstArgScalar (prf p)
{
    bool res;

    DBUG_ENTER ("FirstArgScalar");

    switch (p) {
    case F_add_SxA:
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

static bool
SecondArgScalar (prf p)
{
    bool res;

    DBUG_ENTER ("SecondArgScalar");

    switch (p) {
    case F_add_AxS:
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
        res = TRUE;
        break;

    default:
        res = FALSE;
        break;
    }

    DBUG_RETURN (res);
}

static node *
RevertExprs (node *exprs, node *agg)
{
    node *res;

    DBUG_ENTER ("RevertExprs");

    if (exprs == NULL) {
        res = agg;
    } else {
        res = EXPRS_NEXT (exprs);
        EXPRS_NEXT (exprs) = agg;
        res = RevertExprs (res, exprs);
    }

    DBUG_RETURN (res);
}

static node *
RevertAssignments (node *ass, node *agg)
{
    node *res;

    DBUG_ENTER ("RevertAssignments");

    if (ass == NULL) {
        res = agg;
    } else {
        res = ASSIGN_NEXT (ass);
        ASSIGN_NEXT (ass) = agg;
        res = RevertAssignments (res, ass);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * PRF unrolling traversal (uprf_tab)
 *
 * prefix: UPRF
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *UPRFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UPRFfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDEC (arg_info) = FUNDEF_VARDEC (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_VARDEC (arg_node) = INFO_VARDEC (arg_info);
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
 * @fn node *UPRFassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UPRFassign");

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
 * @fn node *UPRFlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UPRFlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UPRFprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
UPRFprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("UPRFprf");

    if ((PRFUnrollOracle (PRF_PRF (arg_node)))
        && (TYisAKS (IDS_NTYPE (INFO_LHS (arg_info))))
        && (TYgetDim (IDS_NTYPE (INFO_LHS (arg_info))) == 1)) {
        int len;
        ntype *nt1, *nt2;

        len = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LHS (arg_info))));
        nt1 = NTCnewTypeCheck_Expr (PRF_ARG1 (arg_node));
        nt2 = NTCnewTypeCheck_Expr (PRF_ARG2 (arg_node));

        if ((TUshapeKnown (nt1)) && (TUshapeKnown (nt2)) && (len < global.wlunrnum)) {
            ntype *scl;
            node *avis1, *avis2;
            node *ass = NULL;
            node *elems = NULL;
            int i;

            /*
             * Create new assignments for the two arguments
             */
            avis1 = TBmakeAvis (TRAVtmpVar (), TYcopyType (nt1));
            INFO_VARDEC (arg_info) = TBmakeVardec (avis1, INFO_VARDEC (arg_info));
            ass = TBmakeAssign (TBmakeLet (TBmakeIds (avis1, NULL),
                                           DUPdoDupNode (PRF_ARG1 (arg_node))),
                                ass);
            AVIS_SSAASSIGN (avis1) = ass;

            avis2 = TBmakeAvis (TRAVtmpVar (), TYcopyType (nt2));
            INFO_VARDEC (arg_info) = TBmakeVardec (avis2, INFO_VARDEC (arg_info));
            ass = TBmakeAssign (TBmakeLet (TBmakeIds (avis2, NULL),
                                           DUPdoDupNode (PRF_ARG2 (arg_node))),
                                ass);
            AVIS_SSAASSIGN (avis2) = ass;

            scl = TYmakeAKS (TYcopyType (TYgetScalar (nt1)), SHmakeShape (0));

            for (i = 0; i < len; i++) {
                node *argavis1, *argavis2, *resavis;
                if (FirstArgScalar (PRF_PRF (arg_node))) {
                    argavis1 = avis1;
                } else {
                    node *selarravis
                      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (1, 1)));
                    INFO_VARDEC (arg_info)
                      = TBmakeVardec (selarravis, INFO_VARDEC (arg_info));
                    ass = TBmakeAssign (TBmakeLet (TBmakeIds (selarravis, NULL),
                                                   TCmakeIntVector (
                                                     TBmakeExprs (TBmakeNum (i), NULL))),
                                        ass);
                    AVIS_SSAASSIGN (selarravis) = ass;

                    argavis1 = TBmakeAvis (TRAVtmpVar (), TYcopyType (scl));
                    INFO_VARDEC (arg_info)
                      = TBmakeVardec (argavis1, INFO_VARDEC (arg_info));
                    ass
                      = TBmakeAssign (TBmakeLet (TBmakeIds (argavis1, NULL),
                                                 TCmakePrf2 (F_sel, TBmakeId (selarravis),
                                                             TBmakeId (avis1))),
                                      ass);
                    AVIS_SSAASSIGN (argavis1) = ass;
                }

                if (SecondArgScalar (PRF_PRF (arg_node))) {
                    argavis2 = avis2;
                } else {
                    node *selarravis
                      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (1, 1)));
                    INFO_VARDEC (arg_info)
                      = TBmakeVardec (selarravis, INFO_VARDEC (arg_info));
                    ass = TBmakeAssign (TBmakeLet (TBmakeIds (selarravis, NULL),
                                                   TCmakeIntVector (
                                                     TBmakeExprs (TBmakeNum (i), NULL))),
                                        ass);
                    AVIS_SSAASSIGN (selarravis) = ass;

                    argavis2 = TBmakeAvis (TRAVtmpVar (), TYcopyType (scl));
                    INFO_VARDEC (arg_info)
                      = TBmakeVardec (argavis2, INFO_VARDEC (arg_info));
                    ass
                      = TBmakeAssign (TBmakeLet (TBmakeIds (argavis2, NULL),
                                                 TCmakePrf2 (F_sel, TBmakeId (selarravis),
                                                             TBmakeId (avis2))),
                                      ass);
                    AVIS_SSAASSIGN (argavis2) = ass;
                }

                resavis = TBmakeAvis (TRAVtmpVar (), TYcopyType (scl));
                INFO_VARDEC (arg_info) = TBmakeVardec (resavis, INFO_VARDEC (arg_info));
                ass = TBmakeAssign (TBmakeLet (TBmakeIds (resavis, NULL),
                                               TCmakePrf2 (NormalizePrf (
                                                             PRF_PRF (arg_node)),
                                                           TBmakeId (argavis1),
                                                           TBmakeId (argavis2))),
                                    ass);
                AVIS_SSAASSIGN (resavis) = ass;

                elems = TBmakeExprs (TBmakeId (resavis), elems);
            }

            scl = TYfreeType (scl);

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TCmakeVector (TYmakeAKS (TYcopyType (TYgetScalar (
                                                  IDS_NTYPE (INFO_LHS (arg_info)))),
                                                SHmakeShape (0)),
                                     RevertExprs (elems, NULL));

            INFO_PREASSIGN (arg_info) = RevertAssignments (ass, NULL);
        }

        nt1 = TYfreeType (nt1);
        nt2 = TYfreeType (nt2);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
