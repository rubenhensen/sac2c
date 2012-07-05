/** <!--********************************************************************-->
 *
 * @defgroup Insert CUDA memory transfer primitives
 *
 *
 *   This module inserts CUDA type conversion primitives before and after
 *   each cudarizable N_with. The two primitives are <dist2conc> and
 *   <conc2dist>. They are used to trasfer the data of a host(device) array
 *   variable to a device(host) array variable. This is essentially
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
 *   b_dev = dist2conc( b_host);
 *   c_dev = dist2conc( c_host);
 *   d_dev = dist2conc( d_host);
 *   a_dev = with
 *            {
 *              ... = b_dev;
 *              ... = c_dev;
 *              ... = d_dev;
 *            }:genarray( shp);
 *   a_host = conc2dist( a_dev);
 *
 *   Note that simple scalar variables need not be type converted since they
 *   can be passed as function parameters directly to CUDA kernels.
 *
 * @ingroup
 *
 * @{ASSIGN_STMT( arg_node)
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file cuda_type_conversion.c
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
#include "globals.h"

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
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "remove_dfms.h"
#include "infer_dfms.h"
#include "NumLookUpTable.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool in_wl;
    bool cudarizable;
    node *postassigns;
    node *preassigns;
    lut_t *lut;
    lut_t *notran;
    node *let_expr;
    bool is_modarr;
    bool in_cexprs;
    bool from_ap;
    node *letids;
    node *apids;
    node *topblock;
    nlut_t *at_nlut;
};

/*
 * INFO_FUNDEF        N_fundef node of the enclosing function
 *
 * INFO_INWL          Flag indicating whether the code currently being
 *                    traversed is in a N_with
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
 *                    e.g. Given a_dev = dist2conc( a_host),
 *                    Avis(a_host)->Avis(a_dev) will be stored into the table
 *
 * INFO_NOTRAN        Lookup table storing N_avis of arrays varaibles that
 *                    no data transfers should be created.
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INWL(n) (n->in_wl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_NOTRAN(n) (n->notran)
#define INFO_LETEXPR(n) (n->let_expr)
#define INFO_IS_MODARR(n) (n->is_modarr)
#define INFO_IN_CEXPRS(n) (n->in_cexprs)
#define INFO_FROM_AP(n) (n->from_ap)
#define INFO_LETIDS(n) (n->letids)
#define INFO_APIDS(n) (n->apids)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_AT_NLUT(n) (n->at_nlut)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INWL (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_NOTRAN (result) = NULL;
    INFO_IS_MODARR (result) = FALSE;
    INFO_IN_CEXPRS (result) = FALSE;
    INFO_FROM_AP (result) = FALSE;
    INFO_LETIDS (result) = NULL;
    INFO_APIDS (result) = NULL;
    INFO_TOPBLOCK (result) = NULL;
    INFO_AT_NLUT (result) = NULL;

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

static void Createdist2conc (node **id, node *host_avis, node *dev_avis, info *arg_info);
static bool AssignInTopBlock (node *assign, info *arg_info);

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

    /*
     * Infer dataflow masks
     */
    // syntax_tree = INFDFMSdoInferDfms( syntax_tree, HIDE_LOCALS_NEVER);

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

/** <!--********************************************************************-->
 *
 * @fn node* CUisConcreteTypeNew( ntype* ty)
 *
 * @brief Returns whether ty is a concrete type or not
 *
 *****************************************************************************/
