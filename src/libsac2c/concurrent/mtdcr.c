/*****************************************************************************
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
 *   Beware that this is *not* a general purpose dead code removal, but
 *   very much geared to this very case!!
 *
 *   The mechanism is based on three data flow mask (or conceptually variable
 *   sets): alloc, free and block. Set alloc collects all identifiers holding
 *   memory allocated through F_alloc. Whenever we hit a reference to an
 *   identifier (left or right hand side), we clear the identifier from the
 *   alloc set as it is obviously not dead. When we hit a second nested
 *   F_alloc to some identifier, we most likely want to remove the outer alloc
 *   and free operations. All references to the variable between the inner
 *   alloc and free are harmless. Therefore, we also include the variable in
 *   the block set in that time. Whenver we encounter a free operation, we
 *   either remove the variable from the block set (as now the outer instance
 *   is relevamt again) or we put it into the free set. On our way back up
 *   the assignment spine we remove all free and alloc operations involving
 *   variables that are in the alloc and free set but not in the block set.
 *
 *****************************************************************************/

#include "mtdcr.h"

#define DBUG_PREFIX "MTDCR"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "memory.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "print.h"
#include <stdio.h>

#ifndef DBUG_OFF
static FILE *outfile = NULL;
#endif

/**
 * INFO structure
 */

struct INFO {
    dfmask_base_t *dfmbase;
    dfmask_t *dfmalloc;
    dfmask_t *dfmfree;
    dfmask_t *dfmblock;
    node *lhs;
    bool ignore;
    bool kill;
    bool dokill;
    bool check;
    bool inwithid;
};

/**
 * INFO macros
 */

