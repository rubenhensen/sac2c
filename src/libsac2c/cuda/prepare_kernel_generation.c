
#include "prepare_kernel_generation.h"

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
    node *fundef;
    node *preassign;
    bool incudawl;
    node *alloc_assigns;
    node *free_assigns;
    lut_t *lut;
    node *lhs;
    node *lastassign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGN(n) (n->preassign)
#define INFO_INCUDAWL(n) (n->incudawl)
#define INFO_ALLOC_ASSIGNS(n) (n->alloc_assigns)
#define INFO_FREE_ASSIGNS(n) (n->free_assigns)
#define INFO_LUT(n) (n->lut)
#define INFO_LHS(n) (n->lhs)
#define INFO_LASTASSIGN(n) (n->lastassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_ALLOC_ASSIGNS (result) = NULL;
    INFO_FREE_ASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;

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
PKNLGdoPrepareKernelGeneration (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_pknlg);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PKNLGfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PKNLGassign (node *arg_node, info *arg_info)
{
    node *next;

    DBUG_ENTER ();

    INFO_LASTASSIGN (arg_info) = arg_node;
    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (!INFO_INCUDAWL (arg_info)) {
        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_FREE_ASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (arg_node, INFO_FREE_ASSIGNS (arg_info));
            INFO_FREE_ASSIGNS (arg_info) = NULL;
        }

        if (INFO_ALLOC_ASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_ALLOC_ASSIGNS (arg_info), arg_node);
            INFO_ALLOC_ASSIGNS (arg_info) = NULL;
        }

        node *last_assign = arg_node;
        while (ASSIGN_NEXT (last_assign) != NULL) {
            last_assign = ASSIGN_NEXT (last_assign);
        }

        ASSIGN_NEXT (last_assign) = next;
        ASSIGN_NEXT (last_assign) = TRAVopt (ASSIGN_NEXT (last_assign), arg_info);

    } else {
        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_PREASSIGN (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
            INFO_PREASSIGN (arg_info) = NULL;
        }

        node *last_assign = arg_node;
        while (ASSIGN_NEXT (last_assign) != NULL) {
            last_assign = ASSIGN_NEXT (last_assign);
        }

        ASSIGN_NEXT (last_assign) = next;
        ASSIGN_NEXT (last_assign) = TRAVopt (ASSIGN_NEXT (last_assign), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PKNLGlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PKNLGwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_LUT (arg_info) = LUTgenerateLut ();

        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
        INFO_INCUDAWL (arg_info) = FALSE;

        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    } else if (INFO_INCUDAWL (arg_info)) {
        WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    } else {
        /* Not cudarizable, not in cuda withloop, ignore */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/

node *
PKNLGwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH2_WITHOP (arg_node) = TRAVopt (WITH2_WITHOP (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVopt (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PKNLGgenarray (node *arg_node, info *arg_info)
{
    node *alloc_assign, *avis;
    ntype *scalar_type;
    simpletype sty;
    node *modarray;

    DBUG_ENTER ();

    avis = ID_AVIS (GENARRAY_MEM (arg_node));
    alloc_assign = LUTsearchInLutPp (INFO_LUT (arg_info), avis);

    if (alloc_assign != avis) {
        scalar_type = TYgetScalar (AVIS_TYPE (avis));
        sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
        scalar_type = TYsetSimpleType (scalar_type, sty);
        INFO_ALLOC_ASSIGNS (arg_info)
          = TCappendAssign (DUPdoDupNode (alloc_assign), INFO_ALLOC_ASSIGNS (arg_info));
        INFO_FREE_ASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (NULL,
                                     TBmakePrf (F_free,
                                                TBmakeExprs (TBmakeId (avis), NULL))),
                          INFO_FREE_ASSIGNS (arg_info));

        /* Change the orginal allocation into a F_noop. A better
         * solution would be to remove the assign completely. However, since
         * we are tracing back to the allocation, it's difficult to remove
         * it here. */
        ASSIGN_LHS (alloc_assign) = FREEdoFreeNode (ASSIGN_LHS (alloc_assign));
        ASSIGN_RHS (alloc_assign) = FREEdoFreeNode (ASSIGN_RHS (alloc_assign));
        ASSIGN_RHS (alloc_assign) = TBmakePrf (F_noop, NULL);

        /* Change the N_genarray to N_modarray. Because we are actually
         * passing an (uninitialised) array into the kernel and modify
         * it inside the kernel. It's no longer generating a new array
         * any more. */
        modarray = TBmakeModarray (TBmakeId (avis));
        MODARRAY_MEM (modarray) = TBmakeId (avis);
        MODARRAY_IDX (modarray) = GENARRAY_IDX (arg_node);
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = modarray;
    }

    /* Can there be more than one generator?? */
    // GENARRAY_NEXT( arg_node) = TRAVopt( GENARRAY_NEXT( arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PKNLGprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *
 *****************************************************************************/
node *
PKNLGprf (node *arg_node, info *arg_info)
{
    node *id, *avis;

    DBUG_ENTER ();

    if (INFO_INCUDAWL (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_sel_VxA:
            id = PRF_ARG2 (arg_node);
            DBUG_ASSERT (NODE_TYPE (id) == N_id, "2nd arg of F_sel_VxA is no N_id!");

            avis = ID_AVIS (id);

            if (TYisAKV (AVIS_TYPE (avis))) {
                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (id), NULL),
                                             COconstant2AST (
                                               TYgetValue (AVIS_TYPE (avis)))),
                                  NULL);
            }
            break;
        case F_alloc:
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                                   INFO_LASTASSIGN (arg_info));
            break;
        case F_cond_wl_assign:
            EXPRS_EXPRS6 (PRF_ARGS (arg_node))
              = FREEdoFreeTree (EXPRS_EXPRS6 (PRF_ARGS (arg_node)));
            EXPRS_NEXT (EXPRS_EXPRS5 (PRF_ARGS (arg_node))) = NULL;
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
