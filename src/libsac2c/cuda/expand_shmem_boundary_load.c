
#include "expand_shmem_boundary_load.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"
#include "memory.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "new_types.h"
#include "shape.h"
#include "types.h"
#include "cuda_utils.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *cond_ass;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_COND_ASS(n) (n->cond_ass)

/*
 * INFO macros
 */

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
    INFO_COND_ASS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
Expand (node *prf, info *arg_info)
{
    node *cond, *shmem, *shmemshp, *mem, *memshp;
    int dim, i;
    node *shmem_ids, *shmem_idx, *mem_ids, *mem_idx, *elem, *offsets, *avis;
    node *shmem_ids_off = NULL, *mem_ids_off = NULL;
    node *then_assigns = NULL, *assign;
    node *vardecs = NULL;

    DBUG_ENTER ();

    cond = PRF_ARG1 (prf);
    shmem = PRF_ARG2 (prf);
    shmemshp = PRF_ARG3 (prf);
    mem = PRF_ARG4 (prf);
    memshp = PRF_ARG5 (prf);

    DBUG_ASSERT (NODE_TYPE (PRF_ARG6 (prf)) == N_num, "Non number found for dimension!");
    dim = NUM_VAL (PRF_ARG6 (prf));

    shmem_ids = PRF_EXPRS7 (prf);

    mem_ids = shmem_ids;
    for (i = 0; i < dim; i++) {
        mem_ids = EXPRS_NEXT (mem_ids);
    }

    offsets = mem_ids;
    for (i = 0; i < dim; i++) {
        offsets = EXPRS_NEXT (offsets);
    }

    for (i = 0; i < dim; i++) {
        avis = TBmakeAvis (TRAVtmpVarName ("shmemids"),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        vardecs = TBmakeVardec (avis, vardecs);

        then_assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                     TBmakePrf (F_add_SxS,
                                                TBmakeExprs (DUPdoDupNode (
                                                               EXPRS_EXPR (shmem_ids)),
                                                             TBmakeExprs (DUPdoDupNode (
                                                                            EXPRS_EXPR (
                                                                              offsets)),
                                                                          NULL)))),
                          then_assigns);

        shmem_ids_off
          = TCcombineExprs (shmem_ids_off, TBmakeExprs (TBmakeId (avis), NULL));

        avis = TBmakeAvis (TRAVtmpVarName ("memids"),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        vardecs = TBmakeVardec (avis, vardecs);
        then_assigns
          = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                     TBmakePrf (F_add_SxS,
                                                TBmakeExprs (DUPdoDupNode (
                                                               EXPRS_EXPR (mem_ids)),
                                                             TBmakeExprs (DUPdoDupNode (
                                                                            EXPRS_EXPR (
                                                                              offsets)),
                                                                          NULL)))),
                          then_assigns);

        mem_ids_off = TCappendExprs (mem_ids_off, TBmakeExprs (TBmakeId (avis), NULL));

        shmem_ids = EXPRS_NEXT (shmem_ids);
        mem_ids = EXPRS_NEXT (mem_ids);
        offsets = EXPRS_NEXT (offsets);
    }

    /**********************************************************************/
    shmem_idx = TBmakeAvis (TRAVtmpVarName ("shmemidx"),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    vardecs = TBmakeVardec (shmem_idx, vardecs);
    assign = TBmakeAssign (TBmakeLet (TBmakeIds (shmem_idx, NULL),
                                      TBmakePrf (F_idxs2offset,
                                                 TBmakeExprs (DUPdoDupNode (shmemshp),
                                                              shmem_ids_off))),
                           NULL);

    then_assigns = TCappendAssign (then_assigns, assign);

    /**********************************************************************/
    mem_idx = TBmakeAvis (TRAVtmpVarName ("memidx"),
                          TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
    vardecs = TBmakeVardec (mem_idx, vardecs);
    assign = TBmakeAssign (TBmakeLet (TBmakeIds (mem_idx, NULL),
                                      TBmakePrf (F_idxs2offset,
                                                 TBmakeExprs (DUPdoDupNode (memshp),
                                                              mem_ids_off))),
                           NULL);

    then_assigns = TCappendAssign (then_assigns, assign);

    /**********************************************************************/
    elem = TBmakeAvis (TRAVtmpVarName ("elem"),
                       TYmakeAKS (TYmakeSimpleType (CUd2hSimpleTypeConversion (
                                    TYgetSimpleType (TYgetScalar (ID_NTYPE (mem))))),
                                  SHmakeShape (0)));
    vardecs = TBmakeVardec (elem, vardecs);

    assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (elem, NULL),
                                 TBmakePrf (F_idx_sel,
                                            TBmakeExprs (TBmakeId (mem_idx),
                                                         TBmakeExprs (DUPdoDupNode (mem),
                                                                      NULL)))),
                      NULL);

    then_assigns = TCappendAssign (then_assigns, assign);

    /**********************************************************************/

    assign = TBmakeAssign (
      TBmakeLet (TBmakeIds (ID_AVIS (shmem), NULL),
                 TBmakePrf (F_idx_modarray_AxSxS,
                            TBmakeExprs (DUPdoDupNode (shmem),
                                         TBmakeExprs (TBmakeId (shmem_idx),
                                                      TBmakeExprs (TBmakeId (elem),
                                                                   NULL))))),
      NULL);

    then_assigns = TCappendAssign (then_assigns, assign);

    assign
      = TBmakeAssign (TBmakeCond (DUPdoDupNode (cond), TBmakeBlock (then_assigns, NULL),
                                  TBmakeBlock (TBmakeEmpty (), NULL)),
                      NULL);

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TCappendVardec (FUNDEF_VARDEC (INFO_FUNDEF (arg_info)), vardecs);

    DBUG_RETURN (assign);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESBLdoExpandShmemBoundaryLoad( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
ESBLdoExpandShmemBoundaryLoad (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    TRAVpush (TR_esbl);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();
    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESBLfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ESBLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISCUDAGLOBALFUN (arg_node) || FUNDEF_ISCUDASTGLOBALFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESBLassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ESBLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_COND_ASS (arg_info) = NULL;
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_COND_ASS (arg_info) != NULL) {
        arg_node = FREEdoFreeNode (arg_node);
        ASSIGN_NEXT (INFO_COND_ASS (arg_info)) = arg_node;
        arg_node = INFO_COND_ASS (arg_info);
        INFO_COND_ASS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ESBLprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
ESBLprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_shmem_boundary_load) {
        INFO_COND_ASS (arg_info) = Expand (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
