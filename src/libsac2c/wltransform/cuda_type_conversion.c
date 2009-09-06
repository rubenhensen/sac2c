/*****************************************************************************
 *
 *
 * file:   cuda_type_conversion.c
 *
 * prefix: CUTYCV
 *
 * description:
 *
 *   This module inserts CUDA type conversion primitives before and after
 *   each cudarizable N_with. The two primitives are <host2device> and
 *   <device2host>. They are used to convert a host(device) type array
 *   variable to a device(host) type array variable. This is essentially
 *   compiled into host<->device memory transfers in the backend. As an
 *   example:
 *
 *   a_host = with
 *            {
 *              ... = b_host;
 *              ... = c_host;
 *              ... = d_host;
 *            }:genarray( shp);
 *
 *   is transformed into:
 *
 *   b_dev = host2device( b_host);
 *   c_dev = host2device( c_host);
 *   d_dev = host2device( d_host);
 *   a_dev = with
 *            {
 *              ... = b_dev;
 *              ... = c_dev;
 *              ... = d_dev;
 *            }:genarray(shp);
 *   a_host = device2host( a_dev);
 *
 *   Note that simple scalar variables need not be type converted since they
 *   can be passed as function parameters directly to CUDA kernels.
 *
 *
 *****************************************************************************/

#include "annotate_cuda_withloop.h"

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
#include "new_types.h"
#include "LookUpTable.h"
#include "math_utils.h"
#include "types.h"
#include "type_utils.h"
#include "cuda_utils.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool in_cudawl;
    bool travids;
    node *postassigns;
    node *preassigns;
    lut_t *lut;
    node *doloop;
    bool indoloop;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDAWL(n) (n->in_cudawl)
