/*
 * $Log$
 * Revision 1.2  2001/03/05 17:11:28  nmw
 * no more warnings
 *
 * Revision 1.1  2001/03/05 16:02:25  nmw
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSACSE.c
 *
 * prefix: SSACSE
 *
 * description:
 *
 *   This module does the Common Subexpression Elimination in the AST per
 *   function (including the special functions in order of application).
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "SSACSE.h"

#define SSACSE_TOPLEVEL 0

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACSEids (ids *arg_ids, node *arg_info);

/* functions to handle cseinfo chains */
static node *AddCSEinfo (node *cseinfo, node *let);
static node *CreateNewCSElayer (node *cseinfo);
static node *RemoveTopCSElayer (node *cseinfo);

/* helper functions for internal use only */
static node *FindCSE (node *cselist, node *let);

/******************************************************************************
 *
 * function:
 *   node *AddCSEinfo(node *cseinfo, node *let)
 *
 * description:
 *   Adds an new let node to the actual cse layer.
 *
 *
 ******************************************************************************/
static node *
AddCSEinfo (node *cseinfo, node *let)
{
    DBUG_ENTER ("AddCSEinfo");

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *CreateNewCSElayer(node *cseinfo)
 *
 * description:
 *   starts a new layer of cse let nodes.
 *
 ******************************************************************************/
static node *
CreateNewCSElayer (node *cseinfo)
{
    DBUG_ENTER ("CreateNewCSElayer");

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *RemoveTopCSElayer(node *cseinfo)
 *
 * description:
 *   removes all cseinfo nodes from the current cse layer.
 *
 ******************************************************************************/
static node *
RemoveTopCSElayer (node *cseinfo)
{
    DBUG_ENTER ("RemoveTopCSElayer");

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *FindCSE(node *cselist, node *let)
 *
 * description:
 *
 *
 ******************************************************************************/
static node *
FindCSE (node *cselist, node *let)
{
    node *match;

    DBUG_ENTER ("FindCSE");

    match = NULL;

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args for initialization.
 *   traverses block.
 *   does NOT traverse to next fundef (this must by done by the optimization
 *   loop.
 *
 ******************************************************************************/
node *
SSACSEfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /* Free Cseinfo chain */
    FreeTree (INFO_SSACSE_CSE (arg_info));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEarg(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses chain of args to init SUBST attribute with NULL.
 *
 ******************************************************************************/
node *
SSACSEarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEarg");

    AVIS_SUBST (ARG_AVIS (arg_node)) = NULL;

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses chain of vardecs for initialization.
 *   traverses assignment chain.
 *
 ******************************************************************************/
node *
SSACSEblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* traverse vardecs of block */
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignments of block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEvardec(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACSEvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEvardec");

    AVIS_SUBST (VARDEC_AVIS (arg_node)) = NULL;

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEassign (node *arg_node, node *arg_info)
 *
 * description:
 *   traverses assignment chain top-down and removes unused assignment.
 *
 ******************************************************************************/
node *
SSACSEassign (node *arg_node, node *arg_info)
{
    bool remassign;
    node *tmp;

    DBUG_ENTER ("SSACSEassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "assign node without instruction");

    INFO_SSACSE_REMASSIGN (arg_info) = FALSE;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    remassign = INFO_SSACSE_REMASSIGN (arg_info);

    /* traverse to next assignment in chain */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    /* free this assignment if unused anymore */
    if (remassign) {
        tmp = arg_node;
        arg_node = ASSIGN_NEXT (arg_node);
        FreeNode (tmp);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEcond(node *arg_node, node *arg_info)
 *
 * description:
 *   does stacking of available cse-expressions for both parts of the
 *   conditional.
 *   traverses condition, then- and else-part (in this order).
 *
 ******************************************************************************/
node *
SSACSEcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* start new cse frame */
    INFO_SSACSE_CSE (arg_info) = CreateNewCSElayer (INFO_SSACSE_CSE (arg_info));

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    /* remove top cse frame */
    INFO_SSACSE_CSE (arg_info) = RemoveTopCSElayer (INFO_SSACSE_CSE (arg_info));

    /* create new cse frame */
    INFO_SSACSE_CSE (arg_info) = CreateNewCSElayer (INFO_SSACSE_CSE (arg_info));

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }
    /* remove top cse frame */
    INFO_SSACSE_CSE (arg_info) = RemoveTopCSElayer (INFO_SSACSE_CSE (arg_info));

    INFO_SSACSE_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEreturn(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses the result expressions
 *
 ******************************************************************************/
node *
SSACSEreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEreturn");

    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    INFO_SSACSE_REMASSIGN (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSElet(node *arg_node, node *arg_info)
 *
 * description:
 *   first do a variable substitution on the right side expression (but
 *   only for simple expressions) or do SSACSE for a right-side with-loop
 *
 *   next compare the right side with all available common subexpressions
 *
 *   in a matching case set the necessary SUBST links to the existing variables
 *   and remove this let.
 ******************************************************************************/
node *
SSACSElet (node *arg_node, node *arg_info)
{
    node *match;

    DBUG_ENTER ("SSACSElet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    match = FindCSE (INFO_SSACSE_CSE (arg_info), LET_EXPR (arg_node));
    if (match != NULL) {
        /* found matching common subexpression */
        /* ##nmw## set SUBST attributes */

        /* remove assignment */
        INFO_SSACSE_REMASSIGN (arg_info) = TRUE;
    } else {
        /* new expression found */
        INFO_SSACSE_CSE (arg_info) = AddCSEinfo (INFO_SSACSE_CSE (arg_info), arg_node);

        /* do not remove assignment */
        INFO_SSACSE_REMASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEap(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses parameter to do correct variable substitution.
 *   if special function application, start traversal of this fundef
 *
 ******************************************************************************/
node *
SSACSEap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEap");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSACSEid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEid");

    ID_IDS (arg_node) = TravIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSENwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACSENwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSENwith");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSENcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACSENcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSENcode");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   this function must never be called in ssaform.
 *
 *
 ******************************************************************************/
node *
SSACSEwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEwhile");

    DBUG_ASSERT ((FALSE), "there must not be any while-statement in ssa-form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEdo(node *arg_node, node *arg_info)
 *
 * description:
 *   this functions must never be called in ssaform.
 *
 ******************************************************************************/
node *
SSACSEdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEdo");

    DBUG_ASSERT ((FALSE), "there must not be any do-statement in ssa-form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSACSEids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
static ids *
SSACSEids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSACSEids");

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSACSEids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSACSE(node* fundef)
 *
 * description:
 *   Starts the traversal for a given fundef.
 *   Starting fundef must not be a special fundef (do, while, cond) created by
 *   lac2fun transformation. These "inline" functions will be traversed in their
 *   order of usage. The traversal mode (on toplevel, in special function) is
 *   annotated in the stacked INFO_SSACSE_DEPTH attribute.
 *
 *
 ******************************************************************************/
node *
SSACSE (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSACSE");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "SSACSE called for non-fundef node");

    /* do not start traversal in special functions */
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();

        INFO_SSACSE_DEPTH (arg_info) = SSACSE_TOPLEVEL; /* start on toplevel */

        old_tab = act_tab;
        act_tab = ssacse_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