static bool
CUisConcreteTypeNew (ntype *ty)
{
    bool res;
    simpletype conc_type;

    DBUG_ENTER ();
    conc_type = TYgetSimpleType (TYgetScalar (ty));

    res = (conc_type == T_float_dev || conc_type == T_int_dev || conc_type == T_double_dev
           || conc_type == T_float || conc_type == T_int || conc_type == T_double);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* CUdist2hostSimpleTypeConversion( simpletype sty)
 *
 * @brief Returns the host type corresponding to simpletype sty.
 *
 *****************************************************************************/
static simpletype
CUdist2hostSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
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
        res = sty;
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* CUdist2devSimpleTypeConversion( simpletype sty)
 *
 * @brief Returns the device type corresponding to simpletype sty.
 *
 *****************************************************************************/
static simpletype
CUdist2devSimpleTypeConversion (simpletype sty)
{
    simpletype res = T_unknown;

    DBUG_ENTER ();

    switch (sty) {
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
        DBUG_ASSERT (0, "Simple type conversion found undefined dist simple type!");
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node* TypeConvert( node *host_avis)
 *
 * @brief
 *
 *****************************************************************************/
static ntype *
TypeConvert (ntype *host_type, nodetype nty, info *arg_info)
{
    ntype *scalar_type, *conc_type = NULL;
    simpletype sty;
    nodetype letexpr_ntype;

    DBUG_ENTER ();

    if (nty == N_id) {
        /* If the N_id is of known dimension and is not a scalar */
        if (TUdimKnown (host_type) && TYgetDim (host_type) > 0) {
            /* If the scalar type is simple, e.g. int, float ... */
            if (TYisSimple (TYgetScalar (host_type))) {
                conc_type = TYcopyType (host_type);
                scalar_type = TYgetScalar (conc_type);
                /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
                if (INFO_CUDARIZABLE (arg_info))
                    sty = CUdist2devSimpleTypeConversion (TYgetSimpleType (scalar_type));
                else
                    sty = CUdist2hostSimpleTypeConversion (TYgetSimpleType (scalar_type));
                /* Set the device simple type */
                scalar_type = TYsetSimpleType (scalar_type, sty);
            }
        }
    }
    /*
     * LHS of a let
     */
    else if (nty == N_ids) {
        letexpr_ntype = NODE_TYPE (INFO_LETEXPR (arg_info));
        /* We convert the LHS if the RHS of the let is a withloop or a primitive
         * function
         */
        if ((letexpr_ntype == N_with || letexpr_ntype == N_prf)
            /* If the scalar type is simple, e.g. int, float ... */
            && TYisSimple (TYgetScalar (host_type))) {
            conc_type = TYcopyType (host_type);
            scalar_type = TYgetScalar (conc_type);
            /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
            if (INFO_CUDARIZABLE (arg_info))
                sty = CUdist2devSimpleTypeConversion (TYgetSimpleType (scalar_type));
            else
                sty = CUdist2hostSimpleTypeConversion (TYgetSimpleType (scalar_type));
            /* Set the device simple type */
            scalar_type = TYsetSimpleType (scalar_type, sty);
        }
    } else {
        DBUG_ASSERT (0, "Neither N_id nor N_ids found in TypeConvert!");
    }

    DBUG_RETURN (conc_type);
}

static node *
ATravWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    NLUTincNum (INFO_AT_NLUT (arg_info), ID_AVIS (arg_node), 1);

    DBUG_RETURN (arg_node);
}

static node *
ATravGenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (GENARRAY_DEFAULT (arg_node) != NULL
        && NLUTgetNum (INFO_AT_NLUT (arg_info), ID_AVIS (GENARRAY_DEFAULT (arg_node)))
             == 0) {
        DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (arg_node)),
                     "Default element of genarray is not N_id!");
        GENARRAY_DEFAULT (arg_node) = FREEdoFreeNode (GENARRAY_DEFAULT (arg_node));
        GENARRAY_DEFAULT (arg_node) = NULL;
    }

    GENARRAY_RC (arg_node) = TRAVopt (GENARRAY_RC (arg_node), arg_info);
    GENARRAY_PRC (arg_node) = TRAVopt (GENARRAY_PRC (arg_node), arg_info);
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static void
Createdist2conc (node **id, node *host_avis, node *dev_avis, info *arg_info)
{
    DBUG_ENTER ();

    ID_AVIS (*id) = dev_avis;
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (dev_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

    INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (dev_avis, NULL),
                                 TBmakePrf (F_dist2conc,
                                            TBmakeExprs (TBmakeId (host_avis), NULL))),
                      INFO_PREASSIGNS (arg_info));

    /* Maintain SSA property */
    AVIS_SSAASSIGN (dev_avis) = INFO_PREASSIGNS (arg_info);

    /* Insert pair dist_avis->conc_avis into lookup table. */
    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), host_avis, dev_avis);

    DBUG_RETURN ();
}

