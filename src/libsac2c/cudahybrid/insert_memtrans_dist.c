/** <!--********************************************************************-->
 *
 * @defgroup Insert Distributed memory transfer primitives
 *
 *
 *   This module inserts distributed type conversion primitives before and after
 *   each usage of an array. The two kinds of primitives are <dist2conc> and
 *   <conc2dist>. They are used to trasfer the data of a distributed array
 *   variable to a device(host) array variable. This is essentially
 *   compiled into host<->device memory transfers in the backend. As an
 *   example:
 *
 *   a_dist = with
 *            {
 *              ... = b_dist;
 *              ... = c_dist;
 *              ... = d_dist;
 *            }:genarray( shp);
 *
 *   is transformed into:
 *
 *   b_conc = dist2conc( b_dist);
 *   c_conc = dist2conc( c_dist);
 *   d_conc = dist2conc( d_dist);
 *   a_conc = with
 *            {
 *              ... = b_dev;
 *              ... = c_dev;
 *              ... = d_dev;
 *            }:genarray( shp);
 *   a_dist = conc2dist( a_conc);
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file insert_memtrans_dist.c
 *
 * Prefix: IMEMDIST
 *
 *****************************************************************************/
#include "insert_memtrans_dist.h"

/*
 * Other includes go here
 */
#include <stdlib.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "shape.h"

#define DBUG_PREFIX "IMEMDIST"
#include "debug.h"

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
#include "shape.h"
#include "infer_memory_accesses.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool in_wl;
    bool in_kernel;
    bool cudarizable;
    node *postassigns;
    node *preassigns;
    lut_t *lut;
    node *idxavis;
    lut_t *access;

    node *dist_avis;
    nodetype let_expr_ntype;
    node *letids;
};

/*
 * INFO_FUNDEF        N_fundef node of the enclosing function
 *
 * INFO_INWL          Flag indicating whether the code currently being
 *                    traversed is in a N_with
 *
 * INFO_INKERNEL      Flag indicating whether the code currently being
 *                    traversed is in a cuda kernel N_fundef
 *
 * INFO_CUDARIZABLE   Flag indicating whether the code currently being
 *                    traversed is in a cudarizable N_with
 *
 * INFO_POSTASSIGNS   Chain of <conc2dist> that needs to be appended
 *                    at the end of the current N_assign
 *
 * INFO_PREASSIGNS    Chain of <dist2conc> that needs to be prepended
 *                    at the beginning of the current N_assign
 *
 * INFO_LUT           Lookup table storing pairs of Avis(host)->Avis(device)
 *                    e.g. Given a_dev = dist2conc( a_dist),
 *                    Avis(a_dist)->Avis(a_dev) will be stored into the table
 *
 * INFO_IDXAVIS       Avis of the index vector
 *
 * INFO_ACCESS        Lookup table with memory access information for each
 *                    withloop (from IMA subphase).
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INWL(n) (n->in_wl)
#define INFO_INKERNEL(n) (n->in_kernel)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_IDXAVIS(n) (n->idxavis)
#define INFO_ACCESS(n) (n->access)

#define INFO_DISTAVIS(n) (n->dist_avis)
#define INFO_LETEXPRNTYPE(n) (n->let_expr_ntype)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INWL (result) = FALSE;
    INFO_INKERNEL (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_IDXAVIS (result) = NULL;
    INFO_ACCESS (result) = NULL;

    INFO_DISTAVIS (result) = NULL;
    INFO_LETEXPRNTYPE (result) = N_undefined;

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

static bool CUisDistributedType (ntype *ty);
// static bool CUisConcreteTypeNew( ntype* ty);
static ntype *DISTNtypeConversion (ntype *dist_type, bool to_dev_type);
static ntype *TypeConvert (ntype *dist_type, nodetype nty, info *arg_info);
static void Createdist2conc (node *id, node *host_avis, node *dev_avis, info *arg_info);
// static void Createdistcont( node *dist_avis, info *arg_info);

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTdoInsertWithloopMemtran( node *syntax_tree)
 *
 *****************************************************************************/
