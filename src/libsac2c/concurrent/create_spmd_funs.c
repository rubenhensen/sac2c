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
 *   function definition and the with-loop itself is replaced by a
 *   call of that function. The newly created fundefs are named and tagged
 *   SPMD functions. These functions will later implement switching from
 *   single-threaded execution to multithreaded execution and vice versa.
 *
 *   The necessary information to create a fully-fledged function definition,
 *   e.g. parameter names and types, return values and types, local variables
 *   and their types, is gathered during a full traversal of each with-loop
 *   tagged for multithreaded execution.
 *
 *   The PARAMETERS/ARGS of the spmd functions are inferred by looking for
 *   RELATIVELY FREE VARIABLES within the with-loop. This includes not only
 *   RFVs in the code but also within the generators and operators of the
 *   with-loop making sure that we capture things such as bounds, neutral
 *   elements, or the memory for the result of genarray with loops.
 *
 *   Although index variables (vectors, scalars, and IDXs) are RFVs as well
 *   they require special treatment. They need to be thread local and are
 *   treated as such (see below). Furthermore, their handling needs to be
 *   shifted from the calling context into the lifted function. Code
 *   for handling their memory will be generated in the spmd functions
 *   through a later traversal MTRMI (restore memory instructions).
 *   The redundant declarations of these in the calling context are being
 *   taken care of in yet another traversal named MTDCR (dead code removal).
 *
 *   The RETURN VALUES and types of the spmd functions are taken from
 *   the left hand side of the with-loops.
 *
 *   For all LOCAL VARIABLES needed in the new spmd functions we collect
 *   those variables that are being locally defined within the with loop
 *   body. We add to these the left hand side variables of the with-loop
 *   and, for fold with loops, we provide freshly named temporary variables
 *   for each return value. These are needed for the fold operation within
 *   the synchronisation barrier.
 *
 *   Finally, the newly generated functions are inserted at the end of the
 *   fundef chain.
 *
 *
 *   For example, we transform
 *
 *      a = with {
 *            ( lb <= iv < ub) {
 *               res = max( b[iv]);
 *            } : res;
 *          } : genarray( [100], 0);
 *
 *   into
 *
 *      a = spmdfun( lb, ub, b);
 *
 *   and a new function
 *
 *      int[100] spmdfun( int[1] lb, int[1] ub, int[100,30] b)
 *      {
 *         int[100] a;
 *         int res;
 *         int[1] iv;
 *
 *         a = with {
 *               ( lb <= iv < ub) {
 *                  res = max( b[iv]);
 *               } : res;
 *             } : genarray( [100], 0);
 *         return a;
 *      }
 *
 *   Note, that in the implementation we always deal with N_with2 here and
 *   that this also ensures that the variable holding the memory for the
 *   result is identified as relatively free variable :-)
 *
 *   Finally, this phase makes one more change. It deals with a subtle issue
 *   that occurs in the context of fold-WLs with non-scalar neutral elements:
 *
 *   To understand the issue, we need to understand how this phase relates to
 *   the memory management in SaC.
 *   The memory handling is done in the mem-phase which precedes the mt phase.
 *   The idea is that the choice as to whether we execute a WL sequentially or
 *   in parallel does not majorly effect the memory handling.
 *   Relatively free variables are kept alive until after the WL is completed
 *   in both scenarios (seq and mt); so there is no need to adjust that when
 *   switching from seq to mt.
 *   By lifting the local variables from the calling context into the spmd
 *   function we implicitly replicate all those across the threads.
 *   For most of these variables, this does not change anything as WL local
 *   variables typically cannot evade the scope of the WL and, therefore, a
 *   replication of the entire WL body is exactly what is needed.
 *
 *   However, there is one potential way for such a replicated local variable
 *   to evade the scope of the WL and that is through a fold-WL. In such a
 *   scenario, it is still not possible for all replicas to evade but, due
 *   to the folding operation, exactly one non-deterministically chosen replica
 *   can evade. Given this, it may seem that the memory assumptions for fold WLs
 *   can be carried over unchanged from seq to mt as well.
 *
 *   Unfortunately, this is not quite the case. In order to maximise reuse, the
 *   memory management of Fold-WLs allows the neutral elements to be reused
 *   for the accumulator whenever the fold operations permits this.
 *   While this is certainly desirable in case of a sequential execution
 *   (here it even would allow for suballoc within nested WLs), it breaks
 *   in the context of mt executions. Here, the neutral element is classified
 *   as relatively free variable, and it is assumed to survive the WL. (This
 *   is inevitable since we could reuse the neutral element for one thread
 *   at most anyways)
 *   While our noRC-in-parallel-WL-bodies regime enables survival of the neutral
 *   element, the non-mt aware mem phase assumes that the neutral element is
 *   always consumed *within* the WL, either by reuse or by discarding.
 *   Consequently, we cannot simply lift fold with-loops without adjusting
 *   the memory management of the neutral element. If we did, we would
 *   generate a space leak on the neutral elements of parallel fold with
 *   loops. We resolve this problem by injecting a _dec_rc_(neutral, 1)
 *   directly after the call to the spmd function.
 *   This is done in this phase as well.
 *
 *   So, for example we transform
 *
 *      neutr = [0,0,0];
 *      a = with {
 *            ( lb <= iv < ub) {
 *               res = foo( b[iv]);
 *            } : res;
 *          } : fold( +, neutr);
 *
 *   into
 *
 *      neutr = [0,0,0];
 *      a = spmdfun (lb, ub, b);
 *      _dec_rc_ (neutr, 1)        <<<< MT-only parallel fold mem adjustment
 *
 *   and a new function
 *
 *      int[3] spmdfun( int[1] lb, int[1] ub, int[100,3] b, int[3] neutr)
 *      {
 *         int[3] fresh_tmp_a;
 *         int[3] a;
 *         int[3] res;
 *         int[1] iv;
 *
 *         a = with {
 *               ( lb <= iv < ub) {
 *                  res = foo( b[iv]);
 *               } : res;
 *             } : fold( +, neutr);
 *         return a;
 *      }
 *
 *   Another aspect to notice here is that this adjustment is ONLY needed
 *   if the fold-WL is on the outermost level! If the fold-WL is inside a
 *   modarray or genarray WL, the RCI phase in mem "understands" that the
 *   neutral element will not be reused but the result of the fold will
 *   be copied into the overall result! It therefore, injects the dec_rc
 *   or free operation after the outer WL. In that case, this adjustment
 *   must not happen! See the test "test-mt-fold-non-scalar-nested.sac"
 *   for an example!
 *
 * implementation:
 *   Most of the analysis is happening in MTSPMDFid and in MTSPMDFids. Here,
 *   the RFVs and local vars are being identified, and the corresponding parts
 *   of the new code are being created.
 *   RFVs are being transformed into parameters (collected in INFO_PARAMS) and
 *   argument expressions (collected in INFO_ARGS). Local variables are
 *   transformed into variable declarations (collected in INFO_VARDECS).
 *   Key to this process is the use of a LUT. Here we keep pointers to
 *   N_avis nodes that we have encountered already. For each such entry
 *   we create a duplicate of the N_avis and hold that in the LUT as well.
 *   Later, when doing the actual function creation, we exchange all
 *   old N_avis nodes by the new ones using that very LUT.
 *   This also allows us to avoid unnecessary duplicates when encountering
 *   the same variable more than once and it allows us to disambiguate
 *   RFVs from uses of local variables.
 *   The actual checking and code generation for the two cases
 *   (RFVs and local vars) are being done in local helper functions
 *      void HandleLocal( node *avis, info *arg_info)     and
 *      void HandleUse( node *avis, info *arg_info)
 *
 *   We use a flag INFO_COLLECT to indicate when we are traversing a
 *   with loop that we want to lift out. It is being set before traversing
 *   the body of a to be parallelised with-loops and is reset once the
 *   N_assign of that with-loop is being reached on the way back up.
 *   The actual function creation is done by a helper function
 *      node * CreateSpmdFundef (node *arg_node, info *arg_info)
 *   which is initiated on the N_let level.
 *
 *   To facilitate the fold with loop memory adjustment, we generate that
 *   code when traversing N_fold and put it into INFO_NEUTRALS.
 *   As this only has to happen for top-level fold-WLs, we use INFO_TOPLEVEL
 *   as an indicator to decide whether to adjust or not.
 *   This code is eventually inserted after the spmd function call in
 *   MTSPMDFassign.
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
    bool toplevel :1;
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
#define INFO_TOPLEVEL(n) ((n)->toplevel)
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
    INFO_TOPLEVEL (result) = TRUE;
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
 * @fn static void HandleLocal( node *avis, info *arg_info)
 *
 * @brief checks whether the N_avis is already in the LUT;
 *   if not, it creates a copy, inserts the pair into the LUT, and it creates
 *   a vardec in INFO_VARDECS
 *
 *****************************************************************************/

