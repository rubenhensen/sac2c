/*
 *
 * $Log$
 * Revision 1.2  2000/03/17 21:06:17  dkr
 * the elimination of superfluous vardecs works :)
 * comments are fairly rare for now ...
 *
 * Revision 1.1  2000/03/17 15:55:04  dkr
 * Initial revision
 *
 */

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"

/*
 * usage of arg_info (INFO_CUD_...)
 * ------------------------------------
 *
 *   ...FUNDEF   pointer to the current fundef
 *
 *   ...REF      stacked DFMmasks (type DFMstack_t):
 *               one DFMmask for each N_block nesting.
 */

#ifndef DBUG_OFF
static int level; /* contains the current N_block nesting level */
#endif

/*
 * compound macro
 */
#define INFO_DFMBASE(arg_info) FUNDEF_DFM_BASE (INFO_CUD_FUNDEF (arg_info))

/******************************************************************************
 *
 * Function:
 *   int MarkVar( DFMmask_t mask, char *id, node *decl)
 *
 * Description:
 *
 *
 ******************************************************************************/

int
MarkVar (DFMmask_t mask, char *id, node *decl)
{
    int res;

    DBUG_ENTER ("MarkVar");

    if ((NODE_TYPE (decl) != N_vardec) && (NODE_TYPE (decl) != N_arg)) {
        DBUG_ASSERT ((NODE_TYPE (decl) == N_objdef),
                     "declaration is neither a N_arg/N_vardec-node nor a N_objdef-node");
        res = 2;
    } else {
        if (DFMTestMaskEntry (mask, id, decl)) {
            DFMSetMaskEntryClear (mask, id, decl);
            res = 1;
        } else {
            res = 0;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Function:
 *   ids *CUDids( ids *id, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static ids *
CUDids (ids *id, node *arg_info)
{
    ids *tmp;

    DBUG_ENTER ("CUDids");

    tmp = id;
    while (tmp != NULL) {
        WhileDFMstack (INFO_CUD_REF (arg_info), &MarkVar, NULL, IDS_VARDEC (tmp));

        tmp = IDS_NEXT (tmp);
    }

    DBUG_RETURN (id);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDfundef( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CUDfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CUDfundef");

    INFO_CUD_FUNDEF (arg_info) = arg_node;
    level = -1;

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        INFO_CUD_REF (arg_info) = GenerateDFMstack ();
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
        DBUG_ASSERT ((IsEmptyDFMstack (INFO_CUD_REF (arg_info))),
                     "The DFMstack is not empty!");
        RemoveDFMstack (&(INFO_CUD_REF (arg_info)));
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
 *
 *
 ******************************************************************************/

node *
CUDblock (node *arg_node, node *arg_info)
{
    node *vardec;
    DFMmask_t mask;

    DBUG_ENTER ("CUDblock");

    level++;

    PushDFMstack (&(INFO_CUD_REF (arg_info)), DFMGenMaskClear (INFO_DFMBASE (arg_info)));

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    mask = PopDFMstack (&(INFO_CUD_REF (arg_info)));

    /*
     * remove superfluous vardecs
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        vardec = BLOCK_VARDEC (arg_node);
        while (VARDEC_NEXT (vardec) != NULL) {
            if (DFMTestMaskEntry (mask, NULL, VARDEC_NEXT (vardec))) {
                DBUG_PRINT ("CUD", ("Variable %s removed in function %s (block level %i)",
                                    VARDEC_NAME (VARDEC_NEXT (vardec)),
                                    FUNDEF_NAME (INFO_CUD_FUNDEF (arg_info)), level));
                VARDEC_NEXT (vardec) = FreeNode (VARDEC_NEXT (vardec));
            } else {
                vardec = VARDEC_NEXT (vardec);
            }
        }
        if (DFMTestMaskEntry (mask, NULL, BLOCK_VARDEC (arg_node))) {
            DBUG_PRINT ("CUD", ("Variable %s removed in function %s (block level %i)",
                                VARDEC_NAME (BLOCK_VARDEC (arg_node)),
                                FUNDEF_NAME (INFO_CUD_FUNDEF (arg_info)), level));
            BLOCK_VARDEC (arg_node) = FreeNode (BLOCK_VARDEC (arg_node));
        }
    }

    DFMRemoveMask (mask);

    level--;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDvardec( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CUDvardec (node *arg_node, node *arg_info)
{
    DFMmask_t mask;

    DBUG_ENTER ("CUDvardec");

    mask = PopDFMstack (&(INFO_CUD_REF (arg_info)));
    DFMSetMaskEntrySet (mask, NULL, arg_node);
    PushDFMstack (&(INFO_CUD_REF (arg_info)), mask);

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
 *
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
 *
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
 *
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
 *
 *
 ******************************************************************************/

node *
CleanupDecls (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("CleanupDecls");

    info_node = MakeInfo ();

    act_tab = cudecls_tab;
    syntax_tree = Trav (syntax_tree, info_node);

    info_node = FreeNode (info_node);

    DBUG_RETURN (syntax_tree);
}
