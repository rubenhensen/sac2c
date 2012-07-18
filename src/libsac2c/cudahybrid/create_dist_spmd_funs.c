/*****************************************************************************
 *
 * file:   create_dist_spmd_funs.c
 *
 * prefix: MTDSPMDF
 *
 * description:
 *
 *   We traverse each ST function body and look for with-loops to be
 *   parallelised. Each such with-loop is then lifted into a separate
 *   function definition. These newly created fundefs are named and tagged
 *   SPMD functions. These functions will later implement switching from
 *   single-threaded execution to multithreaded execution and vice versa.
 *
 *   The necessary information to create a fully-fledged function definition,
 *   e.g. parameter names and types, return values and types, local variables
 *   and their types, is gathered during a full traversal of each with-loop
 *   tagged for multithreaded execution.
 *
 *   The newly generated functions are inserted at the end of the fundef
 *   chain.
 *
 *****************************************************************************/

#include "create_dist_spmd_funs.h"

#define DBUG_PREFIX "MTDSPMDF"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "memory.h"
#include "LookUpTable.h"
#include "namespaces.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "str.h"

/**
 * INFO structure
 */

struct INFO {
    node *spmdfuns;
    node *fundef;
    lut_t *lut;
    node *args;
    node *params;
    node *vardecs;
    node *rets;
    node *retexprs;
    bool collect;
    bool lift;
    bool withid;
    bool isxtfun;
    bool inspmd;
    prf thisprf;
    node *freearg;
    bool iswl;
    bool iscond;
    node *depallocs;
    node *wlassign;
    node *depfrees;
    node *conc2dist;
    node *preassigns;
    node *postassigns;
    node *next;
};

/**
 * INFO macros
 */

