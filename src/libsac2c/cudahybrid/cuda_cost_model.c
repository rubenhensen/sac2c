/** <!--********************************************************************-->
 *
 * @defgroup cucm CUDA Cost Model
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |  msd  | 15/2/2012
 * can be called on N_fundef               |   -----   |  n  |  msd  | 15/2/2012
 * expects LaC funs                        |   -----   |  y  |  msd  | 15/2/2012
 * follows N_ap to LaC funs                |   -----   |  n  |  msd  | 15/2/2012
 * =============================================================================
 * deals with GLF properly                 |    yes    |     |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |     |       |
 * utilises SAA annotations                |   -----   |  n  |  msd  | 15/2/2012
 * =============================================================================
 * tolerates flattened N_array             |    yes    |     |       |
 * tolerates flattened Generators          |    yes    |     |       |
 * tolerates flattened operation parts     |    yes    |     |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |     |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  n  |  msd  | 15/2/2012
 * =============================================================================
 * </pre>
 *
 * @ingroup cudahybrid
 *
 * Prefix: CUCM
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file cuda_cost_model.c
 *
 * Implements a simple cost model for CUDA withloops, similar to what is done
 * for the multithread backend.
 * Withloops marked as cudarizable are unmarked should they fail any of the cost
 * criteria. At the moment, the only criteria is whether the iteration space
 * meets a minimum size requirement, which obviously can only be done here for
 * when iteration size is know statically.
 * Withloops that meet this criteria are then duplicated and a conditional is
 * formed, similar to what is done for the multithread cost model. The idea is
 * to keep a cudarized and uncudarized version of each withloop, as the
 * transformations each will incur are quite different.
 *
 *
 *****************************************************************************/
#include "cuda_cost_model.h"

#define DBUG_PREFIX "CUCM"
#include "debug.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "memory.h"
#include "LookUpTable.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    /* pointer to the current fundef, required for ssa duplication */
    node *fundef;
    /* ids of current withloop, although more than one probably breaks this */
    node *letids;
    /* is current withloop worth cudarization? */
    bool isworth;
    /* Lookup table storing pairs of old<->duplicated code nodes for the host
     with-loops */
    lut_t *hostlut;
    /* uncudarized duplicate of a withloop */
    node *hostwl;
    /* assignment of the condition variable */
    node *preassigns;
};

/**
 * A template entry in the template info structure
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_ISWORTH(n) (n->isworth)
#define INFO_HOSTLUT(n) (n->hostlut)
#define INFO_HOSTWL(n) (n->hostwl)
#define INFO_PREASSIGNS(n) (n->preassigns)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_ISWORTH (result) = FALSE;
    INFO_HOSTLUT (result) = NULL;
    INFO_HOSTWL (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;

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
 * @fn node *CUCMdoCUDACostModel( node *syntax_tree)
 *
 *****************************************************************************/
node *
CUCMdoCUDACostModel (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_cucm);
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
 * @fn node *ApplySizeCriterion(ntype *array_type)
 *
 * @brief Checks whether an array has the minimum shape size to be run on CUDA.
 *
 *   @param array_size  type of the array
 *   @return            FALSE if shape is not big enough, TRUE otherwise
 *
 *****************************************************************************/
static bool
ApplySizeCriterion (ntype *array_type)
{
    bool size_static, result;
    int size;

    DBUG_ENTER ();

    size_static = TUshapeKnown (array_type);

    if (size_static) {

        DBUG_PRINT ("Found with-loop with static shape.");

        size = SHgetUnrLen (TYgetShape (array_type));

        /* minimum size taken from optimal CUDA thread and block counts*/
        if (size >= global.config.cuda_opt_threads) {
            DBUG_PRINT ("With-loop big enough, allowing cudarization.");
            result = TRUE;
        } else {
            DBUG_PRINT ("With-loop not big enough, uncudarizing.");
            result = FALSE;
        }
    } else {
        DBUG_PRINT ("Found with-loop without static shape.");
        result = TRUE;
    }

    DBUG_RETURN (result);
}

///** <!--********************************************************************-->
// *
// * @fn node *CreateConditionVariable(node *arg_node, info *arg_info)
// *
// * @brief Creates the condition variable if a withloop is worth cudarizing.
// *
// *   @param arg_info  info structure
// *   @return            FALSE if shape is not big enough, TRUE otherwise
// *
// *****************************************************************************/
// static
// void CreateConditionVariable(info *arg_info)
//{
//  node *new_avis, *new_rhs, *new_assign;
//  static int counter = 0;
//
//  DBUG_ENTER ();
//
//  /* flag set by helper function above */
//  if ( INFO_ISWORTH(arg_info) && (INFO_CONDITION( arg_info) == NULL) ) {
//    /* create avis of new condition variable */
//    new_avis = TBmakeAvis( TRAVtmpVarName(""),
//                          TYmakeAKS( TYmakeSimpleType( T_bool),
//                                    SHmakeShape( 0)));
//    /* add new avis to variable declarations */
//    FUNDEF_VARDECS( INFO_FUNDEF( arg_info)) =
//    TBmakeVardec( new_avis, FUNDEF_VARDECS( INFO_FUNDEF( arg_info)));
//
//    new_rhs = TBmakePrf(F_is_cuda_thread,
//                        TBmakeExprs(TBmakeNum(counter), NULL));
//    counter++;
//
//    /* create assignment of new condition variable */
//    new_assign = TBmakeAssign( TBmakeLet( TBmakeIds( new_avis,
//                                                    NULL), new_rhs), NULL);
//    INFO_PREASSIGNS(arg_info) = new_assign;
//    AVIS_SSAASSIGN(new_avis) = new_assign; // SSA property
//
//    INFO_CONDITION( arg_info) = new_avis;
//  }
//
//  DBUG_RETURN();
//}

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
 * @fn node *CUCMfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain, skipping sticky functions.
 *        Current function node is saved in info structure.
 *
 *****************************************************************************/
