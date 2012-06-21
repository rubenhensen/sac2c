
#include "prepare_forloop_generation.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "LookUpTable.h"
#include "types.h"
#include "cuda_utils.h"
#include "constants.h"

/*
 * INFO structure
 */
struct INFO {
    node *avis;
    node *lhs;
    node *doloop;
    bool remove;
};

/*
 * INFO macros
 */
#define INFO_AVIS(n) (n->avis)
#define INFO_LHS(n) (n->lhs)
#define INFO_DOLOOP(n) (n->doloop)
#define INFO_REMOVE(n) (n->remove)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_AVIS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_DOLOOP (result) = NULL;
    INFO_REMOVE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
PFGdoPrepareForloopGeneration (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_pfg);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFGfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PFGfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCUDAGLOBALFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFGassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PFGassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_REMOVE (arg_info)) {
        arg_node = FREEdoFreeNode (arg_node);
        INFO_REMOVE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFGlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PFGlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFGdo( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PFGdo (node *arg_node, info *arg_info)
{
    node *old_avis, *old_doloop;

    DBUG_ENTER ();

    if (DO_ISFORLOOP (arg_node)) {
        old_avis = INFO_AVIS (arg_info);
        old_doloop = INFO_DOLOOP (arg_info);
        INFO_AVIS (arg_info) = ID_AVIS (DO_COND (arg_node));
        INFO_DOLOOP (arg_info) = arg_node;

        DO_BODY (arg_node) = TRAVopt (DO_BODY (arg_node), arg_info);

        INFO_AVIS (arg_info) = old_avis;
        INFO_DOLOOP (arg_info) = old_doloop;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFGid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PFGid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ID_AVIS (arg_node) == INFO_AVIS (arg_info)) {
        INFO_REMOVE (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PFGprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PFGprf (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_lt_SxS:
        if (IDS_AVIS (INFO_LHS (arg_info)) == INFO_AVIS (arg_info)) {
            DBUG_ASSERT (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id,
                         "1st argument of F_lt_SxS is not an id!");
            DBUG_ASSERT (NODE_TYPE (PRF_ARG2 (arg_node)) == N_num,
                         "2nd argument of F_lt_SxS is not an number!");
            DBUG_ASSERT (NUM_VAL (PRF_ARG2 (arg_node)) == 0,
                         "2nd argument of F_lt_SxS is not constant 0!");
            DO_RELATIONAL_OP (INFO_DOLOOP (arg_info)) = F_lt_SxS;
            INFO_AVIS (arg_info) = ID_AVIS (PRF_ARG1 (arg_node));
            INFO_REMOVE (arg_info) = TRUE;
        }
        break;
    case F_sub_SxS:
        if (IDS_AVIS (INFO_LHS (arg_info)) == INFO_AVIS (arg_info)) {
            DBUG_ASSERT (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id,
                         "1st argument of F_lt_SxS is not an id!");
            DBUG_ASSERT (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id,
                         "2nd argument of F_lt_SxS is not an id!");
            DO_UPPER_BOUND (INFO_DOLOOP (arg_info)) = ID_AVIS (PRF_ARG2 (arg_node));
            INFO_AVIS (arg_info) = ID_AVIS (PRF_ARG1 (arg_node));
            INFO_REMOVE (arg_info) = TRUE;
        }
        break;
    case F_add_SxS:
        if (IDS_AVIS (INFO_LHS (arg_info)) == INFO_AVIS (arg_info)) {
            DBUG_ASSERT (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id,
                         "1st argument of F_lt_SxS is not an id!");
            DBUG_ASSERT (NODE_TYPE (PRF_ARG2 (arg_node)) == N_num,
                         "2nd argument of F_lt_SxS is not an number!");
            DBUG_ASSERT (NUM_VAL (PRF_ARG2 (arg_node)) == 1,
                         "2nd argument of F_lt_SxS is not constant 1!");

            DO_ITERATOR (INFO_DOLOOP (arg_info)) = ID_AVIS (PRF_ARG1 (arg_node));
            IDS_AVIS (INFO_LHS (arg_info)) = ID_AVIS (PRF_ARG1 (arg_node));
            INFO_AVIS (arg_info) = ID_AVIS (PRF_ARG1 (arg_node));
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}
