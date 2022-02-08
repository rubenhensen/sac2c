/**
 * @defgroup stre Reuse elimination
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file reuseelimination.c
 *
 * Prefix: EMRE
 */
#include "reuseelimination.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMRE"
#include "debug.h"

#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "str.h"
#include "memory.h"
#include "free.h"

/**
 * INFO structure
 */
struct INFO {
    bool remassign;
    node *lhs;
    lut_t *lut;
    dfmask_t *mask;
    node *postass;
    node *fundef;
};

/**
 * INFO macros
 */
#define INFO_REMASSIGN(n) (n->remassign)
#define INFO_LHS(n) (n->lhs)
#define INFO_LUT(n) (n->lut)
#define INFO_MASK(n) (n->mask)
#define INFO_POSTASS(n) (n->postass)
#define INFO_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_REMASSIGN (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_MASK (result) = NULL;
    INFO_POSTASS (result) = NULL;
    INFO_FUNDEF (result) = NULL;

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
 * @fn node *EMREdoReuseElimination( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMREdoReuseElimination (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting reuse elimination.");

    info = MakeInfo ();

    TRAVpush (TR_emre);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Reuse elimination complete.");

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Reuse elimination traversal (emre_tab)
 *
 * prefix: EMRE
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMREassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREassign (node *arg_node, info *arg_info)
{
    bool remassign;
    node *postassign;

    DBUG_ENTER ();

    /*
     * Top-down traversal
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    remassign = INFO_REMASSIGN (arg_info);
    INFO_REMASSIGN (arg_info) = FALSE;

    postassign = INFO_POSTASS (arg_info);
    INFO_POSTASS (arg_info) = NULL;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (postassign != NULL) {
        ASSIGN_NEXT (arg_node) = TCappendAssign (postassign, ASSIGN_NEXT (arg_node));
    }

    if (remassign) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    if (BLOCK_VARDECS (arg_node) != NULL) {
        BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREcond(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREcond (node *arg_node, info *arg_info)
{
    dfmask_t *oldmask;
    lut_t *oldlut;

    DBUG_ENTER ();

    oldmask = INFO_MASK (arg_info);
    oldlut = INFO_LUT (arg_info);

    INFO_MASK (arg_info) = DFMgenMaskCopy (oldmask);
    INFO_LUT (arg_info) = LUTduplicateLut (oldlut);

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_MASK (arg_info) = DFMremoveMask (INFO_MASK (arg_info));
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    INFO_MASK (arg_info) = DFMgenMaskCopy (oldmask);
    INFO_LUT (arg_info) = LUTduplicateLut (oldlut);

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    INFO_MASK (arg_info) = DFMremoveMask (INFO_MASK (arg_info));
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    INFO_MASK (arg_info) = oldmask;
    INFO_LUT (arg_info) = oldlut;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        dfmask_base_t *maskbase;

        DBUG_PRINT ("Performing Reuse elimination in function %s...",
                    FUNDEF_NAME (arg_node));

        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        INFO_MASK (arg_info) = DFMgenMaskClear (maskbase);
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_FUNDEF (arg_info) = arg_node;

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
        INFO_MASK (arg_info) = DFMremoveMask (INFO_MASK (arg_info));

        maskbase = DFMremoveMaskBase (maskbase);

        DBUG_PRINT ("Reuse elimination in function %s complete.", FUNDEF_NAME (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRElet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMRElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREprf(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREprf (node *arg_node, info *arg_info)
{
    int n;
    node *avis, *bavis;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_reuse:
        /*
         * a = reuse( n, b);
         */
        n = NUM_VAL (PRF_ARG1 (arg_node));
        avis = IDS_AVIS (INFO_LHS (arg_info));
        bavis = ID_AVIS (PRF_ARG2 (arg_node));

        /*
         * 1. Mark b in MASK
         */
        DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, bavis);

        if ((FUNDEF_ISLOOPFUN (INFO_FUNDEF (arg_info)))
            && (AVIS_SSAASSIGN (bavis) == NULL)) {
            /*
             * FIX FOR BUG #128!!!
             *
             * IFF b is a function argument of a loop function:
             * 2. convert into copy assignment
             *    a = b
             */
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeId (bavis);
        } else {
            /*
             * Otherwise:
             * 2. Replace a with b in remaining program and remove whole assigment
             */
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, bavis);

            INFO_REMASSIGN (arg_info) = TRUE;
        }

        /*
         * 3. Append inc_rc statement if necessary
         */
        if (n > 1) {
            node *prf;
            prf = TCmakePrf2 (F_inc_rc, TBmakeId (bavis), TBmakeNum (n - 1));

            INFO_POSTASS (arg_info) = TBmakeAssign (TBmakeLet (NULL, prf), NULL);
        }
        break;

    case F_reshape_VxA:
        /*
         * a = reshape( rc, dim, shape, b);
         *
         * Mark b in MASK
         */
        DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, ID_AVIS (PRF_ARG4 (arg_node)));
        break;

    case F_resize:
        /*
         * a = resize( rc, dim, shape, b);
         *
         * Mark b in MASK
         */
        DFMsetMaskEntrySet (INFO_MASK (arg_info), NULL, ID_AVIS (PRF_ARG4 (arg_node)));
        break;

    case F_wl_assign:
    case F_fill:
        /*
         * Replace memory variable with reused variable
         */
        avis
          = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        if (avis != ID_AVIS (PRF_ARG2 (arg_node))) {
            PRF_ARG2 (arg_node) = FREEdoFreeNode (PRF_ARG2 (arg_node));
            PRF_ARG2 (arg_node) = TBmakeId (avis);
        }
        break;

    case F_cond_wl_assign:
        /*
         * Replace memory variable with reused variable
         */
        avis
          = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (PRF_ARG5 (arg_node)));

        if (avis != ID_AVIS (PRF_ARG5 (arg_node))) {
            PRF_ARG5 (arg_node) = FREEdoFreeNode (PRF_ARG5 (arg_node));
            PRF_ARG5 (arg_node) = TBmakeId (avis);
        }
        break;

    case F_suballoc:
        /*
         * Replace memory variable with reused variable
         */
        avis
          = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        if (avis != ID_AVIS (PRF_ARG2 (arg_node))) {

            PRF_ARG2 (arg_node) = FREEdoFreeNode (PRF_ARG2 (arg_node));
            PRF_ARG2 (arg_node) = TBmakeId (avis);
        }
        break;

    case F_dec_rc:
        /*
         * remove dec_rcs of reused variables
         */
        if (DFMtestMaskEntry (INFO_MASK (arg_info), NULL,
                              ID_AVIS (PRF_ARG1 (arg_node)))) {
            INFO_REMASSIGN (arg_info) = TRUE;
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREvardec(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (LUTsearchInLutPp (INFO_LUT (arg_info), VARDEC_AVIS (arg_node))
        != VARDEC_AVIS (arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREgenarray(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREgenarray (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    /*
     * replace memory variables with reused variables
     */
    avis
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (GENARRAY_MEM (arg_node)));

    if (avis != ID_AVIS (GENARRAY_MEM (arg_node))) {
        GENARRAY_MEM (arg_node) = FREEdoFreeNode (GENARRAY_MEM (arg_node));
        GENARRAY_MEM (arg_node) = TBmakeId (avis);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *EMREmodarray(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
EMREmodarray (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    /*
     * replace memory variables with reused variables
     */
    avis
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (MODARRAY_MEM (arg_node)));

    if (avis != ID_AVIS (MODARRAY_MEM (arg_node))) {
        MODARRAY_MEM (arg_node) = FREEdoFreeNode (MODARRAY_MEM (arg_node));
        MODARRAY_MEM (arg_node) = TBmakeId (avis);
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
