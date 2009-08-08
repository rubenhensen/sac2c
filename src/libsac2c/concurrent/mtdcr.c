/*****************************************************************************
 *
 * $Id$
 *
 * file:   mtdcr.c
 *
 * prefix: MTDCR
 *
 * description:
 *
 *   This is a special variant of dead code removal tailor made to be run
 *   in the multithreading phase after creation of spmd functions.
 *
 *   Creation of spmd functions leaves a certain kind of dead code in the
 *   ST function it is lifted from:
 *    - variable declarations of with-loop local identifiers now residing in
 *      the spmd function.
 *      These are *not* handled here but in dead_vardec_removal in precompile.
 *    - alloc and free statements for the withid variables that are now
 *      handled thread-locally within the spmd function.
 *      These are handled here.
 *
 *
 *****************************************************************************/

#include "mtdcr.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "memory.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"

/**
 * INFO structure
 */

struct INFO {
    dfmask_base_t *dfmbase;
    dfmask_t *dfm;
    node *lhs;
    bool ignore;
    bool kill;
    bool dokill;
    bool check;
};

/**
 * INFO macros
 */

#define INFO_DFMBASE(n) ((n)->dfmbase)
#define INFO_DFM(n) ((n)->dfm)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_IGNORE(n) ((n)->ignore)
#define INFO_KILL(n) ((n)->kill)
#define INFO_DOKILL(n) ((n)->dokill)
#define INFO_CHECK(n) ((n)->check)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_DFMBASE (result) = NULL;
    INFO_DFM (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_IGNORE (result) = FALSE;
    INFO_KILL (result) = FALSE;
    INFO_DOKILL (result) = FALSE;
    INFO_CHECK (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    DBUG_ASSERT (INFO_DFM (info) == NULL, "no dfm expected");
    DBUG_ASSERT (INFO_DFMBASE (info) == NULL, "no dfmbase expected");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRmodule");

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRfundef");

    if (FUNDEF_ISSTFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST funs may have contained parallel with-loops.
         * Hence, we constrain our activity accordingly.
         */
        DBUG_PRINT ("MTDCR", ("Entering function %s.", FUNDEF_NAME (arg_node)));

        INFO_DFMBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFMBASE (arg_info) = DFMremoveMaskBase (INFO_DFMBASE (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRblock (node *arg_node, info *arg_info)
{
    dfmask_t *dfm;
    bool check, kill;

    DBUG_ENTER ("MTDCRblock");

    dfm = INFO_DFM (arg_info);
    INFO_DFM (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));

    check = INFO_CHECK (arg_info);
    INFO_CHECK (arg_info) = FALSE;

    kill = INFO_KILL (arg_info);
    INFO_KILL (arg_info) = FALSE;

    BLOCK_INSTR (arg_node) = TRAVopt (BLOCK_INSTR (arg_node), arg_info);

    if (BLOCK_INSTR (arg_node) == NULL) {
        BLOCK_INSTR (arg_node) = TBmakeEmpty ();
    }

    INFO_DFM (arg_info) = DFMremoveMask (INFO_DFM (arg_info));
    INFO_DFM (arg_info) = dfm;

    INFO_CHECK (arg_info) = check;
    INFO_KILL (arg_info) = kill;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRassign");

    INFO_CHECK (arg_info) = TRUE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    INFO_CHECK (arg_info) = FALSE;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_KILL (arg_info) = TRUE;
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    INFO_KILL (arg_info) = FALSE;

    if (INFO_DOKILL (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_DOKILL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    if (INFO_CHECK (arg_info) && !INFO_IGNORE (arg_info)) {
        INFO_IGNORE (arg_info) = FALSE;
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRprf");

    if (INFO_CHECK (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_alloc:
            DFMsetMaskEntrySet (INFO_DFM (arg_info), NULL,
                                IDS_AVIS (INFO_LHS (arg_info)));
            INFO_IGNORE (arg_info) = TRUE;
            break;
        case F_free:
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    if (INFO_KILL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_alloc:
            if (DFMtestMaskEntry (INFO_DFM (arg_info), NULL,
                                  IDS_AVIS (INFO_LHS (arg_info)))) {
                INFO_DOKILL (arg_info) = TRUE;
            }
            break;
        case F_free:
            if (DFMtestMaskEntry (INFO_DFM (arg_info), NULL,
                                  ID_AVIS (PRF_ARG1 (arg_node)))) {
                INFO_DOKILL (arg_info) = TRUE;
            }
            break;
        default:
            INFO_DOKILL (arg_info) = FALSE;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRids");

    if (INFO_CHECK (arg_info)) {
        DFMsetMaskEntryClear (INFO_DFM (arg_info), NULL, IDS_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDCRid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDCRid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTDCRid");

    if (INFO_CHECK (arg_info)) {
        DFMsetMaskEntryClear (INFO_DFM (arg_info), NULL, ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTDCRdoMtDeadCodeRemoval( node *syntax_tree)
 *
 *  @brief initiates traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTDCRdoMtDeadCodeRemoval (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MTDCRdoMtDeadCodeRemoval");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtdcr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
