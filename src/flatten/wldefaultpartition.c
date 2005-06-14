/*
 *
 * $Log$
 * Revision 1.1  2005/06/14 08:58:57  khf
 * Initial revision
 *
 *
 *
 *
 */

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
#include "internal_lib.h"
#include "free.h"
#include "shape.h"
#include "DupTree.h"
#include "globals.h"
#include "dbug.h"
#include "traverse.h"
#include "constants.h"
#include "deserialize.h"
#include "wldefaultpartition.h"

/**
 * INFO structure
 */
struct INFO {
    node *wl;
    node *module;
    node *fundef;
    node *default_expr;
    node *sel_wrapper;
};

#define INFO_WLDP_WL(n) (n->wl)
#define INFO_WLDP_MODULE(n) (n->module)
#define INFO_WLDP_FUNDEF(n) (n->fundef)
#define INFO_WLDP_DEFAULT(n) (n->default_expr)
#define INFO_WLDP_SELWRAPPER(n) (n->sel_wrapper)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_WLDP_WL (result) = NULL;
    INFO_WLDP_MODULE (result) = NULL;
    INFO_WLDP_FUNDEF (result) = NULL;
    INFO_WLDP_DEFAULT (result) = NULL;
    INFO_WLDP_SELWRAPPER (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

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
 *           node *expr        : rhs of BLOCK_INSTR
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

    DBUG_ENTER ("CreateScalarWL");

    DBUG_ASSERT ((dim >= 0), "CreateScalarWl() used with unknown shape!");

    vec_ids = TBmakeIds (TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                               SHcreateShape (1, dim))),
                         NULL);

    vardecs = TBmakeVardec (IDS_AVIS (vec_ids), vardecs);

    for (i = 0; i < dim; i++) {
        tmp_ids
          = TBmakeIds (TBmakeAvis (ILIBtmpVar (),
                                   TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0))),
                       NULL);

        vardecs = TBmakeVardec (IDS_AVIS (tmp_ids), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
    }

    id = TBmakeId (
      TBmakeAvis (ILIBtmpVar (), TYmakeAKS (TYmakeSimpleType (btype), SHmakeShape (0))));
    vardecs = TBmakeVardec (ID_AVIS (id), vardecs);

    wl
      = TBmakeWith (TBmakePart (NULL, TBmakeWithid (vec_ids, scl_ids),
                                TBmakeGenerator (F_le, F_lt,
                                                 TCcreateZeroVector (dim, T_int),
                                                 DUPdoDupNode (array_shape), NULL, NULL)),
                    TBmakeCode (TBmakeBlock (TCmakeAssignLet (ID_AVIS (id), expr), NULL),
                                TBmakeExprs (id, NULL)),
                    TBmakeGenarray (DUPdoDupNode (array_shape), NULL));
    CODE_USED (WITH_CODE (wl))++;
    PART_CODE (WITH_PART (wl)) = WITH_CODE (wl);
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

    DBUG_ENTER ("CreateZeros");

    DBUG_ASSERT ((TYisSimple (array_type) == FALSE), "N_id is no array type!");
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
static node *
CreateArraySel (node *sel_vec, node *sel_array, info *arg_info)
{
    node *sel;
    int len_index, dim_array;

    DBUG_ENTER ("CreateArraySel");

    DBUG_ASSERT ((NODE_TYPE (sel_array) == N_id), "no N_id node found!");

    len_index = SHgetExtent (TYgetShape (IDS_NTYPE (sel_vec)), 0);
    DBUG_ASSERT ((len_index > 0), "illegal index length found!");

    dim_array = TYgetDim (ID_NTYPE (sel_array));
    DBUG_ASSERT ((dim_array > 0), "illegal array dimensionality found!");

    if (len_index > dim_array) {
        DBUG_ASSERT ((0), "illegal array selection found!");
        sel = NULL;
    } else if ((len_index == dim_array)) {
        sel
          = TBmakePrf (F_sel, TBmakeExprs (DUPdupIdsId (sel_vec),
                                           TBmakeExprs (DUPdoDupNode (sel_array), NULL)));
    } else { /* (len_index < dim_array) */

        if (INFO_WLDP_SELWRAPPER (arg_info) == NULL) {

            DSinitDeserialize (INFO_WLDP_MODULE (arg_info));

            INFO_WLDP_SELWRAPPER (arg_info)
              = DSaddSymbolByName ("sel", SET_wrapperhead, "sac2c");
            DSfinishDeserialize (INFO_WLDP_MODULE (arg_info));
        }

        DBUG_ASSERT ((INFO_WLDP_SELWRAPPER (arg_info) != NULL),
                     "no sac2c:sel wrapper found!");
        sel = TCmakeAp2 (INFO_WLDP_SELWRAPPER (arg_info), DUPdupIdsId (sel_vec),
                         DUPdoDupNode (sel_array));
    }

    DBUG_RETURN (sel);
}

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
    DBUG_ENTER ("WLDPmodule");

    INFO_WLDP_MODULE (arg_info) = arg_node;

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

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
    DBUG_ENTER ("WLDPfundef");

    INFO_WLDP_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node)) {
        FUNDEF_INSTR (arg_node) = TRAVdo (FUNDEF_INSTR (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ("WLDPwith");

    INFO_WLDP_WL (arg_info) = arg_node;

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
        DBUG_ASSERT ((NODE_TYPE (PART_GENERATOR (PART_NEXT (WITH_PART (arg_node))))
                      == N_default),
                     "Second partition is no default partition!");
    }

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
    ntype *array_type;

    DBUG_ENTER ("WLDPgenarray");

    if (GENARRAY_DEFAULT (arg_node) == NULL) {
        array_type = ID_NTYPE (EXPRS_EXPR (WITH_CEXPRS (INFO_WLDP_WL (arg_info))));

        if (TYisAKV (array_type) || TYisAKS (array_type)) {
            INFO_WLDP_DEFAULT (arg_info)
              = CreateZeros (array_type, INFO_WLDP_FUNDEF (arg_info));
        } else {
            CTIabortLine (global.linenum,
                          "Genarray with-loop with missing default expression found."
                          " Unfortunately, a default expression is necessary here"
                          " to generate code for new partitions");
        }

    } else {
        INFO_WLDP_DEFAULT (arg_info) = DUPdoDupTree (GENARRAY_DEFAULT (arg_node));

        if ((TYisAKV (ID_NTYPE (GENARRAY_DEFAULT (arg_node)))
             || TYisAKS (ID_NTYPE (GENARRAY_DEFAULT (arg_node))))) {
            GENARRAY_DEFAULT (arg_node) = FREEdoFreeTree (GENARRAY_DEFAULT (arg_node));
        }
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
    node *sel_vec, *sel_array;

    DBUG_ENTER ("WLDPmodarray");

    sel_vec = WITHID_VEC (WITH_WITHID (INFO_WLDP_WL (arg_info)));
    sel_array = MODARRAY_ARRAY (arg_node);

    if ((TYisAKV (IDS_NTYPE (sel_vec)) || TYisAKS (IDS_NTYPE (sel_vec)))
        && (TYisAKV (ID_NTYPE (sel_vec)) || TYisAKS (ID_NTYPE (sel_vec))
            || TYisAKD (ID_NTYPE (sel_vec)))) {

        INFO_WLDP_DEFAULT (arg_info) = CreateArraySel (sel_vec, sel_array, arg_info);
    } else {
        if (INFO_WLDP_SELWRAPPER (arg_info) == NULL) {

            DSinitDeserialize (INFO_WLDP_MODULE (arg_info));

            INFO_WLDP_SELWRAPPER (arg_info)
              = DSaddSymbolByName ("sel", SET_wrapperhead, "sac2c");
            DSfinishDeserialize (INFO_WLDP_MODULE (arg_info));
        }

        DBUG_ASSERT ((INFO_WLDP_SELWRAPPER (arg_info) != NULL),
                     "no sac2c:sel wrapper found!");
        INFO_WLDP_DEFAULT (arg_info)
          = TCmakeAp2 (INFO_WLDP_SELWRAPPER (arg_info), DUPdupIdsId (sel_vec),
                       DUPdoDupNode (sel_array));
    }

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
    node *_ids, *vardec, *idn, *code, *nassign;

    DBUG_ENTER ("WLDPpart");

    DBUG_ASSERT ((INFO_WLDP_DEFAULT (arg_info) != NULL),
                 "default expression is missing!");

    _ids = TBmakeIds (TBmakeAvis (ILIBtmpVar (),
                                  TYeliminateAKV (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (
                                    WITH_CEXPRS (INFO_WLDP_WL (arg_info))))))),
                      NULL);

    vardec = TBmakeVardec (IDS_AVIS (_ids), NULL);

    INFO_WLDP_FUNDEF (arg_info) = TCaddVardecs (INFO_WLDP_FUNDEF (arg_info), vardec);

    idn = DUPdupIdsId (_ids);

    /* create new N_code node  */
    nassign = TBmakeAssign (TBmakeLet (_ids, INFO_WLDP_DEFAULT (arg_info)), NULL);
    /* set correct backref to defining assignment */
    AVIS_SSAASSIGN (IDS_AVIS (_ids)) = nassign;
    code = TBmakeCode (TBmakeBlock (nassign, NULL), TBmakeExprs (idn, NULL));

    INFO_WLDP_DEFAULT (arg_info) = NULL;

    PART_NEXT (arg_node)
      = TBmakePart (code, DUPdoDupTree (PART_WITHID (arg_node)), TBmakeDefault ());
    CODE_USED (code) = 1;

    CODE_NEXT (WITH_CODE (INFO_WLDP_WL (arg_info))) = code;

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

    DBUG_ENTER ("WLDPdoWlDefaultPartition");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_module),
                 "WLDPdoDefaultParttion not started with N_module node");

    DBUG_PRINT ("WLDP", ("starting with-loop default partition"));

    arg_info = MakeInfo ();

    INFO_WLDP_MODULE (arg_info) = arg_node;

    TRAVpush (TR_wldp);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("WLDP", ("with-loop default partition complete"));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
