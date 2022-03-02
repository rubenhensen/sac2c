/**
 *
 * @file wldefaultpartition.c
 *
 * In this traversal
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "shape.h"
#include "DupTree.h"
#include "globals.h"

#define DBUG_PREFIX "WLDP"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "deserialize.h"
#include "namespaces.h"
#include "wldefaultpartition.h"
#include "ctinfo.h"

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *module;
    node *fundef;
    node *defexpr;
    node *wlpreass;
    node *propobjinargs;
    node *propobjinres;
    node *propobjoutargs;
    node *propobjoutres;
    node *selwrapper;
    node *defaultwithid;
};

#define INFO_WL(n) ((n)->wl)
#define INFO_MODULE(n) ((n)->module)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_DEFEXPR(n) ((n)->defexpr)
#define INFO_WLPREASS(n) ((n)->wlpreass)
#define INFO_PROPOBJINARGS(n) ((n)->propobjinargs)
#define INFO_PROPOBJINRES(n) ((n)->propobjinres)
#define INFO_PROPOBJOUTARGS(n) ((n)->propobjoutargs)
#define INFO_PROPOBJOUTRES(n) ((n)->propobjoutres)
#define INFO_SELWRAPPER(n) ((n)->selwrapper)
#define INFO_DEFAULTWITHID(n) ((n)->defaultwithid)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_WL (result) = NULL;
    INFO_MODULE (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_DEFEXPR (result) = NULL;
    INFO_WLPREASS (result) = NULL;
    INFO_PROPOBJINARGS (result) = NULL;
    INFO_PROPOBJINRES (result) = NULL;
    INFO_PROPOBJOUTARGS (result) = NULL;
    INFO_PROPOBJOUTRES (result) = NULL;
    INFO_SELWRAPPER (result) = NULL;
    INFO_DEFAULTWITHID (result) = NULL;

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
 *
 * @fn node *CreateScalarWL( int dim, node *array_shape, simpletype btype,
 *                           node *expr, node *fundef)
 *
 *   @brief  build new genarray WL of shape 'array_shape' and blockinstr.
 *           'expr'
 *
 *   @param  int  *dim         : dimension of iteration space
 *           node *array_shape : shape and upper bound of WL
 *           simpletype btype  : type of 'expr'
 *           node *expr        : rhs of BLOCK_ASSIGNS
 *           node *fundef      : N_FUNDEF
 *   @return node *            : N_Nwith
 ******************************************************************************/
static node *
CreateScalarWL (int dim, node *array_shape, simpletype btype, node *expr, node *fundef)
{
    node *wl;
    node *id;
    node *vardecs = NULL;
    node *vec_ids;
    node *scl_ids = NULL;
    node *tmp_ids;
    int i;

    node *ass;
    node *code, *part, *withop;

    DBUG_ENTER ();

    DBUG_ASSERT (dim >= 0, "CreateScalarWl() used with unknown shape!");

    vec_ids = TBmakeIds (TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                               SHcreateShape (1, dim))),
                         NULL);

    vardecs = TBmakeVardec (IDS_AVIS (vec_ids), vardecs);

    for (i = 0; i < dim; i++) {
        tmp_ids
          = TBmakeIds (TBmakeAvis (TRAVtmpVar (),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0))),
                       NULL);

        vardecs = TBmakeVardec (IDS_AVIS (tmp_ids), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
    }

    id = TBmakeId (
      TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (btype), SHmakeShape (0))));
    vardecs = TBmakeVardec (ID_AVIS (id), vardecs);

    ass = TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (id), NULL), expr), NULL);
    AVIS_SSAASSIGN (ID_AVIS (id)) = ass;

    code = TBmakeCode (TBmakeBlock (ass, NULL), TBmakeExprs (id, NULL));

    part = TBmakePart (code, TBmakeWithid (vec_ids, scl_ids),
                       TBmakeGenerator (F_wl_le, F_wl_lt, TCcreateZeroVector (dim, T_int),
                                        DUPdoDupNode (array_shape), NULL, NULL));

    withop = TBmakeGenarray (DUPdoDupNode (array_shape), NULL);

    wl = TBmakeWith (part, code, withop);

    CODE_USED (WITH_CODE (wl))++;
    WITH_PARTS (wl) = 1;

    fundef = TCaddVardecs (fundef, vardecs);

    DBUG_RETURN (wl);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateZeros( node *array, node *fundef)
 *
 *   @brief creates an array of zeros depending on shape and type of 'array'
 *
 *   @param  node *array    :
 *           node **nassign :  != NULL iff new assignments have been created
 *                             (AKD case)
 *           node *fundef   :  N_FUNDEF
 *   @return node *         :  array of zeros
 ******************************************************************************/

