/* *
 * $Log$
 * Revision 1.1  2004/12/08 12:17:59  mwe
 * Initial revision
 *
 */

/**************************************************************************
 *
 * DESCRIPTION:
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "dbug.h"
#include "internal_lib.h"
#include "optimize.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"

#include "type_upgrade.h"

/************************************************************************
 *
 * LOCAL HELPER FUNCTIONS:
 *
 ***********************************************************************/

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *withid;
};

#define INFO_TUP_FUNDEF(n) (n->fundef)
#define INFO_TUP_WITHID(n) (n->withid)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");
    result = ILIBmalloc (sizeof (info));

    INFO_TUP_FUNDEF (result) = NULL;
    INFO_TUP_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/***********************************************************************
 *
 * function:
 *   ntype *GetLowestType(ntype *a, ntype *b)
 *
 * description:
 *   This function returns a copy of the lowest type of 'a' and 'b'.
 *   !! 'a' and 'b' are released at the end of the function !!
 *
 ***********************************************************************/
static ntype *
GetLowestType (ntype *a, ntype *b)
{

    ntype *result;

    DBUG_ENTER ("GetLowestType");

    result = (TYleTypes (a, b)) ? TYcopyType (a) : TYcopyType (b);

    a = TYfreeType (a);
    b = TYfreeType (b);

    DBUG_RETURN (result);
}

/***********************************************************************
 *
 * function:
 *   node *AssignTypeToExpr(node *expr, ntype* type)
 *
 * description:
 *   The type information 'type' is appended to 'expr' depending on the
 *   NODE_TYPE of 'expr'.
 *   Type information for id nodes is appended at the corresponding
 *   avis-node. Type information for arrays is appended directly at
 *   the array-node.
 *
 ***********************************************************************/
static node *
AssignTypeToExpr (node *expr, ntype *type)
{

    DBUG_ENTER ("AssignTypeToExpr");

    if (NODE_TYPE (expr) == N_id) {

        /*
         * store type in avis node of expr
         */
        DBUG_ASSERT ((NULL != ID_AVIS (expr)), "missing AVIS node!");

        AVIS_TYPE (ID_AVIS (expr)) = TYfreeType (AVIS_TYPE (ID_AVIS (expr)));
        AVIS_TYPE (ID_AVIS (expr)) = TYcopyType (type);

    } else if (NODE_TYPE (expr) == N_array) {

        /*
         * store type in ARRAY_NTYPE
         */

        ARRAY_NTYPE (expr) = TYfreeType (ARRAY_NTYPE (expr));
        ARRAY_NTYPE (expr) = TYcopyType (type);

    } else {

        DBUG_ASSERT ((FALSE), "Unexpected node type found");
    }

    DBUG_RETURN (expr);
}

/**********************************************************************
 *
 * function:
 *   bool *IsArgumentOfSpecialFunction(node *arg)
 *
 * description:
 *   This function returns TRUE iff 'arg' is an argument of a
 *   special function (confun, dofun, whilefun).
 *
 *********************************************************************/
static bool
IsArgumentOfSpecialFunction (node *arg)
{

    bool result;
    DBUG_ENTER ("IsArgumentOfSpecialFunction");

    if (N_array == NODE_TYPE (arg)) {

        result = FALSE;

    } else if (N_id == NODE_TYPE (arg)) {

        if (N_vardec == NODE_TYPE (AVIS_DECL (ID_AVIS (arg))))
            result = FALSE;

        else if (N_arg == NODE_TYPE (AVIS_DECL (ID_AVIS (arg)))) {

            /*
             * arg is an argument of a function.
             * Now we have to verify wheter it is a special function or not.
             */
            DBUG_ASSERT ((ARG_FUNDEF (AVIS_DECL (ID_AVIS (arg))) != NULL),
                         "No pointer to fundef found");

            if ((FUNDEF_ISCONDFUN (ARG_FUNDEF (AVIS_DECL (ID_AVIS (arg)))))
                || (FUNDEF_ISDOFUN (ARG_FUNDEF (AVIS_DECL (ID_AVIS (arg)))))) {
                result = TRUE;
            } else {
                result = FALSE;
            }
        } else {

            result = FALSE;
            DBUG_ASSERT ((FALSE), "unexpected node type found!");
        }

    } else {

        result = FALSE;
        DBUG_ASSERT ((FALSE), "unexpected node type found!");
    }

    DBUG_RETURN (result);
}

