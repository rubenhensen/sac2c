/*
 *
 * $Log$
 * Revision 1.1  2004/07/21 16:52:24  ktr
 * Initial revision
 *
 *
 */

/**
 *
 * @file markmemvals.c
 *
 * In this traversal unnecessary MemVal-variables are removed by means
 * of a substitution traversal.
 *
 * Ex.:
 *   c = alloc( ...);
 *   a = fill( b, c)
 *
 * is transformed into
 *   c = alloc( ...);
 *   c = fill( b, c)
 *
 * All subsequent references to a are renamed into references to c.
 *
 * Furthermore the left hand side of this assignment and a's VARDEC
 * are marked ST_artificial such that they will be eliminated during
 * precompile.
 *
 *   c = alloc( ...);
 *   fill( b, c);
 *
 * As genarray-withloops are nothing but fancy fill-operations after
 * the explicit alloc-operations are inserted, their result is treated
 * exactly like the fill operation's result.
 *
 */
#define NEW_INFO

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "LookUpTable.h"
#include "print.h"

/**
 * INFO structure
 */
struct INFO {
    LUT_t lut;
    ids *lhs;
};

#define INFO_MMV_LUT(n) (n->lut)
#define INFO_MMV_LHS(n) (n->lhs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_MMV_LUT (result) = GenerateLUT ();
    INFO_MMV_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_MMV_LUT (info) = RemoveLUT (INFO_MMV_LUT (info));
    info = Free (info);

    DBUG_RETURN (info);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/

/** <!--******************************************************************-->
 *
 * @fn MMVfundef
 *
 *  @brief Traverses a FUNDEF's body and clears the LUT afterwards.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return the given fundef with substituted identifiers.
 *
 ***************************************************************************/
node *
MMVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    INFO_MMV_LUT (arg_info) = RemoveContentLUT (INFO_MMV_LUT (arg_info));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVid
 *
 *  @brief Substitutes the current reference with a reference from the
 *         LUT if possible
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return potentially, a new N_id node
 *
 ***************************************************************************/
node *
MMVid (node *arg_node, info *arg_info)
{
    char *newname;

    DBUG_ENTER ("MMVid");

    newname = SearchInLUT_SS (INFO_MMV_LUT (arg_info), ID_NAME (arg_node));

    if (newname != ID_NAME (arg_node)) {
        ID_NAME (arg_node) = Free (ID_NAME (arg_node));
        ID_NAME (arg_node) = StringCopy (newname);
        ID_VARDEC (arg_node)
          = SearchInLUT_PP (INFO_MMV_LUT (arg_info), ID_VARDEC (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVlet
 *
 *  @brief Traverses right hand side and substitutes left hand side
 *         identifiers. If a substitution is made, the old vardec is marked
 *         ST_artificial as the new left hand side.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return modified let node.
 *
 ***************************************************************************/
node *
MMVlet (node *arg_node, info *arg_info)
{
    ids *i;
    char *newname;

    DBUG_ENTER ("MMVlet");

    INFO_MMV_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    i = LET_IDS (arg_node);

    while (i != NULL) {
        newname = SearchInLUT_SS (INFO_MMV_LUT (arg_info), IDS_NAME (i));

        if (newname != IDS_NAME (i)) {
            IDS_NAME (i) = Free (IDS_NAME (i));
            IDS_NAME (i) = StringCopy (newname);
            VARDEC_STATUS (IDS_VARDEC (i)) = ST_artificial;
            IDS_VARDEC (i) = SearchInLUT_PP (INFO_MMV_LUT (arg_info), IDS_VARDEC (i));
            IDS_STATUS (i) = ST_artificial;
        }

        i = IDS_NEXT (i);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprf
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         prf is a F_fill.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVprf");

    if (PRF_PRF (arg_node) == F_fill) {
        PRF_ARG1 (arg_node) = Trav (PRF_ARG1 (arg_node), arg_info);
        InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                         ID_NAME (PRF_ARG2 (arg_node)));

        InsertIntoLUT_P (INFO_MMV_LUT (arg_info), IDS_VARDEC (INFO_MMV_LHS (arg_info)),
                         ID_VARDEC (PRF_ARG2 (arg_node)));
    } else {
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVwith");

    if (NWITH_WITHOP (arg_node) != NULL) {
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);
    }

    if (NWITH_PART (arg_node) != NULL) {
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);
    }

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith2
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVwith2");

    if (NWITH2_WITHOP (arg_node) != NULL) {
        NWITH2_WITHOP (arg_node) = Trav (NWITH2_WITHOP (arg_node), arg_info);
    }

    if (NWITH2_SEGS (arg_node) != NULL) {
        NWITH2_SEGS (arg_node) = Trav (NWITH2_SEGS (arg_node), arg_info);
    }

    if (NWITH2_CODE (arg_node) != NULL) {
        NWITH2_CODE (arg_node) = Trav (NWITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwithop
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVwithop");

    switch (NWITHOP_TYPE (arg_node)) {
    case WO_genarray:
        if (NWITHOP_SHAPE (arg_node) != NULL) {
            NWITHOP_SHAPE (arg_node) = Trav (NWITHOP_SHAPE (arg_node), arg_info);
        }
        if (NWITHOP_DEFAULT (arg_node) != NULL) {
            NWITHOP_DEFAULT (arg_node) = Trav (NWITHOP_DEFAULT (arg_node), arg_info);
        }
        if (NWITHOP_MEM (arg_node) != NULL) {
            InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                             ID_NAME (NWITHOP_MEM (arg_node)));

            InsertIntoLUT_P (INFO_MMV_LUT (arg_info),
                             IDS_VARDEC (INFO_MMV_LHS (arg_info)),
                             ID_VARDEC (NWITHOP_MEM (arg_node)));
        }
        break;

    case WO_modarray:
        if (NWITHOP_ARRAY (arg_node) != NULL) {
            NWITHOP_ARRAY (arg_node) = Trav (NWITHOP_ARRAY (arg_node), arg_info);
        }
        if (NWITHOP_MEM (arg_node) != NULL) {
            InsertIntoLUT_S (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                             ID_NAME (NWITHOP_MEM (arg_node)));

            InsertIntoLUT_P (INFO_MMV_LUT (arg_info),
                             IDS_VARDEC (INFO_MMV_LHS (arg_info)),
                             ID_VARDEC (NWITHOP_MEM (arg_node)));
        }
        break;

    case WO_foldprf:
    case WO_foldfun:
        if (NWITHOP_NEUTRAL (arg_node) != NULL) {
            NWITHOP_NEUTRAL (arg_node) = Trav (NWITHOP_NEUTRAL (arg_node), arg_info);
        }
        break;

    case WO_unknown:
        DBUG_ASSERT ((0), "Unknown withop type found!");
    }

    if (NWITHOP_NEXT (arg_node) != NULL) {
        INFO_MMV_LHS (arg_info) = IDS_NEXT (INFO_MMV_LHS (arg_info));
        NWITHOP_NEXT (arg_node) = Trav (NWITHOP_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn MarkMemVals
 *
 *  @brief Starting function of MarkMemVals traversal
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree
 *
 ***************************************************************************/
node *
MarkMemVals (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MarkMemVals");

    info = MakeInfo ();

    act_tab = mmv_tab;
    syntax_tree = Trav (syntax_tree, info);

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
