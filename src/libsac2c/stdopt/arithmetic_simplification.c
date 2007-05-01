/*
 * $Id$
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "constants.h"
#include "new_typecheck.h"
#include "new_types.h"
#include "arithmetic_simplification.h"

/*
 * INFO structure
 */
struct INFO {
    node *preassign;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_FUNDEF(n) ((n)->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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

node *
ContainedPrf (node *expression)
{
    node *contained;

    DBUG_ENTER ("ContainedPrf");

    DBUG_ASSERT ((NODE_TYPE (expression) == N_prf), "prf expected!");
    DBUG_ASSERT ((NODE_TYPE (PRF_ARG1 (expression)) == N_id), "N_id argument expected");
    DBUG_ASSERT ((AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (expression))) != NULL),
                 "SSAASSIGN is NULL!");

    contained
      = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (expression)))));

    DBUG_RETURN (contained);
}

bool
IsSuitableForPropagation (node *expression)
{
    bool result;

    DBUG_ENTER ("IsSuitableForPropagation");

    if ((NODE_TYPE (PRF_ARG1 (expression)) == N_id)
        && (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (expression))) != NULL)) {
        expression = ContainedPrf (expression);

        /*
         * In short, we are looking for add operations where
         * at least one argument is a constant 8-)
         */
        result = ((NODE_TYPE (expression) == N_prf)
                  && ((PRF_PRF (expression) == F_add_SxS)
                      || (PRF_PRF (expression) == F_add_SxA)
                      || (PRF_PRF (expression) == F_add_AxS)
                      || (PRF_PRF (expression) == F_add_AxA))
                  && (COisConstant (PRF_ARG1 (expression))
                      || COisConstant (PRF_ARG2 (expression))));
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

bool
IsNegationOfNegation (node *expression)
{
    bool result;

    DBUG_ENTER ("IsNegationOfNegation");

    if ((NODE_TYPE (PRF_ARG1 (expression)) == N_id)
        && (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (expression))) != NULL)) {
        expression = ContainedPrf (expression);
        result
          = ((NODE_TYPE (expression) == N_prf) && (PRF_PRF (expression) == F_esd_neg));
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

node *
Negate (node *expression, info *info)
{
    node *negexpr;
    ntype *negtype;
    node *avis;

    DBUG_ENTER ("Negate");

    /*
     * transform a into neg( a)
     */
    if (COisConstant (expression)) {
        constant *cexpr = COaST2Constant (expression);
        constant *negcexpr = COneg (cexpr);
        negexpr = COconstant2AST (negcexpr);

        negcexpr = COfreeConstant (negcexpr);
        cexpr = COfreeConstant (cexpr);

        negtype = NTCnewTypeCheck_Expr (negexpr);
    } else {
        ntype *prodtype;

        negexpr = TCmakePrf1 (F_esd_neg, DUPdoDupTree (expression));

        prodtype = NTCnewTypeCheck_Expr (negexpr);
        negtype = TYcopyType (TYgetProductMember (prodtype, 0));
        prodtype = TYfreeType (prodtype);
    }

    avis = TBmakeAvis (TRAVtmpVar (), negtype);

    INFO_PREASSIGN (info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), negexpr), INFO_PREASSIGN (info));

    TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (info)), TBmakeVardec (avis, NULL));

    DBUG_RETURN (TBmakeId (avis));
}

/** <!--********************************************************************-->
 *
 * @fn node *ASdoArithmeticSimplification( node *arg_node)
 *
 * @brief starting point of arithmetic simplification.
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
ASdoArithmeticSimplification (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ASdoArithmeticSimplification");

    info = MakeInfo ();

    TRAVpush (TR_as);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
ASfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
ASassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    while (INFO_PREASSIGN (arg_info) != NULL) {
        node *toprocess = INFO_PREASSIGN (arg_info);
        INFO_PREASSIGN (arg_info) = NULL;

        toprocess = TRAVdo (toprocess, arg_info);
        arg_node = TCappendAssign (toprocess, arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
ASprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASprf");

    if ((PRF_PRF (arg_node) == F_esd_neg) && IsSuitableForPropagation (arg_node)) {
        node *contained = ContainedPrf (arg_node);
        arg_node = FREEdoFreeTree (arg_node);

        arg_node
          = TCmakePrf2 (PRF_PRF (contained), Negate (PRF_ARG1 (contained), arg_info),
                        Negate (PRF_ARG2 (contained), arg_info));
    } else if ((PRF_PRF (arg_node) == F_esd_neg) && IsNegationOfNegation (arg_node)) {
        node *contained = PRF_ARG1 (ContainedPrf (arg_node));
        arg_node = FREEdoFreeTree (arg_node);

        arg_node = DUPdoDupTree (contained);
    }

    DBUG_RETURN (arg_node);
}
