/** <!--********************************************************************-->
 *
 * @file cuda_sink_code.c
 *
 *
 *****************************************************************************/

#include "cuda_sink_code.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "free.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "globals.h"
#include "type_utils.h"
#include "LookUpTable.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "deadcoderemoval.h"

typedef enum { trav_normal, trav_backtrace } travmode_t;

/*
 * INFO structure
 */
struct INFO {
    node *current_block;
    node *sink_code; /* N_assign chain containing the sunk code */
    bool incudawl;
    travmode_t travmode;
    lut_t *lut;
    node *dupassign;
    node *oriassign;
    node *fundef;
    nodelist *nlist;
};

#define INFO_CURRENT_BLOCK(n) (n->current_block)
#define INFO_SINK_CODE(n) (n->sink_code)
#define INFO_INCUDAWL(n) (n->incudawl)
#define INFO_TRAVMODE(n) (n->travmode)
#define INFO_LUT(n) (n->lut)
#define INFO_DUPASSIGN(n) (n->dupassign)
#define INFO_ORIASSIGN(n) (n->oriassign)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_NLIST(n) (n->nlist)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CURRENT_BLOCK (result) = NULL;
    INFO_SINK_CODE (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_TRAVMODE (result) = trav_normal;
    INFO_LUT (result) = NULL;
    INFO_DUPASSIGN (result) = NULL;
    INFO_ORIASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_NLIST (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCdoSinkCode(node *arg_node, info)
 *
 *   @brief  Inits the traversal for this phase
 *
 *   @param arg_node a N_Module
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCdoSinkCode (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cuskc);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCfundef(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCfundef (node *arg_node, info *arg_info)
{
    nodelist *nl;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    /* Tag the execution mode of all sunk N_assigns to CUDA_HOST_SINGLE.
     * We cannot do it when we sink the code because the same N_assign
     * might be sunk to different blocks and if one sinking change the
     * execution mode to CUDA_HOST_SINGLE, it might prevent later sinking
     * as code sinking need to check the execution mode to ensure that
     * it's CUDA_DEVICE_SINGLE. */
    nl = INFO_NLIST (arg_info);
    while (nl != NULL) {
        DBUG_ASSERT (NODE_TYPE (NODELIST_NODE (nl)) == N_assign,
                     "Non N_assign node found in nodelist!");

        // ASSIGN_EXECMODE( NODELIST_NODE( nl)) = CUDA_HOST_SINGLE;
        nl = NODELIST_NEXT (nl);
    }
    INFO_NLIST (arg_info) = NULL;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCblock(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCblock (node *arg_node, info *arg_info)
{
    node *old_current_block, *old_sink_code;
    lut_t *old_lut;

    DBUG_ENTER ();

    /* Push info */
    old_current_block = INFO_CURRENT_BLOCK (arg_info);
    old_sink_code = INFO_SINK_CODE (arg_info);
    old_lut = INFO_LUT (arg_info);

    INFO_CURRENT_BLOCK (arg_info) = arg_node;
    INFO_SINK_CODE (arg_info) = NULL;
    INFO_LUT (arg_info) = LUTgenerateLut ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    /* Pop info */
    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    INFO_CURRENT_BLOCK (arg_info) = old_current_block;
    INFO_LUT (arg_info) = old_lut;

    if (INFO_SINK_CODE (arg_info) != NULL) {
        /* Preppend newly sunk code to the beginning of the current block */
        BLOCK_ASSIGNS (arg_node)
          = TCappendAssign (INFO_SINK_CODE (arg_info), BLOCK_ASSIGNS (arg_node));
    }

    INFO_SINK_CODE (arg_info) = old_sink_code;
    /* INFO_TRAVMODE( arg_info) = trav_normal; */

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCwith(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* Only look at cudarziable N_with and any N_withs nested inside,
     * since we only sink code into these N_withs */
    if (WITH_CUDARIZABLE (arg_node)) {
        /* Note that since only outermost N_withs are considered to
         * be cudarizable, hence we are sure that this N_with is the
         * outermost one */
        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INCUDAWL (arg_info) = FALSE;
    } else if (INFO_INCUDAWL (arg_info)) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    } else {
        /* Skip current withloop */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCassign(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCassign (node *arg_node, info *arg_info)
{
    node *sunk_assign, *old_dupass, *old_oriass;

    DBUG_ENTER ();

    /* If the traverse mode is normal, we perform a normal top-down traversal */
    if (INFO_TRAVMODE (arg_info) == trav_normal) {
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    } else if (INFO_TRAVMODE (arg_info) == trav_backtrace) {
        /* Orginal assign */
        old_oriass = INFO_ORIASSIGN (arg_info);
        /* Duplicate of the original assign */
        old_dupass = INFO_DUPASSIGN (arg_info);

        /* If we are in backtrace mode, we would like to sink the
         * current assign. However, result of this assign may not
         * just be used in the following cuda N_with and maybe used
         * in other context as well. Therefore, we cannot simply move
         * this assign into the N_with. Instead we need to
         * duplicate this assign and sink the duplicate. */
        sunk_assign = DUPdoDupNode (arg_node);
        ASSIGN_NEXT (sunk_assign) = NULL;
        ASSIGN_EXECMODE (sunk_assign) = (mtexecmode_t)CUDA_HOST_SINGLE;

        /* Both DUPASSIGN and ORIASSIGN will be used in CUSKCids to set
         * the SSA links correctly */
        INFO_DUPASSIGN (arg_info) = sunk_assign;
        INFO_ORIASSIGN (arg_info) = arg_node;

        /* Traverse RHS of the duplicate N_assign in case we need to further backtrace */
        ASSIGN_STMT (sunk_assign) = TRAVdo (ASSIGN_STMT (sunk_assign), arg_info);

        INFO_DUPASSIGN (arg_info) = old_dupass;
        INFO_ORIASSIGN (arg_info) = old_oriass;

        INFO_SINK_CODE (arg_info)
          = TCappendAssign (INFO_SINK_CODE (arg_info), sunk_assign);

        if (INFO_NLIST (arg_info) == NULL) {
            INFO_NLIST (arg_info) = TCnodeListAppend (NULL, arg_node, NULL);
        } else {
            INFO_NLIST (arg_info)
              = TCnodeListAppend (INFO_NLIST (arg_info), arg_node, NULL);
        }
    } else {
        DBUG_ASSERT (0, "Unknown traverse mode!");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKClet(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    /* Only traverse N_ids if we are in backtrace mode */
    if (INFO_TRAVMODE (arg_info) == trav_backtrace) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCids(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCids (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ();

    /* Ensure that we are in backtrace mode */
    DBUG_ASSERT (INFO_TRAVMODE (arg_info) == trav_backtrace,
                 "Traversing N_ids in non-backtrace mode!");

    avis = IDS_AVIS (arg_node);

    /* We don't want the SSA of the N_ids to be the newly created N_assign.
     * However, duplicating N_assign automatically sets it to the new N_assign.
     * Therefore, we need to set it back to the original N_assign. */

    /* In backtrace mode, we look at the LHS of the assign to be sunk. */
    new_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);
    if (new_avis != avis) { // && AVIS_SSAASSIGN( new_avis) == NULL) {
        /* Set the ssa assign of the old lhs to the original assign */
        AVIS_SSAASSIGN (avis) = INFO_ORIASSIGN (arg_info);
        AVIS_SSAASSIGN (new_avis) = INFO_DUPASSIGN (arg_info);
        IDS_AVIS (arg_node) = new_avis;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUSKCid(node *arg_node, info *arg_info)
 *
 *
 *   @param arg_node
 *   @param arg_info
 *   @return
 *
 *****************************************************************************/
node *
CUSKCid (node *arg_node, info *arg_info)
{
    node *ssa, *avis, *new_avis;
    travmode_t old_mode;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);
    ssa = AVIS_SSAASSIGN (avis);

    /* We only start backtracing in the following two cases:
     *   1) The current traverse mode is backtrace, OR
     *   2) The N_id is within a cuda withloop */
    if (INFO_TRAVMODE (arg_info) == trav_backtrace || INFO_INCUDAWL (arg_info)) {
        /* If the N_id is scalar (we only sink scalar operaions) and
         * its ssaassign is tagged as CUDA_DEVICE_SINGLE */
        if (TUisScalar (AVIS_TYPE (avis)) && ssa != NULL
            && ASSIGN_EXECMODE (ssa) == (mtexecmode_t)CUDA_DEVICE_SINGLE) {
            new_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);
            /*
             * What this code does can be explained by the
             * following example:
             * e.g.
             *
             *    a = _idx_sel_( arr, idx); ( CUDA_DEVICE_SINGLE N_assign)
             *    ...
             *    b = with {
             *          ... = ..a_..;
             *          ...
             *          ... = ..a..;
             *         }:genarray();         ( Cudarizable N_with)
             *
             *    ==>
             *
             *
             *    a = _idx_sel_( arr, idx);  ( CUDA_DEVICE_SINGLE N_assign)
             *    ...
             *    b = with {
             *          ... = ..a_cuskc..;
             *          ...
             *          ... = ..a_cuskc..;
             *         }:genarray();         ( CudaCUSKCidsrizable N_with)
             *
             * Furthermore, the code insert pair a->a_cuskc into the lookup
             * table so that later encounter of "a" can be replace by a_cuskc.
             *
             */
            if (new_avis == avis) {
                /* Create new name for new variable */
                new_avis = DUPdoDupNode (avis);
                AVIS_NAME (new_avis) = MEMfree (AVIS_NAME (new_avis));
                AVIS_NAME (new_avis) = TRAVtmpVar ();

                /* Create new vardec */
                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
                AVIS_DECL (new_avis) = FUNDEF_VARDECS (INFO_FUNDEF (arg_info));

                /* Set the ssaassign to NULL for now. It will be correctly
                 * set to the sunk N_assign in CUSKCids */
                AVIS_SSAASSIGN (new_avis) = NULL;
                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
                /* Set new avis to this N_id node */
                ID_AVIS (arg_node) = new_avis;

                old_mode = INFO_TRAVMODE (arg_info);
                INFO_TRAVMODE (arg_info) = trav_backtrace;
                /* Start backtracing */
                ssa = TRAVdo (ssa, arg_info);
                INFO_TRAVMODE (arg_info) = old_mode;
            } else {
                /* If the N_id has been come across before, simple set
                 * its avis to the new avis */
                ID_AVIS (arg_node) = new_avis;
            }
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
