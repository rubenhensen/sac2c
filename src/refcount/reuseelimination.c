/*
 *
 * $Log$
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

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"

/**
 * INFO structure
 */
struct INFO {
    bool remassign;
    ids *lhs;
    LUT_t lut;
    DFMmask_t mask;
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

    result = Malloc (sizeof (info));

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

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREReuseElimination( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMREReuseElimination (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMREReuseElimination");

    DBUG_PRINT ("EMRE", ("Starting reuse elimination."));

    info = MakeInfo ();

    act_tab = emre_tab;

    syntax_tree = Trav (syntax_tree, info);

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
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    remassign = INFO_RE_REMASSIGN (arg_info);
    INFO_RE_REMASSIGN (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (remassign) {
        arg_node = FreeNode (arg_node);
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
    DFMmask_t oldmask;
    LUT_t oldlut;

    DBUG_ENTER ("EMREcond");

    oldmask = INFO_RE_MASK (arg_info);
    oldlut = INFO_RE_LUT (arg_info);

    INFO_RE_MASK (arg_info) = DFMGenMaskCopy (oldmask);
    INFO_RE_LUT (arg_info) = DuplicateLUT (oldlut);

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    INFO_RE_MASK (arg_info) = DFMRemoveMask (INFO_RE_MASK (arg_info));
    INFO_RE_LUT (arg_info) = RemoveLUT (INFO_RE_LUT (arg_info));

    INFO_RE_MASK (arg_info) = DFMGenMaskCopy (oldmask);
    INFO_RE_LUT (arg_info) = DuplicateLUT (oldlut);

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    INFO_RE_MASK (arg_info) = DFMRemoveMask (INFO_RE_MASK (arg_info));
    INFO_RE_LUT (arg_info) = RemoveLUT (INFO_RE_LUT (arg_info));

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
        DFMmask_base_t maskbase;

        DBUG_PRINT ("EMRE", ("Performing Reuse elimination in function %s...",
                             FUNDEF_NAME (arg_node)));

        maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        INFO_RE_MASK (arg_info) = DFMGenMaskClear (maskbase);
        INFO_RE_LUT (arg_info) = GenerateLUT ();

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        INFO_RE_LUT (arg_info) = RemoveLUT (INFO_RE_LUT (arg_info));
        INFO_RE_MASK (arg_info) = DFMRemoveMask (INFO_RE_MASK (arg_info));

        maskbase = DFMRemoveMaskBase (maskbase);

        DBUG_PRINT ("EMRE", ("Reuse elimination in function %s complete.",
                             FUNDEF_NAME (arg_node)));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
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

    new_node = Trav (LET_EXPR (arg_node), arg_info);

    if (new_node != LET_EXPR (arg_node)) {
        arg_node = FreeNode (arg_node);
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
    node *vardec;

    DBUG_ENTER ("EMREprf");

    switch (PRF_PRF (arg_node)) {
    case F_reuse:
        /*
         * a = reuse( n, b);
         *
         * 1. Replace a with b in remaining program
         */
        INFO_RE_LUT (arg_info)
          = InsertIntoLUT_P (INFO_RE_LUT (arg_info), IDS_VARDEC (INFO_RE_LHS (arg_info)),
                             ID_VARDEC (PRF_ARG2 (arg_node)));

        /*
         * 2. Mark b in MASK
         */
        DFMSetMaskEntrySet (INFO_RE_MASK (arg_info), NULL,
                            ID_VARDEC (PRF_ARG2 (arg_node)));

        /*
         * 3. Convert into NOOP / F_inc_rc
         */
        if (NUM_VAL (PRF_ARG1 (arg_node)) == 1) {
            INFO_RE_REMASSIGN (arg_info) = TRUE;
        } else {
            DBUG_ASSERT (NUM_VAL (PRF_ARG1 (arg_node)) > 1, "Illegal rc value");

            arg_node = MakeLet (MakePrf2 (F_inc_rc, DupNode (PRF_ARG2 (arg_node)),
                                          MakeNum (NUM_VAL (PRF_ARG1 (arg_node)) - 1)),
                                NULL);
        }
        break;

    case F_wl_assign:
    case F_fill:
        /*
         * Replace memory variable with reused variable
         */
        vardec = SearchInLUT_PP (INFO_RE_LUT (arg_info), ID_VARDEC (PRF_ARG2 (arg_node)));

        if (vardec != ID_VARDEC (PRF_ARG2 (arg_node))) {

            PRF_ARG2 (arg_node) = FreeNode (PRF_ARG2 (arg_node));

            PRF_ARG2 (arg_node)
              = MakeId (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular);
            ID_VARDEC (PRF_ARG2 (arg_node)) = vardec;
            ID_AVIS (PRF_ARG2 (arg_node)) = VARDEC_AVIS (vardec);
        }
        break;

    case F_suballoc:
        /*
         * Replace memory variable with reused variable
         */
        vardec = SearchInLUT_PP (INFO_RE_LUT (arg_info), ID_VARDEC (PRF_ARG1 (arg_node)));

        if (vardec != ID_VARDEC (PRF_ARG1 (arg_node))) {

            PRF_ARG1 (arg_node) = FreeNode (PRF_ARG1 (arg_node));

            PRF_ARG1 (arg_node)
              = MakeId (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular);
            ID_VARDEC (PRF_ARG1 (arg_node)) = vardec;
            ID_AVIS (PRF_ARG1 (arg_node)) = VARDEC_AVIS (vardec);
        }
        break;

    case F_dec_rc:
        /*
         * remove dec_rcs of reused variables
         */
        if (DFMTestMaskEntry (INFO_RE_MASK (arg_info), NULL,
                              ID_VARDEC (PRF_ARG1 (arg_node)))) {
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
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    if (SearchInLUT_PP (INFO_RE_LUT (arg_info), arg_node) != arg_node) {
        arg_node = FreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMREwithop(node *arg_node, info *arg_info)
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
EMREwithop (node *arg_node, info *arg_info)
{
    node *vardec;

    DBUG_ENTER ("EMREwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
    case WO_modarray:
        /*
         * replace memory variables with reused variables
         */
        vardec
          = SearchInLUT_PP (INFO_RE_LUT (arg_info), ID_VARDEC (NWITHOP_MEM (arg_node)));

        if (vardec != ID_VARDEC (NWITHOP_MEM (arg_node))) {

            NWITHOP_MEM (arg_node) = FreeNode (NWITHOP_MEM (arg_node));

            NWITHOP_MEM (arg_node)
              = MakeId (StringCopy (VARDEC_NAME (vardec)), NULL, ST_regular);
            ID_VARDEC (NWITHOP_MEM (arg_node)) = vardec;
            ID_AVIS (NWITHOP_MEM (arg_node)) = VARDEC_AVIS (vardec);
        }

        break;

    default:
        break;
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
