/*
 * $Log$
 * Revision 1.2  2001/03/12 13:41:53  nmw
 * UndoSSA creates unique result variables in multigenerator fold-withloops.
 *
 * Revision 1.1  2001/02/22 13:14:06  nmw
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   UndoSSATransform.c
 *
 * prefix: USSA
 *
 * description:
 *
 * 1. This module renames all artificial identifier to their original
 *    baseid to avoid problems with multiple global object names in the
 *    compiler backend.
 *
 *    All idetifiers marked as:
 *      ST_aritificial
 *      ST_was_reference
 *      ST_unique
 *    are re-renamed.
 *
 * 2. All result-variables of a multigenerator fold-withloop are made identical
 *    by inserting an additional variable and corresponding assignments.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "UndoSSATransform.h"
#include "DupTree.h"

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *USSAids (ids *arg_ids, node *arg_info);

/******************************************************************************
 *
 * function:
 *  node *USSAarg(node *arg_node, node *arg_info)
 *
 * description:
 *  not used now.
 *
 ******************************************************************************/
node *
USSAarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAarg");

    /* nop */

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAvardec(node *arg_node, node *arg_info)
 *
 * description:
 *   checks all artificial vardec for the corresponding base vardec/arg
 *   to undo renaming.
 *
 ******************************************************************************/
node *
USSAvardec (node *arg_node, node *arg_info)
{
    node *tmp;

    DBUG_ENTER ("USSAvardec");
    DBUG_PRINT ("USSA", ("working on vardec %s, avis (%p)", VARDEC_NAME (arg_node),
                         VARDEC_AVIS (arg_node)));

    if (((VARDEC_STATUS (arg_node) == ST_artificial)
         || (VARDEC_ATTRIB (arg_node) == ST_was_reference)
         || (VARDEC_ATTRIB (arg_node) == ST_unique))
        && (strcmp (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))),
                    VARDEC_NAME (arg_node))
            != 0)) {
        /* artificial vardec with renamed ids -> search for original vardec/arg */

        /* first search in arg chain */
        VARDEC_UNDOAVIS (arg_node) = NULL;
        tmp = INFO_USSA_ARGS (arg_info);
        while ((tmp != NULL) && (VARDEC_UNDOAVIS (arg_node) == NULL)) {
            if (strcmp (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))),
                        ARG_NAME (tmp))
                == 0) {
                VARDEC_UNDOAVIS (arg_node) = ARG_AVIS (tmp);
            }
            tmp = ARG_NEXT (tmp);
        }

        tmp = BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info));
        while ((tmp != NULL) && (VARDEC_UNDOAVIS (arg_node) == NULL)) {
            if (strcmp (SSACNT_BASEID (AVIS_SSACOUNT (VARDEC_AVIS (arg_node))),
                        VARDEC_NAME (tmp))
                == 0) {
                VARDEC_UNDOAVIS (arg_node) = VARDEC_AVIS (tmp);
            }
            tmp = VARDEC_NEXT (tmp);
        }

        DBUG_ASSERT ((VARDEC_UNDOAVIS (arg_node) != NULL),
                     "no matching base id found - no re-renaming possible!");
    } else {
        VARDEC_UNDOAVIS (arg_node) = NULL;
    }

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAid(node *arg_node, node *arg_info)
 *
 * description:
 *   checks for consistent back reference from N_id node to N_arg or N_vardec
 *   node together with back reference to N_avis node. Here implemented in
 *   USSAids.
 *
 ******************************************************************************/
