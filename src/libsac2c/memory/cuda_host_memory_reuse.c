/**
 * @file
 * @defgroup chmr
 * @ingroup mm
 *
 *
 * @{
 */
#include "cuda_host_memory_reuse.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"

#define DBUG_PREFIX "CHMR"
#include "debug.h"

#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"

/*
 * INFO structure
 */
struct INFO {
    node *prfalloc;
    node *fillmem;
    /* data masks */
    dfmask_t *h2d_avis; /**< avis found from h2d */
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PRFALLOC(n) ((n)->prfalloc)
#define INFO_FILLMEM(n) ((n)->fillmem)
#define INFO_H2D_AVIS(n) ((n)->h2d_avis)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_PRFALLOC (result) = NULL;
    INFO_FILLMEM (result) = NULL;
    INFO_H2D_AVIS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
CHMRfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t * basemask;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        basemask = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));
        INFO_H2D_AVIS (arg_info) = DFMgenMaskClear (basemask);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_H2D_AVIS (arg_info) = DFMremoveMask (INFO_H2D_AVIS (arg_info));
        basemask = DFMremoveMaskBase (basemask);

        INFO_PRFALLOC (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
Mask2Exprs (dfmask_t *mask, ntype *m)
{
    node *id, *avis, *exprs = NULL;

    DBUG_ENTER ();

    avis = DFMgetMaskEntryAvisSet (mask);
    while (avis != NULL) {
        /* we only take the first matching avis */
        if (TYeqTypes (m, AVIS_TYPE (avis))) {
            id = TBmakeId (avis);
            exprs = TBmakeExprs (id, exprs);

            DFMsetMaskEntryClear (mask, NULL, avis);

            break;
        } else {
            avis = DFMgetMaskEntryAvisSet (NULL);
        }
    }

    DBUG_RETURN (exprs);
}

/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
CHMRprf (node *arg_node, info *arg_info)
{
    node *exprs, *alloc, *args;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_host2device:
        /* capture host memory avis */
        DBUG_PRINT ("adding avis at h2d to mask");
        DFMsetMaskEntrySet (INFO_H2D_AVIS (arg_info), NULL, ID_AVIS (PRF_ARG1 (arg_node)));
        break;

    case F_alloc:
        /* XXX is there always an alloc before an device2host? */
        INFO_PRFALLOC (arg_info) = arg_node;
        break;

    case F_device2host:
        /* check if decrc_avis contains avis that matches shape (dim?)
         * of host memory avis. If so, replace alloc with alloc_or_reuse
         * and append matching decrc_avis avis */
        exprs = Mask2Exprs (INFO_H2D_AVIS (arg_info), ID_NTYPE (INFO_FILLMEM (arg_info)));

        if (exprs != NULL) {
            DBUG_PRINT ("replacing F_alloc with F_alloc_or_reuse...");
            alloc = INFO_PRFALLOC (arg_info);
            args = TCcombineExprs (DUPdoDupTree (PRF_ARGS (alloc)), exprs);

            PRF_PRF (alloc) = F_alloc_or_reuse;
            PRF_ARGS (alloc) = FREEdoFreeTree (PRF_ARGS (alloc));
            PRF_ARGS (alloc) = args;
        }

        break;

    case F_fill:
        INFO_FILLMEM (arg_info) = PRF_ARG2 (arg_node);
        /* fall-through */

    default:
        /* traverse prf arguments - such as fill() */
        PRF_ARG1 (arg_node) = TRAVdo (PRF_ARG1 (arg_node), arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}


/**
 * @brief
 *
 * @param arg_node
 * @param arg_info
 * @return
 */
node *
CHMRdoMemoryReuse (node *syntax_tree)
{
    info * arg_info;
    DBUG_ENTER ();

    if (STReq (global.config.cuda_alloc, "cuman")
        || STReq (global.config.cuda_alloc, "cumanp")) {
        CTIwarn ("disabling CHMR optimisation, as this conflicts with CUDA managed memory.");
    } else {
        DBUG_PRINT ("Starting CUDA host memory reuse");

        arg_info = MakeInfo ();

        TRAVpush (TR_chmr);
        syntax_tree = TRAVdo (syntax_tree, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("CUDA host memory reuse complete");
    }

    DBUG_RETURN (syntax_tree);
}

/* @} */

#undef DBUG_PREFIX