node *
CreateZeros (ntype *array_type, node *fundef)
{
    node *zero = NULL;
    node *array_shape = NULL;
    simpletype btype;
    shape *shape;
    int dim;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisSimple (array_type) == FALSE, "N_id is no array type!");

    dim = TYgetDim (array_type);
    btype = TYgetSimpleType (TYgetScalar (array_type));
    shape = TYgetShape (array_type);

    if (dim == 0) {
        zero = TCcreateZeroScalar (btype);
    } else {
        array_shape = SHshape2Array (shape);
        zero
          = CreateScalarWL (dim, array_shape, btype, TCcreateZeroScalar (btype), fundef);
        array_shape = FREEdoFreeNode (array_shape);
    }

    DBUG_RETURN (zero);
}

#if 0
/** <!--********************************************************************-->
 *
 * @fn node *CreateArraySel( node *sel_vec, node *sel_array, info *arg_info)
 *
 *   @brief creates an scalar or vector-wise reference on 'sel_array'
 *
 *   @param  node *sel_vec   : N_WITHID_VEC of current WL
 *           node *sel_array :
 *           info *arg_info  :
 *   @return node *          : N_ap or N_prf
 ******************************************************************************/
static
node *CreateArraySel( node *sel_vec, node *sel_array, info *arg_info)
{
  node *sel;
  int len_index, dim_array;

  DBUG_ENTER ();

  DBUG_ASSERT (NODE_TYPE( sel_array) == N_id, "no N_id node found!");

  len_index = SHgetExtent( TYgetShape( IDS_NTYPE( sel_vec)), 0);
  DBUG_ASSERT (len_index > 0, "illegal index length found!");

  dim_array = TYgetDim( ID_NTYPE( sel_array));
  DBUG_ASSERT (dim_array > 0, "illegal array dimensionality found!");

  if (len_index > dim_array) {
    DBUG_UNREACHABLE ("illegal array selection found!");
    sel = NULL;
  }
  else if ((len_index == dim_array)) {
    sel = TBmakePrf( F_sel_VxA, TBmakeExprs( DUPdupIdsId( sel_vec),
                                         TBmakeExprs( DUPdoDupNode( sel_array),
                                                      NULL)));
  }
  else {   /* (len_index < dim_array) */

    if (INFO_SELWRAPPER( arg_info) == NULL){

      DSinitDeserialize(INFO_MODULE( arg_info));

      INFO_SELWRAPPER( arg_info) = DSaddSymbolByName( "sel",
                                                      SET_wrapperhead,
                                                      global.preludename);
      DSfinishDeserialize( INFO_MODULE( arg_info));
    }

    DBUG_ASSERT (INFO_SELWRAPPER( arg_info) != NULL, "Function %s::sel not found", global.preludename);

    sel = TCmakeAp2( INFO_SELWRAPPER( arg_info),
                     DUPdupIdsId( sel_vec), DUPdoDupNode( sel_array));
  }

  DBUG_RETURN (sel);
}
#endif

/** <!--********************************************************************-->
 *
 * @fn node *WLDPmodule(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_module
 *           info *arg_info:  N_info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
WLDPmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DSinitDeserialize (arg_node);

    INFO_MODULE (arg_info) = arg_node;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DSfinishDeserialize (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPfundef(node *arg_node, info *arg_info)
 *
 *   @brief starts the traversal of the given fundef.
 *
 *   @param  node *arg_node:  N_fundef
 *           info *arg_info:  N_info
 *   @return node *        :  N_fundef
 ******************************************************************************/