static node *
TryToSpecializeFunction (node *fundef, node *args, info *arg_info)
{
    DBUG_ENTER ("TryToSpecializeFunction");

    DBUG_RETURN (fundef);
}

static node *
TryStaticDispatch (node *fundef, info *arg_info)
{
    DBUG_ENTER ("TryStaticDispatch");

    DBUG_RETURN (fundef);
}

/************************************************************************
 *
 * GLOBAL FUNCTIONS:
 *
 ************************************************************************/

/************************************************************************
 *
 * function:
 *    node *TypeUpgrade(node* arg_node, node* arg_info)
 *
 * description:
 *    This function is called from optimize.c, it is the starting point
 *    of TypeUpgrade.
 *    arg_node is expected to be an N_fundef node.
 *
 ***********************************************************************/

node *
TUPdoTypeUpgrade (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("TUPdoTypeUpgrade");

    if (arg_node != NULL) {

        arg_info = MakeInfo ();

        INFO_TUP_FUNDEF (arg_info) = arg_node;

        TRAVpush (TR_tup);
        arg_node = TRAVdo (arg_node, arg_info);
        TRAVpop ();
    }

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/***********************************************************************
 *
 * function:
 *   node *TUPblock(node* arg_node, node* arg_info)
 *
 * description:
 *   traverse through block nodes
 *
 **********************************************************************/
node *
TUPblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPblock");

    if (BLOCK_INSTR (arg_node) != NULL) {

        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/***********************************************************************
 *
 * function:
 *   node *TUPassign(node* arg_node, node* arg_info)
 *
 * description:
 *   Traverse through assign nodes. While top-down traversal the
 *   instructions are handled.
 *
 **********************************************************************/
node *
TUPassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPassign");

    if (ASSIGN_INSTR (arg_node) != NULL) {

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {

        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**********************************************************************
 *
 * function:
 *   node *TUPreturn(node* arg_node, node* arg_info)
 *
 * description:
 *   The type of the expressions in the return statement are compared to
 *   the return type of the function. If necessary the return type of
 *   the function is updated.
 *
 **********************************************************************/
node *
TUPreturn (node *arg_node, info *arg_info)
{

    node *fundef;
    DBUG_ENTER ("TUPreturn");

    if (NULL != RETURN_EXPRS (arg_node)) {

        /* convert ntypes of return values to return type of function ; add them to
         * ret-node*/

        fundef = INFO_TUP_FUNDEF (arg_info);
    }

    DBUG_RETURN (arg_node);
}

/************************************************************************
 *
 * function:
 *   node *TUPlet(node* arg_node, node* arg_info)
 *
 * description:
 *   Depending on the 'expression' belonging to the let node, the neccessary
 *   functions for type upgrade are executed.
 *
 *************************************************************************/
node *
TUPlet (node *arg_node, info *arg_info)
{

    ntype *type;

    DBUG_ENTER ("TUPlet");

    if (N_ap == NODE_TYPE (LET_EXPR (arg_node))) {

        /*
         *  expression is a function application
         */

        /* first of all, try to specialize function */
        AP_FUNDEF (LET_EXPR (arg_node))
          = TryToSpecializeFunction (AP_FUNDEF (LET_EXPR (arg_node)),
                                     AP_ARGS (LET_EXPR (arg_node)), arg_info);

        /* now we have the possibility to do an static dispatch */
        AP_FUNDEF (LET_EXPR (arg_node))
          = TryStaticDispatch (AP_FUNDEF (LET_EXPR (arg_node)), arg_info);

        /*
         * TODO: update left side
         *       here is the only possibility for multiple ids on left side
         */
    } else if (N_with == NODE_TYPE (LET_EXPR (arg_node))) {

        /*
         * expression is a with loop
         */

        /* first traverse in withloop, update all expr parts */

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    } else {
        /*
         * all other expressions: no need to traverse in substructures
         */
    }

    if (N_ap != NODE_TYPE (LET_EXPR (arg_node))) {
        DBUG_ASSERT ((IDS_AVIS (LET_IDS (arg_node)) != NULL),
                     "AVIS is NULL in IDS node!");

        type = NTCnewTypeCheck_Expr (arg_node);

        DBUG_ASSERT (TYleTypes (type, AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))),
                     "Lost type information!");

        AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node)))
          = TYfreeType (AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))));
        AVIS_TYPE (IDS_AVIS (LET_IDS (arg_node))) = type;
    }

    DBUG_RETURN (arg_node);
}

