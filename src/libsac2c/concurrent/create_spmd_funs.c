/*****************************************************************************
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

#define DBUG_PREFIX "MTSPMDF"
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
    node *neutrals;
    bool collect :1;
    bool lift:1;
    bool withid :1;
    bool isxtfun :1;
    bool inwiths :1;
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
#define INFO_NEUTRALS(n) ((n)->neutrals)
#define INFO_COLLECT(n) ((n)->collect)
#define INFO_LIFT(n) ((n)->lift)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_ISXTFUN(n) ((n)->isxtfun)
#define INFO_INWITHS(n) ((n)->inwiths)

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
    INFO_NEUTRALS (result) = NULL;
    INFO_COLLECT (result) = FALSE;
    INFO_LIFT (result) = FALSE;
    INFO_WITHID (result) = FALSE;
    INFO_ISXTFUN (result) = FALSE;
    INFO_INWITHS (result) = FALSE;

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
    node *spmd_fundef, *fundef, *body, *withlet, *retexprs, *vardecs;
    node *assigns, *args, *rets, *retur;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_let,
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

    withlet = DUPdoDupTreeLut (arg_node, INFO_LUT (arg_info));
    INFO_LUT (arg_info) = LUTremoveContentLut (INFO_LUT (arg_info));

    retur = TBmakeReturn (retexprs);

    assigns = TBmakeAssign (withlet, TBmakeAssign (retur, NULL));

    body = TBmakeBlock (assigns, vardecs);

    BLOCK_ISMTPARALLELBRANCH (body) = TRUE;

    spmd_fundef
      = TBmakeFundef (TRAVtmpVarName (FUNDEF_NAME (fundef)),
                      NSdupNamespace (FUNDEF_NS (fundef)), rets, args, body, NULL);

    /* TODO: We create two SPMD funs: one for the ST caller and one for the XT caller.
     * But they are identical inside and could be shared. */
    FUNDEF_ISXTSPMDFUN (spmd_fundef) = INFO_ISXTFUN (arg_info);
    FUNDEF_ISSPMDFUN (spmd_fundef) = !INFO_ISXTFUN (arg_info);
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
 * @fn node *MTSPMDFfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTSPMDFfundef (node *arg_node, info *arg_info)
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

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    spmdfuns = TCappendFundef (spmdfuns, FUNDEF_NEXT (arg_node));
    FUNDEF_NEXT (arg_node) = spmdfuns;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTSPMDFdo( node *arg_node, info *arg_info)
 * @brief all we do here is
 *        a) ensure traversal of BODY happens before that of COND. Reason for
 *        this is that COND typically refers to variables defined in BODY which
 *        are, hence, not relatively free.
 *        b) After traversing the body, we also need to traverse the DO_SKIP
 *        part! Skipping the DO_SKIP part is *not* an option. One may believe
 *        that the skip part contains only RC-free operations and therefore
 *        does not need to be inspected. However, fun2lac sometimes inserts
 *        renamings here which rely on the existance of relatively free variables
 *        and that are *not* referenced anywhere in the body.
 *
 *        See bug 1136 for details.
 *
 *****************************************************************************/

node *
MTSPMDFdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT (" do-loop: traversing body");
    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    DBUG_PRINT (" do-loop: traversing cond");
    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    DBUG_PRINT (" do-loop: traversing skip");
    DO_SKIP (arg_node) = TRAVopt (DO_SKIP (arg_node), arg_info);

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

    DBUG_ENTER ();

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
        } else {
            if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
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

    DBUG_ENTER ();

    avis = IDS_AVIS (arg_node);
    new_avis = NULL;

    DBUG_PRINT ("ENTER ids %s", IDS_NAME (arg_node));

    if (INFO_COLLECT (arg_info)) {
        if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
            new_avis = DUPdoDupNode (avis);
            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_LUT (arg_info) = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);
            DBUG_PRINT (">>> ids %s added to LUT", IDS_NAME (arg_node));
        }
    } else {
        if (INFO_LIFT (arg_info)) {
            /*
             * If INFO_LIFT is true, we are on the LHS of the assignment which has the
             * MT-Withloop on the RHS.
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
 *   node *MTSPMDFwiths( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a parallelised with-loop into a function.
 *
 ******************************************************************************/

node *
MTSPMDFwiths (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Start collecting data flow information
     */
    INFO_INWITHS (arg_info) = TRUE;
    INFO_COLLECT (arg_info) = TRUE;

    WITHS_WITH (arg_node) = TRAVdo (WITHS_WITH (arg_node), arg_info);
    WITHS_NEXT (arg_node) = TRAVopt (WITHS_NEXT (arg_node), arg_info);

    INFO_INWITHS (arg_info) = FALSE;
    INFO_COLLECT (arg_info) = FALSE;
    /*
     * Stop collecting data flow information
     */
    INFO_LIFT (arg_info) = TRUE;

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
    DBUG_ENTER ();

    /*
     * to start collecting data flow information, this with-loop must
     * not only be parallelizable, but also not part of a distributed
     * with-loop. In the latter case, the WITHS node starts the data flow
     * collecting.
     */
    if (WITH2_PARALLELIZE (arg_node) && !INFO_INWITHS (arg_info)) {
        /*
         * Start collecting data flow information
         */
        INFO_COLLECT (arg_info) = TRUE;

        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

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
             * parallelised with-loop, then we are currently still looking for a
             * with-loop to be parallelised. This may only occur in the code subtree.
             */
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MTSPMDFwith( node *arg_node, info *arg_info)
 *
 * description:
 *   lifts a parallelised with-loop into a function.
 *
 ******************************************************************************/

node *
MTSPMDFwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_COLLECT (arg_info)) {
        /*
         * If we are already in the collect mode, we currently gather the data
         * flow information. Hence, we must traverse all sons.
         */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
        WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    } else {
        /*
         * If we have no outer parallelised with-loop or this is not part of a
         * distributed with-loop, then we are currently still looking for a
         * with-loop to be parallelised. This may only occur in the code subtree.
         */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
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
    DBUG_ENTER ();

    INFO_WITHID (arg_info) = TRUE;

    WITHID_VEC (arg_node) = TRAVopt (WITHID_VEC (arg_node), arg_info);
    WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);
    WITHID_IDXS (arg_node) = TRAVopt (WITHID_IDXS (arg_node), arg_info);

    INFO_WITHID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTSPMDFfold( node *arg_node, info *arg_info)
 *
 *    @brief traversal function for N_fold node
 *      Here, we need to identify the neutral element(s) and generate
 *      _dec_rc_free( neutral) operation(s) that we collect in
 *      INFO_NEUTRALS (arg_info).
 *
 *    @param arg_node
 *    @param arg_info
 *
 *    @return arg_node
 *
 ******************************************************************************/

node *
MTSPMDFfold (node *arg_node, info *arg_info)
{
    node * neutr;
    node * dec_rc;
    DBUG_ENTER ();
    
    if (INFO_COLLECT (arg_info)) {
        neutr = FOLD_NEUTRAL (arg_node);
        DBUG_ASSERT ((NODE_TYPE (neutr) == N_id),
                     "non N_id neutral element in fold found");
        dec_rc = TCmakePrf2 (F_dec_rc, TBmakeId (ID_AVIS (neutr)), TBmakeNum (1));
        INFO_NEUTRALS (arg_info) = TBmakeAssign (TBmakeLet (NULL, dec_rc),
                                                 INFO_NEUTRALS (arg_info));
        
        // Finally we traverse all sons (including the neutral!)
        arg_node = TRAVcont (arg_node, arg_info);
    }

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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtspmdf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
