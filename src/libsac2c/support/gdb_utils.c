/*
 *
 * $Id$
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "tree_compound.h"
#include <str.h>

#include "gdb_utils.h"

/******************************************************************************
 *
 * function: GDBbreakAtNid( node *arg_node, char *nm)
 *
 * description:
 *
 * This function is intended to assist users of ddd/gdb in
 * making the sac2c compiler stop when a particular node appears
 * in some function. E.g., suppose we want to drop in CFlet
 * when performing:   res = _add_SxS( x, y);
 *
 * Do this in gdb/ddd:
 *
 *     break CFlet
 * Breakpoint 3 at ...
 *     condition 3  BreakAtNid( arg_node, "res")
 *
 * The gdb break will be ignored until the first LET_IDS
 * is "res", at which point the break will be honored.
 *
 ******************************************************************************/
bool
GDBbreakAtNid (node *arg_node, char *nm)
{
    bool z;

    if (NULL == arg_node) {
        z = FALSE;
    } else {
        switch (NODE_TYPE (arg_node)) {

        case N_id:
            z = STReq (nm, AVIS_NAME (ID_AVIS (arg_node)));
            break;

        case N_ids:
            z = STReq (nm, AVIS_NAME (IDS_AVIS (arg_node)));
            break;

        case N_assign:
            z = STReq (nm, AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_INSTR (arg_node)))));
            break;

        case N_let:
            z = STReq (nm, AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
            break;

        case N_avis:
            z = STReq (nm, AVIS_NAME (arg_node));
            break;

        case N_fundef:
            z = STReq (nm, FUNDEF_NAME (arg_node));
            break;

        case N_arg:
            z = STReq (nm, AVIS_NAME (ARG_AVIS (arg_node)));
            break;

        case N_prf:
            z = STReq (nm, AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
            break;

        default:
            z = FALSE;
            break;
        }
    }

    return (z);
}

/******************************************************************************
 *
 * function: GDBwhatIs( char *nm, node *fundef)
 *
 * description:
 *
 * This function is intended to assist users of ddd/gdb in
 * making the sac2c compiler display the value of a variable.
 *
 * Typical usage:
 *   GDBwhatIs( "foo", arg_info->fundef)
 *
 ******************************************************************************/
void
GDBwhatIs (char *nm, node *fundef)
{
    node *vardec;

    if (NULL != nm) {
        vardec = TCfindVardec_Name (nm, fundef);
        if (NULL != vardec) {
            PRTdoPrintNode (vardec);
            if (NULL != AVIS_SSAASSIGN (VARDEC_AVIS (vardec))) {
                PRTdoPrintNode (AVIS_SSAASSIGN (VARDEC_AVIS (vardec)));
            }
        }
    }

    return;
}

/******************************************************************************
 *
 * function: GDBprintPrfArgs( node *arg_node, node *fundef)
 *
 * description:
 *
 * This function is intended to assist users of ddd/gdb in
 * making the sac2c compiler display the value of the arguments
 * of an N_prf.
 *
 * Typical usage:
 *   GDBwhatIs( "foo", arg_info->fundef)
 *
 ******************************************************************************/
void
GDBprintPrfArgs (node *arg_node, node *fundef)
{
    node *exprs;
    node *expr;

    if (NULL != arg_node) {
        exprs = PRF_ARGS (arg_node);
        while (NULL != exprs) {
            expr = EXPRS_EXPR (exprs);
            if ((N_id == NODE_TYPE (expr)) && (NULL != AVIS_SSAASSIGN (ID_AVIS (expr)))) {
                PRTdoPrintNode (AVIS_SSAASSIGN (ID_AVIS (expr)));
            } else {
                PRTdoPrintNode (expr);
            }
            exprs = EXPRS_NEXT (exprs);
        }
    }

    return;
}
