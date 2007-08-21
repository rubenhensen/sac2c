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
#include "str.h"
#include "memory.h"
#include "LookUpTable.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

/**
 *  Controls wether AKS-information should be used when possible
 */
#define USEAKS

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#ifdef BEMT

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/**
 * INFO structure
 */
struct INFO {
    node *spmdfuns;
    node *fundef;
    lut_t *lut;
    node *args;
    node *params;
    node *vardecs;
    node *rets;
    types *rettypes;
    node *retexprs;
    node *allocassigns;
    node *freeassigns;
    bool collect;
    bool lift;
    bool withid;
};

/**
 * INFO macros
 */
#define INFO_SPMDFUNS(n) (n->spmdfuns)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LUT(n) (n->lut)
#define INFO_ARGS(n) (n->args)
#define INFO_PARAMS(n) (n->params)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_RETS(n) (n->rets)
#define INFO_RETTYPES(n) (n->rettypes)
#define INFO_RETEXPRS(n) (n->retexprs)
#define INFO_ALLOCASSIGNS(n) (n->allocassigns)
#define INFO_FREEASSIGNS(n) (n->freeassigns)
#define INFO_COLLECT(n) (n->collect)
#define INFO_LIFT(n) (n->lift)
#define INFO_WITHID(n) (n->withid)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SPMDFUNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETTYPES (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_LIFT (result) = FALSE;
    INFO_WITHID (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn static nnode *CreateSpmdFundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

static node *
CreateSpmdFundef (node *arg_node, info *arg_info)
{
    node *spmd_fundef, *fundef, *body, *withlet, *retexprs, *vardecs;
    node *allocassigns, *freeassigns, *assigns, *args, *rets;

    types *rettypes;

    DBUG_ENTER ("CreateSpmdFundef");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_let,
                 "CreateSpmdFundef( ) called with illegal node type.");

    /*
     * generate body of SPMD function
     */

    fundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    retexprs = INFO_RETEXPRS (arg_info);
    INFO_RETEXPRS (arg_info) = NULL;

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    allocassigns = INFO_ALLOCASSIGNS (arg_info);
    INFO_ALLOCASSIGNS (arg_info) = NULL;

    freeassigns = INFO_FREEASSIGNS (arg_info);
    INFO_FREEASSIGNS (arg_info) = NULL;

    rettypes = INFO_RETTYPES (arg_info);
    INFO_RETTYPES (arg_info) = NULL;

    rets = INFO_RETS (arg_info);
    INFO_RETS (arg_info) = NULL;

    args = INFO_ARGS (arg_info);
    INFO_ARGS (arg_info) = NULL;

    withlet = DUPdoDupTreeLut (arg_node, INFO_LUT (arg_info));
    INFO_LUT (arg_info) = LUTremoveContentLut (INFO_LUT (arg_info));

    assigns = TCappendAssign (allocassigns,
                              TCappendAssign (TBmakeAssign (withlet, NULL),
                                              TCappendAssign (freeassigns,
                                                              TBmakeAssign (TBmakeReturn (
                                                                              retexprs),
                                                                            NULL))));
    body = TBmakeBlock (assigns, vardecs);

    /*
     * create SPMD fundef
     */
    spmd_fundef
      = TBmakeFundef (TRAVtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, body, NULL);

    FUNDEF_TYPES (spmd_fundef) = rettypes;
    FUNDEF_ISSPMDFUN (spmd_fundef) = TRUE;

    /*
     * append return expressions to body of SPMD-function
     *
    FUNDEF_RETURN( new_fundef) = TBmakeReturn( retexprs);
    TCappendAssign( BLOCK_INSTR( body),
                    TBmakeAssign( FUNDEF_RETURN( new_fundef), NULL));
     */
    fundef = NULL;
    /*
      DBUG_RETURN( fundef);
      */

    DBUG_RETURN (spmd_fundef);
}

/******************************************************************************
 *
 * @fn SPMDLdoSpmdLift
 *
 *  @brief initiates traversal spmd lift
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
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
 * @fn node *SPMDLmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLmodule");

    INFO_LUT (arg_info) = LUTgenerateLut ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

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
         * Only ST funs may contain parallel with-loops.
         * So, we may constrain our search.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
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
 *   @brief This function was implemented to remove local with-loop variable
 *          decs, because the withloop is moved to another function at this
 *          point. Problem is now that this is no longer SSA form an the
 *          mechanism is designed for singe variable use.
 *          Maybe a better aproach here is to write a extra traversal which
 *          runs afterwards and removes not longer used vardecs.
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

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLlet (node *arg_node, info *arg_info)
{
    node *spmd_fundef, *spmd_ap;

    DBUG_ENTER ("SPMDLlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (INFO_LIFT (arg_info)) {
        spmd_fundef = CreateSpmdFundef (arg_node, arg_info);
        FUNDEF_NEXT (spmd_fundef) = INFO_SPMDFUNS (arg_info);
        INFO_SPMDFUNS (arg_info) = spmd_fundef;

        spmd_ap = TBmakeAp (spmd_fundef, INFO_PARAMS (arg_info));
        INFO_PARAMS (arg_info) = NULL;

        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = spmd_ap;

        INFO_LIFT (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SPMDLid(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
SPMDLid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLid");

    node *avis = ID_AVIS (arg_node);
    node *new_avis = NULL;

    node *ids = NULL;
    node *dim = NULL;
    node *shape = NULL;
    node *alloc = NULL;
    node *free = NULL;
    DBUG_PRINT ("SPMDL", ("ENTER id %s", ID_NAME (arg_node)));

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), arg_node) == arg_node) {
            DBUG_PRINT ("SPMDL", ("  Not handled before..."));
            new_avis = DUPdoDupNode (avis);
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
        }

        if (INFO_WITHID (arg_info)) {
            DBUG_PRINT ("SPMDL", ("...is Withid-id"));
            if (new_avis == NULL) {
                new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), arg_node);
            }
            /*Mem*/
            /*Build memory allocation for withid*/
            ids = TBmakeIds (new_avis, NULL);

