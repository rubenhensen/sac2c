/*
 * $Id$
 */

#include "DataFlowMaskUtils.h"

#include "new_types.h"
#include "tree_basic.h"
#include "dbug.h"
#include "DataFlowMask.h"
#include "DupTree.h"
#include "LookUpTable.h"

/******************************************************************************
 *
 * function:
 *   types *DFMUdfm2Rets( dfmask_t* mask)
 *
 * description:
 *   Creates a N_rets chain based on the given DFmask.
 *
 *****************************************************************************/

node *
DFMUdfm2Rets (dfmask_t *mask)
{
    node *avis;
    node *rets = NULL;

    DBUG_ENTER ("DFMUdfm2Rets");

    /*
     * build rets
     */
    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        rets = TBmakeRet (TYcopyType (AVIS_TYPE (avis)), rets);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (rets);
}

/******************************************************************************
 *
 * function:
 *   types *DFMUdfm2ReturnTypes( dfmask_t* mask)
 *
 * description:
 *   Creates a types chain based on the given DFmask.
 *
 *****************************************************************************/

types *
DFMUdfm2ReturnTypes (dfmask_t *mask)
{
    node *avis;
    types *tmp;
    types *rettypes = NULL;

    DBUG_ENTER ("DFMUdfm2ReturnTypes");

    /*
     * build return types, return exprs (use SPMD_OUT).
     */
    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {

        tmp = rettypes;
        rettypes = TYtype2OldType (AVIS_TYPE (avis));
        TYPES_NEXT (rettypes) = tmp;

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    /*
     * we must build a void-type if ('rettypes' == NULL) is hold
     */
    if (rettypes == NULL) {
        rettypes = TBmakeTypes1 (T_void);
    }

    DBUG_RETURN (rettypes);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2Vardecs( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates a vardec-node chain based on the given DFmask.
 *   If (lut != NULL) the old/new avis are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFMUdfm2Vardecs (dfmask_t *mask, lut_t *lut)
{
    node *avis;
    node *vardecs = NULL;

    DBUG_ENTER ("DFMUdfm2Vardecs");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        vardecs = TBmakeVardec (DUPdoDupNode (avis), vardecs);
        AVIS_SSAASSIGN (VARDEC_AVIS (vardecs)) = NULL;

        lut = LUTinsertIntoLutP (lut, avis, VARDEC_AVIS (vardecs));

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2Args( dfmask_t* mask, lut_t *lut)
 *
 * description:
 *   Creates a argument list (arg-node chain) based on the given DFmask.
 *   If (lut != NULL) the old/new avis are inserted into the given LUT.
 *
 ******************************************************************************/

node *
DFMUdfm2Args (dfmask_t *mask, lut_t *lut)
{
    node *avis;
    node *args = NULL;

    DBUG_ENTER ("DFMUdfm2Args");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        args = TBmakeArg (DUPdoDupNode (avis), args);
        AVIS_SSAASSIGN (ARG_AVIS (args)) = NULL;

        DBUG_ASSERT ((NODE_TYPE (args) == N_arg), "AAAHHH");

        lut = LUTinsertIntoLutP (lut, avis, ARG_AVIS (args));

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2ReturnExprs( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain for RETURN_EXPRS based on the given DFmask.
 *   If (lut != NULL) the attribute ID_AVIS of the created IDs is set
 *   according to the given LUT, which should contain the old/new avis.
 *
 ******************************************************************************/

node *
DFMUdfm2ReturnExprs (dfmask_t *mask, lut_t *lut)
{
    node *newavis, *id;
    node *exprs = NULL;
    node *avis;

    DBUG_ENTER ("DFMUdfm2ReturnExprs");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        newavis = LUTsearchInLutPp (lut, avis);

        DBUG_ASSERT (newavis != avis, "No mapping for avis found in LUT");

        id = TBmakeId (newavis);
        exprs = TBmakeExprs (id, exprs);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   node *DFMUdfm2ApArgs( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates a exprs/id-node chain for AP/PRF_ARGS based on the given DFmask.
 *   If (lut != NULL) the attribute ID_AVIS of the created IDs is set
 *   according to the given LUT, which should contain the old/new avis.
 *
 ******************************************************************************/

node *
DFMUdfm2ApArgs (dfmask_t *mask, lut_t *lut)
{
    node *avis, *id, *newavis;
    node *exprs = NULL;

    DBUG_ENTER ("DFMUdfm2ApArgs");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        newavis = LUTsearchInLutPp (lut, avis);

        id = TBmakeId (newavis);
        exprs = TBmakeExprs (id, exprs);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (exprs);
}

/******************************************************************************
 *
 * function:
 *   ids *DFMUdfm2LetIds( dfmask_t* mask, LUT_t lut)
 *
 * description:
 *   Creates an ids chain based on the given DFmask.
 *   If (lut != NULL) the attribute IDS_VARDEC of the created IDS is set
 *   according to the given LUT, which should contain the old/new declarations.
 *
 ******************************************************************************/

node *
DFMUdfm2LetIds (dfmask_t *mask, lut_t *lut)
{
    node *avis, *newavis;
    node *ids = NULL;

    DBUG_ENTER ("DFMUdfm2LetIds");

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        newavis = LUTsearchInLutPp (lut, avis);

        ids = TBmakeIds (newavis, ids);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (ids);
}
