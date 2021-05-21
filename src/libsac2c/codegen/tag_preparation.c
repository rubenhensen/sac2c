/*****************************************************************************
 *
 * file:   tag_preparation.c
 *
 * prefix: TP
 *
 * description:
 *
 *   This module add any tag information to the ntype that may not
 *   have been added to the ntype yet.
 *
 *
 *****************************************************************************/

#define DBUG_PREFIX "TP"
#include "debug.h"

#include "tag_preparation.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "node_basic.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "type_utils.h"

/**
 * INFO structure
 */
struct INFO {
    bool thread; /* In a thread function */
};

/**
 * INFO macros
 */
#define INFO_THREAD(n) ((n)->thread)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_THREAD (result) = FALSE;

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
 * @fn node *TParg( node *arg_node, node *arg_info)
 *
 *   @brief Arguments must be tagged as params
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TParg (node *arg_node, info *arg_info)
{
    ntype *type;

    DBUG_ENTER ();

    type = AVIS_TYPE (ARG_AVIS (arg_node));
    DBUG_ASSERT (type != NULL, "missing ntype information");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_THREAD (arg_info)) {
        type = TYsetMutcUsage (type, MUTC_US_THREADPARAM);
    } else {
        type = TYsetMutcUsage (type, MUTC_US_FUNPARAM);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TPfundef( node *arg_node, node *arg_info)
 *
 *   @brief Find inout parameters
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TPfundef (node *arg_node, info *arg_info)
{
    argtab_t *argtab;
    size_t i;

    DBUG_ENTER ();

    DBUG_PRINT ("taging %s function: %s", FUNDEF_ISTHREADFUN (arg_node) ? "thread" : "",
                FUNDEF_NAME (arg_node));

    INFO_THREAD (arg_info) = FUNDEF_ISTHREADFUN (arg_node);
    arg_node = TRAVcont (arg_node, arg_info);

    argtab = FUNDEF_ARGTAB (arg_node);
    for (i = 1; i < argtab->size; i++) {
        if (argtab->tag[i] == ATG_inout) {
            if (FUNDEF_ISTHREADFUN (arg_node)) {
                TYsetMutcUsage (AVIS_TYPE (ARG_AVIS (argtab->ptr_in[i])),
                                MUTC_US_THREADPARAMIO);
            } else {
                TYsetMutcUsage (AVIS_TYPE (ARG_AVIS (argtab->ptr_in[i])),
                                MUTC_US_FUNPARAMIO);
            }
        }
        if (argtab->tag[i] == ATG_out) {
            if (FUNDEF_ISTHREADFUN (arg_node)) {
                TYsetMutcUsage (RET_TYPE (argtab->ptr_out[i]), MUTC_US_THREADPARAM);
            } else {
                TYsetMutcUsage (RET_TYPE (argtab->ptr_out[i]), MUTC_US_FUNPARAM);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TPdoTagPreparation( node *arg_node)
 *
 *   @brief Add information to the ntype needed for tags.
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TPdoTagPreparation (node *syntax_tree)
{
    info *arg_info;

    DBUG_ENTER ();

    TRAVpush (TR_tp);

    arg_info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    arg_info = FreeInfo (arg_info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