node *
USSAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAid");

    DBUG_ASSERT ((ID_IDS (arg_node) != NULL), "missing IDS in N_id!");

    ID_IDS (arg_node) = TravIDS (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSAlet(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal in ids chain.
 *
 ******************************************************************************/
node *
USSAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAlet");

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
 *   node *USSANwithid(node *arg_node, node *arg_info)
 *
 * description:
 *   starts traversal for ids chains in Nwithid nodes.
 *
 ******************************************************************************/
node *
USSANwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSANwithid");

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
 *   node *USSANcode(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses code blocks of with loop and inserts unique result name for
 *   multigenerator fold-withloops.
 *
 ******************************************************************************/
node *
USSANcode (node *arg_node, node *arg_info)
{
    node *src_id;

    DBUG_ENTER ("USSANcode");

    if (NCODE_CBLOCK (arg_node) != NULL) {
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
    }

    if (INFO_USSA_FOLDTARGET (arg_info) != NULL) {
        /* create source id node */
        src_id = MakeId (VARDEC_OR_ARG_NAME (
                           AVIS_VARDECORARG (ID_AVIS (NCODE_CEXPR (arg_node)))),
                         NULL, ST_regular);
        ID_VARDEC (src_id) = AVIS_VARDECORARG (ID_AVIS (NCODE_CEXPR (arg_node)));
        ID_AVIS (src_id) = ID_AVIS (NCODE_CEXPR (arg_node));

        /*
         * append copy assignment: <fold-target> = cexprvar;
         * to block
         */
        BLOCK_INSTR (NCODE_CBLOCK (arg_node))
          = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)),
                          MakeAssignLet (VARDEC_NAME (AVIS_VARDECORARG (
                                           INFO_USSA_FOLDTARGET (arg_info))),
                                         AVIS_VARDECORARG (
                                           INFO_USSA_FOLDTARGET (arg_info)),
                                         src_id));

        /* set new fold target as cexpr */
        DBUG_PRINT ("USSA",
                    ("set new fold target %s",
                     VARDEC_NAME (AVIS_VARDECORARG (INFO_USSA_FOLDTARGET (arg_info)))));

        ID_VARDEC (NCODE_CEXPR (arg_node))
          = AVIS_VARDECORARG (INFO_USSA_FOLDTARGET (arg_info));
        ID_AVIS (NCODE_CEXPR (arg_node)) = INFO_USSA_FOLDTARGET (arg_info);
#ifndef NO_ID_NAME
        /* for compatiblity only
         * there is no real need for name string in ids structure because
         * you can get it from vardec without redundancy.
         */
        FREE (ID_NAME (NCODE_CEXPR (arg_node)));
        ID_NAME (NCODE_CEXPR (arg_node))
          = StringCopy (VARDEC_NAME (AVIS_VARDECORARG (INFO_USSA_FOLDTARGET (arg_info))));
#endif
    } else {
        /* do standard traversal */
        if (NCODE_CEXPR (arg_node) != NULL) {
            NCODE_CEXPR (arg_node) = Trav (NCODE_CEXPR (arg_node), arg_info);
        }
    }

    if (NCODE_NEXT (arg_node) != NULL) {
        NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *USSANwith(node *arg_node, node *arg_info)
 *
 * description:
 *   if this is a fold-withloop. we have to create a new unique result valiable
 *   for all results of a multigenerator withloop. The renaming and inserting
 *   of the necessary copy assignment is done by USSANcode.
 *
 ******************************************************************************/
node *
USSANwith (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("USSANwith");
    /* stack arg_info node, copy pointer to vardec/args lists */
    new_arg_info = MakeInfo ();
    INFO_USSA_TOPBLOCK (new_arg_info) = INFO_USSA_TOPBLOCK (arg_info);
    INFO_USSA_ARGS (new_arg_info) = INFO_USSA_ARGS (arg_info);

    /*
     * first check for fold-withloop with at least two code segments
     * (first code has a next attribute set) that needs a new
     * unique target variable
     */
    if ((NWITH_IS_FOLD (arg_node)) && (NWITH_CODE (arg_node) != NULL)
        && (NCODE_NEXT (NWITH_CODE (arg_node)) != NULL)) {
        DBUG_ASSERT ((NCODE_CEXPR (NWITH_CODE (arg_node)) != NULL),
                     "fold-withloop without target expression");
        DBUG_ASSERT ((NODE_TYPE (NCODE_CEXPR (NWITH_CODE (arg_node))) == N_id),
                     "fold-withloop without target variable");

        /* make new unique vardec as fold target and append it to vardec chain */
        BLOCK_VARDEC (INFO_USSA_TOPBLOCK (new_arg_info))
          = MakeVardec (TmpVar (),
                        DupTypes (VARDEC_OR_ARG_TYPE (
                          ID_VARDEC (NCODE_CEXPR (NWITH_CODE (arg_node))))),
                        BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info)));

        DBUG_PRINT ("USSA", ("create unique fold target %s",
                             VARDEC_NAME (BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info)))));

        /* set as new fold-target (will be inserted by USSANcode */
        INFO_USSA_FOLDTARGET (new_arg_info)
          = VARDEC_AVIS (BLOCK_VARDEC (INFO_USSA_TOPBLOCK (arg_info)));
    } else {
        /* no new target needed */
        INFO_USSA_FOLDTARGET (new_arg_info) = NULL;
    }

    /* now traverse all sons */
    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), new_arg_info);
    }

    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), new_arg_info);
    }

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), new_arg_info);
    }

    /* free new_arg_info node */
    FreeNode (new_arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* USSAfundef(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
USSAfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAfundef");

    /*
     * there is no need to traverse the args, because the args
     * are never renamed. Only new vardec as rename target of
     * an arg might exist.
     */
    INFO_USSA_ARGS (arg_info) = FUNDEF_ARGS (arg_node);

    if (FUNDEF_BODY (arg_node) != NULL) {
        /* there is a block */

        /* save begin of vardec chain for later access */
        INFO_USSA_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);

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
 *    node* USSAblock(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses vardec nodes and assignments in this order.
 *
 ******************************************************************************/
node *
USSAblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("USSAblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        /*
         * there are some vardecs, check for artificial ones
         */
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
 *   node *USSAids(node *arg_ids, node *arg_info)
 *
 * description:
 *   re-renames artificial vardecs to their original name
 *
 ******************************************************************************/
static ids *
USSAids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("USSAids");

    if (NODE_TYPE (AVIS_VARDECORARG (IDS_AVIS (arg_ids))) == N_vardec) {

        if (VARDEC_UNDOAVIS (AVIS_VARDECORARG (IDS_AVIS (arg_ids))) != NULL) {
            /* restore rename back to undo vardec */
            IDS_AVIS (arg_ids) = VARDEC_UNDOAVIS (AVIS_VARDECORARG (IDS_AVIS (arg_ids)));
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
    }

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
    arg_ids = USSAids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* CheckAvis(node* syntax_tree)
 *
 * description:
 *   Starts traversal of AST to restore original artificial identifier.
 *
 ******************************************************************************/
node *
UndoSSATransform (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("UndoSSATransform");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = undossa_tab;

    syntax_tree = Trav (syntax_tree, arg_info);

    act_tab = old_tab;

    FREE (arg_info);

    DBUG_RETURN (syntax_tree);
}