#define INFO_DFMBASE(n) ((n)->dfmbase)
#define INFO_DFMALLOC(n) ((n)->dfmalloc)
#define INFO_DFMFREE(n) ((n)->dfmfree)
#define INFO_DFMBLOCK(n) ((n)->dfmblock)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_IGNORE(n) ((n)->ignore)
#define INFO_KILL(n) ((n)->kill)
#define INFO_DOKILL(n) ((n)->dokill)
#define INFO_CHECK(n) ((n)->check)
#define INFO_INWITHID(n) ((n)->inwithid)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_DFMBASE (result) = NULL;
    INFO_DFMALLOC (result) = NULL;
    INFO_DFMFREE (result) = NULL;
    INFO_DFMBLOCK (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_IGNORE (result) = FALSE;
    INFO_KILL (result) = FALSE;
    INFO_DOKILL (result) = FALSE;
    INFO_CHECK (result) = FALSE;
    INFO_INWITHID (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_DFMALLOC (info) == NULL, "no dfm expected");
    DBUG_ASSERT (INFO_DFMFREE (info) == NULL, "no dfm expected");
    DBUG_ASSERT (INFO_DFMBLOCK (info) == NULL, "no dfm expected");
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
    DBUG_ENTER ();

    DBUG_EXECUTE (outfile = global.outfile; global.outfile = stderr);

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_EXECUTE (global.outfile = outfile);

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
    DBUG_ENTER ();

    if ((FUNDEF_ISSTFUN (arg_node) || FUNDEF_ISXTFUN (arg_node))
        && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST and XT funs may have contained parallel with-loops.
         * Hence, we constrain our activity accordingly.
         */
        DBUG_PRINT ("Entering function %s.", FUNDEF_NAME (arg_node));

        INFO_DFMBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        INFO_DFMALLOC (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));
        INFO_DFMFREE (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));
        INFO_DFMBLOCK (arg_info) = DFMgenMaskClear (INFO_DFMBASE (arg_info));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFMALLOC (arg_info) = DFMremoveMask (INFO_DFMALLOC (arg_info));
        INFO_DFMFREE (arg_info) = DFMremoveMask (INFO_DFMFREE (arg_info));
        INFO_DFMBLOCK (arg_info) = DFMremoveMask (INFO_DFMBLOCK (arg_info));

        INFO_DFMBASE (arg_info) = DFMremoveMaskBase (INFO_DFMBASE (arg_info));

        DBUG_PRINT ("Leaving function %s.\n", FUNDEF_NAME (arg_node));
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
    DBUG_ENTER ();

    DBUG_PRINT ("Entering block");
    DBUG_EXECUTE (PRTdoPrint (arg_node));

    DBUG_PRINT ("DFM ALLOC:");
    DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMALLOC (arg_info)));

    DBUG_PRINT ("DFM FREE:");
    DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMFREE (arg_info)));

    DBUG_PRINT ("DFM BLOCK:");
    DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMBLOCK (arg_info)));

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    DBUG_PRINT ("Leaving block");
    DBUG_EXECUTE (PRTdoPrint (arg_node));

    DBUG_PRINT ("DFM ALLOC:");
    DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMALLOC (arg_info)));

    DBUG_PRINT ("DFM FREE:");
    DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMFREE (arg_info)));

    DBUG_PRINT ("DFM BLOCK:");
    DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMBLOCK (arg_info)));

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
    DBUG_ENTER ();

    INFO_CHECK (arg_info) = TRUE;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    INFO_CHECK (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("Reached end of block");

        DBUG_PRINT ("DFM ALLOC:");
        DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMALLOC (arg_info)));

        DBUG_PRINT ("DFM FREE:");
        DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMFREE (arg_info)));

        DBUG_PRINT ("DFM BLOCK:");
        DBUG_EXECUTE (DFMprintMask (stderr, "%s ", INFO_DFMBLOCK (arg_info)));
    }

    /*
     * The following test is not quite in the spirit of the traversal mechanism.
     * However, we face a dilemma here. We must ensure that we do not traverse
     * into anything than prf nodes on our way back up. Otherwise the inference
     * mechanism with the three masks does not work. The only alternative to this
     * hard coded check here would be traversal functions for all other potential
     * expression types for the sole reason to avoid traversal into that expression.
     * Now, this solution is not without pitfalls either: if we ever add a new
     * such structure to the AST, we have to add the appropriate code here as well.
     * That is extremely error-prone as no-one will be aware of this connection.
     * Hence, I (cg) chose the solution below.
     */

    if ((NODE_TYPE (ASSIGN_STMT (arg_node)) == N_let)
        && (NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))) == N_prf)) {
        INFO_KILL (arg_info) = TRUE;
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        INFO_KILL (arg_info) = FALSE;
    }

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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    if (INFO_CHECK (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_alloc:
            if (DFMtestMaskEntry (INFO_DFMALLOC (arg_info),
                                  IDS_AVIS (INFO_LHS (arg_info)))) {
                /*
                 * The variable is already in the alloc set, so we are here in a second
                 * nested alloc and put the variable into the block set. If it is in the
                 * block set already, we raise an exception because three levels of
                 * nesting must not occur.
                 */
                DBUG_ASSERT (!DFMtestMaskEntry (INFO_DFMBLOCK (arg_info),
                                                IDS_AVIS (INFO_LHS (arg_info))),
                             "More than two levels of alloc/free to same identifier "
                             "found");
                DFMsetMaskEntrySet (INFO_DFMBLOCK (arg_info),
                                    IDS_AVIS (INFO_LHS (arg_info)));
            } else {
                /*
                 * The variable has not been found yet, so we put it into the alloc set.
                 */
                DFMsetMaskEntrySet (INFO_DFMALLOC (arg_info),
                                    IDS_AVIS (INFO_LHS (arg_info)));
            }
            INFO_IGNORE (arg_info) = TRUE;
            break;
        case F_free:
            if (DFMtestMaskEntry (INFO_DFMBLOCK (arg_info),
                                  ID_AVIS (PRF_ARG1 (arg_node)))) {
                /*
                 * The variable is in the block set, so we have reached the end of the
                 * inner instance life time and clear it from the block set.
                 */
                DFMsetMaskEntryClear (INFO_DFMBLOCK (arg_info),
                                      ID_AVIS (PRF_ARG1 (arg_node)));
            } else {
                /*
                 * We have found the outer free and collect the variable in the free
                 * set.
                 */
                DFMsetMaskEntrySet (INFO_DFMFREE (arg_info),
                                    ID_AVIS (PRF_ARG1 (arg_node)));
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    if (INFO_KILL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_alloc:
            if (DFMtestMaskEntry (INFO_DFMALLOC (arg_info),
                                  IDS_AVIS (INFO_LHS (arg_info)))
                && DFMtestMaskEntry (INFO_DFMFREE (arg_info),
                                     IDS_AVIS (INFO_LHS (arg_info)))
                && !DFMtestMaskEntry (INFO_DFMBLOCK (arg_info),
                                      IDS_AVIS (INFO_LHS (arg_info)))) {
                INFO_DOKILL (arg_info) = TRUE;
            }
            break;
        case F_free:
            if (DFMtestMaskEntry (INFO_DFMALLOC (arg_info),
                                  ID_AVIS (PRF_ARG1 (arg_node)))
                && DFMtestMaskEntry (INFO_DFMFREE (arg_info),
                                     ID_AVIS (PRF_ARG1 (arg_node)))
                && !DFMtestMaskEntry (INFO_DFMBLOCK (arg_info),
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
    DBUG_ENTER ();

    if (INFO_CHECK (arg_info)) {
        if (!DFMtestMaskEntry (INFO_DFMBLOCK (arg_info), IDS_AVIS (arg_node))) {
            DFMsetMaskEntryClear (INFO_DFMALLOC (arg_info), IDS_AVIS (arg_node));
        }

        IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ();

    if (INFO_CHECK (arg_info)) {
        if (!DFMtestMaskEntry (INFO_DFMBLOCK (arg_info), ID_AVIS (arg_node))) {
            DFMsetMaskEntryClear (INFO_DFMALLOC (arg_info), ID_AVIS (arg_node));
        }
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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtdcr);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
