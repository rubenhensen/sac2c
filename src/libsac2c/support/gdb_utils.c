#include "tree_basic.h"
#include "tree_compound.h"
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
    bool z = FALSE;

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
            z = STReq (nm, AVIS_NAME (IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)))));
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
            if (N_id == NODE_TYPE (PRF_ARG1 (arg_node))) { /* eschew type_conv */
                z = STReq (nm, AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
            }
            break;

        case N_exprs:
            z = GDBbreakAtNid (EXPRS_EXPR (arg_node), nm);
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
            if (N_vardec == NODE_TYPE (vardec)) {
                if (NULL != AVIS_SSAASSIGN (VARDEC_AVIS (vardec))) {
                    PRTdoPrintNode (AVIS_SSAASSIGN (VARDEC_AVIS (vardec)));
                }
            } else {
                if (NULL != AVIS_SSAASSIGN (ARG_AVIS (vardec))) {
                    PRTdoPrintNode (AVIS_SSAASSIGN (ARG_AVIS (vardec)));
                }
            }
        }
    }

    return;
}

/******************************************************************************
 *
 * function: GDBwhatIsNid( node *nid, node *fundef)
 *
 * description:
 *
 * This function is intended to assist users of ddd/gdb in
 * making the sac2c compiler display the value of a variable.
 *
 * Typical usage:
 *   GDBwhatIsNid( arg_node, arg_info->fundef)
 *
 ******************************************************************************/
void
GDBwhatIsNid (node *arg_node, node *fundef)
{
    node *args;
    node *arg;

    if (NULL != arg_node) {
        PRTdoPrintNode (arg_node);
        switch (NODE_TYPE (arg_node)) {
        case N_id:
            GDBwhatIs (AVIS_NAME (ID_AVIS (arg_node)), fundef);
            break;
        case N_avis:
            GDBwhatIs (AVIS_NAME (arg_node), fundef);
            break;
        case N_prf:
            args = PRF_ARGS (arg_node);
            while (NULL != args) {
                arg = EXPRS_EXPR (args);
                GDBwhatIs (AVIS_NAME (ID_AVIS (arg)), fundef);
                args = EXPRS_NEXT (args);
            }
            break;
        default:
            break;
        }
    }

    return;
}

/******************************************************************************
 *
 * function: GDBwhatAreNid( node *arg_node, node *fundef)
 *
 * description:
 *
 * This function is intended to assist users of ddd/gdb in
 * making the sac2c compiler display the value of the arguments
 * of an N_prf or an N_exprs chain
 *
 * This will display all elements of an N_prf/ EXPRS chain.
 *
 * Typical usage:
 *   GDBwhatAre( "foo", arg_info->fundef)
 *
 ******************************************************************************/
void
GDBwhatAreNid (node *arg_node, node *fundef)
{
    node *exprs;
    node *expr;

    if (NULL != arg_node) {
        if (N_prf == NODE_TYPE (arg_node)) {
            exprs = PRF_ARGS (arg_node);
        } else {
            exprs = arg_node;
        }

        while (NULL != exprs) {
            expr = EXPRS_EXPR (exprs);
            if ((N_id == NODE_TYPE (expr))) {
                GDBwhatIsNid (expr, fundef);
            } else {
                PRTdoPrintNode (expr);
            }
            exprs = EXPRS_NEXT (exprs);
        }
    }

    return;
}

/******************************************************************************
 *
 * function: GDBwhatAre( char *nm, node *fundef)
 *
 * description: Like GDBwhatAreNid, except it takes a character
 *              string, instead of an N_id, as PRF_ARG1.
 *
 * This will display all elements of an N_prf EXPRS chain.
 *
 * Typical usage:
 *
 *   GDBwhatAre( "foo", arg_info->fundef)
 *
 ******************************************************************************/
void
GDBwhatAre (char *nm, node *fundef)
{
    node *exprs;
    node *expr;
    node *vardec;
    node *assgn;

    if (NULL != nm) {
        vardec = TCfindVardec_Name (nm, fundef);
        assgn = (NULL != vardec) ? AVIS_SSAASSIGN (VARDEC_AVIS (vardec)) : NULL;
        if (NULL != assgn) {
            exprs = PRF_ARGS (LET_EXPR (ASSIGN_STMT (assgn)));
            while (NULL != exprs) {
                expr = EXPRS_EXPR (exprs);
                if ((N_id == NODE_TYPE (expr))) {
                    GDBwhatIsNid (expr, fundef);
                } else {
                    PRTdoPrintNode (expr);
                }
                exprs = EXPRS_NEXT (exprs);
            }
        }
    }

    return;
}

/******************************************************************************
 *
 * function: GDBprintAvisName( node *avis)
 *
 * description: Print avis name.
 *
 *
 * Typical usage:
 *    GDBprintAvisName( N_avisNode)
 *
 ******************************************************************************/
void
GDBprintAvisName (node *avis)
{

    if (NULL != avis) {
        printf ("Avis for %s is:%p, AVIS_ISLCLASS=%d, AVIS_ISLINDEX=%d\n",
                AVIS_NAME (avis), (void *)avis, AVIS_ISLCLASS (avis),
                AVIS_ISLINDEX (avis));
    }
    return;
}

/******************************************************************************
 *
 * function: GDBprintAvisForFundef( node *fundef)
 *
 * description: Print avis names and addresses for fundef.
 *
 *
 ******************************************************************************/
void
GDBprintAvisForFundef (node *fundef)
{
    node *args;
    node *vardecs;

    args = FUNDEF_ARGS (fundef);
    printf ("Fundef is %s\n Args are:\n", FUNDEF_NAME (fundef));
    while (NULL != args) {
        GDBprintAvisName (ARG_AVIS (args));
        args = ARG_NEXT (args);
    }

    printf ("\nVardecs are:\n");
    vardecs = FUNDEF_VARDECS (fundef);
    while (NULL != vardecs) {
        GDBprintAvisName (VARDEC_AVIS (vardecs));
        vardecs = VARDEC_NEXT (vardecs);
    }

    return;
}
