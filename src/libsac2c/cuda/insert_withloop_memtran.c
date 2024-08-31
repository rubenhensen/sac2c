/**
 * @file
 * @defgroup iwlmem Insert CUDA memory transfer primitives
 * @ingroup cuda
 *
 * This module inserts CUDA type conversion primitives before and after
 * each cudarizable N_with. The two primitives are <host2device> and
 * <device2host>. They are used to trasfer the data of a host(device) array
 * variable to a device(host) array variable. This is essentially
 * compiled into host<->device memory transfers in the backend. As an
 * example:
 *
 * ~~~~
 * a_host = with
 *          {
 *            ... = b_host;
 *            ... = c_host;
 *            ... = d_host;
 *          }:genarray( shp);
 * ~~~~
 *
 * is transformed into:
 *
 * ~~~~
 * b_dev = host2device( b_host);
 * c_dev = host2device( c_host);
 * d_dev = host2device( d_host);
 * a_dev = with
 *          {
 *            ... = b_dev;
 *            ... = c_dev;
 *            ... = d_dev;
 *          }:genarray( shp);
 * a_host = device2host( a_dev);
 * ~~~~
 *
 * @note
 * Simple scalar variables need not be type converted since they
 * can be passed as function parameters directly to CUDA kernels.
 *
 * @{
 */
#include "insert_withloop_memtran.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"

#define DBUG_PREFIX "IWLMEM"
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

/** @name INFO structure
 *  @{
 */
