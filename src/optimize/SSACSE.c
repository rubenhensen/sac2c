/*
 * $Log$
 * Revision 1.4  2001/03/12 09:17:05  nmw
 * do not substitute SSAPHITARGET
 *
 * Revision 1.3  2001/03/07 15:58:35  nmw
 * SSA Common Subexpression Elimination implemented
 *
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
#include "compare_tree.h"
#include "optimize.h"

#define SSACSE_TOPLEVEL 0

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACSEids (ids *arg_ids, node *arg_info);

/* functions to handle cseinfo chains */
static node *AddCSEinfo (node *cseinfo, node *let);
static node *CreateNewCSElayer (node *cseinfo);
static node *RemoveTopCSElayer (node *cseinfo);
static ids *SetSubstAttributes (ids *subst, ids *with);

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

    DBUG_ASSERT ((cseinfo != NULL), "cseinfo layer stack is NULL");
    DBUG_ASSERT ((let != NULL), "add NULL let to cseinfo layer stack");

    DBUG_PRINT ("SSACSE", ("add let node %p to cse layer %p", let, cseinfo));

    if (CSEINFO_LET (cseinfo) == NULL) {
        /* add letnode top existing empty cseinfo node */
        CSEINFO_LET (cseinfo) = let;
    } else {
        /*
         * create new cseinfo node with let attribute on actual layer
         * and adds it in front of the cseinfo chain.
         */
        cseinfo = MakeCSEinfo (cseinfo, CSEINFO_LAYER (cseinfo), let);
    }

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

    cseinfo = MakeCSEinfo (cseinfo, NULL, NULL);

    DBUG_PRINT ("SSACSE", ("create new cse layer %p", cseinfo));

    /* set selfreference to mark new starting layer */
    CSEINFO_LAYER (cseinfo) = cseinfo;

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *RemoveTopCSElayer(node *cseinfo)
 *
 * description:
 *   removes all cseinfo nodes from the current cse layer. last cseinfo
 *   node on layer is marked with selfreference in CSEINFO_LAYER.
 *
 ******************************************************************************/
static node *
RemoveTopCSElayer (node *cseinfo)
{
    node *tmp;
    node *freetmp;

    DBUG_ENTER ("RemoveTopCSElayer");

    tmp = cseinfo;

    while (tmp != NULL) {
        freetmp = tmp;
        cseinfo = CSEINFO_NEXT (tmp);

        if (tmp == CSEINFO_LAYER (tmp)) {
            tmp = NULL;
        } else {
            tmp = CSEINFO_NEXT (tmp);
        }
        DBUG_PRINT ("SSACSE", ("removing csenode %p", freetmp));
        FreeNode (freetmp);
    }

    DBUG_RETURN (cseinfo);
}

/******************************************************************************
 *
 * function:
 *   node *FindCSE(node *cselist, node *let)
 *
 * description:
 *   traverses the cseinfo chain until matching expression is found and
 *   return this let node. else return NULL.
 *
 ******************************************************************************/
static node *
FindCSE (node *cselist, node *let)
{
    node *match;
    node *csetmp;

    DBUG_ENTER ("FindCSE");
    DBUG_ASSERT ((let != NULL), "FindCSE is called with empty let node");

    match = NULL;
    csetmp = cselist;

    while ((csetmp != NULL) && (match == NULL)) {
        if ((CSEINFO_LET (csetmp) != NULL)
            && (CompareTree (LET_EXPR (let), LET_EXPR (CSEINFO_LET (csetmp)))
                == CMPT_EQ)) {
            match = CSEINFO_LET (csetmp);
        }
        csetmp = CSEINFO_NEXT (csetmp);
    }

    DBUG_RETURN (match);
}

/******************************************************************************
 *
 * function:
 *   ids *SetSubstAttributes(ids *subst, ids *with)
 *
 * description:
 *   set AVIS_SUBST attribute for all ids to the corresponding avis node.
 *
 ******************************************************************************/
static ids *
SetSubstAttributes (ids *subst, ids *with)
{
    ids *tmpsubst;
    ids *tmpwith;

    DBUG_ENTER ("SetSubstAttributes");

    tmpsubst = subst;
    tmpwith = with;

    while (tmpsubst != NULL) {
        DBUG_PRINT ("SSACSE",
                    ("substitute ids %s with %s",
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (tmpsubst))),
                     VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (tmpwith)))));

        AVIS_SUBST (IDS_AVIS (tmpsubst)) = IDS_AVIS (tmpwith);

        tmpsubst = IDS_NEXT (tmpsubst);
        tmpwith = IDS_NEXT (tmpwith);
    }

    DBUG_RETURN (subst);
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

    INFO_SSACSE_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* traverse args of fundef */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* traverse block of fundef */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

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
 *   starts new cse frame
 *   traverses assignment chain.
 *   remove top cse frame (and so all available expressions defined in this
 *   block)
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

    /* start new cse frame */
    INFO_SSACSE_CSE (arg_info) = CreateNewCSElayer (INFO_SSACSE_CSE (arg_info));

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* traverse assignments of block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    /* remove top cse frame */
    INFO_SSACSE_CSE (arg_info) = RemoveTopCSElayer (INFO_SSACSE_CSE (arg_info));

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
 *   the stacking of available cse-expressions for both parts of the
 *   conditional is done by SSACSEblock.
 *   traverses condition, then- and else-part (in this order).
 *
 ******************************************************************************/
