/** <!--********************************************************************-->
 *
 * @defgroup wlds With-Loop Descalarization
 *
 * @brief
 *
 *
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file wl_descalarization.c
 *
 * Prefix: WLDS
 *
 *****************************************************************************/
#include "wl_descalarization.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "new_types.h"
#include "print.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "shape.h"
#include "phase.h"
#include "new_types.h"
#include "constants.h"
#include "deadcoderemoval.h"
#include "free.h"
#include "LookUpTable.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool onefundef;
    node *fundef;
    node *preassigns;
    node *newgen;
    node *newwithid;
    bool create_newcode;
    node *withop;
    node *letids;
    node *last_newcode;
    node *newcodes;
    node *inner_wlidx;
    node *old_withid;
    node *old_withids;
    node *new_withvec;
    node *part;
};

#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_NEWGEN(n) (n->newgen)
#define INFO_NEWWITHID(n) (n->newwithid)
#define INFO_CREATE_NEWCODE(n) (n->create_newcode)
#define INFO_NEWCODES(n) (n->newcodes)
#define INFO_LAST_NEWCODE(n) (n->last_newcode)
#define INFO_WITHOP(n) (n->withop)
#define INFO_LETIDS(n) (n->letids)
#define INFO_INNER_WLIDX(n) (n->inner_wlidx)
#define INFO_OLDWITHID(n) (n->old_withid)
#define INFO_OLDWITHIDS(n) (n->old_withids)
#define INFO_NEW_WITHVEC(n) (n->new_withvec)
#define INFO_PART(n) (n->part)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_NEWGEN (result) = NULL;
    INFO_NEWWITHID (result) = NULL;
    INFO_CREATE_NEWCODE (result) = FALSE;
    INFO_LAST_NEWCODE (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_NEWCODES (result) = NULL;
    INFO_INNER_WLIDX (result) = NULL;
    INFO_OLDWITHID (result) = NULL;
    INFO_OLDWITHIDS (result) = NULL;
    INFO_NEW_WITHVEC (result) = NULL;
    INFO_PART (result) = NULL;

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
 * @fn node *WLDSdoWithloopUnscalarization( node *fundef)
 *
 * @brief starting point of Withloop Descalarization.
 *
 * @param fundef Fundef-Node to start WLS in.
 *
 * @return modified fundef.
 *
 *****************************************************************************/
node *
WLDSdoWithloopDescalarization (node *syntax_tree)
{
    info *arg_info;
    DBUG_ENTER ();

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_fundef)
                   || (NODE_TYPE (syntax_tree) == N_module),
                 "WLUSdoWithloopUnscalarization called for non-fundef/module node");

    arg_info = MakeInfo ();

    TRAVpush (TR_wlds);
    syntax_tree = TRAVdo (syntax_tree, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLDSfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 ******************************************************************************/
node *
WLDSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDSlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLDSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDSwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLDSwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        node *res_avis = IDS_AVIS (INFO_LETIDS (arg_info));
        if (TYgetDim (AVIS_TYPE (res_avis)) <= 2) {
            DBUG_RETURN (arg_node);
        }
        INFO_NEWCODES (arg_info) = NULL;
        INFO_LAST_NEWCODE (arg_info) = NULL;
        INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        WITH_CODE (arg_node) = INFO_NEWCODES (arg_info);
        INFO_WITHOP (arg_info) = NULL;
        INFO_LAST_NEWCODE (arg_info) = NULL;
        INFO_NEWCODES (arg_info) = NULL;
        INFO_NEW_WITHVEC (arg_info) = NULL;
    } else {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

static node *
CreateInnerGenarrayShape (node *lower_bound, node *upper_bound)
{
    node *shape_elems = NULL;
    node *shape_elem;
    constant *lb_elem, *ub_elem;
    node *lb_elems, *ub_elems;
    node *ret_node;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (lower_bound) == N_array,
                 "Lower bound must be an N_array node!");
    DBUG_ASSERT (NODE_TYPE (upper_bound) == N_array,
                 "Upper bound must be an N_array node!");

    lb_elems = ARRAY_AELEMS (lower_bound);
    ub_elems = ARRAY_AELEMS (upper_bound);

    while (lb_elems != NULL && ub_elems != NULL) {
        lb_elem = COaST2Constant (EXPRS_EXPR (lb_elems));
        ub_elem = COaST2Constant (EXPRS_EXPR (ub_elems));

        DBUG_ASSERT ((lb_elem != NULL && ub_elem != NULL),
                     "Non-AKS cudarizbale N_with found!");

        shape_elem = TBmakeNum (COconst2Int (COsub (ub_elem, lb_elem)));

        shape_elems = TCappendExprs (shape_elems, TBmakeExprs (shape_elem, NULL));

        lb_elems = EXPRS_NEXT (lb_elems);
        ub_elems = EXPRS_NEXT (ub_elems);
    }

    ret_node = TCmakeIntVector (shape_elems);

    DBUG_RETURN (ret_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDSpart(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLDSpart (node *arg_node, info *arg_info)
{
    node *new_code;
    node *inner_part, *inner_with, *inner_withop;
    node *inner_default = NULL, *inner_shape;
    ntype *res_type;
    node *res_avis;
    node *inner_assign;
    lut_t *lut;

    DBUG_ENTER ();

    if (PART_CUDARIZABLE (arg_node)) {
        INFO_OLDWITHID (arg_info) = PART_WITHID (arg_node);
        INFO_OLDWITHIDS (arg_info) = DUPdoDupTree (WITHID_IDS (PART_WITHID (arg_node)));
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
        INFO_OLDWITHID (arg_info) = NULL;

        INFO_PART (arg_info) = arg_node;
        PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
        INFO_PART (arg_info) = NULL;
        INFO_OLDWITHIDS (arg_info) = NULL;

        lut = LUTgenerateLut ();
        LUTinsertIntoLutP (lut, IDS_AVIS (WITHID_IDXS (PART_WITHID (arg_node))),
                           INFO_INNER_WLIDX (arg_info));

        new_code = DUPdoDupNodeLutSsa (PART_CODE (arg_node), lut, INFO_FUNDEF (arg_info));
        CODE_USED (new_code) = 1;

        lut = LUTremoveLut (lut);

        /*
            if ( NODE_TYPE( BLOCK_ASSIGNS( CODE_CBLOCK( new_code))) == N_empty) {
              BLOCK_ASSIGNS( CODE_CBLOCK( new_code)) =
                TCappendAssign( BLOCK_ASSIGNS( CODE_CBLOCK( new_code)), INFO_PREASSIGNS(
           arg_info));
            }
            else {
              BLOCK_ASSIGNS( CODE_CBLOCK( new_code)) =
                TCappendAssign( INFO_PREASSIGNS( arg_info), BLOCK_ASSIGNS( CODE_CBLOCK(
           new_code)));
            }
        */
        if (BLOCK_ASSIGNS (CODE_CBLOCK (new_code)) != NULL) {
            BLOCK_ASSIGNS (CODE_CBLOCK (new_code))
              = TCappendAssign (INFO_PREASSIGNS (arg_info),
                                BLOCK_ASSIGNS (CODE_CBLOCK (new_code)));
        } else {
            PART_INNERWLIDXASSIGN (arg_node) = NULL;
        }

        INFO_PREASSIGNS (arg_info) = NULL;

        inner_part
          = TBmakePart (new_code, INFO_NEWWITHID (arg_info), INFO_NEWGEN (arg_info));

        if (NODE_TYPE (INFO_WITHOP (arg_info)) == N_genarray) {
            inner_default = DUPdoDupNode (GENARRAY_DEFAULT (INFO_WITHOP (arg_info)));
        }

        inner_shape
          = CreateInnerGenarrayShape (GENERATOR_BOUND1 (INFO_NEWGEN (arg_info)),
                                      GENERATOR_BOUND2 (INFO_NEWGEN (arg_info)));

        inner_withop = TBmakeGenarray (inner_shape, inner_default);
        GENARRAY_IDX (inner_withop) = IDS_AVIS (WITHID_IDXS (INFO_NEWWITHID (arg_info)));

        inner_with = TBmakeWith (inner_part, new_code, inner_withop);
        WITH_PARTS (inner_with) = 1;

        /*************************************************************/

        res_type = TYmakeAKS (TYmakeSimpleType (TYgetSimpleType (TYgetScalar (
                                AVIS_TYPE (IDS_AVIS (INFO_LETIDS (arg_info)))))),
                              SHarray2Shape (inner_shape));

        res_avis = TBmakeAvis (TRAVtmpVar (), res_type);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (res_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        inner_assign
          = TBmakeAssign (TBmakeLet (TBmakeIds (res_avis, NULL), inner_with), NULL);

        AVIS_SSAASSIGN (res_avis) = inner_assign;

        /*************************************************************/

        /*
            if( CODE_USED( PART_CODE( arg_node)) > 1) {
              CODE_USED( PART_CODE( arg_node)) -= 1;
            }
            else if( CODE_USED( PART_CODE( arg_node)) == 1){
              PART_CODE( arg_node) = FREEdoFreeNode( PART_CODE( arg_node));
            }
            else {
              // If used count is 0, do nothing
            }
        */

        // node *old_next_code = CODE_NEXT( PART_CODE( arg_node));

        PART_CODE (arg_node) = TBmakeCode (TBmakeBlock (inner_assign, NULL),
                                           TBmakeExprs (TBmakeId (res_avis), NULL));
        CODE_USED (PART_CODE (arg_node)) = 1;

        /*
            if( INFO_LAST_NEWCODE( arg_info) == NULL) {
              CODE_NEXT( PART_CODE( arg_node)) = old_next_code;
              INFO_LAST_NEWCODE( arg_info) = PART_CODE( arg_node);
            }
            else {
              CODE_NEXT( PART_CODE( arg_node)) = old_next_code;
              CODE_NEXT( INFO_LAST_NEWCODE( arg_info)) = PART_CODE( arg_node);
              INFO_LAST_NEWCODE( arg_info) = PART_CODE( arg_node);
            }
        */

        CODE_NEXT (PART_CODE (arg_node)) = INFO_NEWCODES (arg_info);
        INFO_NEWCODES (arg_info) = PART_CODE (arg_node);

        /*************************************************************/

        INFO_NEWWITHID (arg_info) = NULL;
        INFO_NEWGEN (arg_info) = NULL;
    }

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLUScode(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
/*
node *WLUScode( node *arg_node, info *arg_info)
{
  DBUG_ENTER( "WLUScode");

  if( INFO_CREATE_NEWCODE( arg_info)) {
    new_code = DUPdoDupNode( arg_node);

    if ( NODE_TYPE( BLOCK_ASSIGNS( CODE_CBLOCK( new_code))) == N_empty) {
      BLOCK_ASSIGNS( CODE_CBLOCK( new_code)) =
        TCappendAssign( BLOCK_ASSIGNS( CODE_CBLOCK( new_code)),
                        INFO_PREASSIGNS( arg_info));
    }
    else {
      BLOCK_ASSIGNS( CODE_CBLOCK( new_code)) =
        TCappendAssign( INFO_PREASSIGNS( arg_info),
                         BLOCK_ASSIGNS( CODE_CBLOCK( new_code)));
    }

    innerdim = TCcountIds( WITHID_IDS( INFO_NEWWITHID( arg_info)));
  }
  else {
    CODE_CBLOCK( arg_node) = TRAVopt( CODE_CBLOCK( arg_node), arg_info);
    CODE_CEXPRS( arg_node) = TRAVopt( CODE_CEXPRS( arg_node), arg_info);
  }

  DBUG_RETURN( arg_node);
}
*/

/** <!--********************************************************************-->
 *
 * @fn node *WLDSwithid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLDSwithid (node *arg_node, info *arg_info)
{
    node *vec, *outer_ids, *idxs;
    int dim;
    ntype *inner_vectype, *outer_vectype;
    node *avis;
    node *inner_ids;
    node *array_ids;
    node *array;
    node *inner_wlidx_avis;
    node *new_assigns = NULL;

    DBUG_ENTER ();

    vec = WITHID_VEC (arg_node);
    outer_ids = WITHID_IDS (arg_node);
    idxs = WITHID_IDXS (arg_node);

    dim = TCcountIds (outer_ids);

    // array_ids = DUPdoDupTree( outer_ids);
    array_ids = INFO_OLDWITHIDS (arg_info);
    inner_ids = IDS_NEXT (IDS_NEXT (outer_ids));
    IDS_NEXT (IDS_NEXT (outer_ids)) = NULL;

    /************************************************************/

    if (INFO_NEW_WITHVEC (arg_info) == NULL) {

        outer_vectype = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 2));

        avis = TBmakeAvis (TRAVtmpVarName ("iv"), outer_vectype);
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        WITHID_VEC (arg_node) = TBmakeIds (avis, NULL);
        INFO_NEW_WITHVEC (arg_info) = avis;
    } else {
        WITHID_VEC (arg_node) = TBmakeIds (INFO_NEW_WITHVEC (arg_info), NULL);
    }

    /*
      AVIS_TYPE( IDS_AVIS( vec)) = TYfreeType( AVIS_TYPE( IDS_AVIS( vec) ) );
      AVIS_TYPE( IDS_AVIS( vec)) =  TYmakeAKS( TYmakeSimpleType( T_int),
                                               SHcreateShape( 1, 2));
    */

    /************************************************************/

    inner_vectype = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim - 2));

    avis = TBmakeAvis (TRAVtmpVar (), inner_vectype);
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
    /*
      wlidx_avis = TBmakeAvis( TRAVtmpVarName("wlidx"),
                               TYmakeAKS( TYmakeSimpleType( T_int),
                                          SHmakeShape( 0)));

      FUNDEF_VARDECS( INFO_FUNDEF( arg_info)) =
          TBmakeVardec( wlidx_avis,
                        FUNDEF_VARDECS( INFO_FUNDEF( arg_info)));

      WITHID_IDXS( INFO_NEWWITHID( arg_info)) = TBmakeIds( wlidx_avis, NULL);
    */
    INFO_NEWWITHID (arg_info) = TBmakeWithid (TBmakeIds (avis, NULL), inner_ids);

    /************************************************************/

    array = TCmakeIntVector (TCids2Exprs (array_ids));

    /*
      new_vectype = TYmakeAKS( TYmakeSimpleType( T_int),
                               SHcreateShape( 1, 3));

      avis = TBmakeAvis( TRAVtmpVarName( "iv"), new_vectype);
      FUNDEF_VARDECS( INFO_FUNDEF( arg_info)) =
          TBmakeVardec( avis,
                        FUNDEF_VARDECS( INFO_FUNDEF( arg_info)));

      INFO_PREASSIGNS( arg_info) = TBmakeAssign( TBmakeLet( TBmakeIds( avis, NULL),
      array), INFO_PREASSIGNS( arg_info));
    */

    new_assigns = TBmakeAssign (TBmakeLet (vec, array), new_assigns);

    AVIS_SSAASSIGN (avis) = new_assigns;

    /************************************************************/

    inner_wlidx_avis = TBmakeAvis (TRAVtmpVarName ("inner_wlidx"),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (inner_wlidx_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    node *prf = TCmakePrf2 (F_vect2offset,
                            SHshape2Array (
                              TYgetShape (AVIS_TYPE (IDS_AVIS (INFO_LETIDS (arg_info))))),
                            TBmakeId (IDS_AVIS (vec)));

    node *vect2offset_assign
      = TBmakeAssign (TBmakeLet (TBmakeIds (inner_wlidx_avis, NULL), prf), NULL);

    new_assigns = TCappendAssign (new_assigns, vect2offset_assign);

    PART_INNERWLIDXASSIGN (INFO_PART (arg_info)) = vect2offset_assign;

    AVIS_SSAASSIGN (inner_wlidx_avis) = vect2offset_assign;
    WITHID_IDXS (INFO_NEWWITHID (arg_info)) = TBmakeIds (inner_wlidx_avis, NULL);

    INFO_INNER_WLIDX (arg_info) = inner_wlidx_avis;

    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), new_assigns);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDSgenerator(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLDSgenerator (node *arg_node, info *arg_info)
{
    node *lb, *ub, *inner_lb, *inner_ub;
    node *lb_elems, *ub_elems;
    node *inner_lb_elems, *inner_ub_elems;

    node *old_withids;
    node *inner_withids;

    size_t inner_dim;

    constant *inner_lb_const, *inner_ub_const;

    DBUG_ENTER ();

    old_withids = WITHID_IDS (INFO_OLDWITHID (arg_info));
    inner_withids = IDS_NEXT (IDS_NEXT (old_withids));
    inner_dim = TCcountIds (inner_withids);

    lb = GENERATOR_BOUND1 (arg_node);
    ub = GENERATOR_BOUND2 (arg_node);

    DBUG_ASSERT (NODE_TYPE (lb) == N_array, "Lower bound must be an N_array node!");
    DBUG_ASSERT (NODE_TYPE (ub) == N_array, "Upper bound must be an N_array node!");

    lb_elems = ARRAY_AELEMS (lb);
    ub_elems = ARRAY_AELEMS (ub);

    /* Extract dims after 2 */
    inner_lb_elems = EXPRS_NEXT (EXPRS_NEXT (lb_elems));
    inner_ub_elems = EXPRS_NEXT (EXPRS_NEXT (ub_elems));

    EXPRS_NEXT (EXPRS_NEXT (lb_elems)) = NULL;
    EXPRS_NEXT (EXPRS_NEXT (ub_elems)) = NULL;

    inner_lb_const = COaST2Constant (TCmakeIntVector (inner_lb_elems));
    inner_ub_const = COaST2Constant (TCmakeIntVector (inner_ub_elems));

    DBUG_ASSERT ((inner_lb_const != NULL && inner_ub_const != NULL),
                 "Non-AKS CUDA N_with found!");

    if (!COisZero (inner_lb_const, TRUE)) {
        inner_lb = TCcreateZeroVector (inner_dim, T_int);
        inner_ub = COconstant2AST (COsub (inner_ub_const, inner_lb_const));

        node *old_ids_avis, *new_ids_avis;
        node *assign;
        while (inner_withids != NULL) {
            old_ids_avis = IDS_AVIS (inner_withids);
            new_ids_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHmakeShape (0)));
            IDS_AVIS (inner_withids) = new_ids_avis;
            assign
              = TBmakeAssign (TBmakeLet (TBmakeIds (old_ids_avis, NULL),
                                         TCmakePrf2 (F_add_SxS, TBmakeId (new_ids_avis),
                                                     EXPRS_EXPR (inner_lb_elems))),
                              NULL);
            INFO_PREASSIGNS (arg_info)
              = TCappendAssign (INFO_PREASSIGNS (arg_info), assign);

            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_ids_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            inner_withids = IDS_NEXT (inner_withids);
            inner_lb_elems = EXPRS_NEXT (inner_lb_elems);
        }
    } else {
        inner_lb = TCmakeIntVector (inner_lb_elems);
        inner_ub = TCmakeIntVector (inner_ub_elems);
    }

    INFO_NEWGEN (arg_info)
      = TBmakeGenerator (GENERATOR_OP1 (arg_node), GENERATOR_OP2 (arg_node), inner_lb,
                         inner_ub, NULL, NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- WLDS -->
 *****************************************************************************/

#undef DBUG_PREFIX
