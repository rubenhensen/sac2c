/**
 *
 * @file ElimSubDiv.c
 *
 * @brief replace subtraction by addition and division by multiplication
 *
 * We do the following transformations:
 *
 *  a - b  =>  a + -b  // Here, - is a unary negation operator
 *  a / b  =>  a * /b  // Here, / is a unary reciprocal operator
 *
 * The rationale for this transformation is to create an extended domain
 * for associative law optimisation. It will be undone by the complementary
 * UndoElimSubDiv traversal.
 */

#include "ElimSubDiv.h"

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "ESD"
#include "debug.h"

#include "memory.h"
#include "new_typecheck.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *fundef;
    node *newassign;
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NEWASSIGN(n) ((n)->newassign)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEWASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

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
    DBUG_ENTER ();

    switch (op) {
    case F_sub_SxS:
        result = F_add_SxS;
        break;

    case F_sub_SxV:
        result = F_add_SxV;
        break;

    case F_sub_VxS:
        result = F_add_VxS;
        break;

    case F_sub_VxV:
        result = F_add_VxV;
        break;

    case F_div_SxS:
        result = F_mul_SxS;
        break;

    case F_div_SxV:
        result = F_mul_SxV;
        break;

    case F_div_VxS:
        result = F_mul_VxS;
        break;

    case F_div_VxV:
        result = F_mul_VxV;
        break;

    default:
        DBUG_ASSERT (0, "Illegal argument prf!");
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
 * @fn static prf InversionPrf(prf op)
 *
 * @brief returns opposite primitive operator of op
 *
 * @param op primitive operator
 *
 * @return opposite primitive operator of op
 *
 **********************************************************************/

static prf
InversionPrf (prf op, simpletype stype)
{
    prf inv_prf;

    DBUG_ENTER ();

    switch (stype) {
    case T_float:
    case T_double:
        if (!global.enforce_ieee) {
            switch (op) {
            case F_div_SxS:
            case F_div_VxS:
                inv_prf = F_reciproc_S;
                break;
            case F_div_SxV:
            case F_div_VxV:
                inv_prf = F_reciproc_V;
                break;
            default:
                inv_prf = F_unknown;
            }
        }
        /* There is no break missing here. */

    case T_byte:
    case T_short:
    case T_int:
    case T_long:
    case T_longlong:
    case T_ubyte:
    case T_ushort:
    case T_uint:
    case T_ulong:
    case T_ulonglong:
        switch (op) {
        case F_sub_SxS:
        case F_sub_VxS:
            inv_prf = F_neg_S;
            break;
        case F_sub_SxV:
        case F_sub_VxV:
            inv_prf = F_neg_V;
            break;
        default:
            inv_prf = F_unknown;
        }
        break;
    default:
        inv_prf = F_unknown;
    }

    DBUG_RETURN (inv_prf);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESDmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only functions of the module, skipping all the rest for
 *        performance reasons.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
ESDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
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
ESDdoElimSubDiv (node *arg_node)
{
    info *info;
    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_esd);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************-->
 *
 * @fn node *ESDfundef(node arg_node, info *arg_info)
 *
 * @brief Traverses into fundef local LAC fuctions, then function
 *        bodies and finally function next pointers. When traversing
 *        into a body a pointer in the info struct is maintained to
 *        the inner fundef.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 **********************************************************************/
node *
ESDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
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
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_NEWASSIGN (arg_info) = NULL;

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**<!--**************************************************************-->
 *
 * @fn ESDprf(node *arg_node, info *arg_info)
 *
 * @brief removes subtraction and division, introduces negation
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
    prf op = F_unknown;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    op = InversionPrf (PRF_PRF (arg_node),
                       TYgetSimpleType (TYgetScalar (IDS_NTYPE (INFO_LHS (arg_info)))));

    if (op != F_unknown) {
        node *avis, *vardec;
        node *prf = NULL;
        ntype *ptype;

        /*
         * create new assignment
         */
        prf = TBmakePrf (op, EXPRS_NEXT (PRF_ARGS (arg_node)));
        EXPRS_NEXT (PRF_ARGS (arg_node)) = NULL;

        ptype = NTCnewTypeCheck_Expr (prf);
        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (TYgetProductMember (ptype, 0)));
        ptype = TYfreeType (ptype);

        INFO_NEWASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), prf), NULL);
        AVIS_SSAASSIGN (avis) = INFO_NEWASSIGN (arg_info);

        /*
         * create new vardec
         */
        vardec = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info)) = vardec;

        /*
         * change current prf
         */
        EXPRS_NEXT (PRF_ARGS (arg_node)) = TBmakeExprs (TBmakeId (avis), NULL);
        PRF_PRF (arg_node) = TogglePrf (PRF_PRF (arg_node));

        DBUG_PRINT ("replacing prf for %s with %s",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), AVIS_NAME (avis));

        /* ESD is NOT an optimization, merely a transformer for AL/DL/AS.
         * Hence, it should NOT cause an optcounters increment,
         * which could be the cause of never-ending CYC/SAACYC cycles.
         ++global.optcounters.esd_expr;
        */
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
