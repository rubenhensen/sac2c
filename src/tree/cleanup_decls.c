/*
 *
 * $Log$
 * Revision 1.4  2001/03/22 20:03:01  dkr
 * include of tree.h eliminated
 *
 * Revision 1.3  2001/02/13 16:14:39  nmw
 * save/restore of act_tab added
 *
 * Revision 1.2  2000/12/15 18:31:20  dkr
 * initial revision
 *
 * Revision 3.1  2000/11/20 17:59:17  sacbase
 * new release made
 *
 * Revision 1.5  2000/03/21 14:55:23  dkr
 * ASSERT added: CleanupDecls() can be used after type checking only
 *
 * Revision 1.4  2000/03/19 17:11:55  dkr
 * fixed a bug in CUDids(): INFO_CUD_REF may be NULL
 *
 * Revision 1.3  2000/03/19 15:46:44  dkr
 * DFMstack removed (ups, SAC allows no nested/local vardecs ...)
 * comments added
 *
 * Revision 1.2  2000/03/17 21:06:17  dkr
 * the elimination of superfluous vardecs works :)
 * comments are fairly rare for now ...
 *
 * Revision 1.1  2000/03/17 15:55:04  dkr
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"

/*
 *
 * This modul removes all superfluous vardecs from the AST.
 *
 */

/*
 * usage of arg_info (INFO_CUD_...)
 * ------------------------------------
 *
 *   ...FUNDEF   pointer to the current fundef
 *   ...REF      DFMmask
 */

/*
 * compound macro
 */
#define INFO_DFMBASE(arg_info) FUNDEF_DFM_BASE (INFO_CUD_FUNDEF (arg_info))

/******************************************************************************
 *
 * Function:
 *   ids *CUDids( ids *id, node *arg_info)
 *
 * Description:
 *   Unsets the corresponding DFMmask entry of the id.
 *
 ******************************************************************************/

static ids *
CUDids (ids *id, node *arg_info)
{
    node *decl;
    ids *tmp;

    DBUG_ENTER ("CUDids");

    if (INFO_CUD_REF (arg_info) != NULL) {
        tmp = id;
        while (tmp != NULL) {
            decl = IDS_VARDEC (tmp);

            DBUG_ASSERT ((decl != NULL),
                         "Variable declaration missing! "
                         "CleanupDecls() can be used after type checking only!");

            if ((NODE_TYPE (decl) != N_vardec) && (NODE_TYPE (decl) != N_arg)) {
                DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef), "declaration is neither a "
                                                             "N_arg/N_vardec-node nor a "
                                                             "N_objdef-node");
            } else {
                DFMSetMaskEntryClear (INFO_CUD_REF (arg_info), NULL, decl);
            }

            tmp = IDS_NEXT (tmp);
        }
    }

    DBUG_RETURN (id);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDfundef( node *arg_node, node *arg_info)
 *
 * Description:
 *   Creates a new DFM base if not available already.
 *
 ******************************************************************************/

node *
CUDfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CUDfundef");

    INFO_CUD_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        INFO_CUD_REF (arg_info) = NULL;
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_ASSERT ((INFO_CUD_REF (arg_info) == NULL), "INFO_CUD_REF not freed!");
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDblock( node *arg_node, node *arg_info)
 *
 * Description:
 *   Eliminates all superfluous vardecs of the block:
 *   In a first traversal of the vardecs all the found vars are set in a
 *   DFM mask.
 *   Subsequently during the traversal of the instructions all occuring vars
 *   are unset in this mask.
 *   After this all declarations of vars that are still set in the DFM mask
 *   can be removed.
 *
 ******************************************************************************/

node *
CUDblock (node *arg_node, node *arg_info)
{
    node *vardec;
    DFMmask_t mask;

    DBUG_ENTER ("CUDblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        DBUG_ASSERT ((INFO_CUD_REF (arg_info) == NULL), "(nested) local vardecs found!");
        INFO_CUD_REF (arg_info) = DFMGenMaskClear (INFO_DFMBASE (arg_info));

        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        mask = INFO_CUD_REF (arg_info);
        INFO_CUD_REF (arg_info) = NULL;

        /*
         * remove superfluous vardecs
         */
        vardec = BLOCK_VARDEC (arg_node);
        while (VARDEC_NEXT (vardec) != NULL) {
            if (DFMTestMaskEntry (mask, NULL, VARDEC_NEXT (vardec))) {
                DBUG_PRINT ("CUD", ("Variable %s removed in function %s",
                                    VARDEC_NAME (VARDEC_NEXT (vardec)),
                                    FUNDEF_NAME (INFO_CUD_FUNDEF (arg_info))));
                VARDEC_NEXT (vardec) = FreeNode (VARDEC_NEXT (vardec));
            } else {
                vardec = VARDEC_NEXT (vardec);
            }
        }
        if (DFMTestMaskEntry (mask, NULL, BLOCK_VARDEC (arg_node))) {
            DBUG_PRINT ("CUD", ("Variable %s removed in function %s",
                                VARDEC_NAME (BLOCK_VARDEC (arg_node)),
                                FUNDEF_NAME (INFO_CUD_FUNDEF (arg_info))));
            BLOCK_VARDEC (arg_node) = FreeNode (BLOCK_VARDEC (arg_node));
        }

        mask = DFMRemoveMask (mask);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDvardec( node *arg_node, node *arg_info)
 *
 * Description:
 *   First traversal of the vardecs: set the found vardec in the DFMmask.
 *
 ******************************************************************************/

node *
CUDvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CUDvardec");

    DFMSetMaskEntrySet (INFO_CUD_REF (arg_info), NULL, arg_node);

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDlet( node *arg_node, node *arg_info)
 *
 * Description:
 *   ---
 *
 ******************************************************************************/

node *
CUDlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CUDlet");

    LET_IDS (arg_node) = CUDids (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDid( node *arg_node, node *arg_info)
 *
 * Description:
 *   ---
 *
 ******************************************************************************/

node *
CUDid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CUDid");

    ID_IDS (arg_node) = CUDids (ID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDwithid( node *arg_node, node *arg_info)
 *
 * Description:
 *   ---
 *
 ******************************************************************************/

node *
CUDwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CUDwithid");

    NWITHID_VEC (arg_node) = CUDids (NWITHID_VEC (arg_node), arg_info);
    NWITHID_IDS (arg_node) = CUDids (NWITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CleanupDecls( node *syntax_tree)
 *
 * Description:
 *   Removes all superfluous vardecs from the AST.
 *
 ******************************************************************************/

node *
CleanupDecls (node *syntax_tree)
{
    node *info_node;
    funtab *old_tab;

    DBUG_ENTER ("CleanupDecls");

    info_node = MakeInfo ();

    old_tab = act_tab;

    act_tab = cudecls_tab;
    syntax_tree = Trav (syntax_tree, info_node);

    act_tab = old_tab;

    info_node = FreeNode (info_node);

    DBUG_RETURN (syntax_tree);
}
