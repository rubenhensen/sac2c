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
    bool cudarizable;
    node *postassigns;
    node *preassigns;
    lut_t *lut;
    lut_t *access;
    node *device_number;
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
 *                    e.g. Given a_dev = dist2conc( a_dist),
 *                    Avis(a_dist)->Avis(a_dev) will be stored into the table
 *
 * INFO_ACCESS        Lookup table with memory access information for each
 *                    withloop (from IMA subphase).
 *
 * INFO_DEVICENUMBER  N_id for the variable with the CUDA device number.
 *
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INWL(n) (n->in_wl)
#define INFO_CUDARIZABLE(n) (n->cudarizable)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_ACCESS(n) (n->access)
#define INFO_DEVICENUMBER(n) (n->device_number)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INWL (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_ACCESS (result) = NULL;
    INFO_DEVICENUMBER (result) = NULL;

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

static bool CUisDistributedType (ntype *ty);
static ntype *DISTNtypeConversion (ntype *dist_type, bool to_dev_type);
static bool PrfNeedsTransfer (node *rhs);
static bool ApNeedsTransfer (node *rhs);

/** <!--********************************************************************-->
 *
 * @fn bool CUisDistributedType( ntype* ty)
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
 * @fn ntype* DISTNtypeConversion( ntype *dist_type, bool to_dev_type)
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
                res = scalar_simple_type;
                DBUG_UNREACHABLE (
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
 * @fn bool PrfNeedsTransfer( node* rhs);
 *
 * @brief Returns true if rhs is a N_prf and the prf is one of those for which
 * we need to convert the arguments/result.
 *
 *****************************************************************************/
