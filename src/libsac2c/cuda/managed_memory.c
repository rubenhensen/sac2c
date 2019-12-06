/**
 * @file
 * @defgroup cumm
 *
 * @brief Unify host and device memory models for CUDA managed memory mechanism.
 *
 * The memory models of the host system and a CUDA device are different, and the
 * SaC compiler deals with this through types. In this way, appropriate code is
 * generated, using ICMs associated for a particular memory model. When using CUDA
 * managed memory, this distinction in the memory model is no longer relevent.
 *
 * This phase removes all device types, and replaces them with their equivalent host
 * type. Additionally it removes all explicit memcpy operations (host2device and
 * device2host) as managed memory does not need to use these. Instead we replace these
 * with assignments, which should lead to aliasing of arrays.
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

/**
 * INFO structure
 */
struct INFO {
    node *expr;
    node *let;
    int device; /*<< value -1 is CPU, 0 =< are GPU device ordinal */
};

/**
 * INFO macros
 */
#define INFO_EXPR(n) ((n)->expr)
#define INFO_DEVICE(n) ((n)->device)

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
    INFO_DEVICE (result) = -1;

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
 * translates device type (from simpletype) to host type
 *
 * @param type The new type that *is* a device type
 * @return an updated instances of type
 */
static ntype *
devtype2hosttype (ntype *type)
{
    ntype *scl_type;
    simpletype simp;

    DBUG_ENTER ();

    scl_type = TYgetScalar (type);
    simp = CUd2hSimpleTypeConversion (TYgetSimpleType (scl_type));
    scl_type = TYsetSimpleType (scl_type, simp);

    DBUG_RETURN (type);
}

/**
 *
 */
node *
CUMMid (node *arg_node, info *arg_info)
{
    ntype *id_type;

    DBUG_ENTER ();

    id_type = ID_NTYPE (arg_node);

    if (CUisDeviceTypeNew (id_type))
        ID_NTYPE (arg_node) = devtype2hosttype (id_type);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUMMids (node *arg_node, info *arg_info)
{
    ntype *ids_type;

    DBUG_ENTER ();

    ids_type = IDS_NTYPE (arg_node);

    if (CUisDeviceTypeNew (ids_type))
        IDS_NTYPE (arg_node) = devtype2hosttype (ids_type);

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUMMlet (node *arg_node, info *arg_info)
{
    node *expr;

    DBUG_ENTER ();

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_EXPR (arg_info) != NULL)
    {
        /* change current let to be a simple assignment */
        expr = DUPdoDupTree (INFO_EXPR (arg_info));
        INFO_EXPR (arg_info) = NULL;
        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = expr;

        if (STReq (global.config.cuda_alloc, "cumanp"))
        {
            if (global.cuda_arch < CUDA_SM60)
            {
                CTIwarn ("Compiling for CC < 6.0 (Pascal), CUDA prefetching disabled.");
            } else {
                /* create prefetch prf */
                LET_EXPR (arg_node) = TCmakePrf2 (F_cudamemprefetch,
                                                  expr,
                                                  TBmakeNum (INFO_DEVICE (arg_info)));
            }
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

    INFO_DEVICE (arg_info) = 0; // GPU

    switch (PRF_PRF (arg_node)) {
    case F_device2host:
        INFO_DEVICE (arg_info) = -1; // CPU
        /* fall-through */
    case F_host2device:
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