#define INFO_TRAVIDS(n) (n->travids)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_DOLOOP(n) (n->doloop)
#define INFO_INDOLOOP(n) (n->indoloop)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_TRAVIDS (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_DOLOOP (result) = NULL;
    INFO_INDOLOOP (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static void
HandlePrf (node *id, bool set_travids, int must_cuda_doloop, info *arg_info)
{
    node *new_avis;
    ntype *dev_type, *scalar_type;
    simpletype sty;
    bool cond;

    DBUG_ENTER ("HandlePrf");

    DBUG_ASSERT ((NODE_TYPE (id) == N_id), "HandlePrf receives a non-id parameters!");

    cond = INFO_INDOLOOP (arg_info) && !INFO_INCUDAWL (arg_info);

    /* For some primitives, we can only insert type conversion
     * if they are in a CUDA do-loop since the whole do-loop
     * will be executed on the device */
    if (must_cuda_doloop) {
        cond = cond && DO_ISCUDARIZABLE (INFO_DOLOOP (arg_info));
    }

    /* We only insert <host2device> and <device2host> type conversion
     * for the following primitives that are in a cudarizable do
     * loop and is not in a N_with. The reason we do this is to make
     * all operations on arrays in the loop deal with device arrays
     * only. This makes it possible to execute the loop on the device
     * in a single thread. Making all type conversion explicit helps
     * transfer optimization to lift them all out of the loop.
     */
    if (cond) {
        dev_type = TYcopyType (AVIS_TYPE (ID_AVIS (id)));
        scalar_type = TYgetScalar (dev_type);
        sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
        scalar_type = TYsetSimpleType (scalar_type, sty);

        new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
        INFO_PREASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                     TBmakePrf (F_host2device,
                                                TBmakeExprs (TBmakeId (ID_AVIS (id)),
                                                             NULL))),
                          INFO_PREASSIGNS (arg_info));
        AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);
        ID_AVIS (id) = new_avis;

        INFO_TRAVIDS (arg_info) = set_travids;
    }
    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVdoCUDAtypeConversion( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVdoCUDAtypeConversion (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CUTYCVdoCUDAtypeConversion");

    info = MakeInfo ();

    TRAVpush (TR_cutycv);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVfundef( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVassign( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVassign");

    /* We do a bottom-up traversal */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* If we finish traversing a cudarizable N_with, preppend and
     * append type conversion primitives (if there's any).
     */
    if (!INFO_INCUDAWL (arg_info)) {
        if (INFO_POSTASSIGNS (arg_info) != NULL) {
            ASSIGN_NEXT (arg_node)
              = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
            INFO_POSTASSIGNS (arg_info) = NULL;
        }

        if (INFO_PREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
            INFO_PREASSIGNS (arg_info) = NULL;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVlet( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /* N_ids is only traversed if <device2host> need to be inserted. */
    if (INFO_TRAVIDS (arg_info)) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        INFO_TRAVIDS (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVwith( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVwith");

    /* We only traverse cudarizable N_with */
    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_INCUDAWL (arg_info) = FALSE;
        /* Indicate to N_let that we need to traverse the N_ids as
         * well since <device2host> need to be inserted. */
        INFO_TRAVIDS (arg_info) = TRUE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
CUTYCVwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVwith2");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVcode( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVgenarray( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVgenarray");

    /* Be careful not to traverse the N_genarray->shape. Because
     * itan be a N_id node and we do not want to insert
     * <host2device> for it in this case. Therefore, the only thing
     * in N_genarray we want to traverse is the default elment.
     */
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id),
                     "Non-id default element found for N_genarray!");
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVids( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVids (node *arg_node, info *arg_info)
{
    node *new_avis, *old_avis, *ids_avis;
    ntype *ids_type, *dev_type, *scalar_type;
    simpletype sty;

    DBUG_ENTER ("CUTYCVids");

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    /* If the N_ids is of known dimension and is not a scalar */
    if (TUdimKnown (ids_type) && TYgetDim (ids_type) > 0) {
        /* If the scalar type is simple, e.g. int, float ... */
        if (TYisSimple (TYgetScalar (ids_type))) {
            dev_type = TYcopyType (ids_type);
            scalar_type = TYgetScalar (dev_type);
            /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
            sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
            scalar_type = TYsetSimpleType (scalar_type, sty);

            new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
            old_avis = ids_avis;
            IDS_AVIS (arg_node) = new_avis;
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
            INFO_POSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (old_avis, NULL),
                                         TBmakePrf (F_device2host,
                                                    TBmakeExprs (TBmakeId (new_avis),
                                                                 NULL))),
                              INFO_POSTASSIGNS (arg_info));
            /* Maintain SSA property */
            AVIS_SSAASSIGN (old_avis) = INFO_POSTASSIGNS (arg_info);
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVid( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVid (node *arg_node, info *arg_info)
{
    node *new_avis, *old_avis, *id_avis;
    ntype *id_type, *dev_type, *scalar_type;
    simpletype sty;

    DBUG_ENTER ("CUTYCVid");

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* if we are in cudarizable N_with */
    if (INFO_INCUDAWL (arg_info)) {
        old_avis = LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);
        /* if the N_id node hasn't been come across before */
        if (old_avis == id_avis) {
            /* Only arrays with known dimension need to be type converted */
            if (TUdimKnown (id_type) && TYgetDim (id_type) > 0) {
                /* If the scalar type is simple, e.g int, float... */
                if (TYisSimple (TYgetScalar (id_type))) {
                    /* Create device type */
                    dev_type = TYcopyType (id_type);
                    scalar_type = TYgetScalar (dev_type);
                    /* Get the corresponding device simple type e.g. int_dev,
                     * float_dev...*/
                    sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
                    /* Set the simple type of the scalar type to device simple
                     * type, e.g int_dev, float_dev ... */
                    scalar_type = TYsetSimpleType (scalar_type, sty);

                    new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                    ID_AVIS (arg_node) = new_avis;
                    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                      = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
                    INFO_PREASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                                 TBmakePrf (F_host2device,
                                                            TBmakeExprs (TBmakeId (
                                                                           old_avis),
                                                                         NULL))),
                                      INFO_PREASSIGNS (arg_info));
                    /* Insert the pair host_avis->dev_avis into lookup table. */
                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis, new_avis);
                    /* Maintain SSA property */
                    AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);
                }
            }
        } else {
            /* If the N_id has been come across before, replace its
             * N_avis by the device N_avis.
             */
            ID_AVIS (arg_node) = old_avis;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVdo( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVdo (node *arg_node, info *arg_info)
{
    bool old_indoloop;
    node *old_doloop;

    DBUG_ENTER ("CUTYCVdo");

    /* Stack info */
    old_indoloop = INFO_INDOLOOP (arg_info);
    old_doloop = INFO_DOLOOP (arg_info);

    INFO_INDOLOOP (arg_info) = TRUE;
    INFO_DOLOOP (arg_info) = arg_node;

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);
    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    DO_SKIP (arg_node) = TRAVopt (DO_SKIP (arg_node), arg_info);

    /* Pop info */
    INFO_INDOLOOP (arg_info) = old_indoloop;
    INFO_DOLOOP (arg_info) = old_doloop;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVprf( node *arg_node, info *arg_info)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVprf");

    switch (PRF_PRF (arg_node)) {
    case F_idx_sel:
        HandlePrf (PRF_ARG2 (arg_node), FALSE, TRUE, arg_info);
        break;
    case F_shape_A:
        HandlePrf (PRF_ARG1 (arg_node), FALSE, TRUE, arg_info);
        break;
    case F_idx_modarray_AxSxS:
        HandlePrf (PRF_ARG1 (arg_node), TRUE, TRUE, arg_info);
        break;
    case F_idx_modarray_AxSxA:
        HandlePrf (PRF_ARG1 (arg_node), TRUE, FALSE, arg_info);
        HandlePrf (PRF_ARG3 (arg_node), TRUE, FALSE, arg_info);
        break;
    default:
        break;
    }

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
