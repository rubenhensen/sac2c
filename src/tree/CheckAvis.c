/*
 *
 * $Log$
 * Revision 1.6  2001/03/29 09:12:38  nmw
 * tabs2spaces done
 *
 * Revision 1.5  2001/03/22 20:03:49  dkr
 * include of tree.h eliminated
 *
 * Revision 1.4  2001/02/20 15:51:02  nmw
 * debug output added
 *
 * Revision 1.3  2001/02/15 16:56:36  nmw
 * some DBUG_ASSERTS added
 *
 * Revision 1.2  2001/02/13 15:16:34  nmw
 * CheckAvis traversal implemented
 *
 * Revision 1.1  2001/02/12 16:59:27  nmw
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   CheckAvis.c
 *
 * prefix: CAV
 *
 * description:
 *
 *   This module restores the AVIS attribute in N_id, N_vardec/N_arg
 *   when old code did not updates all references correctly.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "CheckAvis.h"

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *CAVids (ids *arg_ids, node *arg_info);

/******************************************************************************
 *
 * function:
 *  node *CAVarg(node *arg_node, node *arg_info)
 *
 * description:
 *   Checks arg node for avis attribute. if missing create an initialize
 *   new avis node. Also checks for correct back reference.
 *
 ******************************************************************************/
node *
CAVarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVarg");

    if (ARG_AVIS (arg_node) == NULL) {
        /* missing avis node */
        DBUG_PRINT ("CAV", ("missing avis node in arg %s added.", ARG_NAME (arg_node)));
        ARG_AVIS (arg_node) = MakeAvis (arg_node);
    } else {
        /* check for correct backref in avis node */
        DBUG_ASSERT ((AVIS_VARDECORARG (ARG_AVIS (arg_node)) == arg_node),
                     "wrong backreference from avis to arg node!");
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CAVvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   Checks vardec node for avis attribute. if missing create an initialize
 *   new avis node. Also checks for correct back reference.
 *
 ******************************************************************************/
node *
CAVvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVvardec");

    if (VARDEC_AVIS (arg_node) == NULL) {
        /* missing avis node */
        DBUG_PRINT ("CAV",
                    ("missing avis node in vardec %s added.", VARDEC_NAME (arg_node)));
        VARDEC_AVIS (arg_node) = MakeAvis (arg_node);
    } else {
        /* check for correct backref */
        DBUG_ASSERT ((AVIS_VARDECORARG (VARDEC_AVIS (arg_node)) == arg_node),
                     "wrong backreference from avis to vardec node!");
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CAVid(node *arg_node, node *arg_info)
 *
 * description:
 *   checks for consistent back reference from N_id node to N_arg or N_vardec
 *   node together with back reference to N_avis node. Here implemented in
 *   CAVids.
 *
 ******************************************************************************/
node *
CAVid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "missing IDS in N_id!");

    ID_IDS (arg_node) = TravIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CAVlet(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal in ids chain.
 *
 ******************************************************************************/
node *
CAVlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVlet");

    if (LET_IDS (arg_node) != NULL) {
        /* there are some ids */
        LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
    }

    DBUG_ASSERT ((LET_EXPR (arg_node) != NULL), "N_let with empty EXPR attribute.");
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CAVNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal for ids chains in Nwithid nodes.
 *
 ******************************************************************************/
node *
CAVNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVNwithid");

    DBUG_ASSERT ((NWITHID_VEC (arg_node) != NULL),
                 "NWITHID node with empty VEC attribute");
    NWITHID_VEC (arg_node) = TravIDS (NWITHID_VEC (arg_node), arg_info);

    DBUG_ASSERT ((NWITHID_IDS (arg_node) != NULL),
                 "NWITHID node with empty IDS attribute");
    NWITHID_IDS (arg_node) = TravIDS (NWITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* CAVfundef(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
CAVfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVfundef");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        /* there are some args */
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* there is a block */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* CAVblock(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses vardec nodes and assignments in this order.
 *
 ******************************************************************************/
node *
CAVblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CAVblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /* there are some vardecs */
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        /* there is a block */
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *CAVids(node *arg_ids, node *arg_info)
 *
 * description:
 *   checks for consistent back reference from ids node to N_arg or N_vardec
 *   node together with back reference to N_avis node. Traversal sheme like
 *   AST Trav.
 *
 ******************************************************************************/
static ids *
CAVids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("CAVids");

    if (IDS_AVIS (arg_ids) != VARDEC_OR_ARG_AVIS (IDS_VARDEC (arg_ids))) {
        /* wrong back reference */
        DBUG_PRINT ("CAV", ("backreference from ids %s to N_avis corrected.",
                            IDS_VARDEC_NAME (arg_ids)));
        IDS_AVIS (arg_ids) = VARDEC_OR_ARG_AVIS (IDS_VARDEC (arg_ids));
    } else {
        DBUG_PRINT ("CAV", ("backreference from ids %s to N_avis ok.",
                            IDS_VARDEC_NAME (arg_ids)));
    }

    DBUG_ASSERT ((IDS_AVIS (arg_ids) != NULL), "AVIS reference still unset.");

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = CAVids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* CheckAvis(node* syntax_tree)
 *
 * description:
 *   Starts traversal of AST to check for correct Avis nodes in vardec/arg
 *   nodes. all backrefs from N_id or IDS structures are checked for
 *   consistent values.
 *   This traversal is needed for compatiblity with old code without knowledge
 *   of the avis nodes.
 *
 ******************************************************************************/
node *
CheckAvis (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("CheckAvis");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = chkavis_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}
