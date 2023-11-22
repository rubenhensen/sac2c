/**
 * @file
 * @defgroup cumm
 *
 * @brief Remove device types throughout for CUDA managed memory mechanism.
 *
 * When using CUDA managed memory, we implicitly make use of UVA (universal virtual
 * address space) provided by CUDA driver which makes all declared variables available
 * in any context (host or device). Through manage memory, any transfers are demand drive,
 * and occur implicitly without any explicit memcpy calls. The SaC CUDA backend normally
 * assumes that the host and device context are distinct when it comes to variables. As
 * such for most host types there are equivalent device types. When using managed memory,
 * this distinction is no longer necessary (and can actually lead to complications).
 *
 * This phase removes all device types, and replaces them with their equivalent host
 * type. Additionally it replaces all explicit transfers (host2device and
 * device2host) with assignments, as managed memory does not need to use these. This
 * should lead to aliasing of arrays.
 *
 * Furthermore, if prefetching is activated (for the 'cumanp' allocation mode), then
 * this traversal places prefhost2device, etc. in place of host2device primitives).
 *
 * Concretely, given some CUDA program with device types:
 *
 * ~~~
 * int[100] k;
 * int_dev a_dev;
 * int_dev[100] b_dev;
 *
 * ... // allocation/genarray/etc.
 *
 * b_dev = h2d (k);
 *
 * ...
 * ~~~
 *
 * becomes
 *
 * ~~~
 * int[100] k;
 * int a_dev;
 * int[100] b_dev;
 *
 * ... // allocation/genarray/etc.
 *
 * b = k;
 *
 * ...
 * ~~~
 *
 * @ingroup cuda
 *
 * @{
 */

#include "managed_memory.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"
#include "print.h"
#include "globals.h"

#define DBUG_PREFIX "CUMM"
#include "debug.h"

#include "cuda_utils.h"
#include "new_types.h"
#include "DupTree.h"
#include "str.h"

/**
 * Typedefs
 */
typedef enum { CUMM_host, CUMM_device, CUMM_unknown } cumm_transfer_e;

/**
 * INFO structure
 */
struct INFO {
    node *expr;
    node *ercs;
    cumm_transfer_e dst; /*<< indicates the destination of the transfer */
};

/**
 * INFO macros
 */
#define INFO_EXPR(n) ((n)->expr)
#define INFO_ERCS(n) ((n)->ercs)
#define INFO_DST(n) ((n)->dst)

/**
 * @name INFO functions
 *
 * @{
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_EXPR (result) = NULL;
    INFO_ERCS (result) = NULL;
    INFO_DST (result) = CUMM_unknown;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** @} */

/**
 *
 */
node *
CUMMlet (node *arg_node, info *arg_info)
{
    node *expr, *ercs = NULL;

    DBUG_ENTER ();

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_EXPR (arg_info) != NULL) {
        /* change current let to be a simple assignment */
        expr = DUPdoDupTree (INFO_EXPR (arg_info));
        if (INFO_ERCS (arg_info) != NULL) {
            ercs = DUPdoDupTree (INFO_ERCS (arg_info));
        }
        INFO_EXPR (arg_info) = NULL;
        INFO_ERCS (arg_info) = NULL;
        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = expr;

        if (STReq (global.config.cuda_alloc, "cumanp") && global.optimize.docuprf) {
            DBUG_PRINT ("Adding CUDA prefetch call...");
            LET_EXPR (arg_node) = TCmakePrf1 (INFO_DST (arg_info) == CUMM_device
                                              ? F_prefetch2device
                                              : F_prefetch2host,
                                              expr);
            PRF_ERC (LET_EXPR (arg_node)) = ercs;
        } else {
            ercs = FREEoptFreeTree(ercs);
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUMMprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    PRF_ERC (arg_node) = TRAVopt (PRF_ERC (arg_node), arg_info);

    INFO_DST (arg_info) = CUMM_device;

    switch (PRF_PRF (arg_node)) {
    case F_device2host:
        INFO_DST (arg_info) = CUMM_host;
        /* fall-through */
    case F_host2device:
        INFO_ERCS (arg_info) = PRF_ERC (arg_node);
        INFO_EXPR (arg_info) = PRF_ARG1 (arg_node);
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUMMavis (node *arg_node, info *arg_info)
{
    ntype *avis_type, *new_type;

    DBUG_ENTER ();

    avis_type = AVIS_TYPE (arg_node);

    if (CUisDeviceTypeNew (avis_type)) {
        DBUG_PRINT ("Found N_avis with device type, convert to host type: %s",
                    AVIS_NAME (arg_node));
        new_type = CUconvertDeviceToHostType (avis_type);
        /* we need to free the old type */
        avis_type = TYfreeType (avis_type);
        AVIS_TYPE (arg_node) = new_type;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUMMdoManagedMemory (node *syntax_tree)
{
    info * arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cumm);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** @} */
#undef DBUG_PREFIX
