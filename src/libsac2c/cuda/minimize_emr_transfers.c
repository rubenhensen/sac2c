/**
 * @file
 * @defgroup memrt Minimize EMR Transfers
 * @ingroup cuda
 *
 * @brief Convert all ERCs in EMRL affected fundefs with CUDA-WL to CUDA device types.
 *
 * The general idea is similar to what MLTRAN does, which is to lift out h2d/d2h memcpys
 * from a loopfun. This traversal though works for a special case, concerning loopfuns
 * which have been effected by the EMRL optimisation *and* where the existing optimisations
 * are not able to lift out the h2d.
 *
 * The latter point is especially important, as traversals like MLTRAN only lift out h2d/d2h
 * if there are no further references to the RHS of h2d/d2h. When using EMRL, this check can
 * fail because of an extra argument in the recursive loopfun application used for the
 * buffer-swapping.
 *
 * A point to consider, MEMRT only works on loopfuns marked with the ISEMRLIFTED flag
 * (on the N_fundef). If we are dealing with a series of nested loops, it will only move
 * the h2d one-level up. This is also limited by the EMRL traversal
 * (@see memory/emr_loop_optimisation.c) which is conservative in how many levels it will
 * lift out allocations. Typically it lifts out allocations from only the innermost loop.
 *
 * To give a concrete example, we have:
 *
 * ~~~~
 * lets_loop (...) {
 *   ...
 *   ret = let_loop_LOOPFUN (..., input, emr_lift);
 *   ...
 * }
 *
 * let_loop_LOOPFUN (..., input, emr_tmp) {
 *   ...
 *   emr_dev = h2d (emr_tmp);
 *   input_dev = h2d (input);
 *   ...
 *   output_dev = wl (input_dev); [ERC: emr_dev]
 *   ...
 *   output = d2h (output_dev);
 *   ...
 *   intra = let_loop_LOOPFUN (..., ouput, input);
 * }
 * ~~~~
 *
 * Through this traversal, we transform the above into:
 *
 * ~~~~
 * lets_loop (...) {
 *   ...
 *   emr_dev = h2d (emr_lift);
 *   ret = let_loop_LOOPFUN (..., input, emr_dev);
 *   ...
 * }
 *
 * let_loop_LOOPFUN (..., input, emr_dev) {
 *   ...
 *   input_dev = h2d (input);
 *   ...
 *   output_dev = wl (input_dev); [ERC: emr_dev]
 *   ...
 *   output = d2h (output_dev);
 *   ...
 *   intra = let_loop_LOOPFUN (..., ouput, output_dev);
 * }
 * ~~~~
 *
 * @{
 */
#include "minimize_emr_transfers.h"

#define DBUG_PREFIX "MEMRT"
#include "debug.h"

#include "types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "globals.h"

#include "free.h"
#include "cuda_utils.h"
#include "LookUpTable.h"
#include "DupTree.h"
#include "deadcoderemoval.h"

enum trav_mode { bypass, inap, afterap };

/**
 * @name INFO structure
 * @{
 */