#ifdef USEAKS
            if (TUdimKnown (AVIS_TYPE (new_avis))) {
                dim = TBmakeNum (TYgetDim (AVIS_TYPE (new_avis)));
            }

            if (TUshapeKnown (AVIS_TYPE (new_avis))) {
                shape = SHshape2Array (TYgetShape (AVIS_TYPE (new_avis)));
            }
#endif

            alloc = TCmakePrf3 (F_alloc, TBmakeNum (1), dim, shape);

            INFO_ALLOCASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, alloc), INFO_ALLOCASSIGNS (arg_info));

            /*Insert frees*/
            free = TCmakePrf1 (F_free, TBmakeId (new_avis));
            INFO_FREEASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (NULL, free), INFO_FREEASSIGNS (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SPMDLids(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
SPMDLids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLids");

    node *avis = IDS_AVIS (arg_node);
    node *new_avis = NULL;

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), arg_node) == NULL) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
        }
    } else {
        if (INFO_LIFT (arg_info)) {
            new_avis = DUPdoDupNode (avis);
            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));

            INFO_RETS (arg_info)
              = TBmakeRet (TYeliminateAKV (AVIS_TYPE (new_avis)), INFO_RETS (arg_info));

            INFO_RETEXPRS (arg_info)
              = TBmakeExprs (TBmakeId (new_avis), INFO_RETEXPRS (arg_info));

            INFO_RETTYPES (arg_info)
              = TCappendTypes (TYtype2OldType (AVIS_TYPE (new_avis)),
                               INFO_RETTYPES (arg_info));

            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDLwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a parallelised with-loop into a function.
 *
 ******************************************************************************/

node *
SPMDLwith2 (node *arg_node, info *arg_info)
{
    /*
    node *fundef, *new_fundef, *body;
    node *vardecs, *args;
    node *retexprs, *rets, *retids;
    node *letap;

    types *rettypes;
    lut_t *lut;
  */
    DBUG_ENTER ("SPMDLwith2");

    if (WITH2_MT (arg_node)) {
        INFO_COLLECT (arg_info) = TRUE;

        /*
         * Collect data flow information
         */

        /*Why is this added to the withloop? Insert it directly into the INFO-Struc
        if (WITH2_ALLOCASSIGNS( arg_node) != NULL) {
          WITH2_ALLOCASSIGNS( arg_node) = TRAVdo( WITH2_ALLOCASSIGNS( arg_node),
                                                  arg_info);
        }
        */
        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        /*
        INFO_ALLOCASSIGNS( arg_info) = WITH2_ALLOCASSIGNS( arg_node);
        WITH2_ALLOCASSIGNS( arg_node) = NULL;

        INFO_FREEASSIGNS( arg_info) = WITH2_FREEASSIGNS( arg_node);
        WITH2_FREEASSIGNS( arg_node) = NULL;
        */
        INFO_COLLECT (arg_info) = FALSE;
        INFO_LIFT (arg_info) = TRUE;
    } else {
        if (INFO_COLLECT (arg_info)) {
            /*
             * If we are in the collect mode, we need to traverse all sons.
             */
            WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
            WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
            WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        } else {
            /*
             * If we are still looking for a parallel with-loop, the codes are
             * the only place to have a look at.
             */
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SPMDLwithid(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
SPMDLwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLwithid");

    INFO_WITHID (arg_info) = TRUE;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITHID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *SPMDLspmd( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLspmd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLspmd");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#else

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

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

    result = MEMmalloc (sizeof (info));

    INFO_SPMDFUNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

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
      = TBmakeFundef (TRAVtmpVarName (FUNDEF_NAME (fundef)),
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

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLlet");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDLwith2( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDLwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLwith2");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SPMDLid(node *arg_node, info *arg_info)
 *
 ******************************************************************************/

node *
SPMDLid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLid");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SPMDLids(node *arg_node, info *arg_info)
 *
 ******************************************************************************/

node *
SPMDLids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLid");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *SPMDLwithid(node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
SPMDLwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDLwithid");

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#endif
