/**
 *
 * @file movesharedmeminstr.c
 *
 *
 */
#include "movesharedmeminstr.h"
#include "str.h"
#include "memory.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "free.h"
#include "new_types.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *preassign;
    node *postassign;
    node *ap;

    int linksign;
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

#define INFO_LINKSIGN(n) ((n)->linksign)
#define INFO_SPMDFUN(n) ((n)->spmdfun)
#define INFO_LUT(n) ((n)->lut)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_ISPREASSIGN(n) ((n)->ispreassign)
#define INFO_ISPOSTASSIGN(n) ((n)->ispostassign)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_POSTASSIGN (result) = NULL;
    INFO_AP (result) = NULL;

    INFO_LINKSIGN (result) = 0;
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
    DBUG_ENTER ();

    info = MEMfree (info);

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

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    /*
     * annotate initial linksign information
     */
    INFO_LINKSIGN (arg_info) = 1;

    FUNDEF_RETS (arg_node) = TRAVopt(FUNDEF_RETS (arg_node), arg_info);

    FUNDEF_ARGS (arg_node) = TRAVopt(FUNDEF_ARGS (arg_node), arg_info);

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

    FUNDEF_BODY (arg_node) = TRAVopt(FUNDEF_BODY (arg_node), arg_info);

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    INFO_SPMDFUN (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIret( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RET_LINKSIGN (arg_node) = INFO_LINKSIGN (arg_info);
    RET_HASLINKSIGNINFO (arg_node) = TRUE;
    INFO_LINKSIGN (arg_info) += 1;

    RET_NEXT (arg_node) = TRAVopt(RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIarg( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_LINKSIGN (arg_node) = INFO_LINKSIGN (arg_info);
    ARG_HASLINKSIGNINFO (arg_node) = TRUE;
    INFO_LINKSIGN (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    BLOCK_VARDECS (arg_node) = TRAVopt(BLOCK_VARDECS (arg_node), arg_info);

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

    DBUG_ENTER ();

    VARDEC_NEXT (arg_node) = TRAVopt(VARDEC_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn int IsOutVar( node *fundef, node *avis)
 *
 *****************************************************************************/
static int
IsOutVar (node *fundef, node *avis)
{
    int res;
    int count = 0;
    node *retexprs;

    DBUG_ENTER ();

    retexprs = RETURN_EXPRS (FUNDEF_RETURN (fundef));
    while (retexprs != NULL) {
        count += 1;
        if (ID_AVIS (EXPRS_EXPR (retexprs)) == avis) {
            break;
        }
        retexprs = EXPRS_NEXT (retexprs);
    }

    res = (retexprs != NULL) ? count : 0;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn void MakeMemArg( node *memavis, node *extfundef, node *extap,
 *                      node *spmdfun, lut_t *lut, int linksign)
 *
 *****************************************************************************/
static void
MakeMemArg (node *memavis, node *extfundef, node *extap, node *spmdfun, lut_t *lut,
            int linksign)
{
    node *avis;
    node *arg;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (memavis)),
                       TYcopyType (AVIS_TYPE (memavis)));

    FUNDEF_VARDECS (extfundef) = TBmakeVardec (avis, FUNDEF_VARDECS (extfundef));

    AP_ARGS (extap) = TBmakeExprs (TBmakeId (avis), AP_ARGS (extap));

    arg = TBmakeArg (memavis, FUNDEF_ARGS (spmdfun));

    ARG_LINKSIGN (arg) = linksign;
    ARG_HASLINKSIGNINFO (arg) = TRUE;

    FUNDEF_ARGS (spmdfun) = arg;

    AVIS_SSAASSIGN (memavis) = NULL;

    LUTinsertIntoLutP (lut, memavis, avis);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn node *COSMIgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
COSMIgenarray (node *arg_node, info *arg_info)
{
    int linksign;

    DBUG_ENTER ();

    linksign = IsOutVar (INFO_SPMDFUN (arg_info), IDS_AVIS (INFO_LHS (arg_info)));

    if (linksign != 0) {
        MakeMemArg (ID_AVIS (GENARRAY_MEM (arg_node)), INFO_FUNDEF (arg_info),
                    INFO_AP (arg_info), INFO_SPMDFUN (arg_info), INFO_LUT (arg_info),
                    linksign);
    }

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    GENARRAY_NEXT (arg_node) = TRAVopt(GENARRAY_NEXT (arg_node), arg_info);

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
    int linksign;

    DBUG_ENTER ();

    linksign = IsOutVar (INFO_SPMDFUN (arg_info), IDS_AVIS (INFO_LHS (arg_info)));

    if (linksign != 0) {
        MakeMemArg (ID_AVIS (MODARRAY_MEM (arg_node)), INFO_FUNDEF (arg_info),
                    INFO_AP (arg_info), INFO_SPMDFUN (arg_info), INFO_LUT (arg_info),
                    linksign);
    }

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    MODARRAY_NEXT (arg_node) = TRAVopt(MODARRAY_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));

    FOLD_NEXT (arg_node) = TRAVopt(FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