node *
SSACSEcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSEcond");

    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "conditional without condition");
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

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

    DBUG_PRINT ("SSACSE", ("inspecting expression for cse"));

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "let without expression");
    DBUG_ASSERT ((LET_IDS (arg_node) != NULL), "let without ids");

    /* traverse right side expression to to variable substitutions */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    match = FindCSE (INFO_SSACSE_CSE (arg_info), arg_node);
    if ((match != NULL) && (AVIS_SSAPHITARGET (IDS_AVIS (LET_IDS (arg_node))) == FALSE)) {
        /* found matching common subexpression, and let is no phicopytarget */
        /* set subst attributes for results */
        LET_IDS (arg_node) = SetSubstAttributes (LET_IDS (arg_node), LET_IDS (match));

        DBUG_PRINT ("SSACSE",
                    ("Common subexpression eliminated in line %d", NODE_LINE (arg_node)));
        cse_expr++;

        /* remove assignment */
        INFO_SSACSE_REMASSIGN (arg_info) = TRUE;
    } else {
        /* new expression found */
        DBUG_PRINT ("SSACSE", ("add new expression to cselist"));
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
    node *new_arg_info;
    DBUG_ENTER ("SSACSEap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion */
    if (((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun))
        && (AP_FUNDEF (arg_node) != INFO_SSACSE_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSACSE", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        INFO_SSACSE_DEPTH (new_arg_info) = INFO_SSACSE_DEPTH (arg_info) + 1;
        INFO_SSACSE_CSE (new_arg_info) = NULL;

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSACSE", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        DBUG_PRINT ("SSACSE", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSEid(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse to ids data to do substitution.
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
 *   traverse NPart, Nwithop and NCode in this order
 *
 *
 ******************************************************************************/
node *
SSACSENwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSENwith");

    /* traverse and do variable substutution in partitions */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    /* traverse and do variable substutution in withops */
    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    /* traverse and do cse in code blocks */
    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACSENcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse codeblock and expression for each Ncode node
 *
 *
 ******************************************************************************/
node *
SSACSENcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACSENcode");

    /* traverse codeblock */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /*traverse expression to do variable substitution */
    if (NCODE_CEXPR (arg_node) != NULL) {
        NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
    }

    /* traverse to next node */
    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

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
 *   traverses chain of ids to do variable substitution as
 *   annotated in AVIS_SUBST attribute
 *
 ******************************************************************************/
static ids *
SSACSEids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSACSEids");

    DBUG_ASSERT ((IDS_AVIS (arg_ids) != NULL), "missing Avis backlink in ids");

    /* check for necessary substitution */
    if (AVIS_SUBST (IDS_AVIS (arg_ids)) != NULL) {
        DBUG_PRINT ("SSACSE", ("substutution: %s -> %s",
                               VARDEC_OR_ARG_NAME (AVIS_VARDECORARG (IDS_AVIS (arg_ids))),
                               VARDEC_OR_ARG_NAME (
                                 AVIS_VARDECORARG (AVIS_SUBST (IDS_AVIS (arg_ids))))));

        /* do renaming to new ssa vardec */
        IDS_AVIS (arg_ids) = AVIS_SUBST (IDS_AVIS (arg_ids));

        /* restore all depended attributes with correct values */
        IDS_VARDEC (arg_ids) = AVIS_VARDECORARG (IDS_AVIS (arg_ids));

#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        FREE (IDS_NAME (arg_ids));
        IDS_NAME (arg_ids) = StringCopy (VARDEC_OR_ARG_NAME (IDS_VARDEC (arg_ids)));
#endif
    }

    /* traverse to next ids in chain */
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
 *   implements a simple TravIDS function like Trav for nodes to have
 *   an similar implementation.
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

    DBUG_PRINT ("SSACSE", ("starting common subexpression elimination in function %s",
                           FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();

        INFO_SSACSE_DEPTH (arg_info) = SSACSE_TOPLEVEL; /* start on toplevel */
        INFO_SSACSE_CSE (arg_info) = NULL;

        old_tab = act_tab;
        act_tab = ssacse_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
