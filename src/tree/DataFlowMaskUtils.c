/*
 *
 * $Log$
 * Revision 1.1  2000/01/21 16:52:08  dkr
 * Initial revision
 *
 *
 */

#include "tree.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "typecheck.h"

/******************************************************************************
 *
 * function:
 *   node *DFM2Vardecs( DFMmask_t mask)
 *
 * description:
 *   Creates a vardec chain based on the given DFmask.
 *
 ******************************************************************************/

node *
DFM2Vardecs (DFMmask_t mask)
{
    node *decl, *tmp;
    node *vardecs = NULL;

    DBUG_ENTER ("DFM2Vardecs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_vardec) {
            tmp = vardecs;
            vardecs = DupNode (decl);
            VARDEC_NEXT (vardecs) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_arg),
                         "mask entry is neither an arg nor a vardec.");
            vardecs = MakeVardec (StringCopy (ARG_NAME (decl)),
                                  DuplicateTypes (ARG_TYPE (decl), 1), vardecs);
        }
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Args( DFMmask_t mask)
 *
 * description:
 *   Creates a argument list based on the given DFmask.
 *
 ******************************************************************************/

node *
DFM2Args (DFMmask_t mask)
{
    node *decl, *tmp;
    node *args = NULL;

    DBUG_ENTER ("DFM2Args");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        if (NODE_TYPE (decl) == N_arg) {
            tmp = args;
            args = DupNode (decl);
            ARG_NEXT (args) = tmp;
        } else {
            DBUG_ASSERT ((NODE_TYPE (decl) == N_vardec),
                         "mask entry is neither an arg nor a vardec.");
            args = MakeArg (StringCopy (VARDEC_NAME (decl)),
                            DuplicateTypes (VARDEC_TYPE (decl), 1), VARDEC_STATUS (decl),
                            VARDEC_ATTRIB (decl), args);
        }
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Exprs( DFMmask_t mask)
 *
 * description:
 *   Creates the return node for a dummy function.
 *
 ******************************************************************************/

node *
DFM2Exprs (DFMmask_t mask)
{
    node *decl;
    node *exprs = NULL;

    DBUG_ENTER ("DFM2Exprs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        exprs = MakeExprs (MakeId1 (VARDEC_OR_ARG_NAME (decl)), exprs);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (exprs);
}
