/*****************************************************************************
 *
 * $Id$
 *
 * file:   create_spmd_funs.c
 *
 * prefix: MTSPMDF
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

#include "create_spmd_funs.h"

#include "dbug.h"
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
    node *allocassigns;
    node *freeassigns;
    bool collect;
    bool lift;
    bool withid;
    bool wlparallel;
    bool allocneeded;
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
#define INFO_ALLOCASSIGNS(n) ((n)->allocassigns)
#define INFO_FREEASSIGNS(n) ((n)->freeassigns)
#define INFO_COLLECT(n) ((n)->collect)
#define INFO_LIFT(n) ((n)->lift)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_WLPARALLEL(n) ((n)->wlparallel)
#define INFO_ALLOCNEEDED(n) ((n)->allocneeded)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_SPMDFUNS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_ARGS (result) = NULL;
    INFO_PARAMS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_RETS (result) = NULL;
    INFO_RETEXPRS (result) = NULL;
    INFO_ALLOCASSIGNS (result) = NULL;
    INFO_FREEASSIGNS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_LIFT (result) = FALSE;
    INFO_WITHID (result) = FALSE;
    INFO_WLPARALLEL (result) = FALSE;
    INFO_ALLOCNEEDED (result) = FALSE;

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
    node *spmd_fundef, *fundef, *body, *withlet, *retexprs, *vardecs;
    node *allocassigns, *freeassigns, *assigns, *args, *rets, *retur;

    DBUG_ENTER ("CreateSpmdFundef");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_let,
                 "CreateSpmdFundef() called with illegal node type.");

    fundef = INFO_FUNDEF (arg_info);

    retexprs = INFO_RETEXPRS (arg_info);
    INFO_RETEXPRS (arg_info) = NULL;

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    allocassigns = INFO_ALLOCASSIGNS (arg_info);
    INFO_ALLOCASSIGNS (arg_info) = NULL;

    freeassigns = INFO_FREEASSIGNS (arg_info);
    INFO_FREEASSIGNS (arg_info) = NULL;

    rets = INFO_RETS (arg_info);
    INFO_RETS (arg_info) = NULL;

    args = INFO_ARGS (arg_info);
    INFO_ARGS (arg_info) = NULL;

    withlet = DUPdoDupTreeLut (arg_node, INFO_LUT (arg_info));
    INFO_LUT (arg_info) = LUTremoveContentLut (INFO_LUT (arg_info));

    retur = TBmakeReturn (retexprs);

    assigns
      = TCappendAssign (allocassigns,
                        TCappendAssign (TBmakeAssign (withlet, NULL),
                                        TCappendAssign (freeassigns,
                                                        TBmakeAssign (retur, NULL))));
    body = TBmakeBlock (assigns, vardecs);

    spmd_fundef
      = TBmakeFundef (TRAVtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, body, NULL);

    FUNDEF_ISSPMDFUN (spmd_fundef) = TRUE;
    FUNDEF_RETURN (spmd_fundef) = retur;

    DBUG_RETURN (spmd_fundef);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTSPMDFmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTSPMDFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTSPMDFmodule");

    INFO_LUT (arg_info) = LUTgenerateLut ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTSPMDFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTSPMDFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTSPMDFfundef");

    if (FUNDEF_ISSTFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST funs may contain parallel with-loops.
         * Hence, we constrain our search accordingly.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    } else {
        /*
         * We have reached the end of the FUNDEF chain. We add the new SPMD functions
         * constructed meanwhile and stored in the info structure to the end and stop
         * the traversal.
         */
        FUNDEF_NEXT (arg_node) = INFO_SPMDFUNS (arg_info);
        INFO_SPMDFUNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTSPMDFlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTSPMDFlet (node *arg_node, info *arg_info)
{
    node *spmd_fundef, *spmd_ap;

    DBUG_ENTER ("MTSPMDFlet");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    if (INFO_LIFT (arg_info)) {
        spmd_fundef = CreateSpmdFundef (arg_node, arg_info);
        FUNDEF_NEXT (spmd_fundef) = INFO_SPMDFUNS (arg_info);
        INFO_SPMDFUNS (arg_info) = spmd_fundef;

        spmd_ap = TBmakeAp (spmd_fundef, INFO_PARAMS (arg_info));
        INFO_PARAMS (arg_info) = NULL;

        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = spmd_ap;

        INFO_LIFT (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFid(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
MTSPMDFid (node *arg_node, info *arg_info)
{
    node *avis, *new_avis, *ids, *dim, *shape, *alloc, *free;

    DBUG_ENTER ("MTSPMDFid");

    avis = ID_AVIS (arg_node);

    DBUG_PRINT ("MTSPMDF", ("ENTER id %s", ID_NAME (arg_node)));

    if (INFO_COLLECT (arg_info)) {
        if (INFO_WITHID (arg_info)) {
            /*
             * As a withid this N_id node actually represents a left hand side variable.
             */
            DBUG_PRINT ("MTSPMDF", ("...is Withid-id"));

            if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
                DBUG_PRINT ("MTSPMDF", ("  Not handled before..."));
                new_avis = DUPdoDupNode (avis);
                INFO_VARDECS (arg_info)
                  = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);

                DBUG_PRINT ("MTSPMDF", (">>> ids %s added to LUT", ID_NAME (arg_node)));

                if (INFO_WLPARALLEL (arg_info) && INFO_ALLOCNEEDED (arg_info)) {
                    /*
                     * This is the top-level with-loop, which is to be parallelised.
                     * We need to recreate the alloc and free statements for placement
                     * inside the spmd function as the original such statements are
                     * outside the with-loop and hence remain in the ST function, from
                     * where they need to be cleared later.
                     */

                    if (TUdimKnown (AVIS_TYPE (new_avis))) {
                        dim = TBmakeNum (TYgetDim (AVIS_TYPE (new_avis)));
                    } else {
                        dim = NULL;
                    }

                    if (TUshapeKnown (AVIS_TYPE (new_avis))) {
                        shape = SHshape2Array (TYgetShape (AVIS_TYPE (new_avis)));
                    } else {
                        shape = NULL;
                    }

                    alloc = TCmakePrf3 (F_alloc, TBmakeNum (1), dim, shape);

                    ids = TBmakeIds (new_avis, NULL);

                    INFO_ALLOCASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (ids, alloc),
                                      INFO_ALLOCASSIGNS (arg_info));

                    free = TCmakePrf1 (F_free, TBmakeId (new_avis));
                    INFO_FREEASSIGNS (arg_info)
                      = TBmakeAssign (TBmakeLet (NULL, free),
                                      INFO_FREEASSIGNS (arg_info));
                }
            }
        } else {
            if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
                DBUG_PRINT ("MTSPMDF", ("  Not handled before..."));
                /*
                 * A right hand side variable that has not been handled before must be a
                 * free variable of the with-loop and that means it needs to become a
                 * parameter of the spmd function to be created.
                 */
                new_avis = DUPdoDupNode (avis);

                INFO_ARGS (arg_info) = TBmakeArg (new_avis, INFO_ARGS (arg_info));
                INFO_PARAMS (arg_info)
                  = TBmakeExprs (TBmakeId (avis), INFO_PARAMS (arg_info));
                INFO_LUT (arg_info)
                  = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFids(node *arg_node, info *arg_info)
 *
 *    @brief
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return
 ******************************************************************************/

node *
MTSPMDFids (node *arg_node, info *arg_info)
{
    node *avis;
    node *new_avis;

    DBUG_ENTER ("MTSPMDFids");

    avis = IDS_AVIS (arg_node);
    new_avis = NULL;

    DBUG_PRINT ("MTSPMDF", ("ENTER ids %s", IDS_NAME (arg_node)));

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT ("MTSPMDF", (">>> ids %s added to LUT", IDS_NAME (arg_node)));
        }
    } else {
        if (INFO_LIFT (arg_info)) {
            /*
             * If INFO_LIFT is true, we are on the LHS of the assignment which has the
             * MT-Withloop on the RHS.
             */
            new_avis = LUTsearchInLutPp (INFO_LUT (arg_info), avis);

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
        }
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTSPMDFwith2( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a parallelised with-loop into a function.
 *
 ******************************************************************************/

node *
MTSPMDFwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTSPMDFwith2");

    if (WITH2_PARALLELIZE (arg_node)) {
        /*
         * Start collecting data flow information
         */
        INFO_COLLECT (arg_info) = TRUE;
        INFO_WLPARALLEL (arg_info) = TRUE;

        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

        INFO_WLPARALLEL (arg_info) = FALSE;

        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

        INFO_COLLECT (arg_info) = FALSE;
        /*
         * Stop collecting data flow information
         */
        INFO_LIFT (arg_info) = TRUE;
    } else {
        if (INFO_COLLECT (arg_info)) {
            /*
             * If we are already in the collect mode, we currently gather the data
             * flow information for an outer parallelised with-loop. Hence, we must
             * traverse all sons.
             */
            WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
            WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
            WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        } else {
            /*
             * If we do not want to parallise this with-loop and we have no outer
             * parallelised with-loop, than we are currently still looking for a
             * with-loop to be parallelised. This may only occur in the code subtree.
             */
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFwithid( node *arg_node, info *arg_info)
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
MTSPMDFwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MTSPMDFwithid");

    INFO_WITHID (arg_info) = TRUE;
    INFO_ALLOCNEEDED (arg_info) = WITHID_VECNEEDED (arg_node);

    WITHID_VEC (arg_node) = TRAVopt (WITHID_VEC (arg_node), arg_info);

    INFO_ALLOCNEEDED (arg_info) = FALSE;

    WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
    WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);

    INFO_WITHID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn MTSPMDFdoCreateSpmdFuns( node *syntax_tree)
 *
 *  @brief initiates traversal for creating SPMD functions
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTSPMDFdoCreateSpmdFuns (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MTSPMDFdoCreateSpmdFuns");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtspmdf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
