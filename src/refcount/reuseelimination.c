/*
 *
 * $Log$
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

/**
 * INFO structure
 */
struct INFO {
    bool remassign;
    ids *lhs;
    LUT_t lut;
};

/**
 * INFO macros
 */
#define INFO_RE_REMASSIGN(n) (n->remassign)
#define INFO_RE_LUT(n) (n->lut)
#define INFO_RE_LHS(n) (n->lhs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_RE_REMASSIGN (result) = NULL;
    INFO_RE_LHS (result) = NULL;
    INFO_RE_LUT (result) = NULL;

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
    LUT_t old_lut;

    DBUG_ENTER ("EMREblock");

    old_lut = INFO_RE_LUT (arg_info);

    INFO_RE_LUT (arg_info) = GenerateLUT ();

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    INFO_RE_LUT (arg_info) = RemoveLUT (INFO_RE_LUT (arg_info));
    INFO_RE_LUT (arg_info) = old_lut;

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
          = InsertIntoLUT_P (INFO_RE_LUT (arg_info), IDS_AVIS (INFO_RE_LHS (arg_info)),
                             ID_AVIS (PRF_ARG2 (arg_node)));
        DBUG_PRINT ("EMRE", ("Added (%s,%s) to LUT.", IDS_NAME (INFO_RE_LHS (arg_info)),
                             ID_NAME (PRF_ARG2 (arg_node))));

        /*
         * 2. Convert into NOOP / F_inc_rc
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

    case F_fill:

        avis = SearchInLUT_PP (INFO_RE_LUT (arg_info), ID_AVIS (PRF_ARG2 (arg_node)));

        DBUG_PRINT ("EMRE", ("Found (%s,%s) in LUT.", ID_NAME (PRF_ARG2 (arg_node)),
                             VARDEC_NAME (AVIS_VARDECORARG (avis))));

        if (avis != ID_AVIS (PRF_ARG2 (arg_node))) {
            PRF_ARG2 (arg_node) = FreeNode (PRF_ARG2 (arg_node));

            PRF_ARG2 (arg_node)
              = MakeId (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                        ST_regular);
            ID_VARDEC (PRF_ARG2 (arg_node)) = AVIS_VARDECORARG (avis);
            ID_AVIS (PRF_ARG2 (arg_node)) = avis;
        }
        break;

    case F_dec_rc:
        avis = SearchInLUT_PP (INFO_RE_LUT (arg_info), ID_AVIS (PRF_ARG1 (arg_node)));

        DBUG_PRINT ("EMRE", ("Found (%s,%s) in LUT.", ID_NAME (PRF_ARG2 (arg_node)),
                             VARDEC_NAME (AVIS_VARDECORARG (avis))));

        if (avis != ID_AVIS (PRF_ARG1 (arg_node))) {
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
    node *avis;

    DBUG_ENTER ("EMREwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
    case WO_modarray:
        avis = SearchInLUT_PP (INFO_RE_LUT (arg_info), ID_AVIS (NWITHOP_MEM (arg_node)));

        DBUG_PRINT ("EMRE", ("Found (%s,%s) in LUT.", ID_NAME (PRF_ARG2 (arg_node)),
                             VARDEC_NAME (AVIS_VARDECORARG (avis))));

        if (avis != ID_AVIS (NWITHOP_MEM (arg_node))) {
            NWITHOP_MEM (arg_node) = FreeNode (NWITHOP_MEM (arg_node));

            NWITHOP_MEM (arg_node)
              = MakeId (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (avis))), NULL,
                        ST_regular);
            ID_VARDEC (NWITHOP_MEM (arg_node)) = AVIS_VARDECORARG (avis);
            ID_AVIS (NWITHOP_MEM (arg_node)) = avis;
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
