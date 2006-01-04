/*****************************************************************************
 *
 * $Id$
 *
 * file:   spmd_lift.c
 *
 * prefix: SPMDL
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   copy spmd-blocks to dedicated functions, the so-called spmd-functions.
 *   These are needed to actually execute spmd-blocks non-sequentially.
 *
 *   The newly generated function is inserted behind the original function.
 *   This allows to traverse it in a second traversal by spmd_lift_tab in
 *   order to adjust back references and data flow masks.
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "internal_lib.h"
#include "LookUpTable.h"
#include "InferDFMs.h"
#include "namespaces.h"
#include "new_types.h"

/**
 * INFO structure
 */
struct INFO {
    bool mt;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_MT(n) (n->mt)
#define INFO_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_MT (result) = FALSE;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn SPMDLdoSpmdLift
 *
 *  @brief
 *
 *  @param syntax_tree
 *
 *  @return
 *
 *****************************************************************************/
node *
SPMDLdoSpmdLift (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SPMDLdoSpmdLift");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

    info = MakeInfo ();

    TRAVpush (TR_spmdl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SPMDLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_MT (arg_info) = FUNDEF_ISSPMDFUN (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLspmd( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a SPMD-region into a function.
 *
 * remarks:
 *   - 'INFO_FUNDEF( arg_info)' points to the current fundef-node.
 *
 ******************************************************************************/

node *
SPMDLspmd (node *arg_node, info *arg_info)
{
    node *avis, *fundef, *new_fundef, *body;
    node *fvardecs;
    node *fargs;
    node *retexprs;
    node *rets;
    types *rettypes;
    lut_t *lut;

    DBUG_ENTER (" SPMDLspmd");

    fundef = INFO_FUNDEF (arg_info);

    /****************************************************************************
     * build fundef for this spmd region
     */

    /*
     * generate LUT (needed to get correct avis pointers during DupTree)
     */
    lut = LUTgenerateLut ();

    /*
     * build vardecs of SPMD_OUT/LOCAL-vars for SPMD function and fill LUT
     */
    fvardecs = NULL;
    avis = DFMgetMaskEntryAvisSet (SPMD_OUT (arg_node));
    while (avis != NULL) {
        /* reduce outs by ins */
        if (!DFMtestMaskEntry (SPMD_IN (arg_node), NULL, avis)) {
            node *newavis = DUPdoDupNode (avis);

            fvardecs = TBmakeVardec (newavis, fvardecs);
            DBUG_PRINT ("SPMDL", ("inserted out variable %s", AVIS_NAME (avis)));

            lut = LUTinsertIntoLutP (lut, avis, newavis);
        }
        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    avis = DFMgetMaskEntryAvisSet (SPMD_LOCAL (arg_node));
    while (avis != NULL) {
        node *newavis = DUPdoDupNode (avis);

        fvardecs = TBmakeVardec (newavis, fvardecs);

        lut = LUTinsertIntoLutP (lut, avis, newavis);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    avis = DFMgetMaskEntryAvisSet (SPMD_SHARED (arg_node));
    while (avis != NULL) {
        /* reduce shareds by ins and outs */
        if ((!DFMtestMaskEntry (SPMD_IN (arg_node), NULL, avis))
            && (!DFMtestMaskEntry (SPMD_OUT (arg_node), NULL, avis))) {
            node *newavis = DUPdoDupNode (avis);

            fvardecs = TBmakeVardec (newavis, fvardecs);

            DBUG_PRINT ("SPMDL", ("inserted shared variable%s", AVIS_NAME (avis)));

            lut = LUTinsertIntoLutP (lut, avis, newavis);
        }
        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    /*
     * build formal parameters (SPMD_IN/INOUT) of SPMD function and fill LUT
     */
    fargs = NULL;
    avis = DFMgetMaskEntryAvisSet (SPMD_IN (arg_node));
    while (avis != NULL) {
        node *newavis = DUPdoDupNode (avis);

        fargs = TBmakeArg (newavis, fargs);

        DBUG_PRINT ("SPMDL", ("inserted arg %s", AVIS_NAME (avis)));

        lut = LUTinsertIntoLutP (lut, avis, newavis);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    /*
     * build return types, return exprs (use SPMD_OUT).
     */
    rets = NULL;
    retexprs = NULL;
    rettypes = NULL;
    avis = DFMgetMaskEntryAvisSet (SPMD_OUT (arg_node));
    while (avis != NULL) {
        retexprs = TBmakeExprs (TBmakeId (LUTsearchInLutPp (lut, avis)), retexprs);
        rets = TBmakeRet (TYeliminateAKV (AVIS_TYPE (avis)), rets);
        rettypes = TCappendTypes (TYtype2OldType (AVIS_TYPE (avis)), rettypes);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    /*
     * generate body of SPMD function
     */
    body = DUPdoDupTreeLut (SPMD_REGION (arg_node), lut);
    BLOCK_VARDEC (body) = fvardecs;

    new_fundef
      = TBmakeFundef (ILIBtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, fargs, body, NULL);

    FUNDEF_TYPES (new_fundef) = rettypes;

    FUNDEF_ISSPMDFUN (new_fundef) = TRUE;
    FUNDEF_LIFTEDFROM (new_fundef) = fundef;

    SPMD_FUNDEF (arg_node) = new_fundef;

    /*
     * append return expressions to body of SPMD-function
     */
    FUNDEF_RETURN (new_fundef) = TBmakeReturn (retexprs);
    TCappendAssign (BLOCK_INSTR (body), TBmakeAssign (FUNDEF_RETURN (new_fundef), NULL));

    /*
     * update DFMs for the new fundef
     */
    new_fundef = INFDFMSdoInferDfms (new_fundef, HIDE_LOCALS_NEVER);

    /*
     * insert SPMD-function into fundef-chain of modul
     *
     * CAUTION: we must insert the SPMD-fundef *behind* the current fundef,
     *          because it must be traversed to correct the vardec-pointers of
     *          all id's and to generate new DFMasks!!
     */

    if (FUNDEF_NEXT (fundef) != NULL) {
        FUNDEF_NEXT (new_fundef) = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = new_fundef;
    } else {
        FUNDEF_NEXT (fundef) = new_fundef;
    }

    /*
     * remove LUT
     */
    lut = LUTremoveLut (lut);

    /*
     * build fundef for this spmd region
     ***************************************************************************/

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   Generates new DFMasks in WITH2_IN/INOUT/OUT/LOCAL.
 *
 * remark:
 *   During this phase each with-loop is marked as being multi-threaded
 *   or not. A with-loop is multi-threaded iff it is situated on the top level
 *   of an spmd-function. This information is used during code generation
 *   in order to produce slightly different ICMs.
 *
 ******************************************************************************/

node *
SPMDLwith2 (node *arg_node, info *arg_info)
{
    node *vardec, *args;
    dfmask_t *in, *out, *local;

    DBUG_ENTER ("SPMDLwith2");

    /*
     * mark with-loop as being multi-threaded or not depending on arg_info
     */

    WITH2_MT (arg_node) = INFO_MT (arg_info);

    /*
     * traverse sons
     */
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    INFO_MT (arg_info) = FALSE;

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    INFO_MT (arg_info) = WITH2_MT (arg_node);
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * generate new DFMasks
     */

    in = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));
    out = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));
    local = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    /*
     * traverse all args and vardecs
     */
    args = FUNDEF_ARGS (INFO_FUNDEF (arg_info));
    while (args != NULL) {
        if (DFMtestMaskEntry (WITH2_IN_MASK (arg_node), NULL, ARG_AVIS (args))) {
            DFMsetMaskEntrySet (in, NULL, ARG_AVIS (args));
        }
        if (DFMtestMaskEntry (WITH2_OUT_MASK (arg_node), NULL, ARG_AVIS (args))) {
            DFMsetMaskEntrySet (out, NULL, ARG_AVIS (args));
        }
        if (DFMtestMaskEntry (WITH2_LOCAL_MASK (arg_node), NULL, ARG_AVIS (args))) {
            DFMsetMaskEntrySet (local, NULL, ARG_AVIS (args));
        }
        args = ARG_NEXT (args);
    }
    vardec = FUNDEF_VARDEC (INFO_FUNDEF (arg_info));
    while (vardec != NULL) {
        if (DFMtestMaskEntry (WITH2_IN_MASK (arg_node), NULL, VARDEC_AVIS (vardec))) {
            DFMsetMaskEntrySet (in, NULL, VARDEC_AVIS (vardec));
        }
        if (DFMtestMaskEntry (WITH2_OUT_MASK (arg_node), NULL, VARDEC_AVIS (vardec))) {
            DFMsetMaskEntrySet (out, NULL, VARDEC_AVIS (vardec));
        }
        if (DFMtestMaskEntry (WITH2_LOCAL_MASK (arg_node), NULL, VARDEC_AVIS (vardec))) {
            DFMsetMaskEntrySet (local, NULL, VARDEC_AVIS (vardec));
        }
        vardec = VARDEC_NEXT (vardec);
    }

    WITH2_IN_MASK (arg_node) = DFMremoveMask (WITH2_IN_MASK (arg_node));
    WITH2_IN_MASK (arg_node) = in;
    WITH2_OUT_MASK (arg_node) = DFMremoveMask (WITH2_OUT_MASK (arg_node));
    WITH2_OUT_MASK (arg_node) = out;
    WITH2_LOCAL_MASK (arg_node) = DFMremoveMask (WITH2_LOCAL_MASK (arg_node));
    WITH2_LOCAL_MASK (arg_node) = local;

    DBUG_RETURN (arg_node);
}
