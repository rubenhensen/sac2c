/*
 *
 * $Log$
 * Revision 1.3  2002/08/09 14:50:53  dkr
 * CWCap added
 *
 * Revision 1.2  2002/08/09 13:15:20  dkr
 * CWCmodul, CWCfundef added
 *
 * Revision 1.1  2002/08/09 13:00:02  dkr
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"

/******************************************************************************
 *
 * Function:
 *   node *CWCmodul( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CWCmodul");

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCfundef( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCfundef (node *arg_node, node *arg_info)
{
    node *ret_exprs;
    node *assigns;
    node *vardec;
    node *vardecs = NULL;

    DBUG_ENTER ("CWCfundef");

    if (FUNDEF_STATUS (arg_node) == ST_wrapperfun) {
        /*
         * wrapper function -> generate wrapper code
         */
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) == NULL),
                     "wrapper function has already a body!");

        /*
         * generate wrapper code together with the needed vardecs
         */
        assigns = TYType2WrapperCode (FUNDEF_TYPE (arg_node), &vardecs);

        /*
         * vardecs -> return exprs
         */
        ret_exprs = NULL;
        vardec = vardecs;
        while (vardec != NULL) {
            node *ret_expr;
            node *id_node = MakeId_Copy (VARDEC_NAME (vardec));
            ID_VARDEC (id_node) = vardec;
            ID_AVIS (id_node) = VARDEC_AVIS (vardec);

            if (ret_exprs == NULL) {
                ret_expr = ret_exprs = MakeExprs (id_node, NULL);
            } else {
                ret_expr = EXPRS_NEXT (ret_expr) = MakeExprs (id_node, NULL);
            }

            vardec = VARDEC_NEXT (vardec);
        }

        /*
         * append return statement to assignments
         */
        assigns = AppendAssign (assigns, MakeAssign (MakeReturn (ret_exprs), NULL));

        /*
         * insert function body
         */
        FUNDEF_BODY (arg_node) = MakeBlock (assigns, vardecs);

        /*
         * mark wrapper function as a inline function
         */
        FUNDEF_INLINE (arg_node) = TRUE;

        /*
         * create a separate wrapper function for each base type
         */
        /* not implemented yet */
    } else {
        /*
         * no wrapper function -> adjust all AP_FUNDEFs
         *
         * This is needed if the original wrapper function was valid for more than
         * a single base type.
         */
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCap( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CWCap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CWCap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateWrapperCode( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CreateWrapperCode (node *ast)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("CreateWrapperCode");

    tmp_tab = act_tab;
    act_tab = cwc_tab;

    info_node = MakeInfo ();
    ast = Trav (ast, info_node);
    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (ast);
}
