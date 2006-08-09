/* $Id$ */

#include "DataFlowMask.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

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

node *
RDFMSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RDFMSfundef");

    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
RDFMSdoRemoveDfms (node *arg_node)
{
    DBUG_ENTER ("RDFMSdoRemoveDfms");

    TRAVpush (TR_rdfms);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}
