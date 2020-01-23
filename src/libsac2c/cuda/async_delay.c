/**
 * @file
 * @defgroup cuad
 *
 * @brief Traversal to replace all H2D/D2H CUDA primitives with explicit *_start and
 *        *_stop primitives. In this way we can introduce synchronise into the code,
 *        while also maintaining the dataflow map.
 *
 * When using one of CUDA's asynchronous transfer mechanism (such as bog-standard async)
 * the implicit synchronicity between host computation and device computation is lost.
 * Now host computation and device computation and occur at the same time, without regard
 * for the state of one or the other.
 *
 * Concretely, let us consider a simple example:
 *
 * ~~~
 * a = genarray ([1000], 8);
 * a_dev = h2d (a);
 * ... // some more work
 * kernel_one <<<...>>> (a_dev); // is always asynchronous
 * ... // some other work
 * b = d2h (a_dev); // here we implicitly synchronise on the kernel completing
 * ... // some more work on b
 * ~~~
 *
 * Here we create some array, transfer it to the CUDA device, run some kernel, and then
 * transfer the result back, storing the array in `b`. In the standard CUDA case (synchronous),
 * each host operation occurs in sequence, one after another. This is not only at the level of
 * the code, but of the operation on the device as well. Each host-based call to CUDA in this
 * example is added to a queue (which in CUDA is called a _stream_), which the CUDA driver will
 * implicitly synchronise on one-by-one. Kernel launches are a special case, as on the host
 * these calls are non-blocking (the host will not wait on the operation to finish) but are
 * synchronised in the CUDA stream. So even though, when we consider the kernel launch, does
 * not block, we are saved by the CUDA stream synchronising --- as such are D2H operation is
 * atomic.
 *
 * Now lets consider the asynchronous case, using the same example as above, which is identical
 * except for the H2D and D2H now being non-blocking as well. Here, when perform the H2D, we
 * return immediately and continue on to do more work before launching the kernel. Assuming that
 * this work is unrelated to array `a` (performs no modification), everything is all right. If
 * on the other hand we modify `a`, we might cause corrupted date to reach the CUDA device if
 * the transfer is not yet complete! Likewise, if after D2H we were to start accessing `b` we
 * might get corrupt data back (or reference an incorrect address).
 *
 * In hand-written codes, overcoming this problem of race-conditions is straightforward. Either
 * the programmer structures their such that these types of race-conditions can't form, or they
 * place synchronisation calls (like `cudaDeviceSynchronize()`). With generated-code this is not
 * so straightforward, in particular when dealing with a dataflow.
 *
 * We try to solve this with this traversal (and CUADE, @see cuade), which introduces alternative
 * primitives for H2D/D2H operations. We build on the following idea: we need to explicitly
 * synchronise our calls, but want to be careful where and when we do this, as this can have
 * significantly negative effects on performance. So we wish to avoid having to synchronise when
 * possible, but given our limited knowledge on the computation/bandwidth/etc. factors which
 * can affect how fast a transfer can occur, we can only make a guess as when to do this (or
 * not). As such, we create _windows_, encoding these into the dataflow as _start_ and _stop_
 * primitives which set guarantees on when the operation *must* be done by.
 *
 * Using our example from above, we will show what step we take:
 *
 * We create four new primitives:
 *  `host2device_start`, `host2device_end`, `device2host_start`, `device2host_end`.
 *
 * These are analogous to our H2D/D2H primitives, just that we implement new ICMs for them which
 * i. perform the transfer in the *_start case,
 * ii. perform a synchronisation in the *_end case.
 * See the runtime/saclib for further details. Also @see compile.c for code-generation details.
 *
 * When we encounter a H2D/D2H, we replace these with the new primitives, and create some
 * new temporary variables (more below):
 *
 * ~~~
 * a = genarray ([1000], 8);
 * a_tmp = h2d_start (a);
 * a_dev = h2d_start (a_tmp, a);
 * ... // some more work
 * kernel_one <<<...>>> (a_dev);
 * ... // some other work
 * b_tmp = d2h_start (a_dev);
 * b = d2h_end (b_tmp, a_dev);
 * ... // some more work on b
 * ~~~
 *
 * Notice that our *_start primitive is similar to a plain H2D/D2H primitives, the difference
 * is that we return a new N_avis which is our temporary variable. Our *_end primitives are bit
 * more complicated. As we must deal with protecting our arrays from being modified (including
 * freeing!), we need to pass the reference around in the dataflow, to thereby guarantee that
 * any modification happens after the *_end. We not only carry the first argument from
 * *_start, but also the new temporary variable. In this way we know that our temporary variable
 * *and* the other array will not be freed before this point.
 *
 * This traversal only deals with replacing the primitives, not with moving them around.
 * @see cuade for more details.
 *
 * @ingroup cuda
 *
 * @{
 */

