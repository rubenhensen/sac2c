/*
 *
 * $Id$
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
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "globals.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "ctinfo.h"

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

    result = MEMmalloc (sizeof (info));

    INFO_INSTC_RETS (result) = NULL;
    INFO_INSTC_NEW_ASSIGN (result) = NULL;
    INFO_INSTC_RETURN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

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
 *   node *INSTCblock( node *arg_node, info *arg_info)
 *
 * description:
 *   make sure that we traverse the body BEFORE traversing i.e. generalizing
 *   the vardecs.
 *
 ******************************************************************************/

node *
INSTCblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INSTCblock");

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSTCavis( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCavis (node *arg_node, info *arg_info)
{
    ntype *old_type;

    DBUG_ENTER ("INSTCavis");

    old_type = AVIS_TYPE (arg_node);
    if (!TYisAUD (old_type)) {
        AVIS_TYPE (arg_node) = TYmakeAUD (TYmakeSimpleType (T_unknown));
        old_type = TYfreeType (old_type);
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

    if (INFO_INSTC_RETURN (arg_info) != NULL) {
        /**
         * we are dealing with a return:
         */
        if (INFO_INSTC_NEW_ASSIGN (arg_info) != NULL) {
            /**
             * insert new assignments BEFORE this one!
             */
            arg_node = TCappendAssign (INFO_INSTC_NEW_ASSIGN (arg_info), arg_node);
            INFO_INSTC_NEW_ASSIGN (arg_info) = NULL;
        }
        INFO_INSTC_RETURN (arg_info) = NULL;

    } else {
        /**
         * we are dealing with lets / loops / conds:
         */
        if (INFO_INSTC_NEW_ASSIGN (arg_info) != NULL) {
            /**
             * insert new assignments AFTER this N_assign!
             */
            ASSIGN_NEXT (arg_node)
              = TCappendAssign (INFO_INSTC_NEW_ASSIGN (arg_info), ASSIGN_NEXT (arg_node));
            INFO_INSTC_NEW_ASSIGN (arg_info) = NULL;
        }
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
    if (!TYisAUD (IDS_NTYPE (arg_node))
        && (!TYisSimple (scalar_type) || (TYgetSimpleType (scalar_type) != T_unknown))) {

        assign = CreateTypeConv (IDS_AVIS (arg_node), IDS_NTYPE (arg_node));

        ASSIGN_NEXT (assign) = INFO_INSTC_NEW_ASSIGN (arg_info);
        INFO_INSTC_NEW_ASSIGN (arg_info) = assign;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *INSTCwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
INSTCwith (node *arg_node, info *arg_info)
{
    node *old_new_assign;

    DBUG_ENTER ("INSTCwith");
    old_new_assign = INFO_INSTC_NEW_ASSIGN (arg_info);
    INFO_INSTC_NEW_ASSIGN (arg_info) = NULL;

    TRAVcont (arg_node, arg_info);

    INFO_INSTC_NEW_ASSIGN (arg_info) = old_new_assign;

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
    ntype *old_type;

    DBUG_ENTER ("INSTCid");

    if (INFO_INSTC_RETURN (arg_info) != NULL) {

        if (INFO_INSTC_RETS (arg_info) == NULL) {
            CTIerrorLine (NODE_LINE (arg_node),
                          "more expressions returned than return types specified!");
        } else {
            old_type = RET_TYPE (INFO_INSTC_RETS (arg_info));
            if (!TYisAUD (old_type)) {
                assign = CreateTypeConv (ID_AVIS (arg_node), old_type);

                ASSIGN_NEXT (assign) = INFO_INSTC_NEW_ASSIGN (arg_info);
                INFO_INSTC_NEW_ASSIGN (arg_info) = assign;

                RET_TYPE (INFO_INSTC_RETS (arg_info))
                  = TYmakeAUD (TYgetScalar (old_type));
                old_type = TYfreeTypeConstructor (old_type);
            }

            INFO_INSTC_RETS (arg_info) = RET_NEXT (INFO_INSTC_RETS (arg_info));
        }
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

    if (INFO_INSTC_RETS (arg_info) != NULL) {
        CTIerrorLine (NODE_LINE (arg_node),
                      "fewer expressions returned than return types specified in line "
                      "%d!",
                      NODE_LINE (INFO_INSTC_RETS (arg_info)));
    }

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
    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}
