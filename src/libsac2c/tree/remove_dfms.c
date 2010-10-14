/* $Id$ */

#include "DataFlowMask.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool onefundef;
};

/**
 * A template entry in the template info structure
 */
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMSwith( node *arg_node, info *arg_info)
 *
 * @brief Removes the data flow masks from this node.
 *
 * @param arg_node with node
 * @param arg_info NULL
 *
 * @return with without DFMs
 ******************************************************************************/
node *
RDFMSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSwith");

    if (WITH_IN_MASK (arg_node) != NULL) {
        WITH_IN_MASK (arg_node) = DFMremoveMask (WITH_IN_MASK (arg_node));
    }

    if (WITH_OUT_MASK (arg_node) != NULL) {
        WITH_OUT_MASK (arg_node) = DFMremoveMask (WITH_OUT_MASK (arg_node));
    }

    if (WITH_LOCAL_MASK (arg_node) != NULL) {
        WITH_LOCAL_MASK (arg_node) = DFMremoveMask (WITH_LOCAL_MASK (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMSwith2( node *arg_node, info *arg_info)
 *
 * @brief Removes the data flow masks from this node.
 *
 * @param arg_node with2 node
 * @param arg_info NULL
 *
 * @return with2 without DFMs
 ******************************************************************************/
node *
RDFMSwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSwith2");

    if (WITH2_IN_MASK (arg_node) != NULL) {
        WITH2_IN_MASK (arg_node) = DFMremoveMask (WITH2_IN_MASK (arg_node));
    }

    if (WITH2_OUT_MASK (arg_node) != NULL) {
        WITH2_OUT_MASK (arg_node) = DFMremoveMask (WITH2_OUT_MASK (arg_node));
    }

    if (WITH2_LOCAL_MASK (arg_node) != NULL) {
        WITH2_LOCAL_MASK (arg_node) = DFMremoveMask (WITH2_LOCAL_MASK (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMSwith3( node *arg_node, info *arg_info)
 *
 * @brief Removes the data flow masks from this node.
 *
 * @param arg_node with3 node
 * @param arg_info NULL
 *
 * @return with3 without DFMs
 ******************************************************************************/
node *
RDFMSwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSwith3");

    if (WITH3_IN_MASK (arg_node) != NULL) {
        WITH3_IN_MASK (arg_node) = DFMremoveMask (WITH3_IN_MASK (arg_node));
    }

    if (WITH3_OUT_MASK (arg_node) != NULL) {
        WITH3_OUT_MASK (arg_node) = DFMremoveMask (WITH3_OUT_MASK (arg_node));
    }

    if (WITH3_LOCAL_MASK (arg_node) != NULL) {
        WITH3_LOCAL_MASK (arg_node) = DFMremoveMask (WITH3_LOCAL_MASK (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMScond( node *arg_node, info *arg_info)
 *
 * @brief Removes the data flow masks from this node.
 *
 * @param arg_node cond node
 * @param arg_info NULL
 *
 * @return cond without DFMs
 ******************************************************************************/
node *
RDFMScond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMScond");

    if (COND_IN_MASK (arg_node) != NULL) {
        COND_IN_MASK (arg_node) = DFMremoveMask (COND_IN_MASK (arg_node));
    }

    if (COND_OUT_MASK (arg_node) != NULL) {
        COND_OUT_MASK (arg_node) = DFMremoveMask (COND_OUT_MASK (arg_node));
    }

    if (COND_LOCAL_MASK (arg_node) != NULL) {
        COND_LOCAL_MASK (arg_node) = DFMremoveMask (COND_LOCAL_MASK (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMSdo( node *arg_node, info *arg_info)
 *
 * @brief Removes the data flow masks from this node.
 *
 * @param arg_node do node
 * @param arg_info NULL
 *
 * @return do without DFMs
 ******************************************************************************/
node *
RDFMSdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSdo");

    if (DO_IN_MASK (arg_node) != NULL) {
        DO_IN_MASK (arg_node) = DFMremoveMask (DO_IN_MASK (arg_node));
    }

    if (DO_OUT_MASK (arg_node) != NULL) {
        DO_OUT_MASK (arg_node) = DFMremoveMask (DO_OUT_MASK (arg_node));
    }

    if (DO_LOCAL_MASK (arg_node) != NULL) {
        DO_LOCAL_MASK (arg_node) = DFMremoveMask (DO_LOCAL_MASK (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMSblock( node *arg_node, info *arg_info)
 *
 * @brief Removes the data flow masks from this node.
 *
 * @param arg_node block node
 * @param arg_info NULL
 *
 * @return do without DFMs
 ******************************************************************************/
node *
RDFMSblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSblock");

    if (BLOCK_IN_MASK (arg_node) != NULL) {
        BLOCK_IN_MASK (arg_node) = DFMremoveMask (BLOCK_IN_MASK (arg_node));
    }

    if (BLOCK_OUT_MASK (arg_node) != NULL) {
        BLOCK_OUT_MASK (arg_node) = DFMremoveMask (BLOCK_OUT_MASK (arg_node));
    }

    if (BLOCK_LOCAL_MASK (arg_node) != NULL) {
        BLOCK_LOCAL_MASK (arg_node) = DFMremoveMask (BLOCK_LOCAL_MASK (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *RDFMSfundef( node *arg_node, info *arg_info)
 *
 * @brief  Removes the data flow masks from this node.
 *
 * @param arg_node fundef node
 * @param arg_info NULL
 *
 * @return fundef without DFMs
 ******************************************************************************/
node *
RDFMSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSfundef");

    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Removes data flow masks from the ast given as first argument.
 *
 * @param arg_node piece of syntax tree
 *
 * @return cleaned up tree
 ******************************************************************************/
node *
RDFMSdoRemoveDfms (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("RDFMSdoRemoveDfms");

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (NODE_TYPE (arg_node) == N_fundef);

    TRAVpush (TR_rdfms);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