struct INFO {
    node *fundef;       /**< N_fundef node of the enclosing function */
    bool in_cudawl;     /**< Flag indicating whether the code currently being traversed is in
                             a cudarizable N_with */
    bool create_d2h;    /**< Flag indicating whether <device2host> needs to be created for
                             the N_let->N_ids */
    node *postassigns;  /**< Chain of <device2host> that needs to be appended at the end of
                             the current N_assign */
    node *preassigns;   /**< Chain of <host2device> that needs to be prepended at the
                             beginning of the current N_assign */
    lut_t *lut;         /**< Lookup table storing pairs of Avis(host)->Avis(device) e.g. Given
                             a_dev = host2device( a_host), Avis(a_host)->Avis(a_dev) will be stored
                             into the table */
    lut_t *notran;      /**< Lookup table storing N_avis of arrays varaibles that no data
                             transfers should be created. */
    node *let_expr;     /**< Holds the current N_let expressions, used to check if the RHS is
                             a with-loop */
    node *let_ids;      /**< Holds the current N_let N_ids chain */
    bool in_cexprs;     /**< Flag indicating where are in N_code cexprs */
    bool from_ap;       /**< Flag indicating where are coming from a N_ap */
    node *apids;        /**< Holds LHS of current N_ap */
    node *topblock;     /**< Holds the N_block (body) of the current N_fundef */
    nlut_t *at_nlut;    /**< Used to count the number of references of N_avis */
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_INCUDAWL(n) (n->in_cudawl)
#define INFO_CREATE_D2H(n) (n->create_d2h)
#define INFO_POSTASSIGNS(n) (n->postassigns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_LUT(n) (n->lut)
#define INFO_NOTRAN(n) (n->notran)
#define INFO_LETEXPR(n) (n->let_expr)
#define INFO_LETIDS(n) (n->let_ids)
#define INFO_IN_CEXPRS(n) (n->in_cexprs)
#define INFO_FROM_AP(n) (n->from_ap)
#define INFO_APIDS(n) (n->apids)
#define INFO_TOPBLOCK(n) (n->topblock)
#define INFO_AT_NLUT(n) (n->at_nlut)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_INCUDAWL (result) = FALSE;
    INFO_CREATE_D2H (result) = FALSE;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_NOTRAN (result) = NULL;
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

/** @} */

/** @name Entry functions
 *  @{
 */

/**
 * @brief Perform the IWLMEM traversal, and additionally call the EMRTU traversal.
 *
 * @param syntax_tree
 * @return syntax_tree
 */
node *
IWLMEMdoInsertWithloopMemtran (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    /*
     * Infer dataflow masks
     */
    // syntax_tree = INFDFMSdoInferDfms( syntax_tree, HIDE_LOCALS_NEVER);

    info = MakeInfo ();

    TRAVpush (TR_iwlmem);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** @} */

/** @name Static helper functions
 *  @{
 */

/**
 * @brief Create host2device call, and add to the info struct to be added to
 *        the syntax tree later.
 *
 * @param id The argument position to place the device N_avis
 * @param host_avis The host N_avis
 * @param dev_avis The new device N_avis
 * @param info The info struct
 * @return
 */
static void
CreateHost2Device (node **id, node *host_avis, node *dev_avis, info *arg_info)
{
    DBUG_ENTER ();

    ID_AVIS (*id) = dev_avis;
    FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
      = TBmakeVardec (dev_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

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

/**
 * @brief Search through N_block (passed in via the info struct) for a specific
 *        assignment.
 *
 * @param assign The N_assign to search for
 * @param info The info struct (which holds link to the N_block)
 * @return True if the N_assign was found, False otherwise
 */
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

/**
 * @brief Convert from a host ntype to a device ntype, while preserving shape information.
 *
 * @param host_type The host ntype
 * @param nty The nodetype of the node being converted (support N_id and N_ids)
 * @param info The info struct
 * @return A device ntype struct
 */
static ntype *
TypeConvert (ntype *host_type, nodetype nty, info *arg_info)
{
    ntype *dev_type = NULL;

    DBUG_ENTER ();

    if (nty == N_id) {
        dev_type = CUconvertHostToDeviceType (host_type);
        if (!(TYgetDim (host_type) > 0)
            || !TYisSimple (TYgetScalar (host_type))) {
            dev_type = TYfreeType (dev_type);
        }
    }
    /* If the node to be type converted is N_ids, its original type
     * can be AUD as well as long as the N_with on the RHS is cudarizable.
     * The reason a cudarizbale can produce a AUD result illustrated by
     * the following example:
     *
     * ~~~~
     * cond_fun()
     * {
     *   int[*] aa;
     *   int bb;
     *
     *   if( cond) {
     *     aa = with {}:genarray( shp); (cudarizable N_with)
     *   }
     *   else {
     *     bb = 1;
     *   }
     *   ret = cond ? aa : bb;
     * }
     * ~~~~
     *
     */
    else if (nty == N_ids) {
        if (NODE_TYPE (INFO_LETEXPR (arg_info)) == N_with) {
            /* If the scalar type is simple, e.g. int, float ... */
            if (WITH_CUDARIZABLE (INFO_LETEXPR (arg_info))) {
                dev_type = CUconvertHostToDeviceType (host_type);
                if (!(TYgetDim (host_type) > 0)
                    || !TYisSimple (TYgetScalar (host_type))) {
                    dev_type = TYfreeType (dev_type);
                }
            }
        }
    } else {
        DBUG_UNREACHABLE ("Neither N_id nor N_ids found in TypeConvert!");
    }

    DBUG_RETURN (dev_type);
}

/**
 * @brief Anonymouse traversal function (N_with)
 *
 * @param arg_node N_with
 * @param arg_info info struct
 * @return N_with
 */
static node *
ATravWith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Anonymouse traversal function (N_id). For every N_avis, increment a counter.
 *
 * @param arg_node N_id
 * @param arg_info info struct
 * @return N_id
 */
static node *
ATravId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    NLUTincNum (INFO_AT_NLUT (arg_info), ID_AVIS (arg_node), 1);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Anonymouse traversal function (N_genarray). Traverse through all N_id sons.
 *
 * @param arg_node N_genarray
 * @param arg_info info struct
 * @return N_genarray
 */
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
    GENARRAY_ERC (arg_node) = TRAVopt (GENARRAY_ERC (arg_node), arg_info);
    GENARRAY_PRC (arg_node) = TRAVopt (GENARRAY_PRC (arg_node), arg_info);

    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** @} */

/** @name Traversal functions
 *  @{
 */

/**
 * @brief Traverse N_fundef
 *
 * If the current N_fundef is not a LaC function, traverse the body and next. Otherwise,
 * if we are coming from a N_ap that is a LaC function, traverse *only* the body, passing
 * a link to the body. Otherwise, we go to the next N_fundef.
 *
 * @param arg_node N_fundef
 * @param arg_info info struct
 * @return N_fundef node
 */
node *
IWLMEMfundef (node *arg_node, info *arg_info)
{
    node *old_fundef;
    node *old_topblock;

    DBUG_ENTER ();

    DBUG_PRINT ("at %s", FUNDEF_NAME (arg_node));

    /* During the main traversal, we only look at non-lac functions */
    if (!FUNDEF_ISLACFUN (arg_node)) {
        DBUG_PRINT ("...inspecting body");
        INFO_FUNDEF (arg_info) = arg_node;
        INFO_TOPBLOCK (arg_info) = FUNDEF_BODY (arg_node);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("...inspecting LAC body");
        if (INFO_FROM_AP (arg_info)) {
            DBUG_PRINT ("...from application");

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

/**
 * @brief Traverse N_ap which is a LaC function, and *not* the recursive call.
 *
 * If the application is outside a CUDA withloop, we traverse into its N_fundef.
 * If its from within a CUDA withloop, we check all its arguments against the LUT and if
 * we find a match and its not marked for NOTRAN (no transfer), we create a host2device
 * call.
 *
 * @param arg_node N_ap
 * @param arg_info info struct
 * @return N_ap node
 */
node *
IWLMEMap (node *arg_node, info *arg_info)
{
    bool traverse_lac_fun, old_from_ap;
    node *ap_args, *fundef_args, *avis, *new_avis, *dup_avis, *id_avis;
    ntype *dev_type;
    node *fundef, *old_apids;

    DBUG_ENTER ();

    fundef = AP_FUNDEF (arg_node);
    DBUG_PRINT ("ap_fun %s", FUNDEF_NAME (fundef));

    /* For us to traverse a function from calling site, it must be a
     * condictional function or a loop function and must not be the
     * recursive function call in the loop function. */
    traverse_lac_fun = (FUNDEF_ISLACFUN (fundef) && fundef != INFO_FUNDEF (arg_info));

    if (traverse_lac_fun) { /* inside loop or conditional */
        old_from_ap = INFO_FROM_AP (arg_info);
        INFO_FROM_AP (arg_info) = TRUE;

        old_apids = INFO_APIDS (arg_info);
        INFO_APIDS (arg_info) = INFO_LETIDS (arg_info);

        if (!INFO_INCUDAWL (arg_info)) {
            DBUG_PRINT ("...not in CUDAWL");

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
        } else {
            /* Used to add h2d transfers for applications within WL N_code */
            ap_args = AP_ARGS (arg_node);
            fundef_args = FUNDEF_ARGS (fundef);

            while (ap_args != NULL) {
                DBUG_ASSERT (fundef_args != NULL, "# of Ap args != # of Fundef args!");

                DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (ap_args)) == N_id,
                             "N_ap argument is not N_id node!");

                id_avis = ID_AVIS (EXPRS_EXPR (ap_args));
                avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), id_avis);

                /* If the avis has NOT been come across before */
                if (avis == id_avis) {
                    DBUG_PRINT ("new arg for ap_fun %s, id %s", FUNDEF_NAME (fundef),
                                AVIS_NAME (avis));
                    /* If the id is NOT the one we don't want to create data transfer for
                     */
                    if (LUTsearchInLutPp (INFO_NOTRAN (arg_info), id_avis) == id_avis) {
                        DBUG_PRINT ("There will be transfer for %s", AVIS_NAME (id_avis));
                        dev_type = TypeConvert (AVIS_TYPE (id_avis), N_id, arg_info);

                        if( dev_type != NULL /* &&
                NODE_TYPE( AVIS_DECL( avis)) == N_arg */) {
                            new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                            DBUG_PRINT ("Creating host2device for %s -> %s",
                                    AVIS_NAME (id_avis), AVIS_NAME (new_avis));
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
                        DBUG_PRINT ("There will NOT be transfer for %s",
                                AVIS_NAME (id_avis));
                        /* If the N_id is the one we don't want to create host2device for,
                         * propogate that information to the traversal of LAC functions */
                        INFO_NOTRAN (arg_info)
                          = LUTinsertIntoLutP (INFO_NOTRAN (arg_info),
                                               ARG_AVIS (fundef_args), NULL);
                    }
                } else {
                    DBUG_PRINT ("existing arg on ap_fun %s, id %s", FUNDEF_NAME (fundef),
                                AVIS_NAME (avis));
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

                /* make sure the based type of the ap id and the fundef arg is the same */
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

/**
 * @brief  Add newly created <host2device> and <device2host> to
 *         the assign chain.
 *
 * @param arg_node N_assign
 * @param arg_info info struct
 * @return N_assign node
 */
node *
IWLMEMassign (node *arg_node, info *arg_info)
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

/**
 * @brief Traverse N_let, carrying both the LHS and RHS.
 *
 * @param arg_node N_let
 * @param arg_info info struct
 * @return N_let node
 */
node *
IWLMEMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETEXPR (arg_info) = LET_EXPR (arg_node);
    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse N_funcond that are within a CUDA withloop and change N_avis basetypes
 *        to device types.
 *
 * @param arg_node N_funcond
 * @param arg_info info struct
 * @return N_funcond node
 */
node *
IWLMEMfuncond (node *arg_node, info *arg_info)
{
    node *then_id, *else_id;
    node *ids, *apids;
    ntype *then_sclty, *else_sclty, *ids_sclty;
    node *ret_st, *ret_exprs, *fundef_ret;

    DBUG_ENTER ();

    if (INFO_INCUDAWL (arg_info)) {
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

                if (CUisDeviceTypeNew (ID_NTYPE (then_id))
                    && !CUisDeviceTypeNew (ID_NTYPE (else_id))) {
                    TYsetSimpleType (else_sclty, TYgetSimpleType (then_sclty));
                    AVIS_ISCUDALOCAL (ID_AVIS (else_id)) = TRUE;
                    ID_NAME (else_id) = MEMfree (ID_NAME (else_id));
                    ID_NAME (else_id) = TRAVtmpVarName ("dev");
                    TYsetSimpleType (ids_sclty, TYgetSimpleType (then_sclty));
                    IDS_NAME (ids) = MEMfree (IDS_NAME (ids));
                    IDS_NAME (ids) = TRAVtmpVarName ("dev");
                } else if (CUisDeviceTypeNew (ID_NTYPE (else_id))
                           && !CUisDeviceTypeNew (ID_NTYPE (then_id))) {
                    TYsetSimpleType (then_sclty, TYgetSimpleType (else_sclty));
                    AVIS_ISCUDALOCAL (ID_AVIS (then_id)) = TRUE;
                    ID_NAME (then_id) = MEMfree (ID_NAME (then_id));
                    ID_NAME (then_id) = TRAVtmpVarName ("dev");
                    TYsetSimpleType (ids_sclty, TYgetSimpleType (else_sclty));
                    IDS_NAME (ids) = MEMfree (IDS_NAME (ids));
                    IDS_NAME (ids) = TRAVtmpVarName ("dev");
                } else {
                    // .... TODO ...
                    DBUG_UNREACHABLE ("Found arrays of unequal types while not one host "
                                      "type and one device type!");
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
                        IDS_NAME (apids) = TRAVtmpVarName ("dev");
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

/**
 * @brief Traverse both withop and N_code of a cudarizable N_with
 *
 * @param arg_node N_with
 * @param arg_info info struct
 * @return N_with node
 */
node *
IWLMEMwith (node *arg_node, info *arg_info)
{
    lut_t *old_lut;
    info *anon_info;

    DBUG_ENTER ();

    DBUG_PRINT ("at WL");

    /* If the N_with is cudarizable */
    if (WITH_CUDARIZABLE (arg_node)) {

        /************ Anonymous Traversal ************/
        /* This anon traversal remove all default elements in genarray
         * that are not used in the withloop body at all. */
        anontrav_t atrav[4] = {{N_with, &ATravWith},
                               {N_genarray, &ATravGenarray},
                               {N_id, &ATravId},
                               {(nodetype)0, NULL}};

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

        INFO_INCUDAWL (arg_info) = TRUE;
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        old_lut = INFO_NOTRAN (arg_info);
        /* This lookup table stores variables that we do not
         * want to create data transfers for */
        INFO_NOTRAN (arg_info) = LUTgenerateLut ();

        /* We do not want to create a host2device for index vector,
         * We store pair N_id->N_empty to signal this. */
        INFO_NOTRAN (arg_info) = LUTinsertIntoLutP (INFO_NOTRAN (arg_info),
                                                    IDS_AVIS (WITH_VEC (arg_node)), NULL);
        AVIS_ISCUDALOCAL (IDS_AVIS (WITH_VEC (arg_node))) = TRUE;

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /* Cleanup */
        INFO_NOTRAN (arg_info) = old_lut;
        INFO_NOTRAN (arg_info) = LUTremoveLut (INFO_NOTRAN (arg_info));
        INFO_INCUDAWL (arg_info) = FALSE;
        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

        /* We need to create device2host for N_ids
         * on the LHS of this withloop*/
        INFO_CREATE_D2H (arg_info) = TRUE;
    } else if (INFO_INCUDAWL (arg_info)) {
        /* If we are already in a cudarizable N_with but the
         * N_with itself is not a cudarizable N_with */
        DBUG_ASSERT (!WITH_CUDARIZABLE (arg_node),
                     "Found cuda withloop in cuda withloop!");

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        INFO_NOTRAN (arg_info) = LUTinsertIntoLutP (INFO_NOTRAN (arg_info),
                                                    IDS_AVIS (WITH_VEC (arg_node)), NULL);
        AVIS_ISCUDALOCAL (IDS_AVIS (WITH_VEC (arg_node))) = TRUE;

        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    } else {
        /* The following traversal has been commented out because if the outermost
         * N_with is not cudarizable, none of its inner N_withs (if
         * there is any) will be cudarizable since we only cudarize
         * the outermost N_with. */
        /* Do nothing */
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse N_code of withloop.
 *
 * @param arg_node N_code
 * @param arg_info info struct
 * @return N_code node
 */
node *
IWLMEMcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    INFO_IN_CEXPRS (arg_info) = TRUE;
    CODE_CEXPRS (arg_node) = TRAVopt (CODE_CEXPRS (arg_node), arg_info);
    INFO_IN_CEXPRS (arg_info) = FALSE;

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse default element of a N_genarray
 *
 * @param arg_node N_genarray
 * @param arg_info info struct
 * @return N_genarray node
 */
node *
IWLMEMgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INCUDAWL (arg_info)) {
        /* Note that we do not traverse N_genarray->shape. This is
         * because it can be an N_id node and we do not want to insert
         * <host2device> for it in this case. Therefore, the only sons
         * of N_genarray we traverse are the default element and the
         * potential reuse candidates. */
        if (GENARRAY_DEFAULT (arg_node) != NULL) {
            DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id,
                         "Non N_id default element found in N_genarray!");
            GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        }

        GENARRAY_RC (arg_node) = TRAVopt (GENARRAY_RC (arg_node), arg_info);
        GENARRAY_ERC (arg_node) = TRAVopt (GENARRAY_ERC (arg_node), arg_info);

        GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse default element of a N_modarray
 *
 * @param arg_node N_modarray
 * @param arg_info info struct
 * @return N_modarray node
 */
node *
IWLMEMmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INCUDAWL (arg_info)) {
        DBUG_ASSERT (NODE_TYPE (MODARRAY_ARRAY (arg_node)) == N_id,
                     "Non N_id modified array found in N_modarray!");
        MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

        MODARRAY_RC (arg_node) = TRAVopt (MODARRAY_RC (arg_node), arg_info);
        MODARRAY_ERC (arg_node) = TRAVopt (MODARRAY_ERC (arg_node), arg_info);

        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief For N_ids on the LHS of CUDA-WLs, create a new LHS avis with device type, and
 *        create a <device2host> assign to be placed after the WL. If the N_ids is in a
 *        CUDA-WL, add it to the NOTRAN LUT.
 *
 * @param arg_node N_ids
 * @param arg_info info struct
 * @return N_ids node
 */
node *
IWLMEMids (node *arg_node, info *arg_info)
{
    node *new_avis, *ids_avis;
    ntype *ids_type, *dev_type;

    DBUG_ENTER ();

    ids_avis = IDS_AVIS (arg_node);
    ids_type = AVIS_TYPE (ids_avis);

    DBUG_PRINT ("at IDS of %s", AVIS_NAME (ids_avis));

    /* If the array is defined in cuda withloop, we do not create
     * a host2device transfer for it */
    if (INFO_INCUDAWL (arg_info)) {
        if (/* TYisArray( ids_type) */ !TUisScalar (ids_type)) {
            INFO_NOTRAN (arg_info)
              = LUTinsertIntoLutP (INFO_NOTRAN (arg_info), ids_avis, NULL);

            /* If the ids is cuda local and its type is not of shared
             * memory type, we change its base type from host to device */
            AVIS_ISCUDALOCAL (IDS_AVIS (arg_node)) = TRUE;
            if (!CUisShmemTypeNew (ids_type)) {
                TYsetSimpleType (TYgetScalar (ids_type),
                                 CUh2dSimpleTypeConversion (
                                   TYgetSimpleType (TYgetScalar (ids_type))));
            }
        }
    } else { /* not in CUDAWL */
        if (INFO_CREATE_D2H (arg_info)) {
            /* if we come this this point after a CUDAWL, we probably need to
             * create a device2host transfer. */
            dev_type = TypeConvert (ids_type, NODE_TYPE (arg_node), arg_info);
            if (dev_type != NULL) {

                /* create new avis for WL return */
                new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                IDS_AVIS (arg_node) = new_avis;
                DBUG_PRINT ("...replacing WL return %s -> %s", AVIS_NAME (ids_avis),
                            AVIS_NAME (new_avis));

                /* add to fundef vardecs */
                FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
                  = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

                /* create device2host */
                INFO_POSTASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (ids_avis, NULL),
                                             TBmakePrf (F_device2host,
                                                        TBmakeExprs (TBmakeId (new_avis),
                                                                     NULL))),
                                  INFO_POSTASSIGNS (arg_info));
                DBUG_PRINT ("Creating device2host for %s -> %s", AVIS_NAME (new_avis),
                            AVIS_NAME (ids_avis));

                /* maintain SSA property */
                AVIS_SSAASSIGN (new_avis) = AVIS_SSAASSIGN (ids_avis);
                AVIS_SSAASSIGN (ids_avis) = INFO_POSTASSIGNS (arg_info);
            }

            /* We stop creating any further device2host assigns
             * if this is the last N_ids of a WL-LHS. */
            if (IDS_NEXT (arg_node) == NULL)
                INFO_CREATE_D2H (arg_info) = FALSE;
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief For each host array N_id in the cudarizable N_with, either create
 *        type conversion for it (i.e. <host2device>) or set its N_avis to
 *        that of an already converted device array N_id depending on whether
 *        the N_id is encountered for the first time or not.
 *
 * @param arg_node N_id
 * @param arg_info info struct
 * @return N_id node
 */
node *
IWLMEMid (node *arg_node, info *arg_info)
{
    node *new_avis, *avis, *id_avis;
    ntype *dev_type, *id_type;
    node *ssaassign;

    DBUG_ENTER ();

    id_avis = ID_AVIS (arg_node);
    id_type = AVIS_TYPE (id_avis);

    /* if we are in cudarizable N_with */
    if (INFO_INCUDAWL (arg_info)) {
        DBUG_PRINT ("inspecting %s", AVIS_NAME (id_avis));

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

        if (avis == id_avis) {
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

            ssaassign = AVIS_SSAASSIGN (avis);

            if (((INFO_IN_CEXPRS (arg_info) && ssaassign != NULL
                  && AssignInTopBlock (ssaassign, arg_info))
                 || !INFO_IN_CEXPRS (arg_info))
                && !CUisDeviceTypeNew (id_type) && !CUisShmemTypeNew (id_type)
                && LUTsearchInLutPp (INFO_NOTRAN (arg_info), id_avis) == id_avis) {
                dev_type = TypeConvert (id_type, NODE_TYPE (arg_node), arg_info);
                if (dev_type != NULL) {
                    new_avis = TBmakeAvis (TRAVtmpVarName ("dev"), dev_type);
                    DBUG_PRINT ("Creating host2device for %s -> %s",
                            AVIS_NAME (id_avis), AVIS_NAME (new_avis));
                    CreateHost2Device (&arg_node, id_avis, new_avis, arg_info);
                }
            }
        } else {
            /* If the N_avis has been come across before, replace its
             * N_avis by the device N_avis */
            ID_AVIS (arg_node) = avis;
        }
    }
    DBUG_RETURN (arg_node);
}
/** @} */
/** @} */

#undef DBUG_PREFIX
