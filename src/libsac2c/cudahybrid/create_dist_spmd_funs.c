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
    node *spmdstart;
    prf thisprf;
    bool iscond;
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
#define INFO_SPMDSTART(n) ((n)->spmdstart)
#define INFO_THISPRF(n) ((n)->thisprf)
#define INFO_ISCOND(n) ((n)->iscond)
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
    INFO_SPMDSTART (result) = NULL;
    INFO_THISPRF (result) = F_unknown;
    INFO_ISCOND (result) = FALSE;
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

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

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
    node *res, *spmd_fundef;
    info *old_info;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (!INFO_COLLECT (arg_info)) {
        /* we are still looking for a parallel section. */
        if (INFO_THISPRF (arg_info) == F_alloc) {
            /* If this statement is an alloc() primitive, it may be the start of a
             * SPMD, so we save it in info.
             */
            INFO_SPMDSTART (arg_info) = arg_node;
            INFO_THISPRF (arg_info) = F_unknown;

            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

            if (INFO_LIFT (arg_info)) {
                /*
                 * Lift flag is set, we create the SPMD
                 * function call in place of this node.
                 */
                spmd_fundef = CreateSpmdFundef (arg_node, arg_info);
                FUNDEF_NEXT (spmd_fundef) = INFO_SPMDFUNS (arg_info);
                INFO_SPMDFUNS (arg_info) = spmd_fundef;

                res = TBmakeAssign (TBmakeLet (DUPdoDupTree (INFO_RETEXPRS (arg_info)),
                                               TBmakeAp (spmd_fundef,
                                                         INFO_PARAMS (arg_info))),
                                    NULL);
                res = TCappendAssign (res, INFO_NEXTASSIGN (arg_info));

                INFO_PARAMS (arg_info) = NULL;
                INFO_RETEXPRS (arg_info) = NULL;
                INFO_NEXTASSIGN (arg_info) = NULL;
                INFO_LIFT (arg_info) = FALSE;
                INFO_SPMDSTART (arg_info) = NULL;
            } else {
                /*
                 * This is not the start of a parallel section. We return this node.
                 */
                res = arg_node;
            }
        } else if (INFO_THISPRF (arg_info) == F_is_cuda_thread) {
            /*
             * This assignment is the call to _is_cuda_thread_(), so we now know the
             * previous statement (_alloc_()) is the start of a parallel section. We
             * start collecting data for the SPMD function by traversing the previous
             * node, saved in info.
             */
            INFO_THISPRF (arg_info) = F_unknown;
            INFO_COLLECT (arg_info) = TRUE;
            INFO_SPMDSTART (arg_info) = TRAVopt (INFO_SPMDSTART (arg_info), arg_info);
            INFO_COLLECT (arg_info) = FALSE;
            INFO_LIFT (arg_info) = TRUE;
            res = arg_node;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            res = arg_node;
        }
    } else {
        /* we are inside a parallel section collecting data. */
        if (INFO_ISCOND (arg_info)) {
            /*
             * This is the Conditional containing the cuda and host branches of a wl.
             * We only build the SPMD function when traversing back up the tree, so we
             * save the data collected here. We save the next node in the info structure
             * and return NULL, so that we can move the whole assignment chain to the
             * SPMD function easily.
             */
            INFO_ISCOND (arg_info) = FALSE;

            old_info = arg_info;
            arg_info = MakeInfo ();
            INFO_FUNDEF (arg_info) = INFO_FUNDEF (old_info);
            INFO_SPMDFUNS (arg_info) = INFO_SPMDFUNS (old_info);
            INFO_NEXTASSIGN (old_info) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            INFO_SPMDFUNS (old_info) = INFO_SPMDFUNS (arg_info);
            FreeInfo (arg_info);
            arg_info = old_info;

            ASSIGN_NEXT (arg_node) = NULL;
            res = arg_node;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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

    if (self == F_fill) {
        arg = PRF_ARG1 (arg_node);
        if (NODE_TYPE (arg) == N_prf) {
            INFO_THISPRF (arg_info) = PRF_PRF (arg);
        }
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

    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    COND_THEN (arg_node) = TRAVopt (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVopt (COND_ELSE (arg_node), arg_info);

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

    if (INFO_COLLECT (arg_info)
        && (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis)) {
        DBUG_PRINT ("  Not handled before...");
        /*
         * A right hand side variable that has not been handled before must be a
         * free variable of the with-loop and that means it needs to become a
         * parameter of the spmd function to be created.
         */
        new_avis = DUPdoDupNode (avis);

        INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
        INFO_PARAMS (arg_info) = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
        INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
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
        /* check for this avis in the LUT */
        new_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);
        if (new_avis == avis) {
            new_avis = DUPdoDupNode (avis);

            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT (">>> ids %s added to LUT", IDS_NAME (arg_node));

            rhs = INFO_THISPRF (arg_info);
            if (rhs == F_host2dist_spmd || rhs == F_device2dist) {
                /*
                 * If we have a transfer function on the RHS, we are on the LHS of an
                 * assignment which has one of the SPMD results. We add it to info.
                 */

                INFO_RETS (arg_info)
                  = TCappendRet (INFO_RETS (arg_info),
                                 TBmakeRet (TYeliminateAKV (AVIS_TYPE (new_avis)), NULL));

                INFO_RETEXPRS (arg_info)
                  = TCappendExprs (INFO_RETEXPRS (arg_info),
                                   TBmakeExprs (TBmakeId (new_avis), NULL));
            }
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

    if (INFO_COLLECT (arg_info)) {
        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
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
