/*
 * $Id$
 *
 * @brief:
 *   1. Replaces sequence:
 *       c = F_esdneg( b);
 *       d = F_esdneg( c);
 *
 *     by:
 *
 *       d = b;
 *
 *   2. Replaces sequence:
 *       c = add( a, const);    NB.  and others
 *       d = F_esdneg( c);
 *
 *     by:
 *
 *       a' = F_esdneg( a);
 *       b' = F_esdneg( b);
 *       d = add( a', b');
 *
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
#include "pattern_match.h"
#include "arithmetic_simplification.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *preassign;
    node *fundef;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) (n->onefundef)
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

    INFO_ONEFUNDEF (result) = FALSE;
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
 * @fn node *ContainedPrf( node *expression)
 *
 * @brief Locates the expression that created PRF_ARG1( expression)
 *
 * @param arg_node - PRF_ARG1 of an N_prf.
 *
 * @note: We chase the assign chain for PRF_ARG1 back through
 *        mulitple assigns until we hit an N_prf, ignoring any
 *        guard-like primitives we encounter along the way.
 *        The latter behavior is required to allow expression
 *        simplification in the presence of guards and extrema.
 *
 * @return The RHS of the N_prf expression that created PRF_ARG1, if it
 *         of the appropriate type:
 *         If no such expression exists, NULL.
 *
 *****************************************************************************/

node *
ContainedPrf (node *arg_node)
{
    node *val = NULL;

    DBUG_ENTER ("ContainedPrf");

    /* chase back over assigns and guards, then look for any primitive */
    if (PMO (PMOlastVarGuards (&val, arg_node))) {
        val = AVIS_SSAASSIGN (ID_AVIS (val));
        if (NULL != val) {
            val = LET_EXPR (ASSIGN_INSTR (val));
            val = (N_prf == NODE_TYPE (val)) ? val : NULL;
        }
    }

    DBUG_RETURN (val);
}

/** <!--********************************************************************-->
 *
 * @fn bool IsSuitableForPropagation( node *expression)
 *
 * @brief: Predicate for identifying expressions of the form:
 *
 *           _add_SxS_( X, constant)
 *           _add_SxS_( constant, X)
 *           _add_SxV_( X, constant)
 *           _add_SxV_( constant, X)
 *           _add_VxS_( X, constant)
 *           _add_VxS_( constant, X)
 *           _add_VxV_( X, constant)
 *           _add_VxV_( constant, X)
 *
 *
 * @param arg_node - An N_prf
 *
 * @return  TRUE if expression satisfies the brief requirement.
 *
 *****************************************************************************/
bool
IsSuitableForPropagation (node *expression)
{
    node *val1 = NULL;
    node *val2 = NULL;
    constant *con1 = NULL;
    constant *con2 = NULL;
    bool result = FALSE;

    DBUG_ENTER ("IsSuitableForPropagation");

    if ((NODE_TYPE (PRF_ARG1 (expression)) == N_id)
        && (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (expression))) != NULL)
        && ((PRF_PRF (expression) == F_add_SxS) || (PRF_PRF (expression) == F_add_SxV)
            || (PRF_PRF (expression) == F_add_VxS)
            || (PRF_PRF (expression) == F_add_VxV))) {

        if (PMO (PMOconst (&con1, &val1, PRF_ARG1 (expression)))
            || PMO (PMOconst (&con2, &val2, PRF_ARG2 (expression)))) {
            result = TRUE;
        }
        con1 = (NULL != con1) ? COfreeConstant (con1) : NULL;
        con2 = (NULL != con2) ? COfreeConstant (con2) : NULL;
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
        result
          = ((NODE_TYPE (expression) == N_prf) && (PRF_PRF (expression) == F_esd_neg));
    } else {
        result = FALSE;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *Negate( node *arg_node)
 *
 * @brief  Transform arg_node into neg( arg_node)
 *
 * @param arg_node - PRF_ARG1/2 of an N_prf.
 *
 *
 * @return  FIXMEj
 *
 *****************************************************************************/
node *
Negate (node *arg_node, info *info)
{
    node *negexpr;
    ntype *negtype;
    node *avis;
    node *val = NULL;
    constant *cexpr = NULL;
    constant *negcexpr;
    ntype *prodtype;

    DBUG_ENTER ("Negate");

    if (PMO (PMOconst (&cexpr, &val, arg_node))) {
        /* Create negexpr */
        negcexpr = COneg (cexpr);
        negexpr = COconstant2AST (negcexpr);
        negcexpr = COfreeConstant (negcexpr);
        cexpr = COfreeConstant (cexpr);
        negtype = NTCnewTypeCheck_Expr (negexpr);
    } else {
        negexpr = TCmakePrf1 (F_esd_neg, DUPdoDupTree (arg_node));
        prodtype = NTCnewTypeCheck_Expr (negexpr);
        negtype = TYcopyType (TYgetProductMember (prodtype, 0));
        prodtype = TYfreeType (prodtype);
    }

    avis = TBmakeAvis (TRAVtmpVar (), negtype);

    INFO_PREASSIGN (info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), negexpr), INFO_PREASSIGN (info));
    AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (info);

    if (N_id == NODE_TYPE (arg_node)) {
        AVIS_DIM (avis) = DUPdoDupNode (AVIS_DIM (ID_AVIS (arg_node)));
        AVIS_SHAPE (avis) = DUPdoDupNode (AVIS_SHAPE (ID_AVIS (arg_node)));
    }

    TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (info)), TBmakeVardec (avis, NULL));

    DBUG_RETURN (TBmakeId (avis));
}

/** <!--********************************************************************-->
 *
 * @fn node *ASdoArithmeticSimplificationModule( node *arg_node)
 *
 * @brief starting point of arithmetic simplification for a module.
 *
 * @param arg_node - An N_module
 *
 * @return
 *
 *****************************************************************************/
node *
ASdoArithmeticSimplificationModule (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("ASdoArithmeticSimplificationModule");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module, "AS called on non-N_module node");

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = FALSE;

    TRAVpush (TR_as);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ASdoArithmeticSimplification( node *arg_node)
 *
 * @brief starting point of arithmetic simplification for a function.
 *
 * @param arg_node - An N_fundef
 *
 * @return
 *
 *****************************************************************************/
node *
ASdoArithmeticSimplification (node *arg_node)
{
    info *info;

    DBUG_ENTER ("ASdoArithmeticSimplification");

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "AS called on non-N_fundef node");

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_as);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

node *
ASfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("ASfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    INFO_FUNDEF (arg_info) = NULL;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ASassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASassign");

    /*
     * Bottom-up traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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
    node *contained;

    DBUG_ENTER ("ASprf");

    contained = ContainedPrf (PRF_ARG1 (arg_node));
    if ((PRF_PRF (arg_node) == F_esd_neg) && (NULL != contained)) {

        if (IsSuitableForPropagation (contained)) {
            arg_node = FREEdoFreeTree (arg_node);
            arg_node
              = TCmakePrf2 (PRF_PRF (contained), Negate (PRF_ARG1 (contained), arg_info),
                            Negate (PRF_ARG2 (contained), arg_info));
            DBUG_PRINT ("AS", ("Negating both arguments"));
        } else if (IsNegationOfNegation (contained)) {
            DBUG_PRINT ("AS", ("Replacing negation of negation"));
            contained = PRF_ARG1 (contained);
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = DUPdoDupTree (contained);
        }
    }

    DBUG_RETURN (arg_node);
}
