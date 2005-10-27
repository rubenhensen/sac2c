/**
 *
 * $Id$
 *
 * @file ElimSubDiv.c
 *
 * @brief replaces substration and division by introducing special
 * primitive negation and reciprocal operator (F_esd_neg, F_esd_rec)
 *
 */
#include "ElimSubDiv.h"

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "new_typecheck.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *newassign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_NEWASSIGN(n) (n->newassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_NEWASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**<!--**************************************************************-->
 *
 * @fn static prf TogglePrf(prf op)
 *
 * @brief returns opposite primitive operator of op
 *
 * @param op primitive operator
 *
 * @return opposite primitive operator of op
 *
 **********************************************************************/
static prf
TogglePrf (prf op)
{
    prf result;
    DBUG_ENTER ("TogglePrf");

    switch (op) {
    case F_sub_SxS:
        result = F_add_SxS;
        break;

    case F_sub_SxA:
        result = F_add_SxA;
        break;

    case F_sub_AxS:
        result = F_add_AxS;
        break;

    case F_sub_AxA:
        result = F_add_AxA;
        break;

    case F_div_SxS:
        result = F_mul_SxS;
        break;

    case F_div_SxA:
        result = F_mul_SxA;
        break;

    case F_div_AxS:
        result = F_mul_AxS;
        break;

    case F_div_AxA:
        result = F_mul_AxA;
        break;

    default:
        DBUG_ASSERT ((0), "Illegal argument prf!");
        /*
         * the following line initialises result, as the product
         * version will continue execution (the DBUG_ASSERT is
         * ignored!)
         */
        result = F_unknown;
    }

    DBUG_RETURN (result);
}

/**<!--**************************************************************-->
 *
 * @fn node *ESDdoElimSubDiv(node *fundef)
 *
 * @brief start function for traversal
 *
 * @param fundef fundef node
 *
 * @return
 *
 **********************************************************************/
node *
ESDdoElimSubDiv (node *fundef)
{
    info *info;

    DBUG_ENTER ("ESDdoElimSubDiv");

    if (FUNDEF_BODY (fundef) != NULL) {
        info = MakeInfo ();

        INFO_FUNDEF (info) = fundef;

        TRAVpush (TR_esd);
        FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), info);
        TRAVpop ();

        info = FreeInfo (info);
    }

    DBUG_RETURN (fundef);
}

/**<!--***************************************************************-->
 *
 * @fn ESDblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***************************************************************-->
 *
 * @fn ESDassign(node *arg_node, info *arg_info)
 *
 * @brief traverses in instructions and inserts new assignments
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_NEWASSIGN (arg_info) = NULL;

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_NEWASSIGN (arg_info) != NULL) {
        /*
         * insert new assignment
         */
        ASSIGN_NEXT (INFO_NEWASSIGN (arg_info)) = arg_node;
        arg_node = INFO_NEWASSIGN (arg_info);

        INFO_NEWASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**<!--*************************************************************-->
 *
 * @fn ESDlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESDlet");

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************-->
 *
 * @fn ESDprf(node *arg_node, info *arg_info)
 *
 * @brief removes substartion and division, introduces negation
 *        and reciprocal instead
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDprf (node *arg_node, info *arg_info)
{
    prf op = F_noop;

    DBUG_ENTER ("ESDprf");

    /*
     * Determine inverse prf
     */
    switch (PRF_PRF (arg_node)) {

    case F_sub_SxS:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_sub_AxA:
        op = F_esd_neg;
        break;
    case F_div_SxS:
    case F_div_AxS:
    case F_div_SxA:
    case F_div_AxA:
        op = F_esd_rec;
        break;
    default:
        break;
    }

    if (op != F_noop) {
        node *avis, *vardec;
        node *prf = NULL;

        /*
         * create new assignment
         */
        prf = TBmakePrf (op, EXPRS_NEXT (PRF_ARGS (arg_node)));
        EXPRS_NEXT (PRF_ARGS (arg_node)) = NULL;

        avis = TBmakeAvis (ILIBtmpVar (),
                           TYgetProductMember (NTCnewTypeCheck_Expr (prf), 0));

        INFO_NEWASSIGN (arg_info) = TCmakeAssignLet (avis, prf);
        AVIS_SSAASSIGN (avis) = INFO_NEWASSIGN (arg_info);

        /*
         * create new vardec
         */
        vardec = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info)) = vardec;

        /*
         * change current prf
         */
        EXPRS_NEXT (PRF_ARGS (arg_node)) = TBmakeExprs (TBmakeId (avis), NULL);
        PRF_PRF (arg_node) = TogglePrf (PRF_PRF (arg_node));
    }

    DBUG_RETURN (arg_node);
}
