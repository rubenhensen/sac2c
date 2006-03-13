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

#include "spmd_lift.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "internal_lib.h"
#include "LookUpTable.h"
#include "namespaces.h"
#include "new_types.h"

/**
 * INFO structure
 */
struct INFO {
    node *spmdfuns;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_SPMDFUNS(n) (n->spmdfuns)
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

    INFO_SPMDFUNS (result) = NULL;
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

static node *
CreateVardecs (node *spmd, lut_t *lut)
{
    node *vardecs, *avis, *newavis;

    DBUG_ENTER ("CreateVardecs");

    DBUG_ASSERT (NODE_TYPE (spmd) == N_spmd,
                 "CreateVardecs called with non N_spmd node.");

    vardecs = NULL;

    avis = DFMgetMaskEntryAvisSet (SPMD_OUT (spmd));

    while (avis != NULL) {
        newavis = DUPdoDupNode (avis);

        vardecs = TBmakeVardec (newavis, vardecs);
        lut = LUTinsertIntoLutP (lut, avis, newavis);
        DBUG_PRINT ("SPMDL", ("inserted out variable %s", AVIS_NAME (avis)));

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    avis = DFMgetMaskEntryAvisSet (SPMD_LOCAL (spmd));

    while (avis != NULL) {
        newavis = DUPdoDupNode (avis);

        vardecs = TBmakeVardec (newavis, vardecs);
        lut = LUTinsertIntoLutP (lut, avis, newavis);
        DBUG_PRINT ("SPMDL", ("inserted local variable %s", AVIS_NAME (avis)));

        if (SPMD_COND (spmd) == NULL) {
            /*
             * If this spmd block is unconditional, the local variables no longer
             * occur in the original function after we have lifted them to the
             * spmd function. They are marked here, and the corresponding vardec
             * nodes are removed later on.
             */
            AVIS_ISLIFTED (avis) = TRUE;
        }

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (vardecs);
}

static node *
CreateArgs (node *spmd, lut_t *lut)
{
    node *args, *avis, *newavis;

    DBUG_ENTER ("CreateArgs");

    DBUG_ASSERT (NODE_TYPE (spmd) == N_spmd, "CreateArgs called with non N_spmd node.");

    args = NULL;

    avis = DFMgetMaskEntryAvisSet (SPMD_IN (spmd));

    while (avis != NULL) {
        newavis = DUPdoDupNode (avis);

        args = TBmakeArg (newavis, args);
        lut = LUTinsertIntoLutP (lut, avis, newavis);
        DBUG_PRINT ("SPMDL", ("inserted arg %s", AVIS_NAME (avis)));

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    DBUG_RETURN (args);
}

static node *
CreateRetsTypesExprsIds (types **rtypes, node **rexprs, node **rids, node *spmd,
                         lut_t *lut)
{
    node *rets, *retexprs, *retids, *avis;
    types *rettypes;

    DBUG_ENTER ("CreateRetsTypesExprs");

    DBUG_ASSERT (NODE_TYPE (spmd) == N_spmd,
                 "CreateRetsTypesExprs called with non N_spmd node.");

    rets = NULL;
    rettypes = NULL;
    retexprs = NULL;
    retids = NULL;

    avis = DFMgetMaskEntryAvisSet (SPMD_OUT (spmd));

    while (avis != NULL) {
        rets = TBmakeRet (TYeliminateAKV (AVIS_TYPE (avis)), rets);
        rettypes = TCappendTypes (TYtype2OldType (AVIS_TYPE (avis)), rettypes);
        retexprs = TBmakeExprs (TBmakeId (LUTsearchInLutPp (lut, avis)), retexprs);
        retids = TBmakeIds (avis, retids);

        avis = DFMgetMaskEntryAvisSet (NULL);
    }

    *rtypes = rettypes;
    *rexprs = retexprs;
    *rids = retids;

    DBUG_RETURN (rets);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
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

    if (FUNDEF_ISSTFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST funs may contain SPMD blocks. So, we may constrain our search.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /*
         * We have reached the end of the FUNDEF chain. We add the new SPMD functions
         * constructed meanwhile and stored in the info structure to the end and stop
         * the traversal.
         */
        FUNDEF_NEXT (arg_node) = INFO_SPMDFUNS (arg_info);
        INFO_SPMDFUNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (AVIS_ISLIFTED (VARDEC_AVIS (arg_node))) {
        arg_node = FREEdoFreeNode (arg_node);
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
    node *fundef, *new_fundef, *body;
    node *vardecs, *args;
    node *retexprs, *rets, *retids;
    node *letap;

    types *rettypes;
    lut_t *lut;

    DBUG_ENTER ("SPMDLspmd");

    fundef = INFO_FUNDEF (arg_info);

    /*
     * generate LUT (needed to get correct avis pointers during DupTree)
     */
    lut = LUTgenerateLut ();

    /*
     * build vardecs of SPMD_OUT/LOCAL-vars for SPMD function and fill LUT
     */
    vardecs = CreateVardecs (arg_node, lut);

    /*
     * build formal parameters (SPMD_IN) of SPMD function and fill LUT
     */
    args = CreateArgs (arg_node, lut);

    /*
     * build rets, return types, return exprs (use SPMD_OUT).
     */
    rets = CreateRetsTypesExprsIds (&rettypes, &retexprs, &retids, arg_node, lut);

    /*
     * generate body of SPMD function
     */
    body = DUPdoDupTreeLut (SPMD_REGION (arg_node), lut);
    BLOCK_VARDEC (body) = vardecs;

    /*
     * create SPMD fundef
     */
    new_fundef
      = TBmakeFundef (ILIBtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, body, NULL);

    FUNDEF_TYPES (new_fundef) = rettypes;
    FUNDEF_ISSPMDFUN (new_fundef) = TRUE;

    /*
     * append return expressions to body of SPMD-function
     */
    FUNDEF_RETURN (new_fundef) = TBmakeReturn (retexprs);
    TCappendAssign (BLOCK_INSTR (body), TBmakeAssign (FUNDEF_RETURN (new_fundef), NULL));

    /*
     * insert SPMD function into fundef-chain of info structure
     */
    FUNDEF_NEXT (new_fundef) = INFO_SPMDFUNS (arg_info);
    INFO_SPMDFUNS (arg_info) = new_fundef;

    /*
     * remove LUT because we need clean LUT for next SPMD block
     */
    lut = LUTremoveLut (lut);

    /*
     * replace spmd region by call of spmd function
     */
    SPMD_REGION (arg_node) = FREEdoFreeTree (SPMD_REGION (arg_node));

    letap = TBmakeLet (DFMUdfm2LetIds (SPMD_OUT (arg_node), NULL),
                       TBmakeAp (new_fundef, DFMUdfm2ApArgs (SPMD_IN (arg_node), NULL)));

    SPMD_REGION (arg_node) = TBmakeBlock (TBmakeAssign (letap, NULL), NULL);

    DBUG_RETURN (arg_node);
}