static bool
PrfNeedsTransfer (node *rhs)
{
    bool res;

    DBUG_ENTER ();

    if (NODE_TYPE (rhs) == N_prf) {
        switch (PRF_PRF (rhs)) {
        case F_alloc:
        case F_noop:
        case F_inc_rc:
        case F_dec_rc:
        case F_free:
            res = FALSE;
            break;
        default:
            res = TRUE;
            break;
        }
    } else {
        res = FALSE;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool ApNeedsTransfer( node* rhs);
 *
 * @brief Returns true if rhs is a N_ap and the function has been imported.
 *
 *****************************************************************************/
static bool
ApNeedsTransfer (node *rhs)
{
    bool res;

    DBUG_ENTER ();

    if (NODE_TYPE (rhs) == N_ap)
        res = !FUNDEF_ISLOCAL (AP_FUNDEF (rhs));
    else
        res = FALSE;

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
    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTblock( node *arg_node, info *arg_info)
 *
 * @brief  Generate a LUT for each block.
 *
 *****************************************************************************/
node *
IMEMDISTblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (!INFO_INWL (arg_info)) {
        INFO_LUT (arg_info) = LUTgenerateLut ();
        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    } else {
        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
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
     * in a N_with. If there is, we don't want to create a
     * dist2conc when we later come across it in the same block of code.
     */
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    /* If we are no longer in a N_with, we insert
     * data transfer primitives into the syntax tree */
    if (!INFO_INWL (arg_info)) {
        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        if (INFO_PREASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
            INFO_PREASSIGNS (arg_info) = NULL;
        }

        if (INFO_POSTASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (arg_node, INFO_POSTASSIGNS (arg_info));
            INFO_POSTASSIGNS (arg_info) = NULL;
        }

        next = TRAVopt (next, arg_info);
        arg_node = TCappendAssign (arg_node, next);
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
    nodetype expr_ty;
    bool old_inwl, old_cudarizable;
    lut_t *old_lut;
    DBUG_ENTER ();

    expr_ty = NODE_TYPE (LET_EXPR (arg_node));
    if (expr_ty == N_with2 || expr_ty == N_with || ApNeedsTransfer (LET_EXPR (arg_node))
        || PrfNeedsTransfer (LET_EXPR (arg_node))) {
        old_inwl = INFO_INWL (arg_info);
        old_lut = INFO_ACCESS (arg_info);
        old_cudarizable = INFO_CUDARIZABLE (arg_info);

        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
        LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

        /* these are reset only here as the ids traversal needs these flags.*/
        INFO_CUDARIZABLE (arg_info) = old_cudarizable;
        INFO_ACCESS (arg_info) = old_lut;
        INFO_INWL (arg_info) = old_inwl;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTap( node *arg_node, info *arg_info)
 *
 * @brief Create transfer if function's arguments are not distributed arrays
 *
 *****************************************************************************/
node *
IMEMDISTap (node *arg_node, info *arg_info)
{
    node *expr, *arg;
    ntype *expr_type, *arg_type;

    DBUG_ENTER ();

    arg = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    for (expr = AP_ARGS (arg_node); expr != NULL; expr = EXPRS_NEXT (expr)) {
        if (NODE_TYPE (EXPRS_EXPR (expr)) == N_id) {
            expr_type = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (expr)));
            arg_type = AVIS_TYPE (ARG_AVIS (arg));
            if (CUisDistributedType (expr_type) && !CUisDistributedType (arg_type)) {
                EXPRS_EXPR (expr) = TRAVdo (EXPRS_EXPR (expr), arg_info);
            }
        }
    }

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
    node *idxavis;

    DBUG_ENTER ();

    /* If we are not already in a withloop */
    if (!INFO_INWL (arg_info)) {
        arg_node = IMAdoInferMemoryAccesses (arg_node);

        INFO_INWL (arg_info) = TRUE;
        INFO_ACCESS (arg_info) = WITH2_ACCESS (arg_node);

        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

        idxavis = ID_AVIS (WITH2_VEC (arg_node));
        AVIS_TYPE (idxavis) = DISTNtypeConversion (AVIS_TYPE (idxavis), FALSE);

        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    } else {

        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

        idxavis = ID_AVIS (WITH2_VEC (arg_node));
        AVIS_TYPE (idxavis) = DISTNtypeConversion (AVIS_TYPE (idxavis), FALSE);
        AVIS_ISCUDALOCAL (idxavis) = INFO_CUDARIZABLE (arg_info);

        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTwith( node *arg_node, info *arg_info)
 *
 * @brief Traverse both withop and N_code of a N_with
 *
 *****************************************************************************/
node *
IMEMDISTwith (node *arg_node, info *arg_info)
{
    node *idxavis;

    DBUG_ENTER ();

    /* If we are not already in a withloop */
    if (!INFO_INWL (arg_info)) {
        INFO_CUDARIZABLE (arg_info) = WITH_CUDARIZABLE (arg_node);

        arg_node = IMAdoInferMemoryAccesses (arg_node);

        INFO_INWL (arg_info) = TRUE;
        INFO_ACCESS (arg_info) = WITH_ACCESS (arg_node);

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    } else {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        idxavis = ID_AVIS (WITH_VEC (arg_node));
        AVIS_TYPE (idxavis) = DISTNtypeConversion (AVIS_TYPE (idxavis), FALSE);
        AVIS_ISCUDALOCAL (idxavis) = INFO_CUDARIZABLE (arg_info);

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
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
    node *new_conc_avis, *ids_avis, *prf_node;
    ntype *ids_type, *conc_type;
    const char *suffix;
    int stop;

    DBUG_ENTER ();

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    /* check if ids is of a distributed type we want to convert */
    if (CUisDistributedType (ids_type)) {
        new_conc_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ids_avis);
        if (new_conc_avis != ids_avis) {
            /* If the N_avis has been come across before, replace its
             * N_avis by the concrete N_avis */
            IDS_AVIS (arg_node) = new_conc_avis;
        } else {
            conc_type = DISTNtypeConversion (ids_type, INFO_CUDARIZABLE (arg_info));

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
        }

        /* add concrete to distributed transfer */
        if (INFO_CUDARIZABLE (arg_info)) {
            /* the starting and end blocks to transfer depend on the data available.
             * We insert the id's for those in a later traversal (IAL)
             */
            prf_node = TCmakePrf4 (F_device2dist, TBmakeId (ids_avis),
                                   DUPdoDupNode (INFO_DEVICENUMBER (arg_info)),
                                   TBmakeId (ids_avis), TBmakeId (ids_avis));
        } else if (INFO_INWL (arg_info)) {
            /* The starting and ending blocks depend only on the scheduler. The ICM
             macros take care of it. */
            prf_node = TCmakePrf1 (F_host2dist_spmd, TBmakeId (ids_avis));
        } else {
            /* We need to insert the start and end blocks to transfer. Assuming we are
             * transferring the whole array for now. */
            // TODO: infer which blocks to copy rather than all of them
            stop = SHgetExtent (TYgetShape (AVIS_TYPE (ids_avis)), 0);
            prf_node = TCmakePrf3 (F_host2dist_st, TBmakeId (ids_avis), TBmakeNum (0),
                                   TBmakeNum (stop));
        }

        INFO_POSTASSIGNS (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (ids_avis, NULL), prf_node),
                          INFO_POSTASSIGNS (arg_info));
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
    node *new_avis, *id_avis, *vardecs, *prf_node, *last_arg;
    void **lut_pointer;
    ntype *conc_type, *id_type;
    const char *suffix;
    char *name;
    int start, stop;
    offset_t *offset;
    prf dist2conc;

    DBUG_ENTER ();

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* skip non-distributed variables */
    if (CUisDistributedType (id_type)) {
        new_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);
        if (new_avis == id_avis) {
            /* If the avises are equal, this is the first occurence of this variable.
             * Therefore, we create transfer */
            DBUG_PRINT ("ID %s not found, creating transfer", ID_NAME (arg_node));
            suffix = INFO_CUDARIZABLE (arg_info) ? "_cuda" : "_host";
            conc_type = DISTNtypeConversion (id_type, INFO_CUDARIZABLE (arg_info));
            name = STRcat (ID_NAME (arg_node), suffix);
            new_avis = TBmakeAvis (TRAVtmpVarName (name), conc_type);
            MEMfree (name);

            /* Create Dist2Conc */
            vardecs = TCappendVardec (FUNDEF_VARDECS (INFO_FUNDEF (arg_info)),
                                      TBmakeVardec (new_avis, NULL));
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info)) = vardecs;

            lut_pointer = LUTsearchInLutS (INFO_ACCESS (arg_info), AVIS_NAME (id_avis));
            if (lut_pointer != NULL && ((offset_t *)*lut_pointer)->inferred) {
                /* If there is memory access data available, we might find some
                 access pattern for the F_dist2conc */
                offset = (offset_t *)*lut_pointer;
                DBUG_PRINT ("Found entry for %s -> [%d,%d]", AVIS_NAME (id_avis),
                            offset->min, offset->max);

                /* generate correct dist2conc parameters */
                start = offset->min;
                stop = offset->max;
                if (INFO_CUDARIZABLE (arg_info)) {
                    last_arg = DUPdoDupNode (INFO_DEVICENUMBER (arg_info));
                    dist2conc = F_dist2device_rel;
                } else {
                    last_arg = TBmakeBool (offset->own);
                    dist2conc = F_dist2host_rel;
                }
            } else {
                /* Unknown access pattern, so we set the minimum and maximum offsets to
                 * cover all blocks. */
                // TODO: infer which blocks to copy rather than all of them
                DBUG_PRINT ("No entry for %s", AVIS_NAME (id_avis));

                /* generate correct dist2conc parameters */
                start = 0;
                stop = SHgetExtent (TYgetShape (AVIS_TYPE (id_avis)), 0);
                if (INFO_CUDARIZABLE (arg_info)) {
                    last_arg = DUPdoDupNode (INFO_DEVICENUMBER (arg_info));
                    dist2conc = F_dist2device_abs;
                } else {
                    /* In a with-loop, the destination memory we want to mark as owned is
                     * always defined relative to the with-loop offsets. So, if we use
                     * absolute index copy in a with-loop, we are only reading and don't
                     * want to mark array as owned by host. If not on a with-loop, most
                     * likely we do want to be the owner.
                     */
                    /* TODO: figure out whether we want to own a piece of memory or not
                     * outside with-loops. */
                    last_arg = TBmakeBool (!INFO_INWL (arg_info));
                    dist2conc = F_dist2host_abs;
                }
            }

            prf_node = TCmakePrf4 (dist2conc, TBmakeId (id_avis), TBmakeNum (start),
                                   TBmakeNum (stop), last_arg);

            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), prf_node),
                              INFO_PREASSIGNS (arg_info));

            /* Insert pair dist_avis->conc_avis into lookup table. */
            INFO_LUT (arg_info)
              = LUTinsertIntoLutP (INFO_LUT (arg_info), id_avis, new_avis);

        } else {
            DBUG_PRINT ("ID %s found, new ID is %s", ID_NAME (arg_node),
                        AVIS_NAME (new_avis));
        }
        ID_AVIS (arg_node) = new_avis;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IMEMDISTprf( node *arg_node, info *arg_info)
 *
 * @brief Save avis of the cuda device variable.
 *
 *****************************************************************************/
node *
IMEMDISTprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_cuda_set_device) {
        INFO_DEVICENUMBER (arg_info) = PRF_ARG1 (arg_node);
    }
    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

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
