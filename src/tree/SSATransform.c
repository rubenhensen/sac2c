/*
 * $Log$
 * Revision 1.2  2001/02/14 14:40:36  nmw
 * function bodies and traversal order implemented
 *
 * Revision 1.1  2001/02/13 15:16:15  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSATRansform.c
 *
 * prefix: SSA
 *
 * description:
 *    this module traverses the AST and transformes the code in SSA form.
 *
 *****************************************************************************/

#include "dbug.h"
#include "globals.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "SSATransform.h"

static ids *TravLeftIDS (ids *arg_ids, node *arg_info);
static ids *SSAleftids (ids *arg_ids, node *arg_info);
static ids *TravRightIDS (ids *arg_ids, node *arg_info);
static ids *SSArightids (ids *arg_ids, node *arg_info);

/******************************************************************************
 *
 * function:
 *  node *SSAfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses args and block in this order
 *
 ******************************************************************************/
node *
SSAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* there are some args */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* there is a block */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    /* traverse next fundef */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses vardecs and instructions in this order
 *
 ******************************************************************************/
node *
SSAblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* there are some vardecs */
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there are some instructions */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *CAVarg(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses all exprs nodes.
 *  when used in N_return subtree, do the phi-copy-insertions.
 *
 ******************************************************************************/
node *
SSAexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAexprs");
    /* traverse expr */
    DBUG_ASSERT ((EXPRS_EXPR (arg_node) != NULL), "no expression in exprs node!");
    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

    /* traverse to next exprs */
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses assign chain top down.
 *
 ******************************************************************************/
node *
SSAassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAassign");
    /* traverse expr */
    DBUG_ASSERT ((ASSIGN_INSTR (arg_node) != NULL), "no instruction in assign!");
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* traverse next exprs */
    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAlet(node *arg_node, node *arg_info)
 *
 * description:
 *   travereses in expression and assigned ids.
 *
 ******************************************************************************/
node *
SSAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAlet");

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        /* there are some ids */
        LET_IDS (arg_node) = TravLeftIDS (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAarg(node *arg_node, node *arg_info)
 *
 * description:
 *   check for missing SSACOUT attribute in AVIS node. installs and inits
 *   new ssa-counter if necessary (init with 0, means unrenamed argument)
 *
 ******************************************************************************/
node *
SSAarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAarg");

    /* do something */

    /* traverse next arg */
    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   check for missing SSACOUT attribute in AVIS node. installs and inits
 *   new ssa-counter if necessary (init with undef)
 *
 ******************************************************************************/
node *
SSAvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAvardec");

    /* do something */

    /* traverse next vardec */
    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAid(node *arg_node, node *arg_info)
 *
 * description:
 *  does necessary renaming of variables used on right sides of expressions.
 *
 ******************************************************************************/
node *
SSAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "missing IDS in N_id!");

    ID_IDS (arg_node) = TravRightIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSANwith(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses with-op, partitions and code in this order
 *
 ******************************************************************************/
node *
SSANwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANwith");

    /* traverse in with-op */
    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), "Nwith without WITHOP node!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    /* traverse partitions */
    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL), "Nwith without PART node!");
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    /* traverse code */
    DBUG_ASSERT ((NWITH_CODE (arg_node) != NULL), "Nwith without CODE node!");
    NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSANpart(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses generator in this order for all partitions and last
 * (only one time!) withid
 *
 ******************************************************************************/
node *
SSANpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANpart");

    /* traverse generator */
    DBUG_ASSERT ((NPART_GEN (arg_node) != NULL), "Npart without Ngen node!");
    NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

    DBUG_ASSERT ((NPART_WITHID (arg_node) != NULL), "Npart without Nwithid node!");
    if (NPART_NEXT (arg_node) != NULL) {
        /* check for unique withids !!! */

        /* traverse next part */
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    } else {
        /* traverse in withid */
        NPART_WITHID (arg_node) = Trav (NPART_WITHID (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSANcode(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses block and expr in this order. then next Ncode node
 *
 ******************************************************************************/
node *
SSANcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANcode");

    /* traverse block */
    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    /* traverse expression */
    DBUG_ASSERT ((NCODE_CEXPR (arg_node) != NULL), "Ncode without Ncexpr node!");
    NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);

    /* restore old rename stack !!! */

    if (NCODE_NEXT (arg_node) != NULL) {
        /* traverse next part */
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
/******************************************************************************
 *
 * function:
 *  node *SSANwithid(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses in vector and ids strutures
 *
 ******************************************************************************/
node *
SSANwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSANwithid");

    DBUG_ASSERT ((NWITHID_VEC (arg_node) != NULL),
                 "NWITHID node with empty VEC attribute");
    NWITHID_VEC (arg_node) = TravLeftIDS (NWITHID_VEC (arg_node), arg_info);

    DBUG_ASSERT ((NWITHID_IDS (arg_node) != NULL),
                 "NWITHID node with empty IDS attribute");
    NWITHID_IDS (arg_node) = TravLeftIDS (NWITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAcond(node *arg_node, node *arg_info)
 *
 * description:
 *   this top-level conditional requieres stacking of renaming status.
 *   traverses conditional, then and else branch in this order.
 *
 ******************************************************************************/
node *
SSAcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAcond");

    /* traverse conditional */
    DBUG_ASSERT ((COND_COND (arg_node) != NULL), "Ncond without cond node!");
    COND_COND (arg_node) = Trav (COND_COND (arg_node), arg_info);

    /* do some status saving in all vardecs and args */

    /* traverse then */
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    }

    /* so some status restauration */

    /* traverse else */
    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAreturn(node *arg_node, node *arg_info)
 *
 * description:
 *   checks returning variables for different definitions on cond then and else
 *   branch. traverses exprs nodes
 *
 ******************************************************************************/
node *
SSAreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAreturn");

    /* set flag in Ninfo node */

    /* traverse exprs */
    if (RETURN_EXPRS (arg_node) != NULL) {
        RETURN_EXPRS (arg_node) = Trav (RETURN_EXPRS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAdo(node *arg_node, node *arg_info)
 *
 * description:
 *   after lac2fun this node must not be exist! -> ERROR
 *
 ******************************************************************************/
node *
SSAdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAdo");

    DBUG_ASSERT ((FALSE), "no do_loops allowed in ssa form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   after lac2fun this node must not be exist! -> ERROR
 *
 ******************************************************************************/
node *
SSAwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAwhile");

    DBUG_ASSERT ((FALSE), "no while_loops allowed in ssa form!");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAleftids(ids *arg_ids, node *arg_info)
 *
 * description:
 *  creates new (renamed) instance of defined variable.
 *
 ******************************************************************************/
static ids *
SSAleftids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSAleftids");

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *  node *SSArightids(ids *arg_ids, node *arg_info)
 *
 * description:
 *   renames variable to actual ssa renaming counter.
 *
 ******************************************************************************/
static ids *
SSArightids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSArightids");

    DBUG_RETURN (arg_ids);
}

/*  creates new (renamed) instance of defined variable. */

/******************************************************************************
 *
 * function:
 *   ids *Trav[Left,Right]IDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravLeftIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravLeftIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSAleftids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

static ids *
TravRightIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravRightIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSArightids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}
/******************************************************************************
 *
 * function:
 *   node *SSATransform(node *syntax_tree)
 *
 * description:
 *   Starts traversal of AST to transform code in SSA form. Every variable
 *   has exaclty one definition. This code transformtion relies on the
 *   lac2fun transformation!
 *
 ******************************************************************************/
node *
SSATransform (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSATransform");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = ssafrm_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}
