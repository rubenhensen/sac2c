/*
 *
 * $Log$
 * Revision 1.2  2000/02/03 17:29:56  dkr
 * LUTs added
 *
 * Revision 1.1  2000/01/21 16:52:08  dkr
 * Initial revision
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
 *   types *DFM2Types( DFMmask_t mask)
 *
 * description:
 *   Creates a types chain based on the given DFmask.
 *
 ******************************************************************************/

types *
DFM2Types (DFMmask_t mask)
{
    node *decl;
    types *tmp;
    types *rettypes = NULL;

    DBUG_ENTER ("DFM2Types");

    /*
     * build return types, return exprs (use SPMD_OUT).
     */
    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        tmp = rettypes;
        rettypes = DuplicateTypes (VARDEC_OR_ARG_TYPE (decl), 1);
        TYPES_NEXT (rettypes) = tmp;
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    /*
     * CAUTION: FUNDEF_NAME is for the time being a part of FUNDEF_TYPES!!
     *          That's why we must build a void-type, when ('rettypes' == NULL).
     */
    if (rettypes == NULL) {
        rettypes = MakeType (T_void, 0, NULL, NULL, NULL);
    }

    DBUG_RETURN (rettypes);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Vardecs( DFMmask_t mask, lut_t *lut)
 *
 * description:
 *   Creates a vardec-node chain based on the given DFmask.
 *   If (lut != NULL) the old/new declarations are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFM2Vardecs (DFMmask_t mask, lut_t *lut)
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
            VARDEC_REFCNT (vardecs) = ARG_REFCNT (decl);
        }
        lut = InsertIntoLUT (lut, decl, vardecs);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Args( DFMmask_t mask, lut_t *lut)
 *
 * description:
 *   Creates a argument list (arg-node chain) based on the given DFmask.
 *   If (lut != NULL) the old/new declarations are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFM2Args (DFMmask_t mask, lut_t *lut)
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
        lut = InsertIntoLUT (lut, decl, args);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *DFM2Exprs( DFMmask_t mask, lut_t *lut)
 *
 * description:
 *   Creates a exprs-node chain based on the given DFmask.
 *   If (lut != NULL) the attribute ID_VARDEC of the created IDs is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFM2Exprs (DFMmask_t mask, lut_t *lut)
{
    node *decl, *id;
    node *exprs = NULL;

    DBUG_ENTER ("DFM2Exprs");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        id = MakeId1 (VARDEC_OR_ARG_NAME (decl));
        ID_VARDEC (id) = SearchInLUT (lut, decl);
        exprs = MakeExprs (id, exprs);
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   ids *DFM2Ids( DFMmask_t mask, lut_t *lut)
 *
 * description:
 *   Creates an ids chain based on the given DFmask.
 *   If (lut != NULL) the attribute IDS_VARDEC of the created IDS is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

ids *
DFM2Ids (DFMmask_t mask, lut_t *lut)
{
    node *decl;
    ids *tmp;
    ids *ids = NULL;

    DBUG_ENTER ("DFM2Ids");

    decl = DFMGetMaskEntryDeclSet (mask);
    while (decl != NULL) {
        tmp = ids;
        ids = MakeIds1 (VARDEC_OR_ARG_NAME (decl));
        IDS_VARDEC (ids) = SearchInLUT (lut, decl);
        IDS_NEXT (ids) = tmp;
        decl = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (ids);
}
