/** <!--********************************************************************-->
 *
 * @defgroup ec Explicit Copy
 *
 * Makes the conceptual data copying in modarray and idx_modarray explicit.
 * Thus the arguments are protected from being destructively updated illegaly.
 * The copy overhead can hopefully be avoided at a later stage.
 *
 * <pre>
 *   a = modarray( b, iv, val);
 * </pre>
 *   becomes
 * <pre>
 *   b'= copy( b);
 *   a = modarray( b', iv, val);
 * </pre>
 *
 * @ingroup mm
 *
 * @{
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file explicitcopy.c
 *
 * Prefix: EMEC
 *
 *****************************************************************************/
#include "explicitcopy.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "EMEC"
#include "debug.h"

#include "print.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "cuda_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *preassign;
    node *fundef;
};

/**
 * A list of N_assign nodes of the form b' = copy(b) that will be prepended
 * to the an assignment in EMECassign().
 */
#define INFO_PREASSIGN(n) (n->preassign)

/**
 * Points to the currently traversed N_fundef node.
 */
#define INFO_FUNDEF(n) (n->fundef)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PREASSIGN (result) = NULL;
    INFO_FUNDEF (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMECdoExplicitCopy( node *syntax_tree)
 *
 *****************************************************************************/
node *
EMECdoExplicitCopy (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting explicit copy traversal.");

    info = MakeInfo ();

    TRAVpush (TR_emec);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Explicit copy traversal complete.");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CreateCopyId(node *arg_node, info *arg_info)
 *
 * @brief CreateCopyId( a, arg_info) creates a new variable a' in INFO_FUNDEF,
          appends INFO_PREASSIGN with a' = copy( a); and returns a'.
 *
 *****************************************************************************/
static node *
CreateCopyId (node *oldid, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    /*
     * Create a new variable for b'
     */
    avis = TBmakeAvis (TRAVtmpVarName (ID_NAME (oldid)),
                       TYcopyType (AVIS_TYPE (ID_AVIS (oldid))));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    /*
     * Create copy operation
     */
    INFO_PREASSIGN (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), TCmakePrf1 (F_copy, oldid)),
                      INFO_PREASSIGN (arg_info));
    AVIS_SSAASSIGN (avis) = INFO_PREASSIGN (arg_info);

    /*
     * Replace oldid with newid
     */
    oldid = TBmakeId (avis);

    DBUG_RETURN (oldid);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Explicit copy traversal functions (emec_tab)
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMECassign(node *arg_node, info *arg_info)
 *
 * @brief performs a bottom-up traversal and prepends nodes stored
 *        in INFO_PREASSIGN( arg_info) to the current node.
 *
 *****************************************************************************/
node *
EMECassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECfundef(node *arg_node, info *arg_info)
 *
 * @brief Sets INFO_FUNDEF to the current function and traverses BODY and NEXT.
 *
 *****************************************************************************/
node *
EMECfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECap( node *arg_node, info *arg_info)
 *
 * @brief Performs explicit copying for external functions applications
 *        with reference args.
 *
 *****************************************************************************/
node *
EMECap (node *arg_node, info *arg_info)
{
    node *args, *exprs;

    DBUG_ENTER ();

    exprs = AP_ARGS (arg_node);
    args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    while (args != NULL) {
        if (ARG_HASLINKSIGNINFO (args)) {
            node *rets = FUNDEF_RETS (AP_FUNDEF (arg_node));
            while (rets != NULL) {
                if ((RET_HASLINKSIGNINFO (rets))
                    && (RET_LINKSIGN (rets) == ARG_LINKSIGN (args))) {
                    EXPRS_EXPR (exprs) = CreateCopyId (EXPRS_EXPR (exprs), arg_info);
                }
                rets = RET_NEXT (rets);
            }
        }

        args = ARG_NEXT (args);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMECprf(node *arg_node, info *arg_info)
 *
 * @brief Performs explicit copying in order to protect the arguments of
 *        modarray and idx_modarray from being detructively updated.
 *
 * <pre>
 * a = modarray( b, iv, val);
 *
 * is transformed info
 *
 * b' = copy( b);
 * a  = modarray( b', iv, val);
 * </pre>
 *
 *****************************************************************************/
node *
EMECprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_modarray_AxVxS:
    case F_modarray_AxVxA:
    case F_idx_modarray_AxSxS:
    case F_idx_modarray_AxSxA:
        if (!FUNDEF_ISCUDASTGLOBALFUN (INFO_FUNDEF (arg_info))
            && !FUNDEF_ISCUDALACFUN (INFO_FUNDEF (arg_info)) &&
            /* we do not explicitly copy shared memory array */
            !CUisShmemTypeNew (ID_NTYPE (PRF_ARG1 (arg_node)))) {
            PRF_ARG1 (arg_node) = CreateCopyId (PRF_ARG1 (arg_node), arg_info);
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- EMEC traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Explicit copy -->
 *****************************************************************************/

#undef DBUG_PREFIX
