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

#include "minimize_sequential_transfers.h"

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

static bool nochange = TRUE;

/*
 * INFO structure
 */
struct INFO {
    node *lastassign;
    node *letids;
    bool in_cuda_wl;
    bool removable;
    lut_t *lut;
    int round;
};

/*
 * INFO macros
 */

#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_LETIDS(n) (n->letids)
#define INFO_IN_CUDA_WL(n) (n->in_cuda_wl)
#define INFO_LUT(n) (n->lut)
#define INFO_REMOVABLE(n) (n->removable)
#define INFO_ROUND(n) (n->round)

#define ISHOST2DEVICE(assign)                                                            \
    (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                          \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                       \
            ? FALSE                                                                      \
            : (PRF_PRF (ASSIGN_RHS (assign)) == F_host2device ? TRUE : FALSE)))

#define ISDEVICE2HOST(assign)                                                            \
    (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                          \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                       \
            ? FALSE                                                                      \
            : (PRF_PRF (ASSIGN_RHS (assign)) == F_device2host ? TRUE : FALSE)))

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LASTASSIGN (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_IN_CUDA_WL (result) = FALSE;
    INFO_LUT (result) = NULL;
    INFO_REMOVABLE (result) = FALSE;
    INFO_ROUND (result) = 0;

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
 * @brief node *MTRANdoMinimizeTransfers( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
MSTRANdoMinimizeTransfers (node *syntax_tree, bool *flag)
// node *MSTRANdoMinimizeTransfers(node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("MSTRANdoMinimizeTransfers");

    static int round = 1;

    info = MakeInfo ();

    INFO_ROUND (info) = round;

    TRAVpush (TR_mstran);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    *flag = nochange;

    syntax_tree = DCRdoDeadCodeRemovalModule (syntax_tree);

    round++;

    DBUG_RETURN (syntax_tree);
}

node *
MSTRANfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTRANfundef");

    INFO_LUT (arg_info) = LUTgenerateLut ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
MSTRANassign (node *arg_node, info *arg_info)
{
    bool removable;

    DBUG_ENTER ("MTRANassign");

    INFO_LASTASSIGN (arg_info) = arg_node;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    removable = INFO_REMOVABLE (arg_info);

    INFO_REMOVABLE (arg_info) = FALSE;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (removable) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
MSTRANwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTRANwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_IN_CUDA_WL (arg_info) = TRUE;
        WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
        INFO_IN_CUDA_WL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
MSTRANid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("MTRANid");

    if (ISHOST2DEVICE (INFO_LASTASSIGN (arg_info))) {
        /*
         * e.g. a_dev = host2device(a_host);
         * If the a search of a_host in the table hits, it means
         * there's a corresponding a_host = device2host(b_dev);
         * executed before. In this case, insert pair a_dev->b_dev
         * into the table. Thereafter, each reference to a_dev will
         * be replaced as b_dev.
         * However, if the search doesn't hit, insert a_host->a_dev
         * into the table. Therefore a subsequent transfer such as
         * c_dev = host2device(a_host) will cause the pair c_dev->a_dev
         * being inserted into the table.
         */
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        if (avis != ID_AVIS (arg_node) && NODE_TYPE (avis) != N_empty) {
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)),
                                   avis);
            /* If the host var is transferred from a device var,
             * the current host2device can be removed.
             */
            INFO_REMOVABLE (arg_info) = TRUE;
            nochange = FALSE;
        } else if (avis == ID_AVIS (arg_node) && NODE_TYPE (avis) != N_empty) {
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), ID_AVIS (arg_node),
                                   IDS_AVIS (INFO_LETIDS (arg_info)));
        }
    } else if (ISDEVICE2HOST (INFO_LASTASSIGN (arg_info))) {
        /* Insert the pair var_host -> var_device */
        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LETIDS (arg_info)),
                               ID_AVIS (arg_node));
    } else {
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        if (avis != ID_AVIS (arg_node) && NODE_TYPE (avis) != N_empty) {
            if (INFO_IN_CUDA_WL (arg_info)) {
                /* If the ID is in a CUDA Withloop, we replace its AVIS with
                 * the AVIS of the corresponding device variable AVIS.
                 */
                ID_AVIS (arg_node) = avis;
            } else {
                /* if their types are equal, i.e. they are all of device tyoe */
                if (TYeqTypes (AVIS_TYPE (avis), AVIS_TYPE (ID_AVIS (arg_node)))) {
                    ID_AVIS (arg_node) = avis;
                } else {
                    /* If the ID occurs in code other than a CUDA Withloop, we
                     * insert a pair avis -> N_empty to indicate that the following
                     * host2device primitive cannot be removed.
                     */
                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, TBmakeEmpty ());
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

node *
MSTRANlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTRANassign");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
