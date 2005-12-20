/**
 *
 * $Id: datareuse.c 14355 2005-10-30 10:32:28Z ktr $
 *
 * @file removespmd.c
 *
 */
#include "removespmd.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "internal_lib.h"
#include "free.h"
#include "shape.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *preassign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDdoRemoveSpmdBlocks( node *syntax_tree)
 *
 * @brief starting point of Remove Spmd blocks traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
RMSPMDdoRemoveSpmdBlocks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RMSPMDdoRemoveSpmdBlocks");

    info = MakeInfo ();

    TRAVpush (TR_rmspmd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Remove Spmd blocks traversal traversal ( rmspmd_tab)
 *
 * prefix: RMSPMD
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMSPMDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMSPMDfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMSPMDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMSPMDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDspmd( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMSPMDspmd (node *arg_node, info *arg_info)
{
    node *letap;
    node *predavis;
    node *pred, *thenblock, *elseblock;

    DBUG_ENTER ("RMSPMDspmd");

    letap = TBmakeLet (DFMUdfm2LetIds (SPMD_OUT (arg_node), NULL),
                       TBmakeAp (SPMD_FUNDEF (arg_node),
                                 DFMUdfm2ApArgs (SPMD_IN (arg_node), NULL)));
    thenblock = TBmakeBlock (TBmakeAssign (letap, NULL), NULL);

    elseblock = SPMD_REGION (arg_node);
    SPMD_REGION (arg_node) = NULL;

    predavis = TBmakeAvis (ILIBtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (predavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL),
                                 TBmakePrf (F_singlethread, NULL)),
                      NULL);
    pred = TBmakeId (predavis);

    /*
     * Free spmd block
     */
    arg_node = FREEdoFreeNode (arg_node);

    /*
     * return conditional
     */
    arg_node = TBmakeCond (pred, thenblock, elseblock);

    DBUG_RETURN (arg_node);
}
