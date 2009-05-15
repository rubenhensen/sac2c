/*
 * $Id: create_cuda_call.c 15999 2009-02-13 18:44:22Z sah $
 *
 * @file create_cuda_call.c
 *
 * This file inserts cuda function call in sac source
 */

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
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool in_cudawl;
    node *postassigns;
    node *preassigns;
    node *lastassign;
    node *let_ids;
    lut_t *lut;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_IN_CUDAWL(n) (n->in_cudawl)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_LET_IDS(n) (n->let_ids)
#define INFO_LUT(n) (n->lut)

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
    INFO_IN_CUDAWL (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LASTASSIGN (result) = NULL;
    INFO_LET_IDS (result) = NULL;
    INFO_LUT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

node *
CUTYCVdoCUDAtypeConversion (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("CUTYCVdoCUDAtypeConversion");

    info = MakeInfo ();

    INFO_LUT (info) = LUTgenerateLut ();

    TRAVpush (TR_cutycv);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    INFO_LUT (info) = LUTremoveLut (INFO_LUT (info));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

node *
CUTYCVwith2 (node *arg_node, info *arg_info)
{
    node *ids;
    int dim;
    node *new_avis, *old_avis;
    ntype *dev_type, *scalar_type;

    DBUG_ENTER ("CUTYCVwith2");

    if (WITH2_CUDARIZABLE (arg_node)) {
        ids = INFO_LET_IDS (arg_info);
        INFO_LET_IDS (arg_info) = NULL;
        INFO_IN_CUDAWL (arg_info) = WITH2_CUDARIZABLE (arg_node);
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        INFO_IN_CUDAWL (arg_info) = FALSE;

        while (ids != NULL) {
            dim = TYgetDim (AVIS_TYPE (IDS_AVIS (ids)));
            if (dim > 0) {
                /* the scalar type should be simple, i.e. either int or float */
                if (TYisSimple (TYgetScalar (AVIS_TYPE (IDS_AVIS (ids))))) {
                    dev_type = TYcopyType (AVIS_TYPE (IDS_AVIS (ids)));
                    scalar_type = TYgetScalar (dev_type);
                    switch (TYgetSimpleType (scalar_type)) {
                    case T_float:
                        scalar_type = TYsetSimpleType (scalar_type, T_float_dev);
                        break;
                    default:
                        break;
                    }
                    new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                    old_avis = IDS_AVIS (ids);
                    IDS_AVIS (ids) = new_avis;
                    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                      = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
                    INFO_POSTASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (old_avis, NULL),
                                                 TBmakePrf (F_device2host,
                                                            TBmakeExprs (TBmakeId (
                                                                           new_avis),
                                                                         NULL))),
                                      INFO_POSTASSIGNS (arg_info));

                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis, new_avis);
                }
            }
            ids = IDS_NEXT (ids);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
CUTYCVwith (node *arg_node, info *arg_info)
{
    node *ids;
    int dim;
    node *new_avis, *old_avis;
    ntype *dev_type, *scalar_type;

    DBUG_ENTER ("CUTYCVwith");

    if (WITH_CUDARIZABLE (arg_node)) {
        ids = INFO_LET_IDS (arg_info);
        INFO_LET_IDS (arg_info) = NULL;
        INFO_IN_CUDAWL (arg_info) = WITH_CUDARIZABLE (arg_node);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_IN_CUDAWL (arg_info) = FALSE;

        while (ids != NULL) {
            dim = TYgetDim (AVIS_TYPE (IDS_AVIS (ids)));
            if (dim > 0) {
                /* the scalar type should be simple, i.e. either int or float */
                if (TYisSimple (TYgetScalar (AVIS_TYPE (IDS_AVIS (ids))))) {
                    dev_type = TYcopyType (AVIS_TYPE (IDS_AVIS (ids)));
                    scalar_type = TYgetScalar (dev_type);
                    switch (TYgetSimpleType (scalar_type)) {
                    case T_float:
                        scalar_type = TYsetSimpleType (scalar_type, T_float_dev);
                        break;
                    default:
                        break;
                    }
                    new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                    old_avis = IDS_AVIS (ids);
                    IDS_AVIS (ids) = new_avis;
                    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                      = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
                    INFO_POSTASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (old_avis, NULL),
                                                 TBmakePrf (F_device2host,
                                                            TBmakeExprs (TBmakeId (
                                                                           new_avis),
                                                                         NULL))),
                                      INFO_POSTASSIGNS (arg_info));

                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis, new_avis);
                }
            }
            ids = IDS_NEXT (ids);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
CUTYCVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVlet");

    INFO_LET_IDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUTYCVassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVassign");

    node *next, *tmp;

    next = ASSIGN_NEXT (arg_node);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* Prepend preassigns and append postassigns */
    if (!INFO_IN_CUDAWL (arg_info)) {
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

    /* Looking for the last assign in postassign chain and
     * continue tracersal from there.
     */
    tmp = arg_node;
    while (tmp != NULL && ASSIGN_NEXT (tmp) != next) {
        tmp = ASSIGN_NEXT (tmp);
    }

    if (ASSIGN_NEXT (tmp) != NULL) {
        ASSIGN_NEXT (tmp) = TRAVdo (ASSIGN_NEXT (tmp), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CUTYCVfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL)
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUTYCVid (node *arg_node, info *arg_info)
{
    int dim;
    node *new_avis;
    node *old_avis;
    ntype *dev_type, *scalar_type;

    DBUG_ENTER ("CUTYCVid");

    /* if the current wl is tagged as cuda wl */
    if (INFO_IN_CUDAWL (arg_info)) {
        old_avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        if (old_avis == ID_AVIS (arg_node)) {
            /* check the dimention of the id node. we only interested in
             * arrays, which means dim should be greater than 0.
             */
            if (TYisAKV (AVIS_TYPE (ID_AVIS (arg_node)))
                || TYisAKS (AVIS_TYPE (ID_AVIS (arg_node)))
                || TYisAKD (AVIS_TYPE (ID_AVIS (arg_node)))) {
                dim = TYgetDim (AVIS_TYPE (ID_AVIS (arg_node)));
                if (dim > 0) {
                    /* the scalar type should be simple, i.e. either int or float */
                    if (TYisSimple (TYgetScalar (AVIS_TYPE (ID_AVIS (arg_node))))) {
                        dev_type = TYcopyType (AVIS_TYPE (ID_AVIS (arg_node)));
                        scalar_type = TYgetScalar (dev_type);
                        switch (TYgetSimpleType (scalar_type)) {
                        case T_float:
                            scalar_type = TYsetSimpleType (scalar_type, T_float_dev);
                            break;
                        case T_int:
                            scalar_type = TYsetSimpleType (scalar_type, T_int_dev);
                            break;
                        default:
                            break;
                        }

                        new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                        arg_node = TBmakeId (new_avis);
                        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                          = TBmakeVardec (new_avis,
                                          FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
                        INFO_PREASSIGNS (arg_info)
                          = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                                     TBmakePrf (F_host2device,
                                                                TBmakeExprs (TBmakeId (
                                                                               old_avis),
                                                                             NULL))),
                                          INFO_PREASSIGNS (arg_info));
                        INFO_LUT (arg_info)
                          = LUTinsertIntoLutP (INFO_LUT (arg_info), old_avis, new_avis);
                    }
                }
            }
        } else {
            ID_AVIS (arg_node) = old_avis;
        }
    }

    DBUG_RETURN (arg_node);
}