node *
CUCMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* During the main traversal, we only look at non-prelude functions */
    if (!FUNDEF_ISSTICKY (arg_node) && FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMlet(node *arg_node, info *arg_info)
 *
 * @brief Saves let ids, just in case a with-loop comes up.
 *
 *****************************************************************************/
node *
CUCMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_HOSTWL (arg_info) != NULL) {
        LET_EXPR (arg_node)
          = TBmakeWiths (INFO_HOSTWL (arg_info), TBmakeWiths (LET_EXPR (arg_node), NULL));
        INFO_HOSTWL (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMwith(node *arg_node, info *arg_info)
 *
 * @brief Looks at cudarizable with-loops. Depending on the operation, these
 *        will be classified as worth cudarization or not. If they end up being
 *        worth it, the with loop is duplicated with the duplicate marked as
 *        not cudarizable. Otherwise, we clear the cudarizable flag.
 *
 *****************************************************************************/
node *
CUCMwith (node *arg_node, info *arg_info)
{
    node *hostwl, *hostcode;
    lut_t *hostlut;

    DBUG_ENTER ();

    /* for cudarizable with-loops, we need to figure out if cudarization is
     worthwhile. If so, we duplicate the with-loop so we can also get a host
     version */
    if (WITH_CUDARIZABLE (arg_node)) {
        DBUG_PRINT ("Found cudarizable with-loop.");

        INFO_ISWORTH (arg_info) = FALSE;

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /*
         * if there is a condition in the info structure, then we are going to
         * create two versions of this with-loop. We create a duplicate to run on
         * the host here.
         */
        if (INFO_ISWORTH (arg_info)) {
            /* duplicate the code chain in SSA form, keeping the old<->new
             pairs in a LUT */
            hostlut = LUTgenerateLut ();
            hostcode = DUPdoDupTreeLutSsa (WITH_CODE (arg_node), hostlut,
                                           INFO_FUNDEF (arg_info));

            /* Traverse the partitions. These will be duplicated and the code
             pointers are replaced with the new ones from the LUT. */
            INFO_HOSTLUT (arg_info) = hostlut;
            hostwl = TBmakeWith (TRAVdo (WITH_PART (arg_node), arg_info), hostcode,
                                 DUPdoDupTree (WITH_WITHOP (arg_node)));
            WITH_REFERENCED (hostwl) = WITH_REFERENCED (arg_node);
            WITH_ISFOLDABLE (hostwl) = WITH_ISFOLDABLE (arg_node);

            /* copy any pragmas to the host with-loop. */
            WITH_PRAGMA (hostwl) = DUPdoDupTree (WITH_PRAGMA (arg_node));

            INFO_HOSTWL (arg_info) = hostwl;
            INFO_ISWORTH (arg_info) = FALSE;
            INFO_HOSTLUT (arg_info) = LUTremoveLut (hostlut);

            /* set iscudalocal flag on the index vector */
            if (WITH_CUDARIZABLE (arg_node)) {
                AVIS_ISCUDALOCAL (IDS_AVIS (WITH_VEC (arg_node))) = TRUE;
            }
        } else {
            WITH_CUDARIZABLE (arg_node) = FALSE;
        }

    } else {
        DBUG_PRINT ("Found non-cudarizable with-loop.");
        /* Traverse code to find nested withloops which we may want
         * to parallelize */
        /* Only applies to outer fold loops when cuda parallel folding off (?) */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMpart(node *arg_node, info *arg_info)
 *
 * @brief Returns a identical copy of the node, but with a new CODE pointer from
 *        the LUT.
 *
 *****************************************************************************/
node *
CUCMpart (node *arg_node, info *arg_info)
{
    node *new_part, *new_code, *old_code;
    DBUG_ENTER ();

    old_code = PART_CODE (arg_node);
    /* get new pointer for the code node */
    new_code = LUTsearchInLutPp (INFO_HOSTLUT (arg_info), old_code);
    DBUG_ASSERT (new_code != old_code, "New code block not found in LUT!");

    /* create new part node */
    new_part = TBmakePart (new_code, DUPdoDupTree (PART_WITHID (arg_node)),
                           DUPdoDupTree (PART_GENERATOR (arg_node)));

    CODE_USED (new_code) = CODE_USED (old_code);

    PART_NEXT (new_part) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (new_part);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMgenarray(node *arg_node, info *arg_info)
 *
 * @brief Run cost model on genarray withloops
 *
 *****************************************************************************/
node *
CUCMgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {
        INFO_ISWORTH (arg_info) = ApplySizeCriterion (IDS_NTYPE (INFO_LETIDS (arg_info)));
    }

    /* check next operation, although this probably breaks stuff... */
    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMmodarray(node *arg_node, info *arg_info)
 *
 * @brief Run cost model on modarray withloops
 *
 *****************************************************************************/
node *
CUCMmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {
        INFO_ISWORTH (arg_info) = ApplySizeCriterion (IDS_NTYPE (INFO_LETIDS (arg_info)));
    }

    /* check next operation, although this probably breaks stuff... */
    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMfold(node *arg_node, info *arg_info)
 *
 * @brief Run cost model on fold withloops
 *
 *****************************************************************************/
node *
CUCMfold (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {
        /* Fold loops marked as cudarizable have size criteria applied implicitly
          later on */
        INFO_ISWORTH (arg_info) = TRUE;
    }

    /* check next operation, although this probably breaks stuff... */
    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- CUDA Cost Model -->
 *****************************************************************************/