struct INFO {
    int funargnum;         /**< used to assign ordinal values to fundef args */
    bool inemrloop;        /**< flag indicating we are in a EMRL affected loop */
    enum trav_mode apmode; /**< specifies which mode we are for the N_ap traversal */
    node *fundef;          /**< Holds current N_fundef */
    lut_t *lut;            /**< LUT is used for storing EMRL lifted h2d RHS -> LHS mappings */
    lut_t *reclut;         /**< LUT is used to store all h2d RHS -> LHS mappings */
    node *letids;          /**< The the LHS of N_prf */
    node *apargs;          /**< N_ap arguments */
    node *apvardecs;       /**< Used to update vardecs in N_ap calling context */
    node *apassigns;       /**< Used to update assigns in N_ap calling context */
    node *rec_ap;          /**< the recursive loopfun N_ap */
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LUT(n) ((n)->lut)
#define INFO_RECLUT(n) ((n)->reclut)
#define INFO_LETIDS(n) ((n)->letids)
#define INFO_FUNARGNUM(n) ((n)->funargnum)
#define INFO_APARGS(n) ((n)->apargs)
#define INFO_APVARDECS(n) ((n)->apvardecs)
#define INFO_APASSIGNS(n) ((n)->apassigns)
#define INFO_REC_AP(n) ((n)->rec_ap)
#define INFO_INEMRLOOP(n) ((n)->inemrloop)
#define INFO_APMODE(n) ((n)->apmode)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNARGNUM (result) = 0;
    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_RECLUT (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_APARGS (result) = NULL;
    INFO_APVARDECS (result) = NULL;
    INFO_APASSIGNS (result) = NULL;
    INFO_REC_AP (result) = NULL;
    INFO_INEMRLOOP (result) = FALSE;
    INFO_APMODE (result) = bypass;

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

/**
 * @name Anonymous Traversal
 * @{
 */

/**
 * @brief If the application is a the do-loop recursive loop,
 *        store it in the info structure
 *
 * @param arg_node N_ap
 * @param arg_info info structure
 * @return N_ap
 */
static node *
MEMRTapAnon (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_FUNDEF (arg_info) == AP_FUNDEF (arg_node)) {
        DBUG_PRINT ("found recursive application of %s...", FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        INFO_REC_AP (arg_info) = arg_node;
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Store LHS in info structure before traversing RHS
 *
 * @param arg_node N_let
 * @param arg_info info structure
 * @return N_let
 */
static node *
MEMRTletAnon (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief If the N_prf is `F_host2device`, store the mapping of
 *        RHS to LHS in the LUT
 *
 * @param arg_node N_prf
 * @param arg_info info structure
 * @return N_prf
 */
static node *
MEMRTprfAnon (node *arg_node, info *arg_info)
{
    node *arg_avis, *ret_avis;

    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_host2device:
        arg_avis = ID_AVIS (PRF_ARG1 (arg_node));
        ret_avis = IDS_AVIS (INFO_LETIDS (arg_info));
        DBUG_PRINT ("found h2d, adding mapping of arg to ret: %s -> %s", AVIS_NAME (arg_avis), AVIS_NAME (ret_avis));
        INFO_RECLUT (arg_info)
            = LUTinsertIntoLutP (INFO_RECLUT (arg_info), arg_avis, ret_avis);
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Use an anonymous traversal to the recursive do-loop application. Additionally
 *        for all `F_host2device` primitives, store the mapping of RHS to LHS in a LUT.
 *
 * Both the recursive N_ap and the LUT containing mappings is used later to appropriately
 * replace arguments in the recursive N_ap with those matching in the LUT.
 *
 * @param fundef A N_fundef node, from an N_ap node
 * @param arg_info info structure
 * @return the first argument, fundef
 */
static node *
MEMRTtravToRecAp (node *fundef, info *arg_info)
{
    node *old_fundef, *old_letids;
    anontrav_t trav[4] = {{N_let, &MEMRTletAnon}, {N_ap, &MEMRTapAnon}, {N_prf, &MEMRTprfAnon}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "First argument must be a N_fundef node!");
    DBUG_ASSERT (INFO_RECLUT (arg_info) != NULL, "The recursive LUT must be created first!");

    old_fundef = INFO_FUNDEF (arg_info);
    old_letids = INFO_LETIDS (arg_info);
    INFO_FUNDEF (arg_info) = fundef;
    INFO_LETIDS (arg_info) = NULL;

    TRAVpushAnonymous (trav, &TRAVsons);
    FUNDEF_BODY (fundef) = TRAVdo (FUNDEF_BODY (fundef), arg_info);
    TRAVpop ();

    INFO_FUNDEF (arg_info) = old_fundef;
    INFO_LETIDS (arg_info) = old_letids;

    DBUG_RETURN (fundef);
}

/** @} */

/**
 * @name Entry function
 * @{
 */

/**
 * @brief The entry function into the MEMRT traversal.
 *
 * @param syntax_tree
 * @return syntax tree
 */
node *
MEMRTdoMinimizeEMRTransfers (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_memrt);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("invoking DCR");
    syntax_tree = DCRdoDeadCodeRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** @} */

/**
 * @name Traversal functions
 * @{
 */

/**
 * @brief  Traverse N_fundefs, if its an EMRL affected loopfun, traverse
 *         only the body.
 *
 * @param arg_node N_fundef
 * @param arg_info info structure
 * @return N_fundef
 */
node *
MEMRTfundef (node *arg_node, info *arg_info)
{
    bool old_inemrloop;

    DBUG_ENTER ();

    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISEMRLIFTED (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    } else if (INFO_APMODE (arg_info) == inap) { /* EMR lifted loop */
        DBUG_PRINT ("inspecting EMR affected do-loop %s...", FUNDEF_NAME (arg_node));
        /* We assign a sequential number (starting from 0) to each argument of the loopfun */
        INFO_FUNARGNUM (arg_info) = 0;
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

        old_inemrloop = INFO_INEMRLOOP (arg_info);
        INFO_INEMRLOOP (arg_info) = TRUE;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_INEMRLOOP (arg_info) = old_inemrloop;
    } else {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }


    DBUG_RETURN (arg_node);
}

/**
 * @brief Traversal N_fundef arguments and assign an ordinal value
 *
 * With this we can retrieve an argument using the ordinal value.
 *
 * @param arg_node N_arg
 * @param arg_info info structure
 * @return N_arg
 */
node *
MEMRTarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_LINKSIGN (arg_node) = INFO_FUNARGNUM (arg_info);
    INFO_FUNARGNUM (arg_info) += 1;

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * @brief Traverse statements, if statement is initial N_ap of a loopfun,
 *        update the current context with values from info structure.
 *
 * @param arg_node N_assign
 * @param arg_info info structure
 * @return N_assign
 */
node *
MEMRTassign (node *arg_node, info *arg_info)
{
    node *old_next, *newold_assign, *old_ap_assigns, *old_ap_vardecs;

    DBUG_ENTER ();

    /* stack info fields */
    old_ap_assigns = INFO_APASSIGNS (arg_info);
    old_ap_vardecs = INFO_APVARDECS (arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_APMODE (arg_info) == afterap) {
        DBUG_PRINT ("updating assigns in calling context");
        old_next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        /* add h2d in calling context before N_ap */
        if (INFO_APASSIGNS (arg_info) != NULL) {
            arg_node = TCappendAssign (INFO_APASSIGNS (arg_info), arg_node);
            global.optcounters.cuda_min_trans++;
        }

        /* add needed vardecs to calling context */
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TCappendVardec (INFO_APVARDECS (arg_info),
                            FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        /* restore values */
        INFO_APASSIGNS (arg_info) = old_ap_assigns;
        INFO_APVARDECS (arg_info) = old_ap_vardecs;
        INFO_APMODE (arg_info) = bypass;

        /* re-attach original next node to end of new assigns */
        newold_assign = arg_node;
        while (ASSIGN_NEXT (newold_assign) != NULL) {
            newold_assign = ASSIGN_NEXT (newold_assign);
        }

        ASSIGN_NEXT (newold_assign) = old_next;
        ASSIGN_NEXT (newold_assign) = TRAVopt (ASSIGN_NEXT (newold_assign), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief Store LHS of N_let before traversing RHS
 *
 * @param arg_node N_let
 * @param arg_info info structure
 * @return N_let
 */
node *
MEMRTlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/**
 * @brief Replace current N_id with one stored in LUT
 *
 * @param arg_node N_id
 * @param arg_info info structure
 * @return N_id
 */
node *
MEMRTid (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    if (INFO_INEMRLOOP (arg_info)) {
        /* If this N_id occurs in a place other than the argument list
         * of a recursive application of the enclosing do-fun, reset its
         * N_avis to the new N_avis. This is necessary when
         * a <host2device> is lifted out of the do-fun, and therefore
         * the device variable is passed to the do-fun as an argument
         * instead of a locally declared/defined variable. */
        avis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));
        if (avis != ID_AVIS (arg_node)) {
            ID_AVIS (arg_node) = avis;
        }
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief If the N_ap is the initial call for a EMRL affected loopfun,
 *        traverse the loopfun's body and lift out H2D memcpys
 *
 * Here we setup the info structure, creating two LUTs and storing some
 * stateful information. Additionally we call `MEMRTtravToRecAp` to
 * store the recursive loopfun N_ap and populate one of the LUTs.
 *
 * After traversing, we reset the info structure to a previous state.
 *
 * @param arg_node N_ap
 * @param arg_info info structure
 * @return N_ap
 */
node *
MEMRTap (node *arg_node, info *arg_info)
{
    node *old_ap_args, *old_fundef, *old_rec_ap;
    lut_t *old_lut, *old_reclut;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))
        && FUNDEF_ISEMRLIFTED (AP_FUNDEF (arg_node))) {
        if (INFO_FUNDEF (arg_info) != AP_FUNDEF (arg_node)) { /* initial application */
            DBUG_PRINT ("inspecting initial application of %s...", FUNDEF_NAME (AP_FUNDEF (arg_node)));

            /* traverse arguments first */
            AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

            /* stack info fields */
            old_fundef = INFO_FUNDEF (arg_info);
            old_ap_args = INFO_APARGS (arg_info);
            old_rec_ap = INFO_REC_AP (arg_info);
            old_lut = INFO_LUT (arg_info);
            old_reclut = INFO_RECLUT (arg_info);

            /* initialise info fields */
            INFO_APARGS (arg_info) = AP_ARGS (arg_node);
            INFO_APASSIGNS (arg_info) = NULL;
            INFO_APVARDECS (arg_info) = NULL;
            INFO_LUT (arg_info) = LUTgenerateLut ();
            INFO_RECLUT (arg_info) = LUTgenerateLut ();

            /* we find the recursive N_ap and fill RECLUT with h2d arg to ret mappings */
            AP_FUNDEF (arg_node) = MEMRTtravToRecAp (AP_FUNDEF (arg_node), arg_info);

            INFO_APMODE (arg_info) = inap;
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
            INFO_APMODE (arg_info) = afterap;

            /* reset all the info fields */
            INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
            INFO_LUT (arg_info) = old_lut;
            INFO_RECLUT (arg_info) = LUTremoveLut (INFO_RECLUT (arg_info));
            INFO_RECLUT (arg_info) = old_reclut;
            INFO_FUNDEF (arg_info) = old_fundef;
            INFO_APARGS (arg_info) = old_ap_args;
            INFO_REC_AP (arg_info) = old_rec_ap;
        }
    } else {
        AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @brief If we find a `F_host2device` primitive, we check if its argument
 *        was created via EMRL lifting out an allocation. If so, we lift
 *        out the primitive and update the loopfun appropriately.
 *
 * Assuming we are in a EMRL affected loopfun, if the argument of a `F_host2device`
 * primitive is a lifted allocation, we transfer the primitive and declaration via
 * the info structure (see N_assign for application). Additionally we place into
 * LUT the primitives RHS -> LHS, such that we update all subsequent references
 * correctly. Finally we update the correct argument in the recursive N_ap.
 *
 * @param arg_node N_prf
 * @param arg_info info structure
 * @return N_prf
 */
node *
MEMRTprf (node *arg_node, info *arg_info)
{
    node *id, *id_decl, *aparg, *ret_avis, *recaparg, *recapexprs;

    DBUG_ENTER ();

    if (INFO_INEMRLOOP (arg_info)) {
        switch (PRF_PRF (arg_node)) {
        case F_host2device:
            id = PRF_ARG1 (arg_node);
            id_decl = ID_DECL (id);

            if (NODE_TYPE (id_decl) == N_arg) {
                /* host var is passed as argument of do-loop */
                aparg = CUnthApArg (INFO_APARGS (arg_info), ARG_LINKSIGN (id_decl));
                DBUG_ASSERT (NODE_TYPE (aparg) == N_id,
                             "Arguments of N_ap must be N_id nodes!");
                if (AVIS_ISALLOCLIFT (ID_AVIS (aparg))) {
                    /* this var is the result of EMRL alloc lifting */
                    DBUG_PRINT ("Found H2D that was EMRL lifted: %s (ap) -> %s", ID_NAME (aparg), ID_NAME (id));
                    /* We change the argument, e.g. a_host to
                     * device variable, e.g. a_dev */
                    node *vardec = IDS_DECL (INFO_LETIDS (arg_info));
                    ARG_AVIS (id_decl) = DUPdoDupNode (VARDEC_AVIS (vardec));
                    AVIS_SSAASSIGN (ARG_AVIS (id_decl)) = NULL;
                    AVIS_DECL (ARG_AVIS (id_decl)) = id_decl;

                    /* Insert pair [N_vardec->avis] -> [N_arg->avis] into H2D
                     * table. Therefore, N_vardec->avis of any subsequent N_id
                     * nodes will be replaced by N_arg->avis. */
                    INFO_LUT (arg_info)
                      = LUTinsertIntoLutP (INFO_LUT (arg_info), VARDEC_AVIS (vardec),
                                           ARG_AVIS (id_decl));

                    /* Create N_vardec and <host2device> in the calling context
                     * i.e. lifting the <host2device> */
                    node *new_avis = DUPdoDupNode (ARG_AVIS (id_decl));
                    INFO_APVARDECS (arg_info)
                      = TBmakeVardec (new_avis, INFO_APVARDECS (arg_info));

                    INFO_APASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                                 TBmakePrf (F_host2device,
                                                            TBmakeExprs (TBmakeId (
                                                                           ID_AVIS (aparg)),
                                                                         NULL))),
                                      INFO_APASSIGNS (arg_info));

                    /* Replace the N_avis of ap_arg to the new device N_avis */
                    ID_AVIS (aparg) = new_avis;
                    /* Maintain SSA property */
                    AVIS_SSAASSIGN (new_avis) = INFO_APASSIGNS (arg_info);

                    /* update recursive N_ap argument appropriately */
                    recapexprs = TCgetNthExprs ((size_t)ARG_LINKSIGN (id_decl), AP_ARGS (INFO_REC_AP (arg_info)));
                    recaparg = EXPRS_EXPR (recapexprs);
                    ret_avis = LUTsearchInLutPp (INFO_RECLUT (arg_info), ID_AVIS (recaparg));
                    if (ret_avis == ID_AVIS (recaparg)) {
                        DBUG_UNREACHABLE ("%s does not exist in RECLUT!", ID_NAME (recaparg));
                    }
                    DBUG_PRINT ("replacing %s -> %s in recursive N_ap", ID_NAME (recaparg), AVIS_NAME (ret_avis));
                    ID_AVIS (recaparg) = ret_avis;
                }
            }
            break;
        default:
            PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
            break;
        }
    }

    DBUG_RETURN (arg_node);
}

/** @} */
/** @} */
#undef DBUG_PREFIX
