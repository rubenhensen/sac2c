/**
 *
 * $Id$
 *
 * @file ElimSubDiv.c
 *
 * @brief replaces subtraction introducing special
 *        primitive negation operator (F_esd_neg):
 *
 *      d = sub( b, c);
 *
 *    by:
 *
 *      c' = F_esdneg( c);
 *      d  = add( b, c);
 *
 *  This opt was also supposed to replace division by multiplication,
 *  but that is deactivated. See below.
 *
 */
#include "ElimSubDiv.h"

#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "new_typecheck.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEWASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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
    DBUG_ENTER ("TogglePrf");

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
 * @fn node *ESDdoElimSubDivModule(node *arg_node)
 *
 * @brief start function for traversal
 *
 * @param fundef N_module node
 *
 * @return
 *
 **********************************************************************/
node *
ESDdoElimSubDivModule (node *arg_node)
{
    info *info;
    DBUG_ENTER ("ESDdoElimSubDivModule");

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module, "ESD called on non-N_module node");

    INFO_ONEFUNDEF (info) = FALSE;

    TRAVpush (TR_esd);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

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
    DBUG_ENTER ("ESDdoElimSubDiv");

    info = MakeInfo ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "ESD called on non-N_fundef node");

    INFO_ONEFUNDEF (info) = TRUE;

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
    bool old_onefundef;

    DBUG_ENTER ("ESDFundef");

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("ESD", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("ESDblock");

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

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

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_NEWASSIGN (arg_info) = NULL;

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

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
    simpletype st;
    prf op = F_noop;

    DBUG_ENTER ("ESDprf");

    DBUG_PRINT ("ESD",
                ("Looking at prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)))));

    st = TYgetSimpleType (TYgetScalar (IDS_NTYPE (INFO_LHS (arg_info))));

    /*
     * Determine inverse prf
     */
    switch (PRF_PRF (arg_node)) {

    case F_sub_SxS:
    case F_sub_VxS:
    case F_sub_SxV:
    case F_sub_VxV:
        if ((st == T_int) || (st == T_float) || (st == T_double)) {
            op = F_esd_neg;
        }
        break;

#if 0
    /*
     * Deactivated for the time being as this might lead to
     * errornous results especially when evaluated by the TC
     */
  case F_div_SxS:
  case F_div_VxS:
  case F_div_SxV:
  case F_div_VxV:
    op = F_esd_rec;
    break;
#endif

    default:
        break;
    }

    if (op != F_noop) {
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
        vardec = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info)) = vardec;

        /*
         * change current prf
         */
        EXPRS_NEXT (PRF_ARGS (arg_node)) = TBmakeExprs (TBmakeId (avis), NULL);
        PRF_PRF (arg_node) = TogglePrf (PRF_PRF (arg_node));
        DBUG_PRINT ("ESD",
                    ("replacing prf for %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)))));
    }

    DBUG_RETURN (arg_node);
}
