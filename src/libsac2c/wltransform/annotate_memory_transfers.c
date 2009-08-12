/*****************************************************************************
 *
 *
 * file:   annotate_cuda_withloop.c
 *
 * prefix: ACUWL
 *
 * description:
 *
 *
 *****************************************************************************/

#include "annotate_memory_transfers.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "deadcoderemoval.h"

#include "NumLookUpTable.h"

#define ISHOST2DEVICE(assign)                                                            \
    (assign == NULL                                                                      \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                     \
            ? FALSE                                                                      \
            : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                  \
                 ? FALSE                                                                 \
                 : (PRF_PRF (ASSIGN_RHS (assign)) == F_host2device ? TRUE : FALSE))))

#define ISDEVICE2HOST(assign)                                                            \
    (assign == NULL                                                                      \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                     \
            ? FALSE                                                                      \
            : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                  \
                 ? FALSE                                                                 \
                 : (PRF_PRF (ASSIGN_RHS (assign)) == F_device2host ? TRUE : FALSE))))

enum traverse_mode { trav_topdown, trav_bottomup };

/*
 * INFO structure
 */
struct INFO {
    bool indofun;
    nlut_t *nlut;
    enum traverse_mode travmode;
    int funargnum;
    node *lastassign;
    node *secondapargs;
    bool insecondapargs;
    node *fundef;
};

/*
 * INFO macros
 */

#define INFO_NLUT(n) (n->nlut)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_FUNARGNUM(n) (n->funargnum)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_INDOFUN(n) (n->indofun)
#define INFO_SECONDAPARGS(n) (n->secondapargs)
#define INFO_INSECONDAPARGS(n) (n->insecondapargs)
#define INFO_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_NLUT (result) = NULL;
    INFO_TRAVMODE (result) = trav_topdown;
    INFO_FUNARGNUM (result) = 0;
    INFO_LASTASSIGN (result) = NULL;
    INFO_INDOFUN (result) = FALSE;
    INFO_SECONDAPARGS (result) = NULL;
    INFO_INSECONDAPARGS (result) = FALSE;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANdoMinimizeLoopTransfers( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANdoAnnotateMemoryTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("AMTRANdoAnnotateMemoryTransfers");

    info = MakeInfo ();
    TRAVpush (TR_amtran);
    INFO_TRAVMODE (info) = trav_topdown;
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    info = MakeInfo ();
    INFO_TRAVMODE (info) = trav_bottomup;
    TRAVpush (TR_amtran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANassign( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANassign");

    if (INFO_TRAVMODE (arg_info) == trav_topdown) {
        INFO_LASTASSIGN (arg_info) = arg_node;
        ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (arg_node) = FALSE;
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == trav_bottomup) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        INFO_LASTASSIGN (arg_info) = arg_node;
        ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (arg_node) = FALSE;
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANfundef( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_ISDOFUN (arg_node)) {
        INFO_INDOFUN (arg_info) = TRUE;
        if (INFO_TRAVMODE (arg_info) == trav_bottomup) {
            INFO_FUNARGNUM (arg_info) = 0;
            FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        }
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_NLUT (arg_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_NLUT (arg_info) = NLUTremoveNlut (INFO_NLUT (arg_info));
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANarg( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANarg");

    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANap( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANap");

    /* If the application is to a do-fun and the application is not
     * in a do-fun. This prevents the do-fun from being traversed
     * for more than once.
     */
    if (INFO_INDOFUN (arg_info) && INFO_FUNDEF (arg_info) == AP_FUNDEF (arg_node)
        && INFO_TRAVMODE (arg_info) == trav_bottomup) {
        INFO_SECONDAPARGS (arg_info) = AP_ARGS (arg_node);
        INFO_INSECONDAPARGS (arg_info) = TRUE;
    }

    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    INFO_INSECONDAPARGS (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANid( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AMTRANid");

    if (INFO_INDOFUN (arg_info)) {
        if (INFO_TRAVMODE (arg_info) == trav_topdown) {
            if (NODE_TYPE (ID_DECL (arg_node)) == N_arg) {
                NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);
            }
        } else if (INFO_TRAVMODE (arg_info) == trav_bottomup) {
            if (NODE_TYPE (ID_DECL (arg_node)) == N_arg
                && !INFO_INSECONDAPARGS (arg_info)) {
                NLUTincNum (INFO_NLUT (arg_info), ID_AVIS (arg_node), 1);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

static node *
nthApArg (node *args, int n)
{
    int i = 0;
    node *tmp = args;

    DBUG_ENTER ("nthApArg");

    while (i < n) {
        tmp = EXPRS_NEXT (tmp);
        i++;
    }

    tmp = EXPRS_EXPR (tmp);
    DBUG_RETURN (tmp);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *MLTRANprf( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
AMTRANprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ("AMTRANprf");

    if (INFO_INDOFUN (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            id = PRF_ARG1 (arg_node);
            if (STReq (AVIS_NAME (ID_AVIS (id)), "_pinl_1963__f2l_1951_a"))
                printf ("haha got you \n");

            if (NODE_TYPE (ID_DECL (id)) == N_arg) {
                if (NLUTgetNum (INFO_NLUT (arg_info), ID_AVIS (id)) != 0) {
                    ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info)) = TRUE;
                }
                if (INFO_TRAVMODE (arg_info) == trav_bottomup) {
                    int fun_arg_num = ARG_LINKSIGN (ID_DECL (id));
                    node *ap_arg = nthApArg (INFO_SECONDAPARGS (arg_info), fun_arg_num);
                    if (!ISDEVICE2HOST (AVIS_SSAASSIGN (ID_AVIS (ap_arg)))
                        && NODE_TYPE (ID_DECL (ap_arg)) != N_arg) {
                        ASSIGN_ISNOTALLOWEDTOBEMOVEDUP (INFO_LASTASSIGN (arg_info))
                          = TRUE;
                    }
                }
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}
