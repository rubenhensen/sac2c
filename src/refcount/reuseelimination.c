/*
 *
 * $Log$
 * Revision 1.11  2004/12/01 16:35:23  ktr
 * Rule for Block added.
 *
 * Revision 1.10  2004/11/24 14:08:15  ktr
 * MakeLet permutation.
 *
 * Revision 1.9  2004/11/23 22:27:23  ktr
 * renaming.
 *
 * Revision 1.8  2004/11/23 20:12:22  ktr
 * COMPILES!!!
 *
 * Revision 1.7  2004/11/23 20:08:48  jhb
 * compile
 *
 * Revision 1.6  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.5  2004/11/18 17:04:58  ktr
 * Added some DBUG_PRINTs
 *
 * Revision 1.4  2004/11/07 14:25:24  ktr
 * ongoing implementation.
 *
 * Revision 1.3  2004/11/02 14:33:25  ktr
 * Reuseelimination is now performed seperately for each branch of a cond.
 *
 * Revision 1.2  2004/10/22 15:38:19  ktr
 * Ongoing implementation.
 *
 * Revision 1.1  2004/10/22 14:13:50  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup stre Reuse elimination
 * @ingroup rc
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file reuseelimination.c
 *
 *
 */
#define NEW_INFO

#include "reuseelimination.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "internal_lib.h"
#include "free.h"

/**
 * INFO structure
 */
struct INFO {
    bool remassign;
    node *lhs;
    lut_t *lut;
    dfmask_t *mask;
};

/**
 * INFO macros
 */