#include "async_delay.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"
#include "ctinfo.h"
#include "print.h"
#include "globals.h"

#define DBUG_PREFIX "CUAD"
#include "debug.h"

#include "cuda_utils.h"
#include "new_types.h"
#include "DupTree.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *lhs; /*<< LHS of N_let */
    node *nlhs; /*<< New LHS of N_let which is added/replaced */
    node *prfs; /*<< One of the new *_start primitives */
    node *prfe; /*<< One of the new *_end primitives */
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NLHS(n) ((n)->nlhs)
#define INFO_PRFS(n) ((n)->prfs)
#define INFO_PRFE(n) ((n)->prfe)

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

    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NLHS (result) = NULL;
    INFO_PRFS (result) = NULL;
    INFO_PRFE (result) = NULL;

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
CUADfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADassign (node *arg_node, info *arg_info)
{
    node *new;

    DBUG_ENTER ();

    /* top-down */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PRFE (arg_info) != NULL)
    {

        new = TBmakeAssign (TBmakeLet (INFO_LHS (arg_info),
                    INFO_PRFE (arg_info)),
                ASSIGN_NEXT (arg_node));
        AVIS_SSAASSIGN (IDS_AVIS (INFO_LHS (arg_info))) = new;
        ASSIGN_NEXT (arg_node) = new;
        AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node)))) = arg_node;
        INFO_PRFE (arg_info) = NULL;
        INFO_LHS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_PRFS (arg_info) != NULL)
    {
        LET_IDS (arg_node) = INFO_NLHS (arg_info);
        INFO_NLHS (arg_info) = NULL;
        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = INFO_PRFS (arg_info);
        INFO_PRFS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADprf (node *arg_node, info *arg_info)
{
    node *navis, *nvd;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_host2device:

        DBUG_PRINT ("found cuda memcpy, changing N_prf");
        if (AVIS_ISALLOCLIFT (ID_AVIS (PRF_ARG1 (arg_node))))
        {
            DBUG_PRINT ("...not changing as is EMRL allocated!");
            break;
        }
        /* fall-through */

    case F_device2host:

        navis = TBmakeAvis (TRAVtmpVarName (PRF_PRF (arg_node) == F_host2device
                                            ? "dev" : "host"),
                            TYcopyType (IDS_NTYPE (INFO_LHS (arg_info))));
        nvd = TBmakeVardec (navis, NULL);
        AVIS_DECL (navis) = nvd;
        INFO_NLHS (arg_info) = TBmakeIds (navis, NULL);
        INFO_PRFS (arg_info) = TCmakePrf1 (PRF_PRF (arg_node) == F_host2device
                                           ? F_host2device_start
                                           : F_device2host_start,
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        INFO_PRFE (arg_info) = TCmakePrf2 (PRF_PRF (arg_node) == F_host2device
                                           ? F_host2device_end
                                           : F_device2host_end,
                                           TBmakeId (navis),
                                           DUPdoDupNode (PRF_ARG1 (arg_node)));
        INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), nvd);
        break;

    default:

        // do nothing
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 *
 */
node *
CUADdoAsyncDelay (node *syntax_tree)
{
    info * arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_cuad);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** @} */
#undef DBUG_PREFIX
