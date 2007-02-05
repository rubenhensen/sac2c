/*
 *
 * $Id$
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "DataFlowMask.h"
#include "dbug.h"

/******************************************************************************
 *
 *
 *  This modul removes all superfluous vardecs from the AST.
 *
 *
 ******************************************************************************
 *
 *  usage of arg_info (INFO_CUD_...)
 *  --------------------------------
 *
 *    ...FUNDEF   pointer to the current fundef
 *    ...REF      DFMmask
 *
 ******************************************************************************/

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    dfmask_t *ref;
};

/*
 * INFO macros
 */
#define INFO_CUD_FUNDEF(n) (n->fundef)
#define INFO_CUD_REF(n) (n->ref)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_CUD_FUNDEF (result) = NULL;
    INFO_CUD_REF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * compound macro
 */
#define INFO_DFMBASE(arg_info) FUNDEF_DFM_BASE (INFO_CUD_FUNDEF (arg_info))

/******************************************************************************
 *
 * Function:
 *   node *CUDids( node *arg_node, info *arg_info)
 *
 * Description:
 *   Unsets the corresponding DFMmask entry of the ids.
 *
 ******************************************************************************/
node *
CUDids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUDids");

    if (INFO_CUD_REF (arg_info) != NULL) {
        DBUG_ASSERT ((AVIS_DECL (IDS_AVIS (arg_node)) != NULL),
                     "Variable declaration missing! "
                     "CleanupDecls() can be used after type checking only!");
        DFMsetMaskEntryClear (INFO_CUD_REF (arg_info), NULL, IDS_AVIS (arg_node));

        if (IDS_NEXT (arg_node) != NULL) {
            IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CUDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUDfundef");

    INFO_CUD_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        /*
         * create DFM base
         */
        if (FUNDEF_DFM_BASE (arg_node) == NULL) {
            FUNDEF_DFM_BASE (arg_node)
              = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        }

        /*
         * remove superfluous vardecs
         */
        INFO_CUD_REF (arg_info) = NULL;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        DBUG_ASSERT ((INFO_CUD_REF (arg_info) == NULL), "INFO_CUD_REF not freed!");

        /*
         * update DFM base
         */
        FUNDEF_DFM_BASE (arg_node)
          = DFMupdateMaskBase (FUNDEF_DFM_BASE (arg_node), FUNDEF_ARGS (arg_node),
                               FUNDEF_VARDEC (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDblock( node *arg_node, info *arg_info)
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
CUDblock (node *arg_node, info *arg_info)
{
    node *vardec;
    dfmask_t *mask;

    DBUG_ENTER ("CUDblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        DBUG_ASSERT ((INFO_CUD_REF (arg_info) == NULL), "(nested) local vardecs found!");
        INFO_CUD_REF (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));

        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    }

    if (BLOCK_VARDEC (arg_node) != NULL) {
        mask = INFO_CUD_REF (arg_info);
        INFO_CUD_REF (arg_info) = NULL;

        /*
         * remove superfluous vardecs
         */
        vardec = BLOCK_VARDEC (arg_node);
        while (VARDEC_NEXT (vardec) != NULL) {
            if (DFMtestMaskEntry (mask, NULL, VARDEC_AVIS (VARDEC_NEXT (vardec)))) {
                DBUG_PRINT ("CUD", ("Variable %s removed in function %s",
                                    VARDEC_NAME (VARDEC_NEXT (vardec)),
                                    FUNDEF_NAME (INFO_CUD_FUNDEF (arg_info))));
                VARDEC_NEXT (vardec) = FREEdoFreeNode (VARDEC_NEXT (vardec));
            } else {
                vardec = VARDEC_NEXT (vardec);
            }
        }
        if (DFMtestMaskEntry (mask, NULL, VARDEC_AVIS (BLOCK_VARDEC (arg_node)))) {
            DBUG_PRINT ("CUD", ("Variable %s removed in function %s",
                                VARDEC_NAME (BLOCK_VARDEC (arg_node)),
                                FUNDEF_NAME (INFO_CUD_FUNDEF (arg_info))));
            BLOCK_VARDEC (arg_node) = FREEdoFreeNode (BLOCK_VARDEC (arg_node));
        }

        mask = DFMremoveMask (mask);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDvardec( node *arg_node, info *arg_info)
 *
 * Description:
 *   First traversal of the vardecs: set the found vardec in the DFMmask.
 *
 ******************************************************************************/

node *
CUDvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUDvardec");

    DFMsetMaskEntrySet (INFO_CUD_REF (arg_info), NULL, VARDEC_AVIS (arg_node));

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDid( node *arg_node, info *arg_info)
 *
 * Description:
 *   ---
 *
 ******************************************************************************/

node *
CUDid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUDid");

    if (INFO_CUD_REF (arg_info) != NULL) {
        DFMsetMaskEntryClear (INFO_CUD_REF (arg_info), NULL, ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CUDdoCleanupDecls( node *syntax_tree)
 *
 * Description:
 *   Removes all superfluous vardecs from the AST.
 *
 ******************************************************************************/

node *
CUDdoCleanupDecls (node *syntax_tree)
{
    info *info;
    trav_t traversaltable;

    DBUG_ENTER ("CleanupDecls");

    info = MakeInfo ();

    TRAVpush (TR_cud);

    syntax_tree = TRAVdo (syntax_tree, info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_cud), "Popped incorrect traversal table");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