node *
IMEMDISTdoInsertMemtranDist (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_imemdist);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node* CUisDistributedType( ntype* ty)
 *
 * @brief Returns whether ty is a distributed type or not
 *
 *****************************************************************************/
static bool
CUisDistributedType (ntype *ty)
{
    bool res;
    simpletype conc_type;

    DBUG_ENTER ();
    conc_type = TYgetSimpleType (TYgetScalar (ty));

    res = (conc_type == T_float_dist || conc_type == T_int_dist
           || conc_type == T_double_dist);

    DBUG_RETURN (res);
}

///** <!--********************************************************************-->
// *
// * @fn node* CUisConcreteTypeNew( ntype* ty)
// *
// * @brief Returns whether ty is a concrete type or not
// *
// *****************************************************************************/
// static bool CUisConcreteTypeNew( ntype* ty)
//{
//  bool res;
//  simpletype conc_type;
//
//  DBUG_ENTER ();
//  conc_type = TYgetSimpleType( TYgetScalar( ty));
//
//  res = (conc_type == T_float_dev || conc_type == T_int_dev ||
//         conc_type == T_double_dev || conc_type == T_float ||
//         conc_type == T_int || conc_type == T_double);
//
//  DBUG_RETURN (res);
//}
//
/** <!--********************************************************************-->
 *
 * @fn node* DISTNtypeConversion( ntype *dist_type, bool to_dev_type)
 *
 * @brief Returns the host or device type corresponding to type dist_type,
 *        according to argument to_dev_type.
 *
 *****************************************************************************/
static ntype *
DISTNtypeConversion (ntype *dist_type, bool to_dev_type)
{
    ntype *scalar_type, *conc_type = NULL;
    simpletype scalar_simple_type, res;

    DBUG_ENTER ();
    /* If the scalar type is simple, e.g. int, float ... */
    if (TYisSimple (TYgetScalar (dist_type)) && !CUisShmemTypeNew (dist_type)) {
        conc_type = TYcopyType (dist_type);
        scalar_type = TYgetScalar (conc_type);
        scalar_simple_type = TYgetSimpleType (scalar_type);
        /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
        if (to_dev_type) {
            switch (scalar_simple_type) {
            case T_int_dist:
                res = T_int_dev;
                break;
            case T_float_dist:
                res = T_float_dev;
                break;
            case T_double_dist:
                res = T_double_dev;
                break;
            default:
                DBUG_ASSERT (0,
                             "Simple type conversion found undefined dist simple type!");
            }
        } else {
            switch (scalar_simple_type) {
            case T_int_dist:
                res = T_int;
                break;
            case T_float_dist:
                res = T_float;
                break;
            case T_double_dist:
                res = T_double;
                break;
            default:
                res = scalar_simple_type;
            }
        }
        /* Set the device simple type */
        scalar_type = TYsetSimpleType (scalar_type, res);
    }

    DBUG_RETURN (conc_type);
}

/** <!--********************************************************************-->
 *
 * @fn node* TypeConvert( ntype *dist_type, nodetype nty, info *arg_info)
 *
 * @brief Returns a concrete type corresponding to the dist_type. When applied
 * in N_id, converts only AKD/AKS non-scalar arrays. Applied on N_ids, converts
 * only if the RHS is a with-loop or a primitive function.
 *
 *****************************************************************************/
static ntype *
TypeConvert (ntype *dist_type, nodetype nty, info *arg_info)
{
    ntype *conc_type = NULL;
    nodetype letexpr_ntype;

    DBUG_ENTER ();

    if (nty == N_id) {
        /* If the N_id is of known dimension and is not a scalar */
        if (TUdimKnown (dist_type) && TYgetDim (dist_type) > 0) {
            conc_type = DISTNtypeConversion (dist_type, INFO_CUDARIZABLE (arg_info));
        }
    }
    /*
     * LHS of a let
     */
    else if (nty == N_ids) {
        letexpr_ntype = INFO_LETEXPRNTYPE (arg_info);
        /* We convert the LHS if the RHS of the let is a withloop or a primitive
         * function
         */
        if ((letexpr_ntype == N_with2 || letexpr_ntype == N_prf)) {
            conc_type = DISTNtypeConversion (dist_type, INFO_CUDARIZABLE (arg_info));
        }
    } else {
        DBUG_ASSERT (0, "Neither N_id nor N_ids found in TypeConvert!");
    }

    DBUG_RETURN (conc_type);
}

