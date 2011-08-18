/** <!--********************************************************************-->
 *
 * @defgroup Insert CUDA memory transfer primitives
 *
 *
 *   This module inserts CUDA type conversion primitives before and after
 *   each cudarizable N_with. The two primitives are <host2device> and
 *   <device2host>. They are used to trasfer the data of a host(device) array
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
 *   b_dev = host2device( b_host);
 *   c_dev = host2device( c_host);
 *   d_dev = host2device( d_host);
 *   a_dev = with
 *            {
 *              ... = b_dev;
 *              ... = c_dev;
 *              ... = d_dev;
 *            }:genarray( shp);
 *   a_host = device2host( a_dev);
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
 * Prefix: IMEM
 *
 *****************************************************************************/
#include "insert_memory_transfers.h"

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

#define DBUG_PREFIX "UNDEFINED"
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    bool in_cudawl;
    bool create_d2h;
    node *postassigns;
    node *preassigns;
    lut_t *lut;
    lut_t *notran;
    node *let_expr;
    bool is_modarr;
    bool in_cexprs;
    bool from_ap;
};

/*
 * INFO_FUNDEF        N_fundef node of the enclosing function
 *
 * INFO_INCUDAWL      Flag indicating whether the code currently being
 *                    traversed is in a cudarizable N_with
 *
 * INFO_CREATE_D2H    Flag indicating whether <device2host> needs to be
 *                    created for the N_let->N_ids
 *
 * INFO_POSTASSIGNS   Chain of <device2host> that needs to be appended
 *                    at the end of the current N_assign
 *
 * INFO_PREASSIGNS    Chain of <host2device> that needs to be prepended
 *                    at the beginning of the current N_assign
 *
 * INFO_LUT           Lookup table storing pairs of Avis(host)->Avis(device)
 *                    e.g. Given a_dev = host2device( a_host),
 *                    Avis(a_host)->Avis(a_dev) will be stored into the table
 *
 * INFO_NOTRAN        Lookup table storing N_avis of arrays varaibles that
 *                    no data transfers should be created.
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDAWL(n) (n->in_cudawl)
#define INFO_CREATE_D2H(n) (n->create_d2h)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_NOTRAN(n) (n->notran)
#define INFO_LETEXPR(n) (n->let_expr)
#define INFO_IS_MODARR(n) (n->is_modarr)
#define INFO_IN_CEXPRS(n) (n->in_cexprs)
#define INFO_FROM_AP(n) (n->from_ap)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_CREATE_D2H (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_NOTRAN (result) = NULL;
    INFO_IS_MODARR (result) = FALSE;
    INFO_IN_CEXPRS (result) = FALSE;
    INFO_FROM_AP (result) = FALSE;

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

static void CreateHost2Device (node **id, node *host_avis, node *dev_avis,
                               info *arg_info);

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *IMEMdoInsertMemoryTransfers( node *syntax_tree)
 *
 *****************************************************************************/
