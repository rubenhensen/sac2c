/*
 *
 * $Log$
 * Revision 1.1  2005/08/18 07:01:54  sbs
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   insert_type_assert.c
 *
 * prefix: INSTC
 *
 * description:
 *
 *
 * usage of arg_info (INFO_INSTC_...):
 *
 *   ...RETS       holds the pointer to the rets of the current fundef!
 *   ...NEW_ASSIGN holds new assignments to be inserted after the current
 *                 one! (filled during RHS traversal)
 *   ...RETURN     holds the pointer to the return we a traversing!
 *
 *****************************************************************************/

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "new_types.h"
#include "internal_lib.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"

#include "insert_type_conv.h"

/*
 * INFO structure
 */
struct INFO {
    node *rets;
    node *assign;
    node *ret;
};

/*
 * INFO macros
 */
#define INFO_INSTC_RETS(n) ((n)->rets)
#define INFO_INSTC_NEW_ASSIGN(n) ((n)->assign)
#define INFO_INSTC_RETURN(n) ((n)->ret)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_INSTC_RETS (result) = NULL;
    INFO_INSTC_NEW_ASSIGN (result) = NULL;
    INFO_INSTC_RETURN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Helper Functions:
 */

/******************************************************************************
 *
 * function:
 *   node *CreateTypeConv( node *avis, ntype *type)
 *
 * description:
 *   assuming avis points to "unknown[*] a"
 *   and type points to "int[.]"
 *   we create an assignment of the form
 *
 *   a = _type_conv_( int[.], a);
 *
 *
 ******************************************************************************/

static node *
CreateTypeConv (node *avis, ntype *type)
{
    node *res;

    DBUG_ENTER ("CreateTypeConv");

    res
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                 TCmakePrf2 (F_type_conv, TBmakeType (TYcopyType (type)),
                                             TBmakeId (avis))),
                      NULL);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Traversal Functions:
 */

/******************************************************************************
 *
 * function:
 *   node *INSTCfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSTCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_INSTC_RETS (arg_info) = FUNDEF_RETS (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSTCassign( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSTCassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_INSTC_NEW_ASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_INSTC_NEW_ASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_INSTC_NEW_ASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSTCids( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCids (node *arg_node, info *arg_info)
{
    ntype *scalar_type;
    node *assign;

    DBUG_ENTER ("INSTCids");

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    scalar_type = TYgetScalar (IDS_NTYPE (arg_node));
    if (!TYisSimple (scalar_type) || (TYgetSimpleType (scalar_type) != T_unknown)) {

        assign = CreateTypeConv (IDS_AVIS (arg_node), IDS_NTYPE (arg_node));

        ASSIGN_NEXT (assign) = INFO_INSTC_NEW_ASSIGN (arg_info);
        INFO_INSTC_NEW_ASSIGN (arg_info) = assign;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSTCid( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCid (node *arg_node, info *arg_info)
{
    node *assign;

    DBUG_ENTER ("INSTCid");

    if (INFO_INSTC_RETURN (arg_info) != NULL) {

        assign
          = CreateTypeConv (ID_AVIS (arg_node), RET_TYPE (INFO_INSTC_RETS (arg_info)));

        ASSIGN_NEXT (assign) = INFO_INSTC_NEW_ASSIGN (arg_info);
        INFO_INSTC_NEW_ASSIGN (arg_info) = assign;

        INFO_INSTC_RETS (arg_info) = RET_NEXT (INFO_INSTC_RETS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSTCreturn( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSTCreturn");

    INFO_INSTC_RETURN (arg_info) = arg_node;

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = TRAVdo (RETURN_EXPRS (arg_node), arg_info);
    }

    INFO_INSTC_RETURN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *INSTCdoInsertTypeConv( node *arg_node)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCdoInsertTypeConv (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("INSTCdoInsertTypeConv");

    TRAVpush (TR_instc);

    arg_info = MakeInfo ();
    arg_node = TRAVdo (arg_node, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}