#define INFO_SPMDFUNS(n) ((n)->spmdfuns)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LUT(n) ((n)->lut)
#define INFO_ARGS(n) ((n)->args)
#define INFO_PARAMS(n) ((n)->params)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_RETS(n) ((n)->rets)
#define INFO_RETEXPRS(n) ((n)->retexprs)
#define INFO_COLLECT(n) ((n)->collect)
#define INFO_LIFT(n) ((n)->lift)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_INSPMD(n) ((n)->inspmd)
#define INFO_THISPRF(n) ((n)->thisprf)
#define INFO_FREEARG(n) ((n)->freearg)
#define INFO_ISWL(n) ((n)->iswl)
#define INFO_ISCOND(n) ((n)->iscond)
#define INFO_DEPALLOCS(n) ((n)->depallocs)
#define INFO_WLASSIGN(n) ((n)->wlassign)
#define INFO_DEPFREES(n) ((n)->depfrees)
#define INFO_CONC2DIST(n) ((n)->conc2dist)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_NEXTASSIGN(n) ((n)->next)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SPMDFUNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_LIFT (result) = FALSE;
    INFO_WITHID (result) = FALSE;
    INFO_INSPMD (result) = FALSE;
    INFO_THISPRF (result) = F_unknown;
    INFO_FREEARG (result) = NULL;
    INFO_ISWL (result) = FALSE;
    INFO_ISCOND (result) = FALSE;
    INFO_DEPALLOCS (result) = NULL;
    INFO_WLASSIGN (result) = NULL;
    INFO_DEPFREES (result) = NULL;
    INFO_CONC2DIST (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_POSTASSIGNS (result) = NULL;
    INFO_NEXTASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static node *
ATravCAVexprs (node *arg_node, info *arg_info)
{
    node *vardecs, *vardec;

    DBUG_ENTER ();

    vardecs = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

    vardec = TRAVdo (EXPRS_EXPR (arg_node), arg_info);

    VARDEC_NEXT (vardec) = vardecs;

    DBUG_RETURN (vardec);
}

static node *
ATravCAVid (node *arg_node, info *arg_info)
{
    node *vardec;

    DBUG_ENTER ();

    vardec = TBmakeVardec (TBmakeAvis (TRAVtmpVarName (ID_NAME (arg_node)),
                                       TYcopyType (AVIS_TYPE (ID_AVIS (arg_node)))),
                           NULL);
    VARDEC_ISSTICKY (vardec) = TRUE;

    DBUG_RETURN (vardec);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateAuxiliaryVardecsFromRets( node *rets)
 *
 * @brief generates one fresh vardec for each return identifier. These will
 *   later be used during code generation for the final folding code within
 *   the synchronisation barrier of the SPMD function.
 *
 *****************************************************************************/

static node *
CreateAuxiliaryVardecsFromRetExprs (node *retexprs)
{
    anontrav_t cav_trav[3]
      = {{N_exprs, &ATravCAVexprs}, {N_id, &ATravCAVid}, {(nodetype)0, NULL}};
    node *vardecs;

    DBUG_ENTER ();

    TRAVpushAnonymous (cav_trav, &TRAVsons);

    vardecs = TRAVopt (retexprs, NULL);

    TRAVpop ();

    DBUG_RETURN (vardecs);
}

/** <!--********************************************************************-->
 *
 * @fn static node *CreateSpmdFundef( node *arg_node, info *arg_info)
 *
 * @brief generates SPMD fundef from data gathered in the info node during
 *   preceding traversal of MT-tagged with-loop
 *
 *****************************************************************************/

static node *
CreateSpmdFundef (node *arg_node, info *arg_info)
{
    node *spmd_fundef, *fundef, *body, *retexprs, *vardecs;
    node *assigns, *args, *rets, *retur;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign,
                 "CreateSpmdFundef() called with illegal node type.");

    fundef = INFO_FUNDEF (arg_info);

    retexprs = INFO_RETEXPRS (arg_info);
    INFO_RETEXPRS (arg_info) = NULL;

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    rets = INFO_RETS (arg_info);
    INFO_RETS (arg_info) = NULL;

    args = INFO_ARGS (arg_info);
    INFO_ARGS (arg_info) = NULL;

    vardecs = TCappendVardec (CreateAuxiliaryVardecsFromRetExprs (retexprs), vardecs);

    assigns = DUPdoDupTreeLut (arg_node, INFO_LUT (arg_info));
    INFO_LUT (arg_info) = LUTremoveContentLut (INFO_LUT (arg_info));

    retur = TBmakeReturn (retexprs);

    assigns = TCappendAssign (assigns, TBmakeAssign (retur, NULL));

    body = TBmakeBlock (assigns, vardecs);

    BLOCK_ISMTPARALLELBRANCH (body) = TRUE;

    spmd_fundef
      = TBmakeFundef (TRAVtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, body, NULL);

    FUNDEF_RETURN (spmd_fundef) = retur;

    DBUG_RETURN (spmd_fundef);
}

// static
// node *EliminateDist2ConcFrees( node *arg_node, info *arg_info)
//{
//  node *res;
//
//  DBUG_ENTER();
//
//  if (INFO_FREEARG(arg_info) == LUTsearchInLutPp(INFO_LUT(arg_info),
//                                                 INFO_FREEARG(arg_info))) {
//    /*
//     * Since the argument of the free was not on the LUT, this free relates
//     * to one of the dist2conc() arrays. These should not be free'ed, so we
//     * eliminate this assignment.
//     */
//    res = ASSIGN_NEXT(arg_node);
//    ASSIGN_NEXT(arg_node) = NULL;
//    //if there are no frees but those we are removing, set info accordingly
//    if( INFO_DEPFREES(arg_info) == arg_node) {
//      INFO_DEPFREES(arg_info) = NULL;
//    }
//    FREEdoFreeTree(arg_node);
//  } else {
//    res = arg_node;
//  }
//
//  DBUG_RETURN(res);
//}

/* TODO: insert collects */
static node *
ATravLiftMemoryOpsFromSPMDassign (node *arg_node, info *arg_info)
{
    node *res, *avis;

    DBUG_ENTER ();

    INFO_THISPRF (arg_info) = F_unknown;
    INFO_ISWL (arg_info) = FALSE;
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_THISPRF (arg_info) == F_alloc && INFO_DEPALLOCS (arg_info) == NULL) {
        /*
         * This is the first alloc() assignment we have to move out of the
         * parallel section. We record it in info as the start of the pre-SPMD
         * assignment chain. For the current assignment chain, we'll return the
         * next statement we want to keep in the parallel section, which is the
         * one with the with-loop.
         */
        INFO_DEPALLOCS (arg_info) = arg_node;
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        res = INFO_WLASSIGN (arg_info);
    } else if (INFO_ISWL (arg_info)) {
        /*
         * This is the with-loop assignment, we save it in the info structure.
         * We return NULL to end the chain of the previous assignments, which we
         * want to move out.
         */
        INFO_WLASSIGN (arg_info) = arg_node;
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        // return self if there are no dependencies to allocate
        if (INFO_DEPALLOCS (arg_info) != NULL)
            res = NULL;
        else
            res = arg_node;
    } else if (INFO_THISPRF (arg_info) == F_free && INFO_WLASSIGN (arg_info) != NULL) {
        /* This is one of the free() assignments after the with-loop */
        if (INFO_DEPFREES (arg_info) == NULL) {
            /*
             * This is the first free() assignment after the with-loop, relating to
             * its dependencies. These will be lifted out of the parallel section
             * too. We'll return the next statement we want to keep in the parallel
             * section, which is the one with the concrete to distributed transfer.
             * This is also where we eliminate the free() assignments of the
             * transfered arrays.
             */
            INFO_DEPFREES (arg_info) = arg_node;
            avis = INFO_FREEARG (arg_info);
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            INFO_FREEARG (arg_info) = avis;
            // arg_node = EliminateDist2ConcFrees(arg_node, arg_info);
            if (arg_node != INFO_DEPFREES (arg_info)) {
                /*
                 * we removed the first of the free() assignments, so there are no
                 * dependencies to free
                 */
                INFO_DEPFREES (arg_info) = NULL;
            }
            res = INFO_CONC2DIST (arg_info);
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            // res = EliminateDist2ConcFrees(arg_node, arg_info);
            res = arg_node;
        }
    } else if (INFO_THISPRF (arg_info) == F_device2dist
               || INFO_THISPRF (arg_info) == F_host2dist_spmd) {
        /* This is the transfer from concrete to distributed. */
        INFO_CONC2DIST (arg_info) = arg_node;
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        // return self if there are no dependencies to free
        if (INFO_DEPFREES (arg_info) != NULL)
            res = NULL;
        else
            res = arg_node;
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        res = arg_node;
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDSPMDFmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDSPMDFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LUT (arg_info) = LUTgenerateLut ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDSPMDFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDSPMDFfundef (node *arg_node, info *arg_info)
{
    node *spmdfuns = NULL;

    DBUG_ENTER ();

    if ((FUNDEF_ISSTFUN (arg_node) || FUNDEF_ISXTFUN (arg_node))
        && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * ST and XT funs may contain parallel with-loops.
         * Hence, we constrain our search accordingly.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;

        spmdfuns = INFO_SPMDFUNS (arg_info);
        INFO_SPMDFUNS (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    spmdfuns = TCappendFundef (spmdfuns, FUNDEF_NEXT (arg_node));
    FUNDEF_NEXT (arg_node) = spmdfuns;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDSPMDFassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDSPMDFassign (node *arg_node, info *arg_info)
{
    node *res, *spmd_fundef, *spmd_ap;
    info *old_info;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_THISPRF (arg_info) == F_is_cuda_thread) {
        /*
         * This assignment is the call to _is_cuda_thread_(), we are definitely
         * at the start of a parallel section. Signal that in info, traverse the
         * next nodes and signal the parent node to lift the parallel section.
         */
        INFO_THISPRF (arg_info) = F_unknown;
        INFO_INSPMD (arg_info) = TRUE;
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        INFO_LIFT (arg_info) = TRUE;
        res = arg_node;
    } else if (INFO_INSPMD (arg_info) && INFO_ISCOND (arg_info)) {
        /*
         * This is the Conditional containing the cuda and host branches of a wl.
         * We only build the SPMD function when traversing back up the tree, so we
         * save the data collected here.
         */
        INFO_ISCOND (arg_info) = FALSE;
        INFO_INSPMD (arg_info) = FALSE;

        old_info = arg_info;
        arg_info = MakeInfo ();
        INFO_FUNDEF (arg_info) = INFO_FUNDEF (old_info);
        INFO_NEXTASSIGN (old_info) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        FreeInfo (arg_info);
        arg_info = old_info;

        ASSIGN_NEXT (arg_node) = NULL;
        res = arg_node;
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        if (INFO_LIFT (arg_info)) {
            /*
             * The lift flag is set, so we create the SPMD function call in place of
             * this alloc() assignment, as the alloc() is now in the SPMD function.
             */
            spmd_fundef = CreateSpmdFundef (arg_node, arg_info);
            FUNDEF_NEXT (spmd_fundef) = INFO_SPMDFUNS (arg_info);
            INFO_SPMDFUNS (arg_info) = spmd_fundef;

            spmd_ap
              = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_RETS (arg_info)),
                                         TBmakeAp (spmd_fundef, INFO_PARAMS (arg_info))),
                              NULL);
            INFO_PARAMS (arg_info) = NULL;

            res
              = TCappendAssign (INFO_POSTASSIGNS (arg_info), INFO_NEXTASSIGN (arg_info));
            res = TCappendAssign (spmd_ap, res);
            res = TCappendAssign (INFO_PREASSIGNS (arg_info), res);
            INFO_PREASSIGNS (arg_info) = NULL;
            INFO_POSTASSIGNS (arg_info) = NULL;

            INFO_LIFT (arg_info) = FALSE;
        } else {
            /*
             * The lift flag is not set, so this was not the alloc for the
             * _is_cuda_thread_() result. We return this node.
             */
            res = arg_node;
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDSPMDFprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDSPMDFprf (node *arg_node, info *arg_info)
{
    prf self;
    node *arg;

    DBUG_ENTER ();

    PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);

    self = PRF_PRF (arg_node);
    INFO_THISPRF (arg_info) = self;

    if (self == F_free) {
        INFO_FREEARG (arg_info) = ID_AVIS (PRF_ARG1 (arg_node));
    } else if (self == F_fill) {
        arg = PRF_ARG1 (arg_node);
        if (NODE_TYPE (arg) == N_prf) {
            INFO_THISPRF (arg_info) = PRF_PRF (arg);
        }
    } else if (self == F_host2dist_st && INFO_INSPMD (arg_info)) {
        PRF_PRF (arg_node) = F_host2dist_spmd;
        INFO_THISPRF (arg_info) = F_host2dist_spmd;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDSPMDFcond( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDSPMDFcond (node *arg_node, info *arg_info)
{
    anontrav_t lift_trav[8] = {{N_assign, &ATravLiftMemoryOpsFromSPMDassign},
                               {N_let, &MTDSPMDFlet},
                               {N_id, &MTDSPMDFid},
                               {N_ids, &MTDSPMDFids},
                               {N_with2, &MTDSPMDFwith2},
                               {N_with, &MTDSPMDFwith},
                               {N_prf, &MTDSPMDFprf},
                               {(nodetype)0, NULL}};

    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    if (INFO_INSPMD (arg_info)) {

        TRAVpushAnonymous (lift_trav, &TRAVsons);

        INFO_DEPALLOCS (arg_info) = NULL;
        INFO_WLASSIGN (arg_info) = NULL;
        INFO_DEPFREES (arg_info) = NULL;
        INFO_CONC2DIST (arg_info) = NULL;
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        INFO_PREASSIGNS (arg_info) = INFO_DEPALLOCS (arg_info);
        INFO_POSTASSIGNS (arg_info) = INFO_DEPFREES (arg_info);

        INFO_DEPALLOCS (arg_info) = NULL;
        INFO_WLASSIGN (arg_info) = NULL;
        INFO_DEPFREES (arg_info) = NULL;
        INFO_CONC2DIST (arg_info) = NULL;
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
        INFO_PREASSIGNS (arg_info)
          = TCappendAssign (INFO_DEPALLOCS (arg_info), INFO_PREASSIGNS (arg_info));
        INFO_POSTASSIGNS (arg_info)
          = TCappendAssign (INFO_DEPFREES (arg_info), INFO_POSTASSIGNS (arg_info));

        TRAVpop ();
    } else {
        COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);
    }

    INFO_ISCOND (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTDSPMDFlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTDSPMDFlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTDSPMDFid(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
MTDSPMDFid (node *arg_node, info *arg_info)
{
    node *avis, *new_avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);

    DBUG_PRINT ("ENTER id %s", ID_NAME (arg_node));

    if (INFO_COLLECT (arg_info)) {
        if (INFO_WITHID (arg_info)) {
            /*
             * As a withid this N_id node actually represents a left hand side variable.
             */
            DBUG_PRINT ("...is Withid-id");

            if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
                DBUG_PRINT ("  Not handled before...");
                new_avis = DUPdoDupNode (avis);
                INFO_VARDECS (arg_info)
                  = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);

                DBUG_PRINT (">>> ids %s added to LUT", ID_NAME (arg_node));
            }
        } else if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            DBUG_PRINT ("  Not handled before...");
            /*
             * A right hand side variable that has not been handled before must be a
             * free variable of the with-loop and that means it needs to become a
             * parameter of the spmd function to be created.
             */
            new_avis = DUPdoDupNode (avis);

            INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
            INFO_PARAMS (arg_info)
              = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTDSPMDFids(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
MTDSPMDFids (node *arg_node, info *arg_info)
{
    node *avis;
    node *new_avis;
    prf rhs;

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_node);
    new_avis = NULL;

    DBUG_PRINT ("ENTER ids %s", IDS_NAME (arg_node));

    if (INFO_COLLECT (arg_info)) {
        rhs = INFO_THISPRF (arg_info);
        if (rhs == F_host2dist_spmd || rhs == F_device2dist) {
            /*
             * If we have a transfer function on the RHS, we are on the LHS of an
             * assignment which has one of the withloop results.
             */
            new_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);

            if (new_avis == avis) {
                new_avis = DUPdoDupNode (avis);
                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
                INFO_VARDECS (arg_info)
                  = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            }

            INFO_RETS (arg_info)
              = TCappendRet (INFO_RETS (arg_info),
                             TBmakeRet (TYeliminateAKV (AVIS_TYPE (new_avis)), NULL));

            INFO_RETEXPRS (arg_info)
              = TCappendExprs (INFO_RETEXPRS (arg_info),
                               TBmakeExprs (TBmakeId (new_avis), NULL));
        } else if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT (">>> ids %s added to LUT", IDS_NAME (arg_node));
        }
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTDSPMDFwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a parallelised with-loop into a function.
 *
 ******************************************************************************/

node *
MTDSPMDFwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_INSPMD (arg_info)) {
        // treat these assignments as if they were outside the parallel section
        TRAVpush (TR_mtdspmdf);

        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

        TRAVpop ();

        INFO_ISWL (arg_info) = TRUE;
    } else {
        /*
         * We are currently still looking for a distributed parallel section. This
         * may only occur in the code subtree.
         */
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTDSPMDFwith( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/

node *
MTDSPMDFwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // treat these assignments as if they were outside the parallel section
    TRAVpush (TR_mtdspmdf);

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    TRAVpop ();

    INFO_ISWL (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTDSPMDFwithid( node *arg_node, info *arg_info)
 *
 *    @brief traversal function for N_withid node
 *      We memoise the fact that we now traverse a withid to do the right things
 *      when encountering id and ids nodes.
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return arg_node
 *
 ******************************************************************************/

node *
MTDSPMDFwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITHID (arg_info) = TRUE;

    WITHID_VEC (arg_node) = TRAVopt (WITHID_VEC (arg_node), arg_info);
    WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
    WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);

    INFO_WITHID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTDSPMDFdoCreateDistributedSpmdFuns( node *syntax_tree)
 *
 *  @brief initiates traversal for creating SPMD functions
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTDSPMDFdoCreateDistributedSpmdFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtdspmdf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
