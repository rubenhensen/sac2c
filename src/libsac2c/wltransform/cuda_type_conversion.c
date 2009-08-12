/*****************************************************************************
 *
 *
 * file:   annotate_cuda_withloop.c
 *
 * prefix: CUTYCV
 *
 * description:
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
    bool dev_var;
    node *postassigns;
    node *preassigns;
    bool indoloop;
    lut_t *lut;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_IN_CUDAWL(n) (n->in_cudawl)
#define INFO_DEV_VAR(n) (n->dev_var)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_INDOLOOP(n) (n->indoloop)
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
    INFO_DEV_VAR (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_INDOLOOP (result) = FALSE;
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
 * @brief node *CUTYCVfundef( node *syntax_tree)
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

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVassign( node *syntax_tree)
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
    /*
      node *next, *tmp;

      next = ASSIGN_NEXT( arg_node);

      ASSIGN_INSTR( arg_node) =  TRAVdo( ASSIGN_INSTR( arg_node), arg_info);

      // Prepend preassigns and append postassigns
      if( !INFO_IN_CUDAWL( arg_info)){
        if ( INFO_POSTASSIGNS( arg_info) != NULL) {
          ASSIGN_NEXT( arg_node) = TCappendAssign( INFO_POSTASSIGNS( arg_info),
                                                   ASSIGN_NEXT( arg_node));
          INFO_POSTASSIGNS( arg_info) = NULL;
        }

        if( INFO_PREASSIGNS( arg_info) != NULL) {
          arg_node = TCappendAssign( INFO_PREASSIGNS( arg_info), arg_node);
          INFO_PREASSIGNS( arg_info) = NULL;
        }
      }

      //Looking for the last assign in postassign chain and continue traversal from there.

      tmp = arg_node;
      while( tmp != NULL && ASSIGN_NEXT( tmp) != next) {
        tmp = ASSIGN_NEXT( tmp);
      }

      if( ASSIGN_NEXT( tmp) != NULL) {
        ASSIGN_NEXT( tmp) = TRAVdo( ASSIGN_NEXT( tmp), arg_info);
      }
    */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

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

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVlet( node *syntax_tree)
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

    /* IDS are only traversed if we have a cudarizable
     * Withloop on the right hand side.
     */
    if (INFO_DEV_VAR (arg_info)) {
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
        INFO_DEV_VAR (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief node *CUTYCVwith( node *syntax_tree)
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

    /* we only look at cudarizable Withloops */
    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_IN_CUDAWL (arg_info) = WITH_CUDARIZABLE (arg_node);
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_IN_CUDAWL (arg_info) = FALSE;
        INFO_DEV_VAR (arg_info) = TRUE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    }

    DBUG_RETURN (arg_node);
}