/** <!--********************************************************************-->
 *
 * @fn node* Createdist2conc( node *id, node *dist_avis,
 *                           node *conc_avis, info *arg_info)
 *
 * @brief Create distributed to concrete convertion primitive function
 *
 *****************************************************************************/
static void
Createdist2conc (node *id, node *dist_avis, node *conc_avis, info *arg_info)
{
    offset_t *offset;
    void **lut_pointer;
    int extent;

    DBUG_ENTER ();

    ID_AVIS (id) = conc_avis;
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (conc_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    if (INFO_INWL (arg_info)
        && (lut_pointer = LUTsearchInLutP (INFO_ACCESS (arg_info), dist_avis)) != NULL) {
        offset = (offset_t *)*lut_pointer;
        INFO_PREASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (conc_avis, NULL),
                                     TCmakePrf4 (F_dist2conc, TBmakeId (dist_avis),
                                                 TBmakeNum (offset->min),
                                                 TBmakeNum (offset->max),
                                                 TBmakeBool (
                                                   INFO_CUDARIZABLE (arg_info)))),
                          INFO_PREASSIGNS (arg_info));
    } else {
        extent = SHgetExtent (TYgetShape (AVIS_TYPE (dist_avis)), 0);
        INFO_PREASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (conc_avis, NULL),
                                     TCmakePrf4 (F_dist2conc, TBmakeId (dist_avis),
                                                 TBmakeNum (-extent + 1),
                                                 TBmakeNum (extent),
                                                 TBmakeBool (
                                                   INFO_CUDARIZABLE (arg_info)))),
                          INFO_PREASSIGNS (arg_info));
    }

    /* Maintain SSA property */
    AVIS_SSAASSIGN (conc_avis) = INFO_PREASSIGNS (arg_info);

    /* Insert pair dist_avis->conc_avis into lookup table. */
    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), dist_avis, conc_avis);
    DBUG_RETURN ();
}

