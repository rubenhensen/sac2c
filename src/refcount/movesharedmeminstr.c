/**
 * $Id: datareuse.c 14355 2005-10-30 10:32:28Z ktr $
 */

/**
 *
 * @file movesharedmeminstr.c
 *
 *
 */
#include "movesharedmeminstr.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "free.h"
#include "new_types.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *preassign;
    node *postassign;
    node *ap;

    node *spmdfun;
    lut_t *lut;
    node *lhs;
    bool ispreassign;
    bool ispostassign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)
#define INFO_POSTASSIGN(n) ((n)->postassign)
#define INFO_AP(n) ((n)->ap)

#define INFO_SPMDFUN(n) ((n)->spmdfun)
#define INFO_LUT(n) ((n)->lut)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_ISPREASSIGN(n) ((n)->ispreassign)
#define INFO_ISPOSTASSIGN(n) ((n)->ispostassign)

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
    INFO_POSTASSIGN (result) = NULL;
    INFO_AP (result) = NULL;

    INFO_SPMDFUN (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_ISPREASSIGN (result) = FALSE;
    INFO_ISPOSTASSIGN (result) = FALSE;

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
 * @fn node *MVSMIdoMoveSharedMemoryManagementInstructions( node *syntax_tree)
 *
 * @brief starting point of move shared memory management instruction traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
MVSMIdoMoveSharedMemoryManagementInstructions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MVSMIdoMoveSharedMemoryManagementInstructions");

    info = MakeInfo ();

    TRAVpush (TR_mvsmi);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * move shared memory management instruction traversal ( mvsmi_tab)
 *
 * prefix: MVSMI
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MVSMIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MVSMIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MVSMIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MVSMIassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MVSMIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MVSMIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_POSTASSIGN (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGN (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MVSMIap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MVSMIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MVSMIap");

    if (FUNDEF_ISSPMDFUN (AP_FUNDEF (arg_node))) {
        INFO_AP (arg_info) = arg_node;

        TRAVpush (TR_cosmi);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        TRAVpop ();

        INFO_AP (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * collect shared memory management instruction traversal ( cosmi_tab)
 *
 * prefix: COSMI
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *COSMIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIfundef (node *arg_node, info *arg_info)
{
    node *args, *apargs;

    DBUG_ENTER ("COSMIfundef");

    INFO_SPMDFUN (arg_info) = arg_node;
    INFO_LUT (arg_info) = LUTgenerateLut ();

    args = FUNDEF_ARGS (arg_node);
    apargs = AP_ARGS (INFO_AP (arg_info));

    while (args != NULL) {
        LUTinsertIntoLutP (INFO_LUT (arg_info), ARG_AVIS (args),
                           ID_AVIS (EXPRS_EXPR (apargs)));

        args = ARG_NEXT (args);
        apargs = EXPRS_NEXT (apargs);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    INFO_SPMDFUN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIvardec (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("COSMIvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    avis = VARDEC_AVIS (arg_node);
    if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) != avis) {
        VARDEC_AVIS (arg_node) = NULL;
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_ISPOSTASSIGN (arg_info)) {
        INFO_POSTASSIGN (arg_info)
          = TCappendAssign (DUPdoDupNodeLut (arg_node, INFO_LUT (arg_info)),
                            INFO_POSTASSIGN (arg_info));

        arg_node = FREEdoFreeNode (arg_node);
        INFO_ISPOSTASSIGN (arg_info) = FALSE;
    }

    if (INFO_ISPREASSIGN (arg_info)) {
        INFO_PREASSIGN (arg_info)
          = TCappendAssign (INFO_PREASSIGN (arg_info),
                            DUPdoDupNodeLut (arg_node, INFO_LUT (arg_info)));

        arg_node = FREEdoFreeNode (arg_node);
        INFO_ISPREASSIGN (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIprf");

    switch (PRF_PRF (arg_node)) {
    case F_dec_rc:
    case F_free: {
        node *avis = ID_AVIS (PRF_ARG1 (arg_node));
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) != avis) {
            INFO_ISPOSTASSIGN (arg_info) = TRUE;
        }
    } break;

    case F_alloc:
    case F_alloc_or_reuse:
    case F_alloc_or_reshape: {
        node *avis = IDS_AVIS (INFO_LHS (arg_info));
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) != avis) {
            INFO_ISPREASSIGN (arg_info) = TRUE;
        }
    } break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIwith2( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIwith2");

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IsOutVar( node *fundef, node *avis)
 *
 *****************************************************************************/
static bool
IsOutVar (node *fundef, node *avis)
{
    bool res;
    node *retexprs;

    DBUG_ENTER ("IsOutVar");

    retexprs = RETURN_EXPRS (FUNDEF_RETURN (fundef));
    while (retexprs != NULL) {
        if (ID_AVIS (EXPRS_EXPR (retexprs)) == avis) {
            break;
        }
        retexprs = EXPRS_NEXT (retexprs);
    }

    res = (retexprs != NULL);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn void MakeMemArg( node *memavis, node *extfundef, node *extap,
 *                      node *spmdfun, lut_t *lut)
 *
 *****************************************************************************/
static void
MakeMemArg (node *memavis, node *extfundef, node *extap, node *spmdfun, lut_t *lut)
{
    node *avis;

    DBUG_ENTER ("MakeMemArg");

    avis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (memavis)),
                       TYcopyType (AVIS_TYPE (memavis)));

    FUNDEF_VARDEC (extfundef) = TBmakeVardec (avis, FUNDEF_VARDEC (extfundef));

    AP_ARGS (extap) = TBmakeExprs (TBmakeId (avis), AP_ARGS (extap));

    FUNDEF_ARGS (spmdfun) = TBmakeArg (memavis, FUNDEF_ARGS (spmdfun));

    AVIS_SSAASSIGN (memavis) = NULL;

    LUTinsertIntoLutP (lut, memavis, avis);

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIgenarray");

    if (IsOutVar (INFO_SPMDFUN (arg_info), IDS_AVIS (INFO_LHS (arg_info)))) {
        MakeMemArg (ID_AVIS (GENARRAY_MEM (arg_node)), INFO_FUNDEF (arg_info),
                    INFO_AP (arg_info), INFO_SPMDFUN (arg_info), INFO_LUT (arg_info));
    }

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMImodarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMImodarray");

    if (IsOutVar (INFO_SPMDFUN (arg_info), IDS_AVIS (INFO_LHS (arg_info)))) {
        MakeMemArg (ID_AVIS (MODARRAY_MEM (arg_node)), INFO_FUNDEF (arg_info),
                    INFO_AP (arg_info), INFO_SPMDFUN (arg_info), INFO_LUT (arg_info));
    }

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIfold( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("COSMIfold");

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