#define INFO_RE_REMASSIGN(n) (n->remassign)
#define INFO_RE_LHS(n) (n->lhs)
#define INFO_RE_LUT(n) (n->lut)
#define INFO_RE_MASK(n) (n->mask)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_RE_REMASSIGN (result) = FALSE;
    INFO_RE_LHS (result) = NULL;
    INFO_RE_LUT (result) = NULL;
    INFO_RE_MASK (result) = NULL;

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

    DBUG_ENTER ("EMREdoReuseElimination");

    DBUG_PRINT ("EMRE", ("Starting reuse elimination."));

    info = MakeInfo ();

    TRAVpush (TR_emre);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("EMRE", ("Reuse elimination complete."));

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

    DBUG_ENTER ("EMREassign");

    /*
     * Top-down traversal
     */
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    remassign = INFO_RE_REMASSIGN (arg_info);
    INFO_RE_REMASSIGN (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("EMREblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
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

    DBUG_ENTER ("EMREcond");

    oldmask = INFO_RE_MASK (arg_info);
    oldlut = INFO_RE_LUT (arg_info);

    INFO_RE_MASK (arg_info) = DFMgenMaskCopy (oldmask);
    INFO_RE_LUT (arg_info) = LUTduplicateLut (oldlut);

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_RE_MASK (arg_info) = DFMremoveMask (INFO_RE_MASK (arg_info));
    INFO_RE_LUT (arg_info) = LUTremoveLut (INFO_RE_LUT (arg_info));

    INFO_RE_MASK (arg_info) = DFMgenMaskCopy (oldmask);
    INFO_RE_LUT (arg_info) = LUTduplicateLut (oldlut);

    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    INFO_RE_MASK (arg_info) = DFMremoveMask (INFO_RE_MASK (arg_info));
    INFO_RE_LUT (arg_info) = LUTremoveLut (INFO_RE_LUT (arg_info));

    INFO_RE_MASK (arg_info) = oldmask;
    INFO_RE_LUT (arg_info) = oldlut;

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
    DBUG_ENTER ("EMREfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        dfmask_base_t *maskbase;

        DBUG_PRINT ("EMRE", ("Performing Reuse elimination in function %s...",
                             FUNDEF_NAME (arg_node)));

        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        INFO_RE_MASK (arg_info) = DFMgenMaskClear (maskbase);
        INFO_RE_LUT (arg_info) = LUTgenerateLut ();

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_RE_LUT (arg_info) = LUTremoveLut (INFO_RE_LUT (arg_info));
        INFO_RE_MASK (arg_info) = DFMremoveMask (INFO_RE_MASK (arg_info));

        maskbase = DFMremoveMaskBase (maskbase);

        DBUG_PRINT ("EMRE", ("Reuse elimination in function %s complete.",
                             FUNDEF_NAME (arg_node)));
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
    node *new_node;

    DBUG_ENTER ("EMRElet");

    INFO_RE_LHS (arg_info) = LET_IDS (arg_node);

    new_node = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (new_node != LET_EXPR (arg_node)) {
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = new_node;
    }

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
    node *avis;

    DBUG_ENTER ("EMREprf");

    switch (PRF_PRF (arg_node)) {
    case F_reuse:
        /*
         * a = reuse( n, b);
         *
         * 1. Replace a with b in remaining program
         */
        INFO_RE_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_RE_LUT (arg_info), IDS_AVIS (INFO_RE_LHS (arg_info)),
                               ID_AVIS (PRF_ARG2 (arg_node)));

        /*
         * 2. Mark b in MASK
         */
        DFMsetMaskEntrySet (INFO_RE_MASK (arg_info), NULL, ID_AVIS (PRF_ARG2 (arg_node)));

        /*
         * 3. Convert into NOOP / F_inc_rc
         */
        if (NUM_VAL (PRF_ARG1 (arg_node)) == 1) {
            INFO_RE_REMASSIGN (arg_info) = TRUE;
        } else {
            DBUG_ASSERT (NUM_VAL (PRF_ARG1 (arg_node)) > 1, "Illegal rc value");

            arg_node
              = TBmakeLet (NULL,
                           TCmakePrf2 (F_inc_rc, DUPdoDupNode (PRF_ARG2 (arg_node)),
                                       TBmakeNum (NUM_VAL (PRF_ARG1 (arg_node)) - 1)));
        }
        break;

    case F_reshape:
        /*
         * a = reshape( rc, dim, shape, b);
         *
         * Mark b in MASK
         */
        DFMsetMaskEntrySet (INFO_RE_MASK (arg_info), NULL, ID_AVIS (PRF_ARG4 (arg_node)));
        break;

    case F_wl_assign:
    case F_fill:
        /*
         * Replace memory variable with reused variable
         */
        avis = LUTsearchInLutPp (INFO_RE_LUT (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        if (avis != ID_AVIS (PRF_ARG2 (arg_node))) {

            PRF_ARG2 (arg_node) = FREEdoFreeNode (PRF_ARG2 (arg_node));
            PRF_ARG2 (arg_node) = TBmakeId (avis);
        }
        break;

    case F_suballoc:
        /*
         * Replace memory variable with reused variable
         */
        avis = LUTsearchInLutPp (INFO_RE_LUT (arg_info), ID_AVIS (PRF_ARG1 (arg_node)));

        if (avis != ID_AVIS (PRF_ARG1 (arg_node))) {

            PRF_ARG1 (arg_node) = FREEdoFreeNode (PRF_ARG1 (arg_node));
            PRF_ARG1 (arg_node) = TBmakeId (avis);
        }
        break;

    case F_dec_rc:
        /*
         * remove dec_rcs of reused variables
         */
        if (DFMtestMaskEntry (INFO_RE_MASK (arg_info), NULL,
                              ID_AVIS (PRF_ARG1 (arg_node)))) {
            INFO_RE_REMASSIGN (arg_info) = TRUE;
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
    DBUG_ENTER ("EMREvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (LUTsearchInLutPp (INFO_RE_LUT (arg_info), VARDEC_AVIS (arg_node))
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

    DBUG_ENTER ("EMREgenarray");

    /*
     * replace memory variables with reused variables
     */
    avis = LUTsearchInLutPp (INFO_RE_LUT (arg_info), ID_AVIS (GENARRAY_MEM (arg_node)));

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

    DBUG_ENTER ("EMREmodarray");

    /*
     * replace memory variables with reused variables
     */
    avis = LUTsearchInLutPp (INFO_RE_LUT (arg_info), ID_AVIS (MODARRAY_MEM (arg_node)));

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