node *
CUTYCVcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVcode");

    /* we do not traverse exprs list. */
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUTYCVgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVgenarray");

    GENARRAY_MEM (arg_node) = TRAVopt (GENARRAY_MEM (arg_node), arg_info);

    GENARRAY_SUB (arg_node) = TRAVopt (GENARRAY_SUB (arg_node), arg_info);

    GENARRAY_DEFAULT (arg_node) = TRAVopt (GENARRAY_DEFAULT (arg_node), arg_info);

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
 * @brief node *CUTYCVid( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVids (node *arg_node, info *arg_info)
{
    int dim;
    node *new_avis, *old_avis;
    ntype *dev_type, *scalar_type;

    DBUG_ENTER ("CUTYCVids");

    dim = TYgetDim (AVIS_TYPE (IDS_AVIS (arg_node)));
    if (dim > 0) {
        /* the scalar type should be simple, i.e. either int or float */
        if (TYisSimple (TYgetScalar (AVIS_TYPE (IDS_AVIS (arg_node))))) {
            dev_type = TYcopyType (AVIS_TYPE (IDS_AVIS (arg_node)));
            scalar_type = TYgetScalar (dev_type);
            switch (TYgetSimpleType (scalar_type)) {
            case T_float:
                scalar_type = TYsetSimpleType (scalar_type, T_float_dev);
                break;
            case T_int:
                scalar_type = TYsetSimpleType (scalar_type, T_int_dev);
                break;
            default:
                break; /* in the future, it should support more basic types, i.e. double.
                        */
            }
            new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
            old_avis = IDS_AVIS (arg_node);
            IDS_AVIS (arg_node) = new_avis;
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
            INFO_POSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (old_avis, NULL),
                                         TBmakePrf (F_device2host,
                                                    TBmakeExprs (TBmakeId (new_avis),
                                                                 NULL))),
                              INFO_POSTASSIGNS (arg_info));
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
 * @brief node *CUTYCVid( node *syntax_tree)
 *
 * @param
 * @param
 * @return
 *
 *****************************************************************************/
node *
CUTYCVid (node *arg_node, info *arg_info)
{
    int dim;
    node *new_avis, *old_avis;
    ntype *dev_type, *scalar_type;

    DBUG_ENTER ("CUTYCVid");

    /* if we are in cudarizable Withloop */
    if (INFO_IN_CUDAWL (arg_info)) {
        old_avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        /* if the ID node hasn't been come across before */

        if (old_avis == ID_AVIS (arg_node)) {
            /* check the dimention of the ID node. we only interested in
             * arrays, which means dim should be greater than 0.
             */
            if (TYisAKV (AVIS_TYPE (ID_AVIS (arg_node)))
                || TYisAKS (AVIS_TYPE (ID_AVIS (arg_node)))
                || TYisAKD (AVIS_TYPE (ID_AVIS (arg_node)))) {
                dim = TYgetDim (AVIS_TYPE (ID_AVIS (arg_node)));
                if (dim > 0) {
                    /* the scalar type should be simple, i.e. either int or float */
                    if (TYisSimple (TYgetScalar (AVIS_TYPE (ID_AVIS (arg_node))))) {
                        /* create the corresponding 'device' type */
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
                        AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);
                    }
                }
            }
        } else {
            ID_AVIS (arg_node) = old_avis;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
CUTYCVdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVdo");

    INFO_INDOLOOP (arg_info) = TRUE;

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    DO_SKIP (arg_node) = TRAVopt (DO_SKIP (arg_node), arg_info);

    INFO_INDOLOOP (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

node *
CUTYCVprf (node *arg_node, info *arg_info)
{
    node *to_id, *from_id, *to_new_avis, *from_new_avis;
    ntype *to_dev_type, *from_dev_type, *to_scalar_type, *from_scalar_type;

    DBUG_ENTER ("CUTYCVprf");

    /*
      switch( PRF_PRF( arg_node)) {
        case F_idx_sel:
          if( INFO_INDOLOOP( arg_info) &&
              !INFO_IN_CUDAWL( arg_info)) {
            id = PRF_ARG2( arg_node);
            dev_type = TYcopyType( AVIS_TYPE( ID_AVIS( id)));
            scalar_type = TYgetScalar( dev_type);
            switch( TYgetSimpleType( scalar_type)) {
              case T_float:
                scalar_type = TYsetSimpleType( scalar_type, T_float_dev);
                break;
              case T_int:
                scalar_type = TYsetSimpleType( scalar_type, T_int_dev);
                break;
              default:
                break;
            }
            new_avis = TBmakeAvis( TRAVtmpVarName("dev") ,dev_type);
            FUNDEF_VARDEC( INFO_FUNDEF( arg_info)) =
              TBmakeVardec( new_avis,  FUNDEF_VARDEC( INFO_FUNDEF( arg_info)));
            INFO_PREASSIGNS( arg_info) =
              TBmakeAssign( TBmakeLet( TBmakeIds( new_avis, NULL),
                                       TBmakePrf( F_host2device,
                                                  TBmakeExprs( TBmakeId( ID_AVIS( id)),
      NULL))), INFO_PREASSIGNS( arg_info)); AVIS_SSAASSIGN( new_avis) = INFO_PREASSIGNS(
      arg_info); ID_AVIS( id) = new_avis;
          }
          break;
        case F_idx_modarray_AxSxS:
          if( INFO_INDOLOOP( arg_info) &&
              !INFO_IN_CUDAWL( arg_info)) {
            id = PRF_ARG1( arg_node);
            dev_type = TYcopyType( AVIS_TYPE( ID_AVIS( id)));
            scalar_type = TYgetScalar( dev_type);
            switch( TYgetSimpleType( scalar_type)) {
              case T_float:
                scalar_type = TYsetSimpleType( scalar_type, T_float_dev);
                break;
              case T_int:
                scalar_type = TYsetSimpleType( scalar_type, T_int_dev);
                break;
              default:
                break;
            }
            new_avis = TBmakeAvis( TRAVtmpVarName("dev") ,dev_type);
            FUNDEF_VARDEC( INFO_FUNDEF( arg_info)) =
              TBmakeVardec( new_avis,  FUNDEF_VARDEC( INFO_FUNDEF( arg_info)));
            INFO_PREASSIGNS( arg_info) =
              TBmakeAssign( TBmakeLet( TBmakeIds( new_avis, NULL),
                                       TBmakePrf( F_host2device,
                                                  TBmakeExprs( TBmakeId( ID_AVIS( id)),
      NULL))), INFO_PREASSIGNS( arg_info)); AVIS_SSAASSIGN( new_avis) = INFO_PREASSIGNS(
      arg_info); ID_AVIS( id) = new_avis; INFO_DEV_VAR( arg_info) = TRUE;
          }
          break;
        default:
          break;
      }
    */
    switch (PRF_PRF (arg_node)) {
    case F_idx_modarray_AxSxA:
        if (INFO_INDOLOOP (arg_info) && !INFO_IN_CUDAWL (arg_info)) {
            to_id = PRF_ARG1 (arg_node);
            from_id = PRF_ARG3 (arg_node);
            to_dev_type = TYcopyType (AVIS_TYPE (ID_AVIS (to_id)));
            from_dev_type = TYcopyType (AVIS_TYPE (ID_AVIS (from_id)));
            to_scalar_type = TYgetScalar (to_dev_type);
            from_scalar_type = TYgetScalar (from_dev_type);
            switch (TYgetSimpleType (to_scalar_type)) {
            case T_float:
                to_scalar_type = TYsetSimpleType (to_scalar_type, T_float_dev);
                from_scalar_type = TYsetSimpleType (from_scalar_type, T_float_dev);
                break;
            case T_int:
                to_scalar_type = TYsetSimpleType (to_scalar_type, T_int_dev);
                from_scalar_type = TYsetSimpleType (from_scalar_type, T_int_dev);
                break;
            default:
                break;
            }
            to_new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), to_dev_type);
            from_new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), from_dev_type);
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (to_new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (from_new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (to_new_avis, NULL),
                                         TBmakePrf (F_host2device,
                                                    TBmakeExprs (TBmakeId (
                                                                   ID_AVIS (to_id)),
                                                                 NULL))),
                              INFO_PREASSIGNS (arg_info));
            AVIS_SSAASSIGN (to_new_avis) = INFO_PREASSIGNS (arg_info);
            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (from_new_avis, NULL),
                                         TBmakePrf (F_host2device,
                                                    TBmakeExprs (TBmakeId (
                                                                   ID_AVIS (from_id)),
                                                                 NULL))),
                              INFO_PREASSIGNS (arg_info));
            AVIS_SSAASSIGN (from_new_avis) = INFO_PREASSIGNS (arg_info);
            ID_AVIS (to_id) = to_new_avis;
            ID_AVIS (from_id) = from_new_avis;
            INFO_DEV_VAR (arg_info) = TRUE;
        }
        break;
    default:
        break;
    }
    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