static void HandleLocal( node *avis, info *arg_info)
{
    node * new_avis;

    DBUG_ENTER ();
    if (LUTsearchInLutPp (INFO_LUT (arg_info), avis) == avis) {
        DBUG_PRINT ("  Not handled before...");
        new_avis = DUPdoDupNode (avis);
        INFO_VARDECS (arg_info)
          = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
        INFO_LUT (arg_info)
          = LUTinsertIntoLutP (INFO_LUT (arg_info), avis, new_avis);

        DBUG_PRINT (">>> local %s added to LUT", AVIS_NAME (avis));
    }
    DBUG_RETURN ();
}

/** <!--********************************************************************-->
 *
 * @fn static void HandleUse( node *avis, info *arg_info)
 *
 * @brief checks whether the N_avis is already in the LUT;
 *   if not the variable is deemed RFV and it creates a copy, inserts the pair
 *   into the LUT, and it creates an N_arg of the new avis in INFO_ARGS and
 *    an N_id of the old one in INFO_PARAMS.
 *
 *****************************************************************************/

static void HandleUse( node *avis, info *arg_info)
{
    node *new_avis;

    DBUG_ENTER ();
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
    DBUG_RETURN ();
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

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

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

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

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
MTSPMDFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_LIFT (arg_info)) {
        if (INFO_NEUTRALS (arg_info) != NULL) {
           ASSIGN_NEXT (arg_node) = TCappendAssign( INFO_NEUTRALS (arg_info),
                                                    ASSIGN_NEXT (arg_node));
           INFO_NEUTRALS (arg_info) = NULL;
        }

        INFO_LIFT (arg_info) = FALSE;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

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

    LET_IDS (arg_node) = TRAVopt(LET_IDS (arg_node), arg_info);

    if (INFO_LIFT (arg_info)) {
        spmd_fundef = CreateSpmdFundef (arg_node, arg_info);
        FUNDEF_NEXT (spmd_fundef) = INFO_SPMDFUNS (arg_info);
        INFO_SPMDFUNS (arg_info) = spmd_fundef;

        spmd_ap = TBmakeAp (spmd_fundef, INFO_PARAMS (arg_info));
        INFO_PARAMS (arg_info) = NULL;

        LET_EXPR (arg_node) = FREEdoFreeTree (LET_EXPR (arg_node));
        LET_EXPR (arg_node) = spmd_ap;
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
    node *avis;

    DBUG_ENTER ();

    avis = ID_AVIS (arg_node);

    DBUG_PRINT ("ENTER id %s", ID_NAME (arg_node));

    if (INFO_COLLECT (arg_info)) {
        if (INFO_WITHID (arg_info)) {
            /*
             * As a withid this N_id node actually represents a left hand side variable.
             */
            DBUG_PRINT ("...is Withid-id");
            HandleLocal (avis, arg_info);
        } else {
            HandleUse (avis, arg_info);
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

    DBUG_PRINT ("ENTER ids %s", IDS_NAME (arg_node));

    if (INFO_COLLECT (arg_info)) {
        HandleLocal (avis, arg_info);
    } else {
        if (INFO_LIFT (arg_info)) {
            /*
             * If INFO_LIFT is true, we are on the LHS of the assignment which has the
             * MT-Withloop on the RHS.
             */
            HandleLocal (avis, arg_info);

            new_avis = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), avis);

            INFO_RETS (arg_info)
              = TCappendRet (INFO_RETS (arg_info),
                             TBmakeRet (TYeliminateAKV (AVIS_TYPE (new_avis)), NULL));

            INFO_RETEXPRS (arg_info)
              = TCappendExprs (INFO_RETEXPRS (arg_info),
                               TBmakeExprs (TBmakeId (new_avis), NULL));
        }
    }

    IDS_NEXT (arg_node) = TRAVopt(IDS_NEXT (arg_node), arg_info);

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
    bool toplevel;
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
        toplevel = INFO_TOPLEVEL (arg_info);

        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

        INFO_TOPLEVEL (arg_info) = FALSE;
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        INFO_TOPLEVEL (arg_info) = toplevel;

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
        if (INFO_TOPLEVEL (arg_info)) {
            neutr = FOLD_NEUTRAL (arg_node);
            DBUG_ASSERT ((NODE_TYPE (neutr) == N_id),
                         "non N_id neutral element in fold found");
            dec_rc = TCmakePrf2 (F_dec_rc, TBmakeId (ID_AVIS (neutr)), TBmakeNum (1));
            INFO_NEUTRALS (arg_info) = TBmakeAssign (TBmakeLet (NULL, dec_rc),
                                                     INFO_NEUTRALS (arg_info));
        }

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