node *
IMEMdoInsertMemoryTransfers (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    /*
     * Infer dataflow masks
     */
    // syntax_tree = INFDFMSdoInferDfms( syntax_tree, HIDE_LOCALS_NEVER);

    TRAVpush (TR_imem);
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
 * @fn node* TypeConvert( node *host_avis)
 *
 * @brief
 *
 *****************************************************************************/
static ntype *
TypeConvert (ntype *host_type, nodetype nty, info *arg_info)
{
    ntype *scalar_type, *dev_type = NULL;
    simpletype sty;

    DBUG_ENTER ();

    if (nty == N_id) {
        /* If the N_ids is of known dimension and is not a scalar */
        DBUG_ASSERT (TUdimKnown (host_type), "AUD N_id found in cudarizable N_with!");
        if (TYgetDim (host_type) > 0) {
            /* If the scalar type is simple, e.g. int, float ... */
            if (TYisSimple (TYgetScalar (host_type))) {
                dev_type = TYcopyType (host_type);
                scalar_type = TYgetScalar (dev_type);
                /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
                sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
                /* Set the device simple type */
                scalar_type = TYsetSimpleType (scalar_type, sty);
            }
        }
    }
    /* If the node to be type converted is N_ids, its original type
     * can be AUD as well as long as the N_with on the RHS is cudarizable.
     * The reason a cudarizbale can produce a AUD result illustrated by
     * the following example:
     *
     *   cond_fun()
     *   {
     *     int[*] aa;
     *     int bb;
     *
     *     if( cond) {
     *       aa = with {}:genarray( shp); (cudarizable N_with)
     *     }
     *     else {
     *       bb = 1;
     *     }
     *     ret = cond ? aa : bb;
     *   }
     *
     */
    else if (nty == N_ids) {
        if (NODE_TYPE (INFO_LETEXPR (arg_info)) == N_with) {
            /* If the scalar type is simple, e.g. int, float ... */
            if (WITH_CUDARIZABLE (INFO_LETEXPR (arg_info))
                && TYisSimple (TYgetScalar (host_type))) {
                dev_type = TYcopyType (host_type);
                scalar_type = TYgetScalar (dev_type);
                /* Get the corresponding device simple type e.g. int_dev, float_dev...*/
                sty = CUh2dSimpleTypeConversion (TYgetSimpleType (scalar_type));
                /* Set the device simple type */
                scalar_type = TYsetSimpleType (scalar_type, sty);
            }
        }
    } else {
        DBUG_ASSERT (0, "Neither N_id nor N_ids found in TypeConvert!");
    }

    DBUG_RETURN (dev_type);
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
 * @fn node *IMEMfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;

    DBUG_ENTER ();

    /* During the main traversal, we only look at non-lac functions */
    if (!FUNDEF_ISLACFUN (arg_node)) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;

        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        if (INFO_FROM_AP (arg_info)) {
            old_fundef = INFO_FUNDEF (arg_info);
            INFO_FUNDEF (arg_info) = arg_node;
            /* Traversal of lac functions are initiated from the calling site */
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
            INFO_FUNDEF (arg_info) = old_fundef;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMap (node *arg_node, info *arg_info)
{
    bool traverse_lac_fun, old_from_ap;
    node *ap_args, *fundef_args;
    node *avis, *id_avis, *new_avis, *dup_avis;
    ntype *dev_type;
    node *fundef;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);

    /* For us to traverse a function from calling site, it must be a
     * condictional function or a loop function and must not be the
     * recursive function call in the loop function. */
    traverse_lac_fun = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    if (traverse_lac_fun) {
        old_from_ap = INFO_FROM_AP (arg_info);
        INFO_FROM_AP (arg_info) = TRUE;
        if (!INFO_INCUDAWL (arg_info)) {
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
                    /* If the id is NOT the one we don't want to create data transfer for
                     */
                    if (LUTsearchInLutPp (INFO_NOTRAN (arg_info), id_avis) == id_avis) {
                        dev_type = TypeConvert (AVIS_TYPE (id_avis), N_id, arg_info);

                        if( dev_type != NULL /* &&
                NODE_TYPE( AVIS_DECL( avis)) == N_arg */) {
                            new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                            CreateHost2Device (&EXPRS_EXPR (ap_args), id_avis, new_avis,
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
                        /* If the N_id is the one we don't want to create host2device for,
                         * propogate that information to the traversal of LAC functions */
                        INFO_NOTRAN (arg_info)
                          = LUTinsertIntoLutP (INFO_NOTRAN (arg_info),
                                               ARG_AVIS (fundef_args), TBmakeEmpty ());
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

                ap_args = EXPRS_NEXT (ap_args);
                fundef_args = ARG_NEXT (fundef_args);
            }

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        }

        INFO_FROM_AP (arg_info) = old_from_ap;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMassign( node *arg_node, info *arg_info)
 *
 * @brief  Add newly created <host2device> and <device2host> to
 *         the assign chain.
 *
 *****************************************************************************/
node *
IMEMassign (node *arg_node, info *arg_info)
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

    /* If we are no longer in a cudarizable N_with, we insert
     * data transfer primitives into the AST */
    if (!INFO_INCUDAWL (arg_info)) {
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
 * @fn node *IMEMlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
IMEMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETEXPR (arg_info) = LET_EXPR (arg_node);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse both withop and N_code of a cudarizable N_with
 *
 *****************************************************************************/
node *
IMEMwith (node *arg_node, info *arg_info)
{
    lut_t *old_lut;

    DBUG_ENTER ();

    /* If the N_with is cudarizable */
    if (WITH_CUDARIZABLE (arg_node)) {
        INFO_LUT (arg_info) = LUTgenerateLut ();
        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        old_lut = INFO_NOTRAN (arg_info);
        INFO_NOTRAN (arg_info) = LUTgenerateLut ();

        /* we do not want to create a host2device for index vector */
        INFO_NOTRAN (arg_info)
          = LUTinsertIntoLutP (INFO_NOTRAN (arg_info), IDS_AVIS (WITH_VEC (arg_node)),
                               TBmakeEmpty ());

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_NOTRAN (arg_info) = old_lut;
        INFO_NOTRAN (arg_info) = LUTremoveLut (INFO_NOTRAN (arg_info));

        INFO_INCUDAWL (arg_info) = FALSE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

        /* We need to create <device2host> for N_ids on the LHS */
        INFO_CREATE_D2H (arg_info) = TRUE;
    } else if (INFO_INCUDAWL (arg_info)) {
        /* If we are already in a cudarizable N_with but the
         * N_with itself is not a cudarizable N_with */

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        INFO_NOTRAN (arg_info)
          = LUTinsertIntoLutP (INFO_NOTRAN (arg_info), IDS_AVIS (WITH_VEC (arg_node)),
                               TBmakeEmpty ());

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    } else {
        /* The following traversal has been commented out because if the outermost
         * N_with is not cudarizable, none of its inner N_withs (if
         * there is any) will be cudarizable since we only cudarize
         * the outermost N_with. */

        /* WITH_CODE( arg_node) = TRAVdo( WITH_CODE( arg_node), arg_info); */
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMcode( node *arg_node, info *arg_info)
 *
 * @brief Traverse the code block
 *
 *****************************************************************************/
node *
IMEMcode (node *arg_node, info *arg_info)
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
 * @fn node *IMEMgenarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_genarray
 *
 *****************************************************************************/
node *
IMEMgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INCUDAWL (arg_info)) {
        /* Note that we do not traverse N_genarray->shape. This is
         * because it can be an N_id node and we do not want to insert
         * <host2device> for it in this case. Therefore, the only son
         * of N_genarray we traverse is the default element. */
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
 * @fn node *IMEMmodarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_modarray
 *
 *****************************************************************************/
node *
IMEMmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INCUDAWL (arg_info)) {
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
 * @fn node *IMEMids( node *arg_node, info *arg_info)
 *
 * @brief For N_ids needed to be type converted, create <device2host>.
 *
 *****************************************************************************/
node *
IMEMids (node *arg_node, info *arg_info)
{
    node *new_avis, *ids_avis;
    ntype *ids_type, *dev_type;

    DBUG_ENTER ();

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    /* If the array is define in Cuda wl, we do not create
     * a host2device transfer for it */
    if (INFO_INCUDAWL (arg_info)) {
        if (TYisArray (ids_type)) {
            INFO_NOTRAN (arg_info)
              = LUTinsertIntoLutP (INFO_NOTRAN (arg_info), ids_avis, TBmakeEmpty ());
        }
    } else {
        if (INFO_CREATE_D2H (arg_info)) {
            dev_type = TypeConvert (ids_type, NODE_TYPE (arg_node), arg_info);
            if (dev_type != NULL) {
                new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                IDS_AVIS (arg_node) = new_avis;
                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

                INFO_POSTASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (ids_avis, NULL),
                                             TBmakePrf (F_device2host,
                                                        TBmakeExprs (TBmakeId (new_avis),
                                                                     NULL))),
                                  INFO_POSTASSIGNS (arg_info));
                /* Maintain SSA property */
                AVIS_SSAASSIGN (new_avis) = AVIS_SSAASSIGN (ids_avis);
                AVIS_SSAASSIGN (ids_avis) = INFO_POSTASSIGNS (arg_info);
            }
            // IDS_NEXT( arg_node) = TRAVopt( IDS_NEXT( arg_node), arg_info);
            INFO_CREATE_D2H (arg_info) = FALSE;
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMid( node *arg_node, info *arg_info)
 *
 * @brief For each host array N_id in the cudarizable N_with, either create
 *        type conversion for it (i.e. <host2device>) or set its N_avis to
 *        that of an already converted device array N_id depending on whether
 *        the N_id is encountered for the first time or not.
 *
 *****************************************************************************/
node *
IMEMid (node *arg_node, info *arg_info)
{
    node *new_avis, *avis, *id_avis;
    ntype *dev_type, *id_type;

    DBUG_ENTER ();

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* if we are in cudarizable N_with */
    if (INFO_INCUDAWL (arg_info)) {
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);

        /* If the N_avis node hasn't been come across before AND the id is
         * NOT in cexprs. This is because we don't want to create a host2device
         * for N_id in the cexprs. However, if the N_id has been come across
         * before, even if it's in cexprs, we still need to replace its avis
         * by the new avis, i.e. the transferred device variable (See the
         * "else" case). This might happen that the N_id in cexprs is not
         * a scalar and it's a default element of the withloop. Therefore,
         * a early traverse of the withop will insert a host2device for this
         * N_id and we here simply need to set it's avis to the device variable
         * avis. (This is fix to the bug discovered in compiling tvd2d.sac) */

        if (avis == id_avis && !INFO_IN_CEXPRS (arg_info)) {
            dev_type = TypeConvert (id_type, NODE_TYPE (arg_node), arg_info);
            /* Definition of the N_id must not be in the same block as
             * reference of the N_id. Otherwise, no host2device will be
             * created. e.g.
             *
             * a = with
             *     {
             *       b = [x, y, z];
             *       ...
             *       ... = prf( b);
             *     }:genarray();
             *
             * We do not create b_dev = host2device( b) in this case.
             */
            if (dev_type != NULL
                && (/* NODE_TYPE( AVIS_DECL( avis)) == N_arg || */
                    /* INFO_IS_MODARR( arg_info) || */
                    LUTsearchInLutPp (INFO_NOTRAN (arg_info), id_avis) == id_avis)) {
                new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                CreateHost2Device (&arg_node, id_avis, new_avis, arg_info);
            }
        } else {
            /* If the N_avis has been come across before, replace its
             * N_avis by the device N_avis */
            ID_AVIS (arg_node) = avis;
        }
    }
    DBUG_RETURN (arg_node);
}

static void
CreateHost2Device (node **id, node *host_avis, node *dev_avis, info *arg_info)
{
    DBUG_ENTER ();

    ID_AVIS (*id) = dev_avis;
    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (dev_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (dev_avis, NULL),
                                 TBmakePrf (F_host2device,
                                            TBmakeExprs (TBmakeId (host_avis), NULL))),
                      INFO_PREASSIGNS (arg_info));

    /* Maintain SSA property */
    AVIS_SSAASSIGN (dev_avis) = INFO_PREASSIGNS (arg_info);

    /* Insert pair host_avis->dev_avis into lookup table. */
    INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), host_avis, dev_avis);

    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