static bool
AssignInTopBlock (node *assign, info *arg_info)
{
    bool res = FALSE;
    node *assign_chain;

    DBUG_ENTER ();

    assign_chain = BLOCK_ASSIGNS (INFO_TOPBLOCK (arg_info));

    while (assign_chain != NULL) {
        if (assign_chain == assign) {
            res = TRUE;
            break;
        }
        assign_chain = ASSIGN_NEXT (assign_chain);
    }

    DBUG_RETURN (res);
}

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
    node *old_fundef;
    node *old_topblock;

    DBUG_ENTER ();

    /* During the main traversal, we only look at non-lac functions */
    if (!FUNDEF_ISLACFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROM_AP (arg_info)) {
            old_fundef = INFO_FUNDEF (arg_info);
            old_topblock = INFO_TOPBLOCK (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            /* Traversal of lac functions are initiated from the calling site */
            INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = old_fundef;
            INFO_TOPBLOCK (arg_info) = old_topblock;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
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
    bool traverse_lac_fun, old_from_ap;
    node *ap_args, *fundef_args;
    node *avis, *id_avis, *new_avis, *dup_avis;
    ntype *conc_type;
    node *fundef, *old_apids;
    char *tmpVarName;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    /* For us to traverse a function from calling site, it must be a
     * condictional function or a loop function and must not be the
     * recursive function call in the loop function. */
    traverse_lac_fun = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    if (traverse_lac_fun) {
        old_from_ap = INFO_FROM_AP (arg_info);
        INFO_FROM_AP (arg_info) = TRUE;

        old_apids = INFO_APIDS (arg_info);
        INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);

        if (!INFO_INWL (arg_info)) {
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        } else {
            ap_args = AP_ARGS (arg_node);
            fundef_args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

            while (ap_args != NULL) {
                DBUG_ASSERT (fundef_args != NULL, "# of Ap args != # of Fundef args!");

                DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (ap_args)) == N_id,
                             "N_ap argument is not N_id node!");

                id_avis = ID_AVIS (EXPRS_EXPR (ap_args));
                avis = LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);

                /* If the avis has NOT been come across before */
                if (avis == id_avis) {
                    DBUG_PRINT ("fundef %s, id %s\n", FUNDEF_NAME (AP_FUNDEF (arg_node)),
                                AVIS_NAME (avis));
                    /* If the id is NOT the one we don't want to create data transfer for
                     */
                    if (LUTsearchInLutPp (INFO_NOTRAN (arg_info), id_avis) == id_avis) {
                        DBUG_PRINT ("There will be transfer for %s\n",
                                    AVIS_NAME (id_avis));
                        conc_type = TypeConvert (AVIS_TYPE (id_avis), N_id, arg_info);

                        if (conc_type != NULL) {
                            tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";
                            new_avis
                              = TBmakeAvis (TRAVtmpVarName (tmpVarName), conc_type);
                            Createdist2conc (&EXPRS_EXPR (ap_args), id_avis, new_avis,
                                             arg_info);

                            dup_avis = DUPdoDupNode (new_avis);
                            AVIS_SSAASSIGN (dup_avis) = NULL;

                            INFO_LUT (arg_info)
                              = LUTinsertIntoLutP (INFO_LUT (arg_info),
                                                   ARG_AVIS (fundef_args), dup_avis);
                            ARG_AVIS (fundef_args) = dup_avis;
                            AVIS_DECL (dup_avis) = fundef_args;
                        }
                    } else {
                        DBUG_PRINT ("There will NOT be transfer for %s\n",
                                    AVIS_NAME (id_avis));
                        /* If the N_id is the one we don't want to create dist2conc for,
                         * propogate that information to the traversal of LAC functions */
                        INFO_NOTRAN (arg_info)
                          = LUTinsertIntoLutP (INFO_NOTRAN (arg_info),
                                               ARG_AVIS (fundef_args), NULL);
                    }
                } else {
                    /* If the N_avis has been come across before, replace its
                     * N_avis by the device N_avis */
                    ID_AVIS (EXPRS_EXPR (ap_args)) = avis;
                    dup_avis = DUPdoDupNode (avis);
                    AVIS_SSAASSIGN (dup_avis) = NULL;

                    /* Insert the pair of N_avis(fun arg)->N_avis(device variable)
                     * into the lookup table, so that when we later traverse the
                     * body of the fundef, old reference to the arg will be replaced
                     * by the new device varaible.  */
                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), ARG_AVIS (fundef_args),
                                           dup_avis);

                    /* Change N_avis of the fun arg to the device variable */
                    ARG_AVIS (fundef_args) = dup_avis;
                    AVIS_DECL (dup_avis) = fundef_args;
                }

                /* make sure the based type of the ap id and the fundef arg is the same
                 */
                if (TYgetSimpleType (TYgetScalar (ID_NTYPE (EXPRS_EXPR (ap_args))))
                    != TYgetSimpleType (
                         TYgetScalar (AVIS_TYPE (ARG_AVIS (fundef_args))))) {
                    TYsetSimpleType (TYgetScalar (AVIS_TYPE (ARG_AVIS (fundef_args))),
                                     TYgetSimpleType (
                                       TYgetScalar (ID_NTYPE (EXPRS_EXPR (ap_args)))));
                }

                ap_args = EXPRS_NEXT (ap_args);
                fundef_args = ARG_NEXT (fundef_args);
            } // wnd of while loop

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        }

        INFO_FROM_AP (arg_info) = old_from_ap;
        INFO_APIDS (arg_info) = old_apids;
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

    DBUG_ENTER ();

    /*
     * Here we have to do a top-down traversal for the following reason:
     * We need to check whether there is any array variables being defined
     * in a cudarizable N_with. If there is, we don't want to create a
     * host2devcice when we later come across it in the same block of code.
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* If we are no longer in a N_with, we insert
     * data transfer primitives into the AST */
    if (!INFO_INWL (arg_info)) {
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

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETEXPR (arg_info) = LET_EXPR (arg_node);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMDISTfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    node *ids, *apids;
    ntype *then_sclty, *else_sclty, *ids_sclty;
    node *ret_st, *ret_exprs, *fundef_ret;
    char *tmpVarName;

    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

        then_id = FUNCOND_THEN (arg_node);
        else_id = FUNCOND_ELSE (arg_node);
        ids = INFO_LETIDS (arg_info);

        if (TYisArray (IDS_NTYPE (ids))) {
            then_sclty = TYgetScalar (ID_NTYPE (then_id));
            else_sclty = TYgetScalar (ID_NTYPE (else_id));
            ids_sclty = TYgetScalar (IDS_NTYPE (ids));

            if (TYgetSimpleType (then_sclty) != TYgetSimpleType (else_sclty)) {
                apids = INFO_APIDS (arg_info);
                tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";

                if (CUisConcreteTypeNew (ID_NTYPE (then_id))
                    && !CUisConcreteTypeNew (ID_NTYPE (else_id))) {
                    TYsetSimpleType (else_sclty, TYgetSimpleType (then_sclty));
                    AVIS_ISCUDALOCAL (ID_AVIS (else_id)) = TRUE;
                    ID_NAME (else_id) = MEMfree (ID_NAME (else_id));
                    ID_NAME (else_id) = TRAVtmpVarName (tmpVarName);
                    TYsetSimpleType (ids_sclty, TYgetSimpleType (then_sclty));
                    IDS_NAME (ids) = MEMfree (IDS_NAME (ids));
                    IDS_NAME (ids) = TRAVtmpVarName (tmpVarName);
                } else if (CUisConcreteTypeNew (ID_NTYPE (else_id))
                           && !CUisConcreteTypeNew (ID_NTYPE (then_id))) {
                    TYsetSimpleType (then_sclty, TYgetSimpleType (else_sclty));
                    AVIS_ISCUDALOCAL (ID_AVIS (then_id)) = TRUE;
                    ID_NAME (then_id) = MEMfree (ID_NAME (then_id));
                    ID_NAME (then_id) = TRAVtmpVarName (tmpVarName);
                    TYsetSimpleType (ids_sclty, TYgetSimpleType (else_sclty));
                    IDS_NAME (ids) = MEMfree (IDS_NAME (ids));
                    IDS_NAME (ids) = TRAVtmpVarName (tmpVarName);
                } else {
                    // .... TODO ...
                    DBUG_ASSERT (0, "Found arrays of unequal types while not one"
                                    " distributed type and one concrete type!");
                }

                AVIS_ISCUDALOCAL (IDS_AVIS (ids)) = TRUE;

                ret_st = FUNDEF_RETURN (INFO_FUNDEF (arg_info));
                DBUG_ASSERT (ret_st != NULL, "N_return is null for lac fun!");
                ret_exprs = RETURN_EXPRS (ret_st);
                fundef_ret = FUNDEF_RETS (INFO_FUNDEF (arg_info));

                while (ret_exprs != NULL && fundef_ret != NULL && apids != NULL) {
                    if (ID_AVIS (EXPRS_EXPR (ret_exprs)) == IDS_AVIS (ids)) {
                        TYsetSimpleType (TYgetScalar (RET_TYPE (fundef_ret)),
                                         TYgetSimpleType (ids_sclty));
                        TYsetSimpleType (TYgetScalar (IDS_NTYPE (apids)),
                                         TYgetSimpleType (ids_sclty));
                        AVIS_ISCUDALOCAL (IDS_AVIS (apids)) = TRUE;
                        IDS_NAME (apids) = MEMfree (IDS_NAME (apids));
                        IDS_NAME (apids) = TRAVtmpVarName (tmpVarName);
                    }
                    ret_exprs = EXPRS_NEXT (ret_exprs);
                    fundef_ret = RET_NEXT (fundef_ret);
                    apids = IDS_NEXT (apids);
                }
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse both withop and N_code of a cudarizable N_with
 *
 *****************************************************************************/
node *
IMEMDISTwith (node *arg_node, info *arg_info)
{
    lut_t *old_lut;
    info *anon_info;
    node *idxvec_avis;
    ntype *idxvec_type;

    DBUG_ENTER ();

    /* If we are not already in a withloop */
    if (!INFO_INWL (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = WITH_CUDARIZABLE (arg_node);

        /************ Anonymous Traversal ************/
        /* This anon traversal remove all default elements in genarray
         * that are not used in the withloop body at all. */
        anontrav_t atrav[4] = {{N_with, &ATravWith},
                               {N_genarray, &ATravGenarray},
                               {N_id, &ATravId},
                               {0, NULL}};

        TRAVpushAnonymous (atrav, &TRAVsons);

        anon_info = MakeInfo ();

        INFO_AT_NLUT (anon_info)
          = NLUTgenerateNlut (FUNDEF_ARGS (INFO_FUNDEF (arg_info)),
                              FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));
        arg_node = TRAVdo (arg_node, anon_info);

        INFO_AT_NLUT (anon_info) = NLUTremoveNlut (INFO_AT_NLUT (anon_info));

        anon_info = FreeInfo (anon_info);
        TRAVpop ();
        /*********************************************/

        INFO_LUT (arg_info) = LUTgenerateLut ();

        INFO_INWL (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        old_lut = INFO_NOTRAN (arg_info);
        /* This lookup table stores variables that we do not
         * want to create data transfers for */
        INFO_NOTRAN (arg_info) = LUTgenerateLut ();

        /* We do not want to create a dist2conc for index vector,
         * We store pair N_id->N_empty to signal this. */
        idxvec_avis = IDS_AVIS (WITH_VEC (arg_node));
        INFO_NOTRAN (arg_info)
          = LUTinsertIntoLutP (INFO_NOTRAN (arg_info), idxvec_avis, NULL);
        if (WITH_CUDARIZABLE (arg_node)) {
            AVIS_ISCUDALOCAL (idxvec_avis) = TRUE;
        }
        // change the index vector back to host type
        idxvec_type = AVIS_TYPE (idxvec_avis);
        TYsetSimpleType (TYgetScalar (idxvec_type),
                         CUdist2hostSimpleTypeConversion (
                           TYgetSimpleType (TYgetScalar (idxvec_type))));

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /* Cleanup */
        INFO_NOTRAN (arg_info) = old_lut;
        INFO_NOTRAN (arg_info) = LUTremoveLut (INFO_NOTRAN (arg_info));
        INFO_INWL (arg_info) = FALSE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    } else {

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        INFO_NOTRAN (arg_info) = LUTinsertIntoLutP (INFO_NOTRAN (arg_info),
                                                    IDS_AVIS (WITH_VEC (arg_node)), NULL);
        AVIS_ISCUDALOCAL (IDS_AVIS (WITH_VEC (arg_node))) = TRUE;

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTcode( node *arg_node, info *arg_info)
 *
 * @brief Traverse the code block
 *
 *****************************************************************************/
node *
IMEMDISTcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    INFO_IN_CEXPRS (arg_info) = TRUE;
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);
    INFO_IN_CEXPRS (arg_info) = FALSE;

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTgenarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_genarray
 *
 *****************************************************************************/
node *
IMEMDISTgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        /* Note that we do not traverse N_genarray->shape. This is
         * because it can be an N_id node and we do not want to insert
         * <dist2conc> for it in this case. Therefore, the only sons
         * of N_genarray we traverse are the default element and the
         * potential reuse candidates. */
        if (GENARRAY_DEFAULT (arg_node) != NULL) {
            DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id,
                         "Non N_id default element found in N_genarray!");
            GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        }

        GENARRAY_RC (arg_node) = TRAVopt (GENARRAY_RC (arg_node), arg_info);

        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTmodarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_modarray
 *
 *****************************************************************************/
node *
IMEMDISTmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INWL (arg_info)) {
        DBUG_ASSERT (NODE_TYPE (MODARRAY_ARRAY (arg_node)) == N_id,
                     "Non N_id modified array found in N_modarray!");
        INFO_IS_MODARR (arg_info) = TRUE;
        MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
        INFO_IS_MODARR (arg_info) = FALSE;
        MODARRAY_RC (arg_node) = TRAVopt (MODARRAY_RC (arg_node), arg_info);
        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
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
    node *new_avis, *ids_avis;
    ntype *ids_type, *conc_type;
    char *tmpVarName;
    prf conc2dist;
    simpletype conc_simple_type, dist_simple_type;

    DBUG_ENTER ();

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    /* If the array is defined in the withloop, we do not create
     * a dist2conc transfer for it (if we can access it outside, we should...)*/
    if (INFO_INWL (arg_info)) {
        if (!TUisScalar (ids_type)) {
            INFO_NOTRAN (arg_info)
              = LUTinsertIntoLutP (INFO_NOTRAN (arg_info), ids_avis, NULL);

            AVIS_ISCUDALOCAL (IDS_AVIS (arg_node)) = TRUE;
            /* If the ids' type is not of shared
             * memory type, we change its base type from distributed to concrete */
            if (!CUisShmemTypeNew (ids_type)) {
                dist_simple_type = TYgetSimpleType (TYgetScalar (ids_type));
                if (INFO_CUDARIZABLE (arg_info)) {
                    conc_simple_type = CUdist2devSimpleTypeConversion (dist_simple_type);
                } else {
                    conc_simple_type = CUdist2hostSimpleTypeConversion (dist_simple_type);
                }
                TYsetSimpleType (TYgetScalar (ids_type), conc_simple_type);
            }
        }
    } else if (CUisDistributedType (ids_type)) {
        conc_type = TypeConvert (ids_type, NODE_TYPE (arg_node), arg_info);
        if (conc_type != NULL) {
            tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";
            new_avis = TBmakeAvis (TRAVtmpVarName (tmpVarName), conc_type);
            IDS_AVIS (arg_node) = new_avis;
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            conc2dist = INFO_CUDARIZABLE (arg_info) ? F_device2dist : F_host2dist_st;
            INFO_POSTASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (ids_avis, NULL),
                                         TBmakePrf (conc2dist,
                                                    TBmakeExprs (TBmakeId (new_avis),
                                                                 NULL))),
                              INFO_POSTASSIGNS (arg_info));
            /* Maintain SSA property */
            AVIS_SSAASSIGN (new_avis) = AVIS_SSAASSIGN (ids_avis);
            AVIS_SSAASSIGN (ids_avis) = INFO_POSTASSIGNS (arg_info);
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
    node *ssaassign;
    char *tmpVarName;

    DBUG_ENTER ();

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* if we are in a N_with */
    // if( INFO_INWL( arg_info)) {
    avis = LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);

    /* If the N_avis node hasn't been come across before AND the id is
     * NOT in cexprs. This is because we don't want to create a dist2conc
     * for N_id in the cexprs. However, if the N_id has been come across
     * before, even if it's in cexprs, we still need to replace its avis
     * by the new avis, i.e. the transferred device variable (See the
     * "else" case). This might happen that the N_id in cexprs is not
     * a scalar and it's a default element of the withloop. Therefore,
     * a early traverse of the withop will insert a dist2conc for this
     * N_id and we here simply need to set it's avis to the device variable
     * avis. (This is fix to the bug discovered in compiling tvd2d.sac) */

    // if( avis == id_avis) {
    /* Definition of the N_id must not be in the same block as
     * reference of the N_id. Otherwise, no dist2conc will be
     * created. e.g.
     *
     * a = with
     *     {
     *       b = [x, y, z];
     *       ...
     *       ... = prf( b);
     *     }:genarray();
     *
     * We do not create b_dev = dist2conc( b) in this case.
     */

    ssaassign = AVIS_SSAASSIGN (avis);

    if (((INFO_IN_CEXPRS (arg_info) && ssaassign != NULL
          && AssignInTopBlock (ssaassign, arg_info))
         || !INFO_IN_CEXPRS (arg_info))
        && !CUisShmemTypeNew (id_type)
        && LUTsearchInLutPp (INFO_NOTRAN (arg_info), id_avis) == id_avis) {
        conc_type = TypeConvert (id_type, NODE_TYPE (arg_node), arg_info);
        if (conc_type != NULL) {
            tmpVarName = INFO_CUDARIZABLE (arg_info) ? "dev" : "host";
            new_avis = TBmakeAvis (TRAVtmpVarName (tmpVarName), conc_type);
            Createdist2conc (&arg_node, id_avis, new_avis, arg_info);
        }
    }
    /*    }
        else {
          / * If the N_avis has been come across before, replace its
           * N_avis by the device N_avis * /
          ID_AVIS( arg_node) = avis;
        }*/
    //}
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