node *
WLDPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("traversing body of function %s", CTIitemName (arg_node));

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_ASSIGNS (arg_node) = TRAVdo (FUNDEF_ASSIGNS (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPassign(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  info
 *   @return node *        :  N_module
 ******************************************************************************/

node *
WLDPassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_WLPREASS (arg_info) != NULL) {
        ASSIGN_NEXT (INFO_WLPREASS (arg_info)) = arg_node;
        arg_node = INFO_WLPREASS (arg_info);
        INFO_WLPREASS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPwith(node *arg_node, info *arg_info)
 *
 *   @brief  start traversal of this WL and store information in arg_info
 *           node. Every WL gets a additional default partition to its single
 *           existing N_part node.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
WLDPwith (node *arg_node, info *arg_info)
{
    node *lastdefwithid;

    DBUG_ENTER ();

    DBUG_PRINT ("traversing With-Loop in line #%zu ", NODE_LINE (arg_node));
    /*
     * We have to stack the withloop info here to cater for nested withloops
     */
    lastdefwithid = INFO_DEFAULTWITHID (arg_info);

    /*
     * Visit with-loop body recursively before transforming
     * current with-loop.
     */
    if (global.ssaiv) {
        /* Copy first withid to serve as default partition withid. */
        INFO_DEFAULTWITHID (arg_info)
          = DUPdoDupTreeSsa (WITH_WITHID (arg_node), INFO_FUNDEF (arg_info));
    } else {
        INFO_DEFAULTWITHID (arg_info) = DUPdoDupTree (WITH_WITHID (arg_node));
    }

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    if ((WITH_TYPE (arg_node) == N_genarray) || (WITH_TYPE (arg_node) == N_modarray)) {

        INFO_WL (arg_info) = arg_node;

        /*
         * we traverse the Wl operation for generating default expression.
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        if (PART_NEXT (WITH_PART (arg_node)) == NULL) {
            /*
             * traverse the one and only regular PART.
             * a new default Part will be added
             */
            WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        } else {
            DBUG_ASSERT (NODE_TYPE (PART_GENERATOR (PART_NEXT (WITH_PART (arg_node))))
                           == N_default,
                         "Second partition is no default partition!");
        }
    }

    INFO_DEFAULTWITHID (arg_info) = lastdefwithid;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPgenarray( node *arg_node, info *arg_info)
 *
 *   @brief  generates default expression
 *
 *   @param  node *arg_node:  N_genarray
 *           info *arg_info:  N_info
 *   @return node *        :  N_genarray
 ******************************************************************************/

node *
WLDPgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    if (GENARRAY_DEFAULT (arg_node) == NULL) {
        ntype *array_type;

        array_type = ID_NTYPE (EXPRS_EXPR (WITH_CEXPRS (INFO_WL (arg_info))));

        if (TYisAKV (array_type) || TYisAKS (array_type)) {
            node *avis, *ids, *vardec;

            avis = TBmakeAvis (TRAVtmpVar (), TYeliminateAKV (array_type));
            ids = TBmakeIds (avis, NULL);
            vardec = TBmakeVardec (avis, NULL);

            INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), vardec);

            INFO_WLPREASS (arg_info)
              = TBmakeAssign (TBmakeLet (ids, CreateZeros (array_type,
                                                           INFO_FUNDEF (arg_info))),
                              NULL);
            /* set correct backref to defining assignment */
            AVIS_SSAASSIGN (IDS_AVIS (ids)) = INFO_WLPREASS (arg_info);

            INFO_DEFEXPR (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_DEFEXPR (arg_info));

        } else {
            CTIabort (LINE_TO_LOC (global.linenum),
                          "Genarray with-loop with missing default expression found."
                          " Unfortunately, a default expression is necessary here"
                          " to generate code for new partitions");
        }

    } else {
        INFO_DEFEXPR (arg_info) = TBmakeExprs (DUPdoDupTree (GENARRAY_DEFAULT (arg_node)),
                                               INFO_DEFEXPR (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPmodarray( node *arg_node, info *arg_info)
 *
 *   @brief  generates default expression.
 *
 *   @param  node *arg_node:  N_modarray
 *           info *arg_info:  N_info
 *   @return node *        :  N_modarray
 ******************************************************************************/

node *
WLDPmodarray (node *arg_node, info *arg_info)
{
    node *sel_vec, *sel_array, *sel_ap;

    DBUG_ENTER ();

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    sel_vec = WITHID_VEC (INFO_DEFAULTWITHID (arg_info));
    sel_array = MODARRAY_ARRAY (arg_node);

#if 0
  if ( ( TYisAKV(IDS_NTYPE( sel_vec)) ||
         TYisAKS(IDS_NTYPE( sel_vec))) &&
       ( TYisAKV(ID_NTYPE( sel_array)) ||
         TYisAKS(ID_NTYPE( sel_array)) ||
         TYisAKD(ID_NTYPE( sel_array)))) {

    INFO_DEFEXPR( arg_info) = CreateArraySel( sel_vec,
                                                   sel_array,
                                                   arg_info);
  }
  else{
#endif
    sel_ap
      = DSdispatchFunCall (NSgetNamespace (global.preludename), "sel",
                           TBmakeExprs (DUPdupIdsId (sel_vec),
                                        TBmakeExprs (DUPdoDupNode (sel_array), NULL)));
    DBUG_ASSERT (sel_ap != NULL, "missing instance of sel in sac-prelude");
    INFO_DEFEXPR (arg_info) = TBmakeExprs (sel_ap, INFO_DEFEXPR (arg_info));
#if 0
  }
#endif

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPpropagate( node *arg_node, info *arg_info)
 *
 * @brief  generates default expression.
 *
 * @param  arg_node propagate node
 * @param  arg_info info structure
 * @return unchanged propagate node
 ******************************************************************************/

node *
WLDPpropagate (node *arg_node, info *arg_info)
{
    node *inres, *outres;
    ntype *type;

    DBUG_ENTER ();

    if (PROPAGATE_NEXT (arg_node) != NULL) {
        PROPAGATE_NEXT (arg_node) = TRAVdo (PROPAGATE_NEXT (arg_node), arg_info);
    }

    DBUG_ASSERT (NODE_TYPE (PROPAGATE_DEFAULT (arg_node)) == N_id,
                 "N_id node expected as propagate default");

    /*
     * construct the argument and result chains for
     * prop_obj_in and prop_obj_out
     */
    type = AVIS_TYPE (ID_AVIS (PROPAGATE_DEFAULT (arg_node)));

    inres = TBmakeAvis (TRAVtmpVar (), TYcopyType (type));
    outres = TBmakeAvis (TRAVtmpVar (), TYcopyType (type));

    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (inres,
                      TBmakeVardec (outres, FUNDEF_VARDECS (INFO_FUNDEF (arg_info))));

    INFO_PROPOBJINARGS (arg_info)
      = TBmakeExprs (DUPdoDupTree (PROPAGATE_DEFAULT (arg_node)),
                     INFO_PROPOBJINARGS (arg_info));

    INFO_PROPOBJINRES (arg_info) = TBmakeIds (inres, INFO_PROPOBJINRES (arg_info));

    INFO_PROPOBJOUTARGS (arg_info)
      = TBmakeExprs (TBmakeId (inres), INFO_PROPOBJOUTARGS (arg_info));

    INFO_PROPOBJOUTRES (arg_info) = TBmakeIds (outres, INFO_PROPOBJOUTRES (arg_info));

    /*
     * the result of prop_obj_out is the default expression
     */
    INFO_DEFEXPR (arg_info) = TBmakeExprs (TBmakeId (outres), INFO_DEFEXPR (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPpart(node *arg_node, info *arg_info)
 *
 *   @brief adds default partition
 *
 *   @param  node *arg_node:  N_part
 *           info *arg_info:  N_info
 *   @return node *        :  N_part
 ******************************************************************************/

node *
WLDPpart (node *arg_node, info *arg_info)
{
    node *_ids, *vardec, *idn, *nassign;
    node *code;
    node *expriter, *temp, *idniter;
    node *newavis;

    DBUG_ENTER ();

    DBUG_ASSERT (INFO_DEFEXPR (arg_info) != NULL, "default expression is missing!");

    _ids = NULL;
    idn = NULL;
    nassign = NULL;
    vardec = NULL;
    idniter = NULL;
    expriter = INFO_DEFEXPR (arg_info);
    INFO_DEFEXPR (arg_info) = NULL;
    /*
     * 1) construct prop_obj_out
     */
    if (INFO_PROPOBJOUTARGS (arg_info) != NULL) {
        nassign = TBmakeAssign (TBmakeLet (INFO_PROPOBJOUTRES (arg_info),
                                           TBmakePrf (F_prop_obj_out,
                                                      INFO_PROPOBJOUTARGS (arg_info))),
                                nassign);

        INFO_PROPOBJOUTRES (arg_info)
          = TCsetSSAAssignForIdsChain (INFO_PROPOBJOUTRES (arg_info), nassign);

        INFO_PROPOBJOUTRES (arg_info) = NULL;
        INFO_PROPOBJOUTARGS (arg_info) = NULL;
    }

    /*
     * construct default expression assignments if
     * the expression is not an identifier
     * already
     */
    while (expriter != NULL) {
        if (NODE_TYPE (EXPRS_EXPR (expriter)) != N_id) {
            _ids = TBmakeIds (TBmakeAvis (TRAVtmpVar (),
                                          TYeliminateAKV (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (
                                            WITH_CEXPRS (INFO_WL (arg_info))))))),
                              NULL);

            vardec = TBmakeVardec (IDS_AVIS (_ids), vardec);

            temp = TBmakeExprs (DUPdupIdsId (_ids), NULL);

            /* create new N_assign node  */
            nassign = TBmakeAssign (TBmakeLet (_ids, EXPRS_EXPR (expriter)), nassign);

            /* set correct backref to defining assignment */
            AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;
        } else {
            temp = TBmakeExprs (EXPRS_EXPR (expriter), NULL);
        }

        if (idn == NULL) {
            idn = temp;
            idniter = idn;
        } else {
            EXPRS_NEXT (idniter) = temp;
            idniter = temp;
        }

        /*
         * free shell and move on to next
         */
        EXPRS_EXPR (expriter) = NULL;
        expriter = FREEdoFreeNode (expriter);
    }

    INFO_FUNDEF (arg_info) = TCaddVardecs (INFO_FUNDEF (arg_info), vardec);

    /*
     * 3) construct prop_obj_in
     */
    newavis = IDS_AVIS (WITHID_VEC (INFO_DEFAULTWITHID (arg_info)));
    if (INFO_PROPOBJINARGS (arg_info) != NULL) {
        nassign = TBmakeAssign (TBmakeLet (INFO_PROPOBJINRES (arg_info),
                                           TBmakePrf (F_prop_obj_in,
                                                      TBmakeExprs (TBmakeId (newavis),
                                                                   INFO_PROPOBJINARGS (
                                                                     arg_info)))),
                                nassign);

        INFO_PROPOBJINRES (arg_info)
          = TCsetSSAAssignForIdsChain (INFO_PROPOBJINRES (arg_info), nassign);

        INFO_PROPOBJINRES (arg_info) = NULL;
        INFO_PROPOBJINARGS (arg_info) = NULL;
    }

    code = TBmakeCode (TBmakeBlock (nassign, NULL), idn);
    PART_NEXT (arg_node)
      = TBmakePart (code, INFO_DEFAULTWITHID (arg_info), TBmakeDefault ());
    CODE_USED (code) = 1;
    CODE_NEXT (WITH_CODE (INFO_WL (arg_info))) = PART_CODE (PART_NEXT (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLDPdoWlDefaultPartition(node *arg_node)
 *
 *   @brief  Starting point for traversal WL default partition.
 *
 *   @param  node *arg_node       :  N_module
 *   @return node *               :  modified N_module
 ******************************************************************************/

node *
WLDPdoWlDefaultPartition (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "WLDPdoDefaultPartition not started with N_module node");

    DBUG_PRINT ("starting with-loop default partition");

    arg_info = MakeInfo ();

    INFO_MODULE (arg_info) = arg_node;

    TRAVpush (TR_wldp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("with-loop default partition complete");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
