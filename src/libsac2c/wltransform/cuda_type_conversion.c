/** <!--********************************************************************-->
 *
 * @defgroup Insert CUDA type conversion primitives
 *
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
 *            }:genarray( shp);
 *   a_host = device2host( a_dev);
 *
 *   Note that simple scalar variables need not be type converted since they
 *   can be passed as function parameters directly to CUDA kernels.
 *
 * @ingroup
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file cuda_type_conversion.c
 *
 * Prefix: CUTYCV
 *
 *****************************************************************************/
#include "annotate_cuda_withloop.h"

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
    node *let_expr;
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
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDAWL(n) (n->in_cudawl)
#define INFO_CREATE_D2H(n) (n->create_d2h)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_LETEXPR(n) (n->let_expr)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_CREATE_D2H (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
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
 * @fn node *CUTYCVdoCUDAtypeConversion( node *syntax_tree)
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

    DBUG_ENTER ("TypeConvert");

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
        DBUG_ASSERT ((0), "Neither N_id nor N_ids passed to TypeConvert!");
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
 * @fn node *CUTYCVfundef( node *arg_node, info *arg_info)
 *
 * @brief
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
 * @fn node *CUTYCVassign( node *arg_node, info *arg_info)
 *
 * @brief  Add newly created <host2device> and <device2host> to
 *         the assign chain.
 *
 *****************************************************************************/
node *
CUTYCVassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVassign");

    /* Bottom-up traversal */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /* If we finish traversing a cudarizable N_with, insert <host2device>
     * and <device2host> (if there's any) into the AST. */
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
 * @fn node *CUTYCVlet( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUTYCVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETEXPR (arg_info) = LET_EXPR (arg_node);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTYCVwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse both withop and N_code of a cudarizable N_with
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
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

        /* We need to create <device2host> for N_ids on the LHS */
        INFO_CREATE_D2H (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTYCVcode( node *arg_node, info *arg_info)
 *
 * @brief Traverse the code block
 *
 *****************************************************************************/
node *
CUTYCVcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVcode");

    if (INFO_INCUDAWL (arg_info)) {
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
        CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTYCVgenarray( node *arg_node, info *arg_info)
 *
 * @brief Traverse default element of a N_genarray
 *
 *****************************************************************************/
node *
CUTYCVgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUTYCVgenarray");

    if (INFO_INCUDAWL (arg_info)) {
        /* Note that we do not traverse N_genarray->shape. This is
         * because it can be an N_id node and we do not want to insert
         * <host2device> for it in this case. Therefore, the only son
         * of N_genarray we traverse is the default element. */
        if (GENARRAY_DEFAULT (arg_node) != NULL) {
            DBUG_ASSERT ((NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id),
                         "Non N_id default element found in N_genarray!");
            GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        }
        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTYCVids( node *arg_node, info *arg_info)
 *
 * @brief For N_ids needed to be type converted, create <device2host>.
 *
 *****************************************************************************/
node *
CUTYCVids (node *arg_node, info *arg_info)
{
    node *new_avis, *ids_avis;
    ntype *ids_type, *dev_type;

    DBUG_ENTER ("CUTYCVids");

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    if (INFO_CREATE_D2H (arg_info)) {
        dev_type = TypeConvert (ids_type, NODE_TYPE (arg_node), arg_info);
        if (dev_type != NULL) {
            new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
            IDS_AVIS (arg_node) = new_avis;
            FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
            DBUG_PRINT ("CUTYCV",
                        ("Creating F_device2host for N_ids %s", AVIS_NAME (ids_avis)));
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
        IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);
        INFO_CREATE_D2H (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUTYCVid( node *arg_node, info *arg_info)
 *
 * @brief For each host array N_id in the cudarizable N_with, either create
 *        type conversion for it (i.e. <host2device>) or set its N_avis to
 *        that of an already converted device array N_id depending on whether
 *        the N_id is encountered for the first time or not.
 *
 *****************************************************************************/
node *
CUTYCVid (node *arg_node, info *arg_info)
{
    node *new_avis, *avis, *id_avis;
    ntype *dev_type, *id_type;

    DBUG_ENTER ("CUTYCVid");

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* if we are in cudarizable N_with */
    if (INFO_INCUDAWL (arg_info)) {
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);
        /* if the N_avis node hasn't been come across before */
        if (avis == id_avis) {
            dev_type = TypeConvert (id_type, NODE_TYPE (arg_node), arg_info);
            if (dev_type != NULL) {
                new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                ID_AVIS (arg_node) = new_avis;
                FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (new_avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));
                INFO_PREASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                             TBmakePrf (F_host2device,
                                                        TBmakeExprs (TBmakeId (id_avis),
                                                                     NULL))),
                                  INFO_PREASSIGNS (arg_info));
                /* Maintain SSA property */
                AVIS_SSAASSIGN (new_avis) = INFO_PREASSIGNS (arg_info);
                /* Insert pair host_avis->dev_avis into lookup table. */
                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), id_avis, new_avis);
            }
        } else {
            /* If the N_avis has been come across before, replace its
             * N_avis by the device N_avis */
            ID_AVIS (arg_node) = avis;
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