///** <!--********************************************************************-->
// *
// * @fn node* Createdistcont( node *dist_avis, info *arg_info)
// *
// * @brief Create distributed continuous blocks primitive function
// *
// *****************************************************************************/
// static void Createdistcont( node *dist_avis, info *arg_info)
//{
//  node *cont_avis;
//
//  DBUG_ENTER ();
//
//  cont_avis = TBmakeAvis(TRAVtmpVarName("cont_block"),
//                         TYmakeAKS(TYmakeSimpleType(T_int), SHmakeShape( 0)));
//
//  FUNDEF_VARDECS( INFO_FUNDEF( arg_info)) =
//  TBmakeVardec( cont_avis,  FUNDEF_VARDECS( INFO_FUNDEF( arg_info)));
//
//  INFO_PREASSIGNS( arg_info) =
//  TBmakeAssign( TBmakeLet( TBmakeIds( cont_avis, NULL),
//                          TBmakePrf( F_dist_cont_block,
//                                    TBmakeExprs( TBmakeId( dist_avis),
//                                                NULL))),
//               INFO_PREASSIGNS( arg_info));
//
//  DBUG_RETURN ();
//}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* During the main traversal, we only look at non-kernel functions */
    if (!FUNDEF_ISCUDAGLOBALFUN (arg_node) || INFO_INKERNEL (arg_info)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTap (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    if (FUNDEF_ISCUDAGLOBALFUN (AP_FUNDEF (arg_node))) {
        AP_FUNDEF (arg_node) = IMAdoInferMemoryAccesses (AP_FUNDEF (arg_node));
        INFO_ACCESS (arg_info) = FUNDEF_ACCESS (AP_FUNDEF (arg_node));

        INFO_INKERNEL (arg_info) = TRUE;
        old_fundef = INFO_FUNDEF (arg_info);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = old_fundef;
        INFO_INKERNEL (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTassign( node *arg_node, info *arg_info)
 *
 * @brief  Add newly created <dist2conc> and <conc2dist> to
 *         the assign chain.
 *
 *****************************************************************************/
node *
IMEMDISTassign (node *arg_node, info *arg_info)
{
    node *next;
    //  static int counter;

    DBUG_ENTER ();

    /*
     * Here we have to do a top-down traversal for the following reason:
     * We need to check whether there is any array variables being defined
     * in a N_with. If there is, we don't want to create a
     * dist2conc when we later come across it in the same block of code.
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* If we are no longer in a N_with, we insert
     * data transfer primitives into the syntax tree */
    if (!INFO_INWL (arg_info) && !INFO_INKERNEL (arg_info)) {
        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_POSTASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (arg_node, INFO_POSTASSIGNS (arg_info));
            INFO_POSTASSIGNS (arg_info) = NULL;
        }

        if (INFO_PREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
            INFO_PREASSIGNS (arg_info) = NULL;
        }

        //    /* add distributed variable allocations before the withs. */
        //    if (INFO_DISTAVIS(arg_info) != NULL
        //        && NODE_TYPE(ASSIGN_STMT(arg_node)) == N_let
        //        && NODE_TYPE(ASSIGN_RHS(arg_node)) == N_withs) {
        //
        //      /* add distributed variable allocation */
        //      arg_node = TCappendAssign(TBmakeAssign( TBmakeLet( TBmakeIds(
        //      INFO_DISTAVIS( arg_info),
        //                                                                   NULL),
        //                                                        TCmakePrf1(
        //                                                        F_dist_alloc,
        //                                                                   TBmakeNum(
        //                                                                   counter++))),
        //                                             NULL), arg_node);
        //      /* Maintain SSA property */
        //      AVIS_SSAASSIGN( INFO_DISTAVIS( arg_info)) = arg_node;
        //      INFO_DISTAVIS( arg_info) = NULL;
        //    }

        node *last_assign = arg_node;
        while (ASSIGN_NEXT (last_assign) != NULL) {
            last_assign = ASSIGN_NEXT (last_assign);
        }

        ASSIGN_NEXT (last_assign) = next;
        ASSIGN_NEXT (last_assign) = TRAVopt (ASSIGN_NEXT (last_assign), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETEXPRNTYPE (arg_info) = NODE_TYPE (LET_EXPR (arg_node));
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTwith2( node *arg_node, info *arg_info)
 *
 * @brief Traverse both withop and N_code of a N_with2
 *
 *****************************************************************************/
node *
IMEMDISTwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* If we are not already in a withloop */
    if (!INFO_INWL (arg_info) && !INFO_INKERNEL (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = FALSE;

        arg_node = IMAdoInferMemoryAccesses (arg_node);

        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_INWL (arg_info) = TRUE;
        INFO_ACCESS (arg_info) = WITH2_ACCESS (arg_node);

        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

        INFO_IDXAVIS (arg_info) = ID_AVIS (WITH2_VEC (arg_node));

        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

        /* Cleanup */
        INFO_INWL (arg_info) = FALSE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
        INFO_ACCESS (arg_info) = NULL;
    } else {

        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTids( node *arg_node, info *arg_info)
 *
 * @brief For N_ids needed to be type converted, create <conc2dist>.
 *
 *****************************************************************************/
node *
IMEMDISTids (node *arg_node, info *arg_info)
{
    node *new_conc_avis, *new_dist_avis, *ids_avis;
    ntype *ids_type, *conc_type;
    const char *suffix;
    prf conc2dist;

    DBUG_ENTER ();

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    /* check if ids is of a distributed type we want to convert */
    if (CUisDistributedType (ids_type)) {
        conc_type = TypeConvert (ids_type, NODE_TYPE (arg_node), arg_info);
        if (conc_type != NULL) {
            /* we can convert this ids */

            // get a new distributed avis
            if (INFO_DISTAVIS (arg_info) != NULL) {
                /* in sequential section, re-use a distributed avis found on the RHS */
                new_dist_avis = INFO_DISTAVIS (arg_info);
            } else {
                /* create new distributed avis's */
                new_dist_avis
                  = TBmakeAvis (TRAVtmpVarName (STRcat (IDS_NAME (arg_node), "_dist")),
                                TYcopyType (ids_type));
                /* add argument to function */
                FUNDEF_ARGS (INFO_FUNDEF (arg_info))
                  = TCappendArgs (FUNDEF_ARGS (INFO_FUNDEF (arg_info)),
                                  TBmakeArg (new_dist_avis, NULL));

                INFO_DISTAVIS (arg_info) = new_dist_avis;
            }

            /* create new concrete avis's */
            suffix = INFO_CUDARIZABLE (arg_info) ? "_cuda" : "_host";
            new_conc_avis
              = TBmakeAvis (TRAVtmpVarName (STRcat (IDS_NAME (arg_node), suffix)),
                            conc_type);

            /* change current avis*/
            IDS_AVIS (arg_node) = new_conc_avis;

            /* add new vardec */
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TCappendVardec (TBmakeVardec (new_conc_avis, NULL),
                                FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            /* add concrete to distributed transfer */
            conc2dist = INFO_CUDARIZABLE (arg_info) ? F_device2dist : F_host2dist_st;
            INFO_POSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (ids_avis, NULL),
                                         TCmakePrf2 (conc2dist, TBmakeId (new_conc_avis),
                                                     TBmakeId (new_dist_avis))),
                              INFO_POSTASSIGNS (arg_info));
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTid( node *arg_node, info *arg_info)
 *
 * @brief For each distributed array N_id in the N_with, either create
 *        type conversion for it (i.e. <dist2conc>) or set its N_avis to
 *        that of an already converted device array N_id depending on whether
 *        the N_id is encountered for the first time or not.
 *
 *****************************************************************************/
node *
IMEMDISTid (node *arg_node, info *arg_info)
{
    node *new_avis, *avis, *id_avis;
    ntype *conc_type, *id_type;
    const char *suffix;

    DBUG_ENTER ();

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* skip cuda shared memory variables and idx vector */
    if (!CUisShmemTypeNew (id_type) && (INFO_IDXAVIS (arg_info) != id_avis)) {
        conc_type = TypeConvert (id_type, NODE_TYPE (arg_node), arg_info);
        /* if we can convert */
        if (conc_type != NULL) {
            avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);
            if (avis != id_avis) {
                /* If the N_avis has been come across before, replace its
                 * N_avis by the device N_avis */
                ID_AVIS (arg_node) = avis;
            } else {
                /* Otherwise, create transfer */
                suffix = INFO_CUDARIZABLE (arg_info) ? "_cuda" : "_host";
                new_avis
                  = TBmakeAvis (TRAVtmpVarName (STRcat (ID_NAME (arg_node), suffix)),
                                conc_type);
                Createdist2conc (arg_node, id_avis, new_avis, arg_info);

                /*if not in with-loop, we are in a sequential section. The source
                 distributed variable, which we'll replace, is most likely this one*/
                if (!INFO_INWL (arg_info))
                    INFO_DISTAVIS (arg_info) = id_avis;
            }
            //      /* create distributed continous blocks function for with-loops */
            //      if( INFO_INWL(arg_info) / *&& INFO_CUDARIZABLE(arg_info)* /) {
            //        Createdistcont( id_avis, arg_info );
            //      }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTreturn( node *arg_node, info *arg_info)
 *
 * @brief Skip return statements
 *
 *****************************************************************************/
node *
IMEMDISTreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