/********************************************************************************
 *
 * function:
 *   node *TUPwith(node *arg_node, node* arg_info)
 *
 * description:
 *
 ********************************************************************************/
node *
TUPwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPwith");

    /*
     * top node of with loop
     * traverse in substructures
     */

    if (NULL != WITH_PART (arg_node))
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    if (NULL != WITH_CODE (arg_node))
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************
 *
 * function:
 *   node *TUPpart(node *arg_node, node *arg_info)
 *
 * description:
 *
 *****************************************************************************/
node *
TUPpart (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPpart");

    /*
     * partition of with loop
     * traverse in generator, update types
     * traverse in next partition
     */

    if (PART_GENERATOR (arg_node) != NULL)
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

    if (PART_NEXT (arg_node) != NULL)
        PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**************************************************************************
 *
 * function:
 *   node *TUPgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *   All 'expressions' belonging to generator-node should have (if possible)
 *   the same type.
 *   So the best possible type is determined and all expressions are updated
 *   if possible.
 *
 **************************************************************************/
node *
TUPgenerator (node *arg_node, info *arg_info)
{

    ntype *current, *tmp;

    DBUG_ENTER ("TUPgenerator");

    current = NTCnewTypeCheck_Expr (GENERATOR_BOUND1 (arg_node));
    tmp = NTCnewTypeCheck_Expr (GENERATOR_BOUND2 (arg_node));
    current = GetLowestType (current, tmp);

    if (NULL != GENERATOR_STEP (arg_node)) {

        tmp = NTCnewTypeCheck_Expr (GENERATOR_STEP (arg_node));
        current = GetLowestType (current, tmp);

        tmp = NTCnewTypeCheck_Expr (GENERATOR_WIDTH (arg_node));
        current = GetLowestType (current, tmp);
    }

    /*
     * current is the best possible type for all generator values
     * assign current to all generator values iff the updated
     * value is no argument of a special function (funcond, dofun)
     */

    if (IsArgumentOfSpecialFunction (GENERATOR_BOUND1 (arg_node)) == FALSE)
        GENERATOR_BOUND1 (arg_node)
          = AssignTypeToExpr (GENERATOR_BOUND1 (arg_node), current);

    if (IsArgumentOfSpecialFunction (GENERATOR_BOUND2 (arg_node)) == FALSE)
        GENERATOR_BOUND2 (arg_node)
          = AssignTypeToExpr (GENERATOR_BOUND2 (arg_node), current);

    if (NULL != GENERATOR_STEP (arg_node)) {

        if (IsArgumentOfSpecialFunction (GENERATOR_STEP (arg_node)) == FALSE)
            GENERATOR_STEP (arg_node)
              = AssignTypeToExpr (GENERATOR_STEP (arg_node), current);

        if (IsArgumentOfSpecialFunction (GENERATOR_WIDTH (arg_node)) == FALSE)
            GENERATOR_WIDTH (arg_node)
              = AssignTypeToExpr (GENERATOR_WIDTH (arg_node), current);
    }

    /*
     * TODO: update withid with same type
     */

    current = TYfreeType (current);

    DBUG_RETURN (arg_node);
}

/******************************************************************
 *
 * function:
 *   node *TUPcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************/
node *
TUPcode (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("TUPcode");

    /*
     * this is the code block of a with loop
     * CBLOCK contains this code, CEXPRS conains the raturn value(s)
     * CEXPRS has to be an id-node, so no typeupgrade has to be done
     */

    if (CODE_CBLOCK (arg_node) != NULL)
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    if (CODE_NEXT (arg_node) != NULL)
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
