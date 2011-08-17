/*****************************************************************************
 *
 * $Id$
 *
 * file:   restore_mem_instr.c
 *
 * prefix: MTRMI
 *
 * description:
 *
 *   The traversal restores memory allocation and de-allocation instructions
 *   for with-loop index variables of those with-loops that either reside
 *   in an SPMD function or in the sequential branch of a dynamically
 *   parallelised with-loop. This step becomes necessary as the original
 *   memory management instructions inserted in the mem phase are now in
 *   the wrong block or even in the wrong functions. MTDCR takes care of
 *   those and removes them.
 *
 *****************************************************************************/

#include "restore_mem_instr.h"

#define DBUG_PREFIX "MTRMI"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "memory.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

/**
 * INFO structure
 */

struct INFO {
    node *fundef;
    node *allocassigns;
    node *freeassigns;
    bool allocneeded;
    bool restore;
};

/**
 * INFO macros
 */

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ALLOCASSIGNS(n) ((n)->allocassigns)
#define INFO_FREEASSIGNS(n) ((n)->freeassigns)
#define INFO_ALLOCNEEDED(n) ((n)->allocneeded)
#define INFO_RESTORE(n) ((n)->restore)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
    INFO_ALLOCNEEDED (result) = FALSE;
    INFO_RESTORE (result) = FALSE;

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
 * @fn node *MTRMImodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTRMImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTRMIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTRMIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("Entering function %s.", FUNDEF_NAME (arg_node));

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTRMIblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTRMIblock (node *arg_node, info *arg_info)
{
    bool restore;

    DBUG_ENTER ();

    restore = INFO_RESTORE (arg_info);
    INFO_RESTORE (arg_info)
      = BLOCK_ISMTSEQUENTIALBRANCH (arg_node) || BLOCK_ISMTPARALLELBRANCH (arg_node);

    DBUG_PRINT (">> switched to restore mode: %d", INFO_RESTORE (arg_info));

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

    INFO_RESTORE (arg_info) = restore;

    DBUG_PRINT ("<< switched to restore mode: %d", INFO_RESTORE (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTRMIassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTRMIassign (node *arg_node, info *arg_info)
{
    node *alloc_assigns, *free_assigns;

    DBUG_ENTER ();

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    alloc_assigns = INFO_ALLOCASSIGNS (arg_info);
    INFO_ALLOCASSIGNS (arg_info) = NULL;

    free_assigns = INFO_FREEASSIGNS (arg_info);
    INFO_FREEASSIGNS (arg_info) = NULL;

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_EXECUTE ({
        int i = 0;
        node *tmp = alloc_assigns;
        if (tmp != NULL) {
            while (tmp != NULL) {
                tmp = ASSIGN_NEXT (tmp);
                i++;
            }
            DBUG_PRINT ("Alloc instrs found: %d", i);
        }
    });

    DBUG_EXECUTE ({
        int i = 0;
        node *tmp = free_assigns;
        if (tmp != NULL) {
            while (tmp != NULL) {
                tmp = ASSIGN_NEXT (tmp);
                i++;
            }
            DBUG_PRINT ("Free instrs found: %d", i);
        }
    });

    ASSIGN_NEXT (arg_node) = TCappendAssign (free_assigns, ASSIGN_NEXT (arg_node));
    arg_node = TCappendAssign (alloc_assigns, arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTRMIwith2( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTRMIwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTRMIwithid( node *arg_node, info *arg_info)
 *
 *    @brief traversal function for N_withid node
 *      We memoise the fact that we now traverse a withid to do the right things
 *      when encountering id and ids nodes.
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return arg_node
 *
 ******************************************************************************/

node *
MTRMIwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ALLOCNEEDED (arg_info) = TRUE;

    DBUG_PRINT ("withids found");

    if (WITHID_VECNEEDED (arg_node)) {
        WITHID_VEC (arg_node) = TRAVopt (WITHID_VEC (arg_node), arg_info);
    }

    WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
    WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);

    INFO_ALLOCNEEDED (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTRMIid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTRMIid (node *arg_node, info *arg_info)
{
    node *dim, *shape, *alloc, *ids, *free, *avis;

    DBUG_ENTER ();

    if (INFO_ALLOCNEEDED (arg_info) && INFO_RESTORE (arg_info)) {
        DBUG_PRINT ("Creating alloc/free for %s", ID_NAME (arg_node));

        avis = ID_AVIS (arg_node);

        if (TUdimKnown (AVIS_TYPE (avis))) {
            dim = TBmakeNum (TYgetDim (AVIS_TYPE (avis)));
        } else {
            dim = NULL;
        }

        if (TUshapeKnown (AVIS_TYPE (avis))) {
            shape = SHshape2Array (TYgetShape (AVIS_TYPE (avis)));
        } else {
            shape = NULL;
        }

        alloc = TCmakePrf3 (F_alloc, TBmakeNum (1), dim, shape);

        ids = TBmakeIds (avis, NULL);

        INFO_ALLOCASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (ids, alloc), INFO_ALLOCASSIGNS (arg_info));

        free = TCmakePrf1 (F_free, TBmakeId (avis));
        INFO_FREEASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (NULL, free), INFO_FREEASSIGNS (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTRMIdoRestoreMemoryInstrs( node *syntax_tree)
 *
 *  @brief initiates traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTRMIdoRestoreMemoryInstrs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtrmi);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
