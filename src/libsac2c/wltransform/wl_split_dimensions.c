/** <!--********************************************************************-->
 *
 * @defgroup wlsd With-Loop Split Dimensions
 *
 * Module description goes here.
 *
 * @ingroup wlsd
 *
 * @{
 *
 *****************************************************************************/
/*#define MUTC_MODARRAY*/
/** <!--********************************************************************-->
 *
 * @file wl_split_dimensions.c
 *
 * Prefix: WLSD
 *
 *****************************************************************************/
#include "wl_split_dimensions.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "WLSD"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "shape.h"
#include "type_utils.h"
#include "constants.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "pattern_match.h"
#include "LookUpTable.h"
#include "ctinfo.h"
#include "globals.h"
#include "str.h"

#define INC 3

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 * INFO_WITH2_IVECT:           Points to the ids of the index vector of the
 *                             with2 loop that is being transformed.
 * INFO_WITH2_ISCLS:           Points to the ids of the index scalars of the
 *                             with2 loop that is being transformed.
 * INFO_WITH2_OFFSETS:         Points to the offsets (wlidx) of the with2 loop
 *                             that is being transformed. For fold with-loops
 *                             that don't have an offset the N_ids contains
 *                             a NULL pointer.
 * INFO_WITH2_WITHOPS:         Points to the withops of the with2 loop that is
 *                             being transformed.
 * INFO_WITH2_LHS:             Points to the lhs ids of the with2 loop that is
 *                             being transformed.
 * INFO_WITH2_LENGTHS:         Holds an N_set chain of N_exprs nodes encoding
 *                             the unrolling length of each dimension for each
 *                             operator that has an index.
 * INFO_CURRENT_DIM:           Dimension currently being transformed.
 * INFO_CURRENT_SIZE:          Length of current subarray to be computed (used
 *                             for shape of genarray/modarray).
 * INFO_WITH3_ASSIGN:          Assignment node for current with3
 * INFO_INDICES:               New withloop indices in reverse order as a
 *                             chain of N_ids nodes.
 * INFO_OFFSETS:               Current withloop offset as N_ids chain. Exists
 *                             only for those operators that have an offset).
 *                             This offset corresponds to the wlidx of the
 *                             originating with2, i.e., it spans all dimensions.
 *                             Used to translate references to the original
 *                             withloop offset.
 * INFO_VARDECS:               Stores vardecs that need to be joined into
 *                             the fundef vardec chain.
 * INFO_PREASSIGNS:            Holds assignments that need to be inserted before
 *                             the current assign.
 * INFO_LUT:                   lut used sharedly for renaming. Needs always to
 *                             be cleared before traversing on.
 * INFO_FUNDEF:                stores the current fundef node for compatability
 *                             with DupTree
 * INFO_WITH3_NESTING:         The level of with3 body that we are creating
 *                             1 is outer most body.
 * INFO_TRANSFORMED_W2_TO_W3   Transformed with2 loops to with3 loops
 *
 * NOT IMPLEMENTED TRAVERSAL
 *
 * INFO_NIP_RESULT:            Used in anonymous traversal NI to check for
 *                             not yet implemented with-loop operators.
 * INFO_NIP_LHS:               Points to left-hand side of with3 loop that
 *                             is checked for compliance with the current
 *                             implementation status.
 *****************************************************************************/
struct INFO {
    node *with2_ivect;
    node *with2_iscls;
    node *with2_offsets;
    node *with2_withops;
    node *with2_lhs;
    node *with2_lengths;
    int current_dim;
    int dim_frame;
    node *current_size;
    node *with3_assign;
    node *indices;
    node *frame_indices;
    node *offsets;
    node *vardecs;
    node *preassigns;
    node *block_chunk;
    lut_t *lut;
    node *fundef;
    int with3_nesting;
    bool dense;
    bool transformed_w2_to_w3;
    bool incudawl;

    bool nip_result;
    node *nip_lhs; /* pointer info with2_lhs*/
    bool nip_arg;
};

#define INFO_WITH2_IVECT(n) ((n)->with2_ivect)
#define INFO_WITH2_ISCLS(n) ((n)->with2_iscls)
#define INFO_WITH2_OFFSETS(n) ((n)->with2_offsets)
#define INFO_WITH2_WITHOPS(n) ((n)->with2_withops)
#define INFO_WITH2_LHS(n) ((n)->with2_lhs)
#define INFO_WITH2_LENGTHS(n) ((n)->with2_lengths)
#define INFO_CURRENT_DIM(n) ((n)->current_dim)
#define INFO_DIM_FRAME(n) ((n)->dim_frame)
#define INFO_FRAME_INDICES(n) ((n)->frame_indices)
#define INFO_CURRENT_SIZE(n) ((n)->current_size)
#define INFO_WITH3_ASSIGN(n) ((n)->with3_assign)
#define INFO_INDICES(n) ((n)->indices)
#define INFO_OFFSETS(n) ((n)->offsets)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_BLOCK_CHUNK(n) ((n)->block_chunk)
#define INFO_LUT(n) ((n)->lut)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_WITH3_NESTING(n) ((n)->with3_nesting)
#define INFO_DENSE(n) ((n)->dense)
#define INFO_TRANSFORMED_W2_TO_W3(n) ((n)->transformed_w2_to_w3)
#define INFO_INCUDAWL(n) ((n)->incudawl)

#define INFO_NIP_RESULT(n) ((n)->nip_result)
#define INFO_NIP_LHS(n) ((n)->nip_lhs)
#define INFO_NIP_WLSEGS(n) ((n)->nip_wlsegs)
#define INFO_NIP_ARG(n) ((n)->nip_arg)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_WITH2_IVECT (result) = NULL;
    INFO_WITH2_ISCLS (result) = NULL;
    INFO_WITH2_OFFSETS (result) = NULL;
    INFO_WITH2_WITHOPS (result) = NULL;
    INFO_WITH2_LHS (result) = NULL;
    INFO_WITH2_LENGTHS (result) = NULL;
    INFO_CURRENT_DIM (result) = 0;
    INFO_DIM_FRAME (result) = -1;
    INFO_CURRENT_SIZE (result) = NULL;
    INFO_WITH3_ASSIGN (result) = NULL;
    INFO_INDICES (result) = NULL;
    INFO_FRAME_INDICES (result) = NULL;
    INFO_OFFSETS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = LUTgenerateLut ();
    INFO_FUNDEF (result) = NULL;
    INFO_WITH3_NESTING (result) = 0;
    INFO_TRANSFORMED_W2_TO_W3 (result) = FALSE;
    INFO_INCUDAWL (result) = FALSE;
    INFO_BLOCK_CHUNK (result) = NULL;

    INFO_NIP_RESULT (result) = FALSE;
    INFO_NIP_LHS (result) = NULL;
    INFO_NIP_ARG (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_LUT (info) = LUTremoveLut (INFO_LUT (info));

    if (INFO_FRAME_INDICES (info) != NULL) {
        INFO_FRAME_INDICES (info) = FREEdoFreeTree (INFO_FRAME_INDICES (info));
    }

    if (INFO_BLOCK_CHUNK (info) != NULL) {
        INFO_BLOCK_CHUNK (info) = FREEdoFreeTree (INFO_BLOCK_CHUNK (info));
    }

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn info *InitialiseNipInfo( info *info)
 *
 * @brief Initialises the INFO_NIP_LHS and INFO_NIP_RESULT fields to the LHS
 *        of the current with2 and FALSE, respectively.
 *
 * @param info current traversal state
 *
 * @return updated traversal state
 ******************************************************************************/

static info *
InitialiseNipInfo (info *info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (info != NULL, "Need an info to crate a nip info");
    DBUG_ASSERT (INFO_WITH2_LHS (info) != NULL, "Need a lhs to create a nip info");

    INFO_NIP_LHS (info) = INFO_WITH2_LHS (info);
    INFO_NIP_RESULT (info) = FALSE;
    INFO_NIP_ARG (info) = FALSE;

    DBUG_RETURN (info);
}

/** <!-- ****************************************************************** -->
 * @fn info *ResetNipInfo( info *info)
 *
 * @brief Resets the INFO_NIP_* fields of the info structure to their defaults.
 *
 * @param info current traversal state
 *
 * @return updated traversal state
 ******************************************************************************/
static info *
ResetNipInfo (info *info)
{
    DBUG_ENTER ();

    INFO_NIP_LHS (info) = NULL;
    INFO_NIP_RESULT (info) = FALSE;
    INFO_NIP_ARG (info) = FALSE;

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
 * @fn node *WLSDdoWithLoopSplitDimensions( node *syntax_tree)
 *
 *****************************************************************************/
node *
WLSDdoWithLoopSplitDimensions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting to split with-loops by dimension.");

    TRAVpush (TR_wlsd);
    do {
        DBUG_PRINT ("Running wlsd trav");
        INFO_TRANSFORMED_W2_TO_W3 (info) = FALSE;
        syntax_tree = TRAVdo (syntax_tree, info);
    } while (INFO_TRANSFORMED_W2_TO_W3 (info));
    TRAVpop ();

    DBUG_PRINT ("With-loop splitting complete.");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 * @fn node *MakeIntegerVar( node **vardecs)
 *
 * @brief Produces a new avis for an integer variable and adds a corresponding
 *        vardec to the vardecs chain in the info structure.
 *
 * @param vardecs vardecs chain to add to
 *
 * @return N_avis node to be used for an integer value
 ******************************************************************************/
static node *
MakeIntegerVar (node **vardecs)
{
    node *avis;

    DBUG_ENTER ();

    avis
      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    *vardecs = TBmakeVardec (avis, *vardecs);

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 * @fn node *AssignValue( node *avis, node *rhs, node **assigns)
 *
 * @brief adds an assignment of rhs to the indentifier given by avis at the
 *        head of the assigns chain.
 *
 * @param avis avis to use
 * @param rhs  rhs to assign
 * @param *assigns chain to prepend assigment to
 *
 * @return updated avis
 ******************************************************************************/
static node *
AssignValue (node *avis, node *rhs, node **assigns)
{
    node *assign;

    DBUG_ENTER ();

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), NULL);
    AVIS_SSAASSIGN (avis) = assign;

    *assigns = TCappendAssign (*assigns, assign);

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 * @fn node *MakeIntegerConst( int rhs, node ** assigns, node **vardecs)
 *
 * @brief Produces a new avis for an integer variable and adds a corresponding
 *        vardec to the vardecs chain in the info structure. Assign rhs to
 *        avis and save into assignment into vardec chain
 *
 * @param rhs     constant to use
 * @param assigns assigns chain to save into
 * @param vardecs vardecs chain to add to
 *
 * @return N_avis node to be used for an integer value
 ******************************************************************************/
static node *
MakeIntegerConst (int rhs, node **assigns, node **vardecs)
{
    node *avis;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVar (),
                       TYmakeAKV (TYmakeSimpleType (T_int), COmakeConstantFromInt (rhs)));

    *vardecs = TBmakeVardec (avis, *vardecs);

    avis = AssignValue (avis, TBmakeNum (rhs), assigns);

    DBUG_RETURN (avis);
}

/** <!-- ****************************************************************** -->
 * @fn bool IsNum( node *scalar)
 *
 * @brief Returns true if the argument denotes an integer value, either in
 *        from of a N_num node or of a N_id node with constant integer type.
 *
 * @param scalar piece of AST representing a scalar value
 *
 * @return true or false
 ******************************************************************************/
static bool
IsNum (node *scalar)
{
    bool result;

    DBUG_ENTER ();

    DBUG_ASSERT (((NODE_TYPE (scalar) == N_num) || (NODE_TYPE (scalar) == N_id)),
                 "IsNum called with non-id, non-num node");

    result = ((NODE_TYPE (scalar) == N_num)
              || ((NODE_TYPE (scalar) == N_id) && (TYisAKV (AVIS_TYPE (ID_AVIS (scalar))))
                  && (TUisScalar (AVIS_TYPE (ID_AVIS (scalar))))
                  && (TUhasBasetype (AVIS_TYPE (ID_AVIS (scalar)), T_int))));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn int GetNum( node *scalar)
 *
 * @brief Returns the integer value of the given AST node. Note that only
 *        those nodes for which IsNum returns true may be passed as argument
 *        to this function.
 *
 * @param scalar A node denoting a scalar integer value
 *
 * @return the integer value
 ******************************************************************************/
static int
GetNum (node *scalar)
{
    int result;

    DBUG_ENTER ();

    DBUG_ASSERT (IsNum (scalar), "IsNum called with non int-value node");

    if (NODE_TYPE (scalar) == N_num) {
        result = NUM_VAL (scalar);
    } else {
        result = COconst2Int (TYgetValue (AVIS_TYPE (ID_AVIS (scalar))));
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn info *PushDim(info *arg_info)
 *
 * @brief Increases the level of the dimension encoded in the info
 *        structure.  This includes incrementing INFO_CURRENT_DIM and
 *        pushing an empty N_ids node on the withloop indices
 *        stack. Unless there are framed indices, in which case one
 *        index is moved from the frame to the indices stack rather
 *        than creating a new ids.
 *
 * @param arg_info info structure
 *
 * @return updated info structure
 ******************************************************************************/
static info *
PushDim (info *arg_info)
{
    node *zero_avis;

    DBUG_ENTER ();

    INFO_CURRENT_DIM (arg_info)++;
    if (INFO_FRAME_INDICES (arg_info) != NULL) {
        /* move from frame to stack */
        node *index = INFO_FRAME_INDICES (arg_info);
        INFO_FRAME_INDICES (arg_info) = IDS_NEXT (INFO_FRAME_INDICES (arg_info));

        IDS_NEXT (index) = INFO_INDICES (arg_info);

        INFO_INDICES (arg_info) = index;
    } else {
        zero_avis
          = MakeIntegerConst (0, &INFO_PREASSIGNS (arg_info), &INFO_VARDECS (arg_info));
        INFO_INDICES (arg_info) = TBmakeIds (zero_avis, INFO_INDICES (arg_info));
    }

    DBUG_RETURN (arg_info);
}

/** <!-- ****************************************************************** -->
 * @fn info PopDim( info *arg_info)
 *
 * @brief Undoes the changes done by PushDim and removes one level of nesting
 *        from the info structure.
 *
 * @param arg_info info structure
 *
 * @return updated info structure
 ******************************************************************************/
static info *
PopDim (info *arg_info)
{
    DBUG_ENTER ();

    //INFO_DIM_FRAME is set to -1 at times, so casting to fix sign-compare warning
    DBUG_ASSERT ((ssize_t)TCcountIds (INFO_INDICES (arg_info)) > INFO_DIM_FRAME (arg_info),
                 "Stack eroding into frame");
    node *index = INFO_INDICES (arg_info);
    INFO_INDICES (arg_info) = IDS_NEXT (INFO_INDICES (arg_info));

    IDS_NEXT (index) = INFO_FRAME_INDICES (arg_info);
    INFO_FRAME_INDICES (arg_info) = index;

    INFO_CURRENT_DIM (arg_info)--;
    DBUG_ASSERT (INFO_CURRENT_DIM (arg_info) >= 0, "Negative dim found");
    DBUG_RETURN (arg_info);
}

static info *
FrameDim (info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_DIM_FRAME (arg_info) < 0) {
        INFO_DIM_FRAME (arg_info) = INFO_CURRENT_DIM (arg_info);
    }
    DBUG_RETURN (arg_info);
}

static info *
DeFrameDim (info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_DIM_FRAME (arg_info) >= 0) {
        DBUG_ASSERT (INFO_DIM_FRAME (arg_info) <= INFO_CURRENT_DIM (arg_info),
                     "Stack frame corrupted");
        while (INFO_CURRENT_DIM (arg_info) > INFO_DIM_FRAME (arg_info)) {
            arg_info = PopDim (arg_info);
        }
        INFO_DIM_FRAME (arg_info) = -1;
    }

    DBUG_RETURN (arg_info);
}

/** <!-- ****************************************************************** -->
 * @fn bool NeedsFitting( node *lower, node *upper, node *step)
 *
 * @brief Returns true if the step is not one or unknown and
 *                     if the range is not a multiple of the step or unknown.
 *
 * @param lower node denoting the lower bound
 * @param upper node denoting the upper bound
 * @param step node denoting the step
 *
 * @return true if fitting is needed
 ******************************************************************************/
static bool
NeedsFitting (node *lower, node *upper, node *step)
{
    bool result;

    DBUG_ENTER ();

    result = ((!IsNum (step) || (GetNum (step) != 1))
              && (!(IsNum (lower) && IsNum (upper) && IsNum (step))
                  || (((GetNum (upper) - GetNum (lower)) % GetNum (step)) != 0)));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeMax( node *nodea, node *nodeb, node **assigns,
 *                       info *arg_info)
 *
 * @brief Returns an AST expression that denotes the maximum of the two integer
 *        values
 *
 * @param nodea node repesenting an integer value
 * @param nodeb node repesenting an integer value
 * @param *assigns preassigns chain to append to
 * @param arg_info info structure
 *
 * @return an ast node that denotes the maximum of the two values
 ******************************************************************************/
static node *
ComputeMax (node *nodea, node *nodeb, node **assigns, info *arg_info)
{
    node *max;

    DBUG_ENTER ();

    if (IsNum (nodea) && IsNum (nodeb)) {
        /*
         * static maximum
         */
        if (GetNum (nodea) > GetNum (nodeb)) {
            max = DUPdoDupNode (nodea);
        } else {
            max = DUPdoDupNode (nodeb);
        }
    } else {
        /*
         * we have to compute the maximum at runtime
         */
        node *mavis = MakeIntegerVar (&INFO_VARDECS (arg_info));

        mavis = AssignValue (mavis,
                             TCmakePrf2 (F_max_SxS, DUPdoDupNode (nodea),
                                         DUPdoDupNode (nodeb)),
                             assigns);

        max = TBmakeId (mavis);
    }

    DBUG_RETURN (max);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeMin( node *nodea, node *nodeb, node **assigns,
 *                       info *arg_info)
 *
 * @brief Returns an AST expression that denotes the imum of the two integer
 *        values
 *
 * @param nodea node repesenting an integer value
 * @param nodeb node repesenting an integer value
 * @param *assigns preassigns chain to append to
 * @param arg_info info structure
 *
 * @return an ast node that denotes the minimum of the two values
 ******************************************************************************/
static node *
ComputeMin (node *nodea, node *nodeb, node **assigns, info *arg_info)
{
    node *min;

    DBUG_ENTER ();

    if (IsNum (nodea) && IsNum (nodeb)) {
        /*
         * static maximum
         */
        if (GetNum (nodea) < GetNum (nodeb)) {
            min = DUPdoDupNode (nodea);
        } else {
            min = DUPdoDupNode (nodeb);
        }
    } else {
        /*
         * we have to compute the maximum at runtime
         */
        node *mavis = MakeIntegerVar (&INFO_VARDECS (arg_info));

        mavis = AssignValue (mavis,
                             TCmakePrf2 (F_min_SxS, DUPdoDupNode (nodea),
                                         DUPdoDupNode (nodeb)),
                             assigns);

        min = TBmakeId (mavis);
    }

    DBUG_RETURN (min);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeNewBounds( node *upper, node *lower, node *step,
 *                             node **nupper,
 *                             node **assigns, info *arg_info)
 *
 * @brief Computes a new intermediate bound and the length of the overlap
 *        to produce a fitted with-loop.
 *
 * @param upper upper bound of wl3
 * @param lower lower bound of wl3
 * @param step  step of wl3
 * @param *nupper contains the intermediate upper bound after return

 * @param *assigns contains, after return, assignments that need to be performed
 *                  before this range is evaluated.
 * @param arg_info info structure used to store new vardecs
 *
 * @return the size of the overlapping range
 ******************************************************************************/
static node *
ComputeNewBounds (node *upper, node *lower, node *step, node **nupper, node **assigns,
                  info *arg_info)
{
    node *newsize;
    node *length;

    DBUG_ENTER ();

    if (IsNum (upper) && IsNum (lower)) {
        /*
         * static length
         */
        length = TBmakeNum (GetNum (upper) - GetNum (lower));
    } else {
        /*
         * we have to compute the length of the range at runtime
         */
        node *lavis = MakeIntegerVar (&INFO_VARDECS (arg_info));

        lavis = AssignValue (lavis,
                             TCmakePrf2 (F_sub_SxS, DUPdoDupNode (upper),
                                         DUPdoDupNode (lower)),
                             assigns);

        length = TBmakeId (lavis);
    }

    if (IsNum (step) && IsNum (length)) {
        int overlap;
        /*
         * we can compute the new size and bound statically
         */
        overlap = GetNum (length) % GetNum (step);

        newsize = TBmakeNum (overlap);
        *nupper = TBmakeNum (GetNum (upper) - overlap);

        length = FREEdoFreeNode (length);
    } else {
        node *ovlAvis, *nupAvis;
        /*
         * we have to compute the new size and upper bound
         * at runtime
         */
        ovlAvis = MakeIntegerVar (&INFO_VARDECS (arg_info));
        nupAvis = MakeIntegerVar (&INFO_VARDECS (arg_info));

        ovlAvis
          = AssignValue (ovlAvis, TCmakePrf2 (F_mod_SxS, length, DUPdoDupTree (step)),
                         assigns);

        nupAvis
          = AssignValue (nupAvis,
                         TCmakePrf2 (F_sub_SxS, DUPdoDupTree (upper), TBmakeId (ovlAvis)),
                         assigns);

        newsize = TBmakeId (ovlAvis);
        *nupper = TBmakeId (nupAvis);
    }

    {
        /* Make sure that the size is at lest one */
        node *one = TBmakeNum (1);
        node *maxnewsize;
        maxnewsize = ComputeMax (newsize, one, assigns, arg_info);
        newsize = FREEdoFreeTree (newsize);
        one = FREEdoFreeTree (one);
        newsize = maxnewsize;
    }

    DBUG_RETURN (newsize);
}

#if 0 /* not needed at the moment */
/** <!-- ****************************************************************** -->
 * @fn node *ComputeSize( node *lower, node *upper, node **assigns,
 *                        info *arg_info)
 *
 * @brief Returns a node that denotes the size (or length) of the given range.
 *        If the range is fully static, an N_num node will be returned. For
 *        dynamic ranges, code for computing the length at runtime is added to
 *        assigns and an N_id node is returned.
 *
 * @param lower ast node denoting lower bound
 * @param upper ast node denoting upper bound
 * @param *assigns assignment chain to append the runtime code if necessary
 * @param arg_info info structure
 *
 * @return N_num or N_id node representing the length of the range
 ******************************************************************************/
static
node *ComputeSize( node *lower, node *upper, node **assigns, info *arg_info)
{
  node *size;

  DBUG_ENTER ();

  if (IsNum( upper) && IsNum( lower)) {
    /*
     * static length
     */
    size = TBmakeNum( GetNum( upper) - GetNum( lower));

    DBUG_ASSERT (NUM_VAL( size) >= 0, "negative size found");
  } else {
    /*
     * we have to compute the length of the range at runtime
     */
    node *savis = MakeIntegerVar( &INFO_VARDECS( arg_info));

    savis = AssignValue( savis,
                         TCmakePrf2( F_sub_SxS,
                                     DUPdoDupNode( upper),
                                     DUPdoDupNode( lower)),
                         assigns);

    size = TBmakeId( savis);
  }

  DBUG_RETURN (size);
}
#endif

/*
 * int i = _sel_VxA( int n, int[.] vec);
 */
static node *
MakeSel (int n, node *vec, info *arg_info)
{
    node *avis, *assign;
    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVar (),
                       TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL),
                                      TBmakePrf (F_idx_shape_sel,
                                                 TBmakeExprs (TBmakeNum (n),
                                                              TBmakeExprs (TBmakeId (vec),
                                                                           NULL)))),
                           NULL);

    AVIS_SSAASSIGN (avis) = assign;

    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), assign);

    DBUG_RETURN (avis);
}

#if 0
/** <!-- **************************************************************** -->
 * @fn node *MakeModarrayShape( node *array, info *arg_info)
 *
 * @brief
 *
 * @param array    The array that we want the shape of
 * @param arg_info State of the traversal
 ****************************************************************************/
static
node *MakeModarrayShape( node *array, info *arg_info)
{
  node *shapeAvis;
  DBUG_ENTER ();

  DBUG_ASSERT ( NODE_TYPE( array) == N_id, "ID expected");

  shapeAvis =
    TBmakeAvis( TRAVtmpVar(),
                TYmakeAKS(
                  TYmakeSimpleType( T_int),
                  SHcreateShape( 1,
                                 TYgetDim( AVIS_TYPE( ID_AVIS( array))))));

  INFO_VARDECS( arg_info) = TBmakeVardec( shapeAvis,
                                          INFO_VARDECS( arg_info));

  INFO_PREASSIGNS( arg_info) =
    TBmakeAssign( TBmakeLet( TBmakeIds( shapeAvis, NULL),
                             TBmakePrf( F_shape_A,
                                        TBmakeExprs( DUPdoDupTree( array),
                                                     NULL))),
                  INFO_PREASSIGNS( arg_info));
  AVIS_SSAASSIGN( shapeAvis) = INFO_PREASSIGNS( arg_info);

  DBUG_RETURN (shapeAvis);
}
#endif
/** <!-- ****************************************************************** -->
 * @fn node *ATravCNWgenarray( node *arg_node, info *arg_info)
 *
 * @brief Computes a new genarray withop from the current dimension, current
 *        size and the original with2 genarray withop. Furthermore,
 *        a shape expression is computed and annotated at the withop/added
 *        to the info structure. Used in an anonymous traversal by
 *        ComputeNewWithops.
 *
 * @param arg_node N_genarray node
 * @param arg_info info structure
 *
 * @return new N_genarray node
 ******************************************************************************/
static node *
ATravCNWgenarray (node *arg_node, info *arg_info)
{
    node *new_node;
    node *shape = NULL;
    node *sexpr = NULL;
    node *array = NULL;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&array), 1, PMskip (0));

    if (PMmatchFlat (pat, GENARRAY_SHAPE (arg_node))) {
        /*
         * If the surrounding WL has a chunksize, this determines
         * the shape of the inner genarray. Otherwise, the
         * shape is the shape only taking the current dimension
         * into account.
         */
        if (INFO_CURRENT_SIZE (arg_info) != NULL) {
            shape = TBmakeExprs (DUPdoDupNode (INFO_CURRENT_SIZE (arg_info)), NULL);
        } else {
            shape
              = TBmakeExprs (DUPdoDupNode (TCgetNthExprsExpr (INFO_CURRENT_DIM (arg_info),
                                                              ARRAY_AELEMS (array))),
                             NULL);
        }
        shape = TCmakeIntVector (shape);

        /*
         * the shapeexpression is everything from the current dimension
         * onwards.
         */
        sexpr = TCmakeIntVector (DUPdoDupTree (
          TCgetNthExprs (INFO_CURRENT_DIM (arg_info) + 1, ARRAY_AELEMS (array))));
    }
    pat = PMfree (pat);

    DBUG_ASSERT (shape != NULL, "no shape info for genarray constructed");

    new_node = TBmakeGenarray (shape, DUPdoDupNode (GENARRAY_DEFAULT (arg_node)));
    GENARRAY_DEFSHAPEEXPR (new_node) = sexpr;

    /*
     * copy over reuse information if we are at the top level!
     */
    if (INFO_WITH3_NESTING (arg_info) == 0) {
        GENARRAY_RC (new_node) = DUPdoDupTree (GENARRAY_RC (arg_node));
    }

    GENARRAY_NEXT (new_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *MakeZero( ntype *type, info *arg_info)
 *
 * @brief Create an id that is the scaler zero of type
 *
 * @param type     type to create zero of
 * @param arg_info info structure
 *
 * @return new id node
 ******************************************************************************/
static node *
MakeZero (ntype *type, info *arg_info)
{
    node *avis;
    simpletype simple;

    DBUG_ENTER ();

    simple = TYgetSimpleType (TYgetScalar (type));
    type
      = TYmakeAKV (TYcopyType (TYgetScalar (type)), COmakeZero (simple, SHmakeShape (0)));

    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (type));
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    AVIS_SSAASSIGN (avis) = INFO_PREASSIGNS (arg_info)
      = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), TCcreateZeroScalar (simple)),
                      INFO_PREASSIGNS (arg_info));

    DBUG_RETURN (TBmakeId (avis));
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCNWmodarray( node *arg_node, info *arg_info)
 *
 * @brief Computes a new GENARRAY withop from the current dimension, current
 *        size and the original with2 modarray withop. Furthermore,
 *        a shape expression is computed and annotated at the withop/added
 *        to the info structure. Used in an anonymous traversal by
 *        ComputeNewWithops.
 *
 * @param arg_node N_modarray node
 * @param arg_info info structure
 *
 * @return new N_genarray node
 ******************************************************************************/
static node *
ATravCNWmodarray (node *arg_node, info *arg_info)
{
    node *new_node;
    ntype *atype;
    node *shape = NULL;
    node *sexpr = NULL;
    node *zero = NULL;
    int cnt;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (MODARRAY_ARRAY (arg_node)) == N_id,
                 "Unexpected MODARRAY_ARRAY node");

    atype = AVIS_TYPE (ID_AVIS (MODARRAY_ARRAY (arg_node)));

    if (TUshapeKnown (atype)) {
        /*
         * If the surrounding WL has a chunksize, this determines
         * the shape of the inner genarray. Otherwise, the
         * shape is the shape only taking the current dimension
         * into account.
         */
        if (INFO_CURRENT_SIZE (arg_info) != NULL) {
            shape = TBmakeExprs (DUPdoDupNode (INFO_CURRENT_SIZE (arg_info)), NULL);
        } else {
            shape = TBmakeExprs (TBmakeNum (SHgetExtent (TYgetShape (atype),
                                                         INFO_CURRENT_DIM (arg_info))),
                                 NULL);
        }

        /*
         * the shapeexpression is everything from the current dimension
         * onwards.
         */
        sexpr = SHshape2Exprs (TYgetShape (atype));

        for (cnt = 0; cnt <= INFO_CURRENT_DIM (arg_info); cnt++) {
            DBUG_ASSERT (sexpr != NULL, "Ooops, ran out of shape elements!");
            sexpr = FREEdoFreeNode (sexpr);
        }

        sexpr = TCmakeIntVector (sexpr);
    } else {
        /*
         * If the surrounding WL has a chunksize, this determines
         * the shape of the inner genarray. Otherwise, the
         * shape is the shape only taking the current dimension
         * into account.
         */
        if (INFO_CURRENT_SIZE (arg_info) != NULL) {
            shape = TBmakeExprs (DUPdoDupNode (INFO_CURRENT_SIZE (arg_info)), NULL);
        } else {
            shape = TBmakeExprs (TBmakeId (MakeSel (INFO_CURRENT_DIM (arg_info),
                                                    ID_AVIS (MODARRAY_ARRAY (arg_node)),
                                                    arg_info)),
                                 NULL);
        }
        sexpr = NULL;
        for (cnt = (TYgetDim (AVIS_TYPE (ID_AVIS (MODARRAY_ARRAY (arg_node)))) - 1);
             cnt > INFO_CURRENT_DIM (arg_info); cnt--) {
            sexpr
              = TBmakeExprs (TBmakeId (MakeSel (cnt, ID_AVIS (MODARRAY_ARRAY (arg_node)),
                                                arg_info)),
                             sexpr);
        }
        sexpr = TCmakeIntVector (sexpr);
    }

    DBUG_ASSERT (shape != NULL, "no shape info for modarray constructed");

    shape = TCmakeIntVector (shape);

    zero = MakeZero (AVIS_TYPE (ID_AVIS (MODARRAY_ARRAY (arg_node))), arg_info);

    new_node = TBmakeGenarray (shape, zero);
    GENARRAY_DEFSHAPEEXPR (new_node) = sexpr;

    /*
     * copy over reuse information if we are at the top level!
     */
    if (INFO_WITH3_NESTING (arg_info) == 0) {
        GENARRAY_RC (new_node) = DUPdoDupTree (MODARRAY_RC (arg_node));
    }

    GENARRAY_NEXT (new_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCNWfold( node *arg_node, info *arg_info)
 *
 * @brief Copy fold. No changes needed
 *
 * @return copy of fold
 *****************************************************************************/
static node *
ATravCNWfold (node *arg_node, info *arg_info)
{
    node *res;
    DBUG_ENTER ();

    res = DUPdoDupNode (arg_node);

    FOLD_NEXT (res) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeNewWithops( node *withops, info *arg_info)
 *
 * @brief Computes a chain of withops for the with3 at the current nesting
 *        level as given by the info structure. The withops arguments is used
 *        as a template.
 *
 * @param withops template withops of original with2
 * @param arg_info info structure containing current state of transformation
 *
 * @return new withop chain
 ******************************************************************************/
static node *
ComputeNewWithops (node *withops, info *arg_info)
{
    anontrav_t cnw_trav[6]
      = {{N_genarray, &ATravCNWgenarray}, {N_modarray, &ATravCNWmodarray},
         {N_fold, &ATravCNWfold},         {N_propagate, &TRAVerror},
         {N_break, &TRAVerror},           {(nodetype)0, NULL}};
    node *ops;

    DBUG_ENTER ();

    TRAVpushAnonymous (cnw_trav, &TRAVerror);

    ops = TRAVopt (withops, arg_info);

    TRAVpop ();

    DBUG_RETURN (ops);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCNLgenarray( node *arg_node, info *arg_info)
 *
 * @brief Traversal function used in anonymous traversal by ComputeNewLhs.
 *        Constructs a new lhs N_ids node depending on the current state
 *        of the transformation as given in the info structure and the
 *        original withop.
 *        Traverses into the next withop.
 *
 * @param arg_node original withop
 * @param arg_info traversal state in info structure
 *
 * @return chain of N_ids node
 ******************************************************************************/
static node *
ATravCNLgenOrModArray (node *arg_node, info *arg_info)
{
    node *new_node;
    ntype *old_type, *new_type = NULL;
    node *avis;
    node *mylhs;

    DBUG_ENTER ();

    mylhs = INFO_WITH2_LHS (arg_info);

    /*
     * go to next withop with next lhs. We have to reset the info structure
     * again as it will be needed multiple times.
     */
    INFO_WITH2_LHS (arg_info) = IDS_NEXT (INFO_WITH2_LHS (arg_info));
    new_node = TRAVopt (WITHOP_NEXT (arg_node), arg_info);
    INFO_WITH2_LHS (arg_info) = mylhs;

    old_type = AVIS_TYPE (IDS_AVIS (mylhs));

    DBUG_ASSERT (!TYisAKV (old_type), "lhs with known value?");

#if 0
  /*
   * This can not work! What about the shape of the rest of the array if
   * there is more array?
   */
  if ( INFO_CURRENT_SIZE( arg_info) != NULL){
    /* Outer wl has chunksize */
    constant *length = COaST2Constant( INFO_CURRENT_SIZE( arg_info));
    if ( length != NULL){
      new_type = TYmakeAKS( TYcopyType( TYgetScalar( old_type)),
                            SHcreateShape( 1, COconst2Int( length)));
      length = COfreeConstant( length);
    }
  }
#endif

    if (new_type == NULL) {
        if (TUshapeKnown (old_type)) {
            shape *shape;

            if ((INFO_CURRENT_SIZE (arg_info) == NULL)
                || (IsNum (INFO_CURRENT_SIZE (arg_info)))) {
                shape
                  = SHdropFromShape (INFO_CURRENT_DIM (arg_info), TYgetShape (old_type));

                if (INFO_CURRENT_SIZE (arg_info) != NULL) {
                    /*
                     * set outermost size
                     */
                    shape = SHsetExtent (shape, 0, GetNum (INFO_CURRENT_SIZE (arg_info)));
                }

                new_type = TYmakeAKS (TYcopyType (TYgetScalar (old_type)), shape);
            } else {
                /*
                 * the size is unkown so we have to produce an AKD
                 */
                new_type = TYmakeAKD (TYcopyType (TYgetScalar (old_type)),
                                      TYgetDim (old_type) - INFO_CURRENT_DIM (arg_info),
                                      SHcreateShape (0));
            }
        } else if (TUdimKnown (old_type)) {
            new_type = TYmakeAKD (TYcopyType (TYgetScalar (old_type)),
                                  TYgetDim (old_type) - INFO_CURRENT_DIM (arg_info),
                                  SHcreateShape (0));
        } else {
            new_type = TYcopyType (old_type);
        }
    }

    avis = TBmakeAvis (TRAVtmpVar (), new_type);
    AVIS_SSAASSIGN (avis) = INFO_WITH3_ASSIGN (arg_info);
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    new_node = TBmakeIds (avis, new_node);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCNLfold( node *arg_node, info *arg_info)
 *
 * @brief return a new lhs with the same type as the old lhs.
 *
 * @param arg_node current withop
 * @param arg_info current state of transformation in info structure
 *
 * @return lhs for this fold
 *****************************************************************************/

static node *
ATravCNLfold (node *arg_node, info *arg_info)
{
    node *mylhs, *next, *avis;
    DBUG_ENTER ();
    mylhs = INFO_WITH2_LHS (arg_info);

    avis = TBmakeAvis (TRAVtmpVar (),
                       TYcopyType (AVIS_TYPE (IDS_AVIS (INFO_WITH2_LHS (arg_info)))));
    AVIS_SSAASSIGN (avis) = INFO_WITH3_ASSIGN (arg_info);
    /*
     * go to next withop with next lhs. We have to reset the info structure
     * again as it will be needed multiple times.
     */
    INFO_WITH2_LHS (arg_info) = IDS_NEXT (INFO_WITH2_LHS (arg_info));
    next = TRAVopt (WITHOP_NEXT (arg_node), arg_info);
    INFO_WITH2_LHS (arg_info) = mylhs;

    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    mylhs = TBmakeIds (avis, next);

    DBUG_RETURN (mylhs);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeNewLhs( node *withops, info *arg_info)
 *
 * @brief Computes from a chain of withops and the current traversal state
 *        a chain of left-hand sides.
 *
 * @param withops template withops of original with2
 * @param arg_info current state of transformation in info structure
 *
 * @return N_ids chain to be used as lhs for with3 at current level
 ******************************************************************************/
static node *
ComputeNewLhs (node *withops, info *arg_info)
{
    anontrav_t cnw_trav[6] = {{N_genarray, &ATravCNLgenOrModArray},
                              {N_modarray, &ATravCNLgenOrModArray},
                              {N_fold, &ATravCNLfold},
                              {N_propagate, &TRAVerror},
                              {N_break, &TRAVerror},
                              {(nodetype)0, NULL}};
    node *lhs;

    DBUG_ENTER ();

    TRAVpushAnonymous (cnw_trav, &TRAVerror);

    lhs = TRAVopt (withops, arg_info);

    TRAVpop ();

    DBUG_RETURN (lhs);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeOneLengthVector( node *aelems, node *inner, info *arg_info)
 *
 * @brief Given an N_exprs chain of extents per dimension, this function
 *        generates code to compute the unrolling length per dimension.
 *
 *        The argument inner is consumed!
 *
 * @param aelems   N_exprs chain of extents per dimension
 * @param inner    N_num or N_id node denoting the length of one inner element
 *                 CONSUMED!
 * @param arg_info info structure
 *
 * @return N_ids chain of unrolling lengths
 ******************************************************************************/
static node *
ComputeOneLengthVector (node *aelems, node *inner, info *arg_info)
{
    node *exprs;
    node *lavis;
    node *len;

    DBUG_ENTER ();

    if (EXPRS_NEXT (aelems) != NULL) {
        exprs = ComputeOneLengthVector (EXPRS_NEXT (aelems), inner, arg_info);

        if (IsNum (EXPRS_EXPR (EXPRS_NEXT (aelems))) && IsNum (EXPRS_EXPR (exprs))) {
            len = TBmakeNum (GetNum (EXPRS_EXPR (EXPRS_NEXT (aelems)))
                             * GetNum (EXPRS_EXPR (exprs)));
        } else {
            lavis
              = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)),
                             TCmakePrf2 (F_mul_SxS,
                                         DUPdoDupNode (EXPRS_EXPR (EXPRS_NEXT (aelems))),
                                         DUPdoDupTree (EXPRS_EXPR (exprs))),
                             &INFO_PREASSIGNS (arg_info));
            len = TBmakeId (lavis);
        }
        exprs = TBmakeExprs (len, exprs);
    } else {
        exprs = TBmakeExprs (inner, NULL);
    }

    DBUG_RETURN (exprs);
}

static node *
MakeSelExpr (int dim, node *array, info *arg_info, node *next)
{
    DBUG_ENTER ();

    next = TBmakeExprs (TBmakeId (MakeSel (dim, array, arg_info)), next);

    DBUG_RETURN (next);
}

static node *
MakeModarrayExprs (int dropDims, node *array, info *arg_info)
{
    node *exprs = NULL;
    int i;
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (array) == N_avis, "Expected avis");

    for (i = (dropDims - 1); i >= 0; i--) {
        exprs = MakeSelExpr (i, array, arg_info, exprs);
    }

    DBUG_RETURN (exprs);
}

static node *
ModarrayInnerAccu (int skip, node *array, info *arg_info)
{
    int i, length;
    node *accu = NULL;
    DBUG_ENTER ();

    length = TYgetDim (AVIS_TYPE (array));

    accu = MakeIntegerConst (1, &INFO_PREASSIGNS (arg_info), &INFO_VARDECS (arg_info));

    for (i = (skip); i < length; i++) {
        node *newAccu, *assign;
        newAccu = TBmakeAvis (TRAVtmpVar (),
                              TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));
        INFO_VARDECS (arg_info) = TBmakeVardec (newAccu, INFO_VARDECS (arg_info));
        /*
         * newAccu = _mul_SxS_( accu, _sel_VxA_( [i], shape));
         */
        assign = TBmakeAssign (
          TBmakeLet (TBmakeIds (newAccu, NULL),
                     TBmakePrf (F_mul_SxS,
                                TBmakeExprs (TBmakeId (accu),
                                             TBmakeExprs (TBmakeId (
                                                            MakeSel (i, array, arg_info)),
                                                          NULL)))),
          NULL);
        INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), assign);
        accu = newAccu;
    }

    DBUG_RETURN (accu);
}

static node *
ModarrayInner (int dims, node *array, info *arg_info)
{
    node *prod, *exprs;
    DBUG_ENTER ();

    prod = ModarrayInnerAccu (dims, array, arg_info);
    exprs = TBmakeId (prod);

    DBUG_RETURN (exprs);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCDLgenarray( node *arg_node, info *arg_info)
 *
 * @brief Used by anonymous traversal in ComputeLengths to compute the lengths
 *        for a N_genarray operator.
 *
 * @param arg_node N_genarray node
 * @param arg_info info structure
 *
 * @return N_set node with lengths for this operator.
 ******************************************************************************/
static node *
ATravCDLgenarray (node *arg_node, info *arg_info)
{
    node *set, *inner, *sarray, *exprs, *lhs;
    shape *shape;
    size_t outerdims;
    bool match;
    pattern *pat;

    DBUG_ENTER ();

    lhs = INFO_WITH2_LHS (arg_info);
    INFO_WITH2_LHS (arg_info) = IDS_NEXT (INFO_WITH2_LHS (arg_info));
    set = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    INFO_WITH2_LHS (arg_info) = lhs;

    pat = PMarray (1, PMAgetNode (&sarray), 1, PMskip (0));

    match = PMmatchFlat (pat, GENARRAY_SHAPE (arg_node));

    pat = PMfree (pat);
    DBUG_ASSERT (match, "shape not defined as vector");

    /*
     * We need to figure out the size of the inner elements, either by
     *
     * a) computing the size from the result type and the length of
     *    the shape vector or
     * b) by using the default element to infer the size.
     */
    if (TUshapeKnown (AVIS_TYPE (IDS_AVIS (lhs)))) {
        outerdims = TCcountExprs (ARRAY_AELEMS (sarray));
        shape = SHdropFromShape (outerdims, TYgetShape (AVIS_TYPE (IDS_AVIS (lhs))));
        inner = TBmakeNum (SHgetUnrLen (shape));
        shape = SHfreeShape (shape);
    } else if (GENARRAY_DEFAULT (arg_node) != NULL) {
        DBUG_ASSERT (NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id,
                     "default value of genarray is not an id!");

        if (TUisScalar (AVIS_TYPE (ID_AVIS (GENARRAY_DEFAULT (arg_node))))) {
            /*
             * If the default element is scalar the size of one element is 1.
             */
            inner = TBmakeNum (1);
        } else {
            /*
             * Else, we use the size_A_ function to compute the size at runtime.
             */
            inner = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)),
                                 TCmakePrf1 (F_size_A,
                                             DUPdoDupNode (GENARRAY_DEFAULT (arg_node))),
                                 &INFO_PREASSIGNS (arg_info));
            inner = TBmakeId (inner);
        }
    } else {
        /*
         * We are out of options here. All we can do it tell the world that
         * we need a default element and stop compilation.
         */
        inner = NULL;

        CTIerror (EMPTY_LOC, "Default element required in genarray with-loop.");
    }

    exprs = ComputeOneLengthVector (ARRAY_AELEMS (sarray), inner, arg_info);

    set = TBmakeSet (exprs, set);

    DBUG_RETURN (set);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCDLmodarray( node *arg_node, info *arg_info)
 *
 * @brief Used by anonymous traversal in ComputeLengths to compute the lengths
 *        for a N_genarray operator.
 *
 * @param arg_node N_modarray node
 * @param arg_info info structure
 *
 * @return N_set node with lengths for this operator.
 ******************************************************************************/
static node *
ATravCDLmodarray (node *arg_node, info *arg_info)
{
    node *set, *inner = NULL, *exprs, *lhs, *sexprs = NULL;
    shape *shape;
    int outerdims;

    DBUG_ENTER ();

    lhs = INFO_WITH2_LHS (arg_info);
    INFO_WITH2_LHS (arg_info) = IDS_NEXT (INFO_WITH2_LHS (arg_info));
    set = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    INFO_WITH2_LHS (arg_info) = lhs;

    // FIXME grzegorz: outerdim should be changed to size_t but not yet as this is shape related
    outerdims = TCcountIds (INFO_WITH2_ISCLS (arg_info));
    /*
     * We need to figure out the size of the inner elements. For
     * modarrays we do this by looking at the type of the result and
     * the length of the index vector. The latter is checked by
     * looking at the index scalars.
     */
    if (TUshapeKnown (AVIS_TYPE (IDS_AVIS (lhs)))) {
        shape = SHdropFromShape (outerdims, TYgetShape (AVIS_TYPE (IDS_AVIS (lhs))));
        inner = TBmakeNum (SHgetUnrLen (shape));
        shape = SHfreeShape (shape);

        shape = SHtakeFromShape (outerdims, TYgetShape (AVIS_TYPE (IDS_AVIS (lhs))));
        sexprs = SHshape2Exprs (shape);
        shape = SHfreeShape (shape);
    } else if (TUdimKnown (AVIS_TYPE (IDS_AVIS (lhs)))) { /* AKS */
        sexprs
          = MakeModarrayExprs (outerdims, ID_AVIS (MODARRAY_ARRAY (arg_node)), arg_info);
        /* prod( drop( count(ids), shape( mod))) */
        inner = ModarrayInner (outerdims, ID_AVIS (MODARRAY_ARRAY (arg_node)), arg_info);
    } else {
        DBUG_UNREACHABLE ("non-AKD modarray not implemented!");
    }

    exprs = ComputeOneLengthVector (sexprs, inner, arg_info);

    set = TBmakeSet (exprs, set);

    /* inner is consumed but exprs is not! */
    sexprs = FREEdoFreeTree (sexprs);

    DBUG_RETURN (set);
}

/** <!-- ****************************************************************** -->
 * @fn node *ATravCDLfold( node *arg_node, info *arg_info)
 *
 * @brief Lengths are not needed for folds as there is no offsets.
 *
 * @param arg_node N_fold node
 * @param arg_info info structure
 *****************************************************************************/
static node *
ATravCDLfold (node *arg_node, info *arg_info)
{
    node *set, *lhs;
    DBUG_ENTER ();

    lhs = INFO_WITH2_LHS (arg_info);

    INFO_WITH2_LHS (arg_info) = IDS_NEXT (INFO_WITH2_LHS (arg_info));
    set = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    INFO_WITH2_LHS (arg_info) = lhs;

    DBUG_RETURN (set);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeLengths( node *withops, info *arg_info)
 *
 * @brief Computes the unrolling lengths of one element on each dimension of
 *        the result of a modarray/genarray withop. This is used to update the
 *        offsets. If the information is not statically known, corresponding
 *        code to compute the lengths at runtime is generated. The result
 *        is a N_set chain with one N_set node per operator. Each N_set then
 *        holds an N_ids chain of lengths per dimension.
 *
 *        Note that when advancing along the chain of withops, the chain
 *        of left-hand-sides in INFO_WITH2_LHS needs to be advanced, as
 *        well.
 *
 * @param withops  chain of withops to compute lengths for
 * @param arg_info info structure used to insert code
 *
 * @return N_set chain of N_ids chains of lenghts per dimension per operator.
 ******************************************************************************/
static node *
ComputeLengths (node *withops, info *arg_info)
{
    node *result;

    anontrav_t len_trav[4] = {{N_genarray, &ATravCDLgenarray},
                              {N_modarray, &ATravCDLmodarray},
                              {N_fold, &ATravCDLfold},
                              {(nodetype)0, NULL}};

    DBUG_ENTER ();

    TRAVpushAnonymous (len_trav, &TRAVerror);
    result = TRAVopt (withops, arg_info);
    TRAVpop ();

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *InitOffsets( node *lengths, info *arg_info)
 *
 * @brief Generates a N_ids chain for offsets for the current with-loop.
 *        The lenghts argument is used as a template for the number of
 *        offsets required as the offsets are later on updated using this
 *        lengths structure, i.e., they need to be of equal length.
 *        At runtime, the returned ids will evaluate to 0.
 *
 * @param lengths template N_set chain for length of offsets
 * @param arg_info info structure
 *
 * @return N_ids chain of offsets
 ******************************************************************************/
static node *
InitOffsets (node *lengths, info *arg_info)
{
    node *zeroavis;
    node *result = NULL;

    DBUG_ENTER ();

    if (lengths != NULL) {
        zeroavis
          = MakeIntegerConst (0, &INFO_PREASSIGNS (arg_info), &INFO_VARDECS (arg_info));

        do {
            result = TBmakeIds (zeroavis, result);
            lengths = SET_NEXT (lengths);
        } while (lengths != NULL);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *UpdateOffsets( node *index,
 *                          node *offsets,
 *                          int dim,
 *                          node *chunksize,
 *                          node *lengths,
 *                          node **assigns,
 *                          node **localoffsets,
 *                          info *arg_info)
 *
 * @brief Computes new offsets for the current nesting level using the current
 *        loop index and the offsets from the previous (outer) level. In
 *        general this creates code to compute
 *
 *        <code>
 *        local_offset = (length(dim) * index * chunksize);
 *        new_offset = old_offset + local_offset
 *        </code>
 *
 * @param index          N_avis node of current loop index
 * @param offsets        N_ids chain of offsets of outer level
 * @param dim            dimension currently being transformed
 * @param lengths        set of lengths per dimension
 * @param *assigns       assignment chain to append assignments to
 * @param *localoffsets  offset into local result of this level
 * @param arg_info       current state of transformation in info structure
 *
 * @return N_ids chain of new offsets
 ******************************************************************************/
static node *
UpdateOffsets (node *index, node *offsets, int dim, node *chunksize, node *lengths,
               node **assigns, node **localoffsets, info *arg_info)
{
    node *oavis, *tavis;
    node *new_offsets;
    node *len;

    DBUG_ENTER ();

    if (lengths != NULL) {
        new_offsets = UpdateOffsets (index, IDS_NEXT (offsets), dim, chunksize,
                                     SET_NEXT (lengths), assigns, localoffsets, arg_info);

        len = TCgetNthExprsExpr (dim, SET_MEMBER (lengths));

        DBUG_ASSERT (len != NULL, "No length found");

        if (IsNum (len) && (GetNum (len) == 1)) {
            /*
             * nothing to compute here
             */
            tavis = index;
        } else {
            tavis
              = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)),
                             TCmakePrf2 (F_mul_SxS, TBmakeId (index), DUPdoDupNode (len)),
                             assigns);
        }

        oavis = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)),
                             TCmakePrf2 (F_add_SxS, TBmakeId (tavis),
                                         TBmakeId (IDS_AVIS (offsets))),
                             assigns);

        new_offsets = TBmakeIds (oavis, new_offsets);
        *localoffsets = TBmakeIds (tavis, *localoffsets);
    } else {
        new_offsets = NULL;
        *localoffsets = NULL;
    }

    DBUG_RETURN (new_offsets);
}

/** <!-- ****************************************************************** -->
 * @fn lut_t *InsertIndicesIntoLut( lut_t *lut, node **w2ind, node *w3ind)
 *
 * @brief Inserts mappings from with2 to with3 indices into the lut.
 *
 * @param lut     lookup table to amend
 * @param *w2ind  pointer to chain of with2 indices. This argument is
 *                overwritten!
 * @param w3ind   chain of with3 indices (in reversed order).
 *
 * @return
 ******************************************************************************/
static lut_t *
InsertIndicesIntoLut (lut_t *lut, node **w2ind, node *w3ind)
{
    DBUG_ENTER ();

    if (w3ind != NULL) {
        lut = InsertIndicesIntoLut (lut, w2ind, IDS_NEXT (w3ind));

        lut = LUTinsertIntoLutP (lut, IDS_AVIS (*w2ind), IDS_AVIS (w3ind));

        *w2ind = IDS_NEXT (*w2ind);
    }

    DBUG_RETURN (lut);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateIndexVectorExprs( node *indices)
 *
 * @brief Given the with3 indices (in reversed order), this function computes
 *        an N_exprs chain of N_id nodes in the correct order.
 *
 * @param indices chain of with3 indices (in reversed order)
 *
 * @return N_exprs chain of N_id nodes representing the indices as a vector
 ******************************************************************************/
static node *
CreateIndexVectorExprs (node *indices)
{
    node *result = NULL;

    DBUG_ENTER ();

    if (indices != NULL) {
        if (IDS_NEXT (indices) != NULL) {
            result = CreateIndexVectorExprs (IDS_NEXT (indices));
        }
        result
          = TCappendExprs (result, TBmakeExprs (TBmakeId (IDS_AVIS (indices)), NULL));
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn lut_t *PrepareCopyLut( lut_t *lut, node *offsets, node **assigns,
 *                            info *arg_info)
 *
 * @brief Builds a lut for DupTree masking all with2 index variables by the
 *        corresponding with3 counterparts. This includes the index scalars,
 *        the index vector and the loop offsets.
 *
 *        The index vector is constructed from the index scalars and the
 *        corresponding assignments are appended to assigns. These need to
 *        be in scope before the actual loop body!
 *
 *        This function does not clear the lut but merely amends it.
 *
 * @param lut      lut to be inserted into.
 * @param offsets  N_ids chain of offsets to be used
 * @param *assigns assignment chain to append assignments to
 * @param arg_info info structure containing current state of transformation
 *
 * @return updated lookup table
 ******************************************************************************/
static lut_t *
PrepareCopyLut (lut_t *lut, node *offsets, node **assigns, info *arg_info)
{
    node *w2off, *w3off;
    node *w2ind;
    node *ivavis, *ivarray, *ivelems, *ivassign;

    DBUG_ENTER ();

    /* insert offsets */
    w2off = INFO_WITH2_OFFSETS (arg_info);
    w3off = offsets;
    while (w3off != NULL) {
        DBUG_ASSERT (w2off != NULL, "less with2 offsets than with3 offsets");

        lut = LUTinsertIntoLutP (lut, IDS_AVIS (w2off), IDS_AVIS (w3off));

        w2off = IDS_NEXT (w2off);
        w3off = IDS_NEXT (w3off);
    }

    /* insert indices */
    w2ind = INFO_WITH2_ISCLS (arg_info);
    lut = InsertIndicesIntoLut (lut, &w2ind, INFO_INDICES (arg_info));

    /* insert index vector */
    ivelems = CreateIndexVectorExprs (INFO_INDICES (arg_info));
    ivarray = TCmakeIntVector (ivelems);
    ivavis
      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                              SHcreateShape (1, TCcountExprs (ivelems))));
    INFO_VARDECS (arg_info) = TBmakeVardec (ivavis, INFO_VARDECS (arg_info));

    ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivavis, NULL), ivarray), NULL);
    AVIS_SSAASSIGN (ivavis) = ivassign;
    *assigns = TCappendAssign (*assigns, ivassign);

    lut = LUTinsertIntoLutP (lut, IDS_AVIS (INFO_WITH2_IVECT (arg_info)), ivavis);

    DBUG_RETURN (lut);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateFoldAccumulatorsAvis( node *assign, node *lhs, node *ops,
 *                                       info *arg_info)
 *
 * @brief Create a chain of ids for the lhs of accu
 *
 * @param assign     assign for the created avis ssaassign
 * @param lhs        lhs of with2
 * @param ops        with3's withops
 * @param arg_info   state of current transformation
 *
 *****************************************************************************/
static node *
CreateFoldAccumulatorsAvis (node *assign, node *lhs, node *ops, info *arg_info)
{
    node *newLhs = NULL;
    node *avis = NULL;
    DBUG_ENTER ();

    DBUG_ASSERT (lhs != NULL, "No left hand side (arg == NULL)");
    DBUG_ASSERT (ops != NULL, "No withops (arg == NULL)");

    if (NODE_TYPE (ops) == N_fold) {
        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (IDS_AVIS (lhs))));

        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

        AVIS_SSAASSIGN (avis) = assign;

        FOLD_INITIAL (ops) = TBmakeId (avis);
        newLhs = TBmakeIds (avis, newLhs);
    }

    if (IDS_NEXT (lhs) != NULL) {
        node *newNewLhs = NULL;
        newNewLhs = CreateFoldAccumulatorsAvis (assign, IDS_NEXT (lhs), WITHOP_NEXT (ops),
                                                arg_info);
        if (newLhs != NULL) {
            newLhs = TCappendIds (newLhs, newNewLhs);
        } else {
            newLhs = newNewLhs;
        }
    }

    DBUG_RETURN (newLhs);
}

/** <!-- ****************************************************************** -->
 * @fn node *CreateFoldAccumulators( node *index, node *lhs, node *ops,
 *                                  info *arg_info)
 *
 * @brief Create
 *          var = _accu_(iv);
 *
 * @param index      index in current range
 * @param lhs        lhs of with2
 * @param ops        with3's withops
 * @param arg_info   state of current transformation
 *
 *****************************************************************************/
static node *
CreateFoldAccumulators (node *index, node *lhs, node *ops, info *arg_info)
{
    node *accuLhs;
    node *assign = NULL;
    DBUG_ENTER ();

    /* Create assign to be used in aviss ssaassign*/
    assign = TBmakeAssign (NULL, NULL);

    accuLhs = CreateFoldAccumulatorsAvis (assign, lhs, ops, arg_info);

    if (accuLhs != NULL) {
        ASSIGN_STMT (assign)
          = TBmakeLet (accuLhs,
                       TBmakePrf (F_accu,
                                  TBmakeExprs (TBmakeId (IDS_AVIS (index)), NULL)));
    } else {
        /* Did not need assign, cleanup */
        assign = FREEdoFreeTree (assign);
    }

    DBUG_RETURN (assign);
}

static bool
AnyFold (node *ops)
{
    bool ret = FALSE;
    DBUG_ENTER ();

    if (WITHOP_NEXT (ops) != NULL) {
        ret = AnyFold (WITHOP_NEXT (ops));
    }

    ret = (NODE_TYPE (ops) == N_fold) || ret;
    ;

    DBUG_RETURN (ret);
}

/** <!-- ****************************************************************** -->
 * @fn node *MakeRangeBody( node *outerindex, node *contents, node *size,
 *                          node **results, node **offsets, info *arg_info)
 *
 * @brief Constructs the body of a range by building further nested
 *        with3 loops.
 *
 * @param outerindex N_avis node of index of parent range of this body.
 * @param contents   the contents son of the stride/grid this range
 *                   originates from
 * @param size       chunksize of this level. NULL denotes producing scalars
 *                   wrt. the outermost dimension, i.e., a grid.
 * @param newdim     this body is the last one in this dimension
 * @param *results   N_exprs chain of the result expressions of this body
 *                   (also known as cexprs in with2 context)
 * @param *offsets   N_ids chain of local offsets for genarray/modarray
 *                   operations.
 * @param arg_info   state of current transformation
 *
 * @return N_block body subtree for the given range
 ******************************************************************************/
static node *
MakeRangeBody (node *outerindex, node *contents, node *size, int newdim, node **results,
               node **offsets, info *arg_info)
{
    node *body, *ranges, *ops, *lhs, *with3;
    node *assigns = NULL, *accu = NULL;
    node *old_size, *old_preassigns;
    node *old_offsets, *old_with3_assign;
    node *iv_avis, *old_iv_avis;
    node *iv_assigns = NULL;

    DBUG_ENTER ();
    INFO_WITH3_NESTING (arg_info)++;
    /*
     * compute current offset
     */
    old_offsets = INFO_OFFSETS (arg_info);
    INFO_OFFSETS (arg_info)
      = UpdateOffsets (outerindex, INFO_OFFSETS (arg_info), INFO_CURRENT_DIM (arg_info),
                       size, INFO_WITH2_LENGTHS (arg_info), &iv_assigns, offsets,
                       arg_info);
    /*
     * compute current index vector
     */
    DBUG_ASSERT (INFO_INDICES (arg_info) != NULL, "no wl indices found");

    iv_avis = MakeIntegerVar (&INFO_VARDECS (arg_info));
    iv_avis
      = AssignValue (iv_avis,
                     TCmakePrf2 (F_add_SxS, TBmakeId (IDS_AVIS (INFO_INDICES (arg_info))),
                                 TBmakeId (outerindex)),
                     &iv_assigns);

    /*
     * compute body and withops: setup new state and traverse one level below
     */
    old_iv_avis = IDS_AVIS (INFO_INDICES (arg_info));
    IDS_AVIS (INFO_INDICES (arg_info)) = iv_avis;
    old_size = INFO_CURRENT_SIZE (arg_info);
    INFO_CURRENT_SIZE (arg_info) = size;
    old_preassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    /*
     * create the assign node empty to set SSA_ASSIGN properly
     */
    old_with3_assign = INFO_WITH3_ASSIGN (arg_info);
    INFO_WITH3_ASSIGN (arg_info) = TBmakeAssign (NULL, NULL);

    if (newdim == TRUE || newdim == INC) {
        arg_info = PushDim (arg_info);
    }
    ranges = TRAVdo (contents, arg_info);
    ops = ComputeNewWithops (INFO_WITH2_WITHOPS (arg_info), arg_info);
    lhs = ComputeNewLhs (INFO_WITH2_WITHOPS (arg_info), arg_info);
    if (newdim == TRUE) { /* DEC done by DeFrameDim*/
        arg_info = PopDim (arg_info);
    }

    if (AnyFold (ops)) {
        accu = CreateFoldAccumulators (INFO_INDICES (arg_info), INFO_WITH2_LHS (arg_info),
                                       ops, arg_info);
    }
    /*
     * produce with3
     */
    with3 = TBmakeWith3 (ranges, ops);
    WITH3_DENSE (with3) = INFO_DENSE (arg_info);

    ASSIGN_STMT (INFO_WITH3_ASSIGN (arg_info)) = TBmakeLet (lhs, with3);
    ASSIGN_NEXT (INFO_WITH3_ASSIGN (arg_info)) = assigns;
    assigns = INFO_WITH3_ASSIGN (arg_info);
    INFO_WITH3_ASSIGN (arg_info) = NULL;

    /*
     * glue it all together
     */
    assigns = TCappendAssign (accu, assigns);
    assigns = TCappendAssign (INFO_PREASSIGNS (arg_info), assigns);
    assigns = TCappendAssign (iv_assigns, assigns);

    body = TBmakeBlock (assigns, NULL);

    *results = TCcreateExprsFromIds (lhs);

    /*
     * pop state
     */
    IDS_AVIS (INFO_INDICES (arg_info)) = old_iv_avis;
    INFO_PREASSIGNS (arg_info) = old_preassigns;
    INFO_CURRENT_SIZE (arg_info) = old_size;
    if (INFO_OFFSETS (arg_info) != NULL) {
        INFO_OFFSETS (arg_info) = FREEdoFreeTree (INFO_OFFSETS (arg_info));
    }
    INFO_OFFSETS (arg_info) = old_offsets;
    INFO_WITH3_ASSIGN (arg_info) = old_with3_assign;

    INFO_WITH3_NESTING (arg_info)--;
    DBUG_RETURN (body);
}

/** <!-- ****************************************************************** -->
 * @fn node *ProcessStride( int level, int dim, node *lower, node *upper,
 *                          node *step, node *contents, node *next,
 *                          info *arg_info)
 *
 * @brief Computes a range node corresponding to the given stride node.
 *
 *        This function consumes its lower, upper and step arguments!
 *
 * @param level    level attribute of N_wlstride
 * @param dim      dim attribute of N_wlstride
 * @param lower    lower bound as either N_num or N_id.
 * @param upper    upper bound as either N_num or N_id.
 * @param step     step as either N_num or N_id. CONSUMED!
 * @param contents contents son of N_wlstride
 * @param next     next son of N_wlstride
 * @param arg_info current state of traversal
 *
 * @return N_range node corresponding to the given stride.
 ******************************************************************************/

static node *
ProcessStride (int level, int dim, node *lower, node *upper, node *step, node *contents,
               node *next, info *arg_info)
{
    node *index, *body, *results, *offsets;
    node *block_chunk = NULL;
    DBUG_ENTER ();

    if (INFO_BLOCK_CHUNK (arg_info) != NULL) {
        block_chunk = SET_MEMBER (INFO_BLOCK_CHUNK (arg_info));
        INFO_BLOCK_CHUNK (arg_info) = FREEdoFreeNode (INFO_BLOCK_CHUNK (arg_info));
    }

    if (block_chunk != NULL) {
        /* Have a chunk size so do not go past the end */
        upper = ComputeMin (upper, block_chunk, &INFO_PREASSIGNS (arg_info), arg_info);
    }

    /*
     * first process all the remaining strides on this level
     */
    next = TRAVopt (next, arg_info);

    /*
     * We have to potentially compute two ranges here. If (lower - upper) % step
     * is non-zero, we have to produce a first, fitted stride and one stride for
     * the overlapping part. We do so by setting CURRENT_SIZE accordingly to
     * either step or the result of the modulo operation. As we need to support
     * variable strides, this modulo operation might need to be computed at
     * runtime and we have to emit the corresponding code.
     */
    if (NeedsFitting (lower, upper, step)) {
        node *nupper, *over, *body, *index, *results, *offsets;

        index = MakeIntegerVar (&INFO_VARDECS (arg_info));
        over = ComputeNewBounds (upper, lower, step, &nupper, &INFO_PREASSIGNS (arg_info),
                                 arg_info);
        body = MakeRangeBody (index, DUPdoDupTree (contents), over, FALSE, &results,
                              &offsets, arg_info);

        next = TBmakeRange (TBmakeIds (index, NULL), nupper, DUPdoDupTree (upper), over,
                            body, results, offsets, next);

        /*
         * replace old bounds
         */
        upper = nupper;
    }

    index = MakeIntegerVar (&INFO_VARDECS (arg_info));
    body = MakeRangeBody (index, contents, step, FALSE, &results, &offsets, arg_info);

    next
      = TBmakeRange (TBmakeIds (index, NULL), DUPdoDupNode (lower), DUPdoDupNode (upper),
                     DUPdoDupNode (step), body, results, offsets, next);

    if (block_chunk != NULL) {
        INFO_BLOCK_CHUNK (arg_info)
          = TBmakeSet (block_chunk, INFO_BLOCK_CHUNK (arg_info));
    }

    DBUG_RETURN (next);
}

/** <!-- ****************************************************************** -->
 * @fn node *ProcessBlock( int level, int dim, node *lower, node *upper,
 *                         node *step, node *contents, node *next,
 *                         info *arg_info)
 *
 * @brief Computes a range node corresponding to the given stride node.
 *
 *        This function consumes its lower, upper and step arguments!
 *
 * @param level    level attribute of N_wlstride
 * @param dim      dim attribute of N_wlstride
 * @param lower    lower bound as either N_num or N_id.
 * @param upper    upper bound as either N_num or N_id.
 * @param step     step as either N_num or N_id. CONSUMED!
 * @param contents contents son of N_wlstride
 * @param next     next son of N_wlstride
 * @param arg_info current state of traversal
 *
 * @return N_range node corresponding to the given stride.
 ******************************************************************************/

static node *
ProcessBlock (int level, int dim, node *lower, node *upper, node *step, node *contents,
              node *next, info *arg_info)
{
    node *index, *body, *results, *offsets;
    int frame;

    DBUG_ENTER ();

    /*
     * first process all the remaining blocks on this level
     */
    next = TRAVopt (next, arg_info);

    /*
     * We have to potentially compute two ranges here. If (lower - upper) % step
     * is non-zero, we have to produce a first, fitted stride and one stride for
     * the overlapping part. We do so by setting CURRENT_SIZE accordingly to
     * either step or the result of the modulo operation. As we need to support
     * variable strides, this modulo operation might need to be computed at
     * runtime and we have to emit the corresponding code.
     */
    if (NeedsFitting (lower, upper, step)) {
        node *nupper, *over, *body, *index, *results, *offsets;
        frame = INFO_DIM_FRAME (arg_info);
        index = MakeIntegerVar (&INFO_VARDECS (arg_info));
        over = ComputeNewBounds (upper, lower, step, &nupper, &INFO_PREASSIGNS (arg_info),
                                 arg_info);
        INFO_BLOCK_CHUNK (arg_info)
          = TCappendSet (INFO_BLOCK_CHUNK (arg_info), TBmakeSet (over, NULL));
        body = MakeRangeBody (index, DUPdoDupTree (contents), over, INC, &results,
                              &offsets, arg_info);

        next = TBmakeRange (TBmakeIds (index, NULL), nupper, DUPdoDupTree (upper), over,
                            body, results, offsets, next);

        RANGE_ISBLOCKED (next) = TRUE;
        RANGE_ISFITTING (next) = TRUE;

        /*
         * replace old bounds
         */
        upper = nupper;
        INFO_BLOCK_CHUNK (arg_info) = TCdropSet (-1, INFO_BLOCK_CHUNK (arg_info));
        INFO_DIM_FRAME (arg_info) = frame;
    }

    frame = INFO_DIM_FRAME (arg_info);
    index = MakeIntegerVar (&INFO_VARDECS (arg_info));

    INFO_BLOCK_CHUNK (arg_info)
      = TCappendSet (INFO_BLOCK_CHUNK (arg_info), TBmakeSet (step, NULL));
    body = MakeRangeBody (index, contents, step, INC, &results, &offsets, arg_info);

    next
      = TBmakeRange (TBmakeIds (index, NULL), DUPdoDupNode (lower), DUPdoDupNode (upper),
                     DUPdoDupNode (step), body, results, offsets, next);

    RANGE_ISBLOCKED (next) = TRUE;

    INFO_DIM_FRAME (arg_info) = frame;

    INFO_BLOCK_CHUNK (arg_info) = TCdropSet (-1, INFO_BLOCK_CHUNK (arg_info));

    DBUG_RETURN (next);
}

/** <!-- ****************************************************************** -->
 * @fn node *Accu2DimIndexPrf( node *arg_node, info *arg_info)
 *
 ******************************************************************************/
static node *
Accu2DimIndexPrf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_accu) {
        PRF_ARGS (arg_node) = FREEdoFreeTree (PRF_ARGS (arg_node));
        PRF_ARGS (arg_node)
          = TBmakeExprs (TBmakeId (IDS_AVIS (INFO_INDICES (arg_info))), NULL);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *Accu2DimIndex( node *assigns)
 *
 * @brief Find accus not in with loops and convert there argument to
 *        current dims index
 *
 * @param  assign assigns to look for accus in
 * @return corrected assigns
 ******************************************************************************/
static node *
Accu2DimIndex (node *assigns, info *arg_info)
{
    anontrav_t trav[5] = {{N_prf, &Accu2DimIndexPrf},
                          {N_with, &TRAVnone},
                          {N_with2, &TRAVnone},
                          {N_with3, &TRAVnone},
                          {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (trav, &TRAVsons);

    assigns = TRAVopt (assigns, arg_info);

    TRAVpop ();

    DBUG_RETURN (assigns);
}

/** <!-- ****************************************************************** -->
 * @fn node *ProcessGrid( int level, int dim, node *lower, node *upper,
 *                        node *nextdim, node **code, node *next,
 *                        info *arg_info)
 *
 * @brief Computes a range node corresponding to the given grid node. Note
 *        that the code link is consumed, i.e., it is set to NULL and the
 *        CODE_USED counter is decremented.
 *
 * @param level    level attribute of N_wlgrid node
 * @param dim      dim attribute of N_wlgrid node
 * @param lower    lower bound as either N_id or N_num node.
 * @param upper    upper bound as either N_id or N_num node.
 * @param nextdim  nextdim son of N_wlgrid node.
 * @param code     code son of N_wlgrid node.
 * @param next     next son of N_wlgrid node.
 * @param arg_info current state of traversal
 *
 * @return N_range node corresponding to the given grid.
 ******************************************************************************/
static node *
ProcessGrid (int level, int dim, node *lower, node *upper, node *nextdim, node **code,
             node *next, info *arg_info)
{
    node *index, *max, *body, *res = NULL, *result, *rangeoffsets = NULL,
                              *resultindices = NULL;

    DBUG_ENTER ();

    /*
     * first process all remaining grids on this level
     */
    next = TRAVopt (next, arg_info);

    /*
     * now build the range for this level
     */
    index = MakeIntegerVar (&INFO_VARDECS (arg_info));

    max = ComputeMax (lower, upper, &INFO_PREASSIGNS (arg_info), arg_info);

    if (INFO_CURRENT_SIZE (arg_info) != NULL) {
        node *newMax;
        /* Have a chunk size so do not go past the end */
        newMax = ComputeMin (max, INFO_CURRENT_SIZE (arg_info),
                             &INFO_PREASSIGNS (arg_info), arg_info);
        max = FREEdoFreeTree (max);
        max = newMax;
    }

    if (*code != NULL) {
        node *preassigns = NULL;
        node *iv_avis, *old_iv_avis, *final_offsets;
        lut_t *lut;

        DBUG_ASSERT (nextdim == NULL, "code and nextdim?");

        /*
         * compute current offset
         */
        final_offsets
          = UpdateOffsets (index, INFO_OFFSETS (arg_info), INFO_CURRENT_DIM (arg_info),
                           NULL, INFO_WITH2_LENGTHS (arg_info), &preassigns,
                           &rangeoffsets, arg_info);
        /*
         * compute current index vector
         */
        DBUG_ASSERT (INFO_INDICES (arg_info) != NULL, "no wl indices found");

        iv_avis = MakeIntegerVar (&INFO_VARDECS (arg_info));
        iv_avis = AssignValue (iv_avis,
                               TCmakePrf2 (F_add_SxS,
                                           TBmakeId (IDS_AVIS (INFO_INDICES (arg_info))),
                                           TBmakeId (index)),
                               &preassigns);
        old_iv_avis = IDS_AVIS (INFO_INDICES (arg_info));
        IDS_AVIS (INFO_INDICES (arg_info)) = iv_avis;
        lut = PrepareCopyLut (INFO_LUT (arg_info), final_offsets, &preassigns, arg_info);
        if (BLOCK_ASSIGNS (CODE_CBLOCK (*code)) == NULL) {
            body = TBmakeBlock (preassigns, NULL);
        } else {
            if (CODE_USED (*code) > 1) {
                body
                  = DUPdoDupTreeLutSsa (CODE_CBLOCK (*code), lut, INFO_FUNDEF (arg_info));
            } else {
                DBUG_ASSERT (CODE_USED (*code) == 1, "used counter out of sync!");

                body = DUPdoDupTreeLut (CODE_CBLOCK (*code), lut);
            }
            BLOCK_ASSIGNS (body) = TCappendAssign (preassigns, BLOCK_ASSIGNS (body));

            INFO_INDICES (arg_info) = TBmakeIds (iv_avis, INFO_INDICES (arg_info));
            body = Accu2DimIndex (body, arg_info);
            INFO_INDICES (arg_info) = FREEdoFreeNode (INFO_INDICES (arg_info));
        }

        res = DUPdoDupTreeLut (CODE_CEXPRS (*code), lut);

        /* consume this reference */
        CODE_USED (*code)--;
        *code = NULL;

        /* only remove contents! the lut is reused! */
        lut = LUTremoveContentLut (lut);

        /* compute resultindices */
        resultindices = TCids2Exprs (final_offsets);

        /* free final offsets */
        if (final_offsets != NULL) {
            final_offsets = FREEdoFreeTree (final_offsets);
        }

        IDS_AVIS (INFO_INDICES (arg_info)) = old_iv_avis;
    } else if (nextdim != NULL) {
        body = MakeRangeBody (index, nextdim, NULL, TRUE, &res, &rangeoffsets, arg_info);
        /* no resultindices here as not innermost level */
        resultindices = NULL;
    } else {
        /* DBUG_ASSERT( ( TCcountWithopsNeq( INFO_WITH2_WITHOPS( arg_info),
           N_fold) == 0),
           "Must just be folds if doing nothing"); */
        body = NULL;
    }

    if (body != NULL) {
        node *min;
        if (INFO_CURRENT_SIZE (arg_info) != NULL) {
            /* Have a chunk size so do not go past the end */
            min = ComputeMin (lower, INFO_CURRENT_SIZE (arg_info),
                              &INFO_PREASSIGNS (arg_info), arg_info);
        } else {
            min = DUPdoDupNode (lower);
        }
        result = TBmakeRange (TBmakeIds (index, NULL), min, max,
                              NULL, /* grids have no chunksize */
                              body, res, rangeoffsets, next);
        RANGE_IIRR (result) = resultindices;
    } else {
        /* This range is a nop so do not create it */
        result = next;
    }

    DBUG_RETURN (result);
}

static node *
ATravNIfail (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NIP_RESULT (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

static node *
ATravNIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NIP_LHS (arg_info) = IDS_NEXT (INFO_NIP_LHS (arg_info));
    GENARRAY_NEXT (arg_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravNImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#ifdef MUTC_MODARRAY
    if (TUshapeKnown (AVIS_TYPE (IDS_AVIS (INFO_NIP_LHS (arg_info))))) {
#else
    if (TUdimKnown (AVIS_TYPE (IDS_AVIS (INFO_NIP_LHS (arg_info))))) {
#endif
        INFO_NIP_LHS (arg_info) = IDS_NEXT (INFO_NIP_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVopt (MODARRAY_NEXT (arg_node), arg_info);
    } else {
        INFO_NIP_RESULT (arg_info) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

static node *
ATravNIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#if !FUNS_IN_WL_SUPPORTED
    if (!FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_NIP_ARG (arg_info) = FALSE;

        arg_node = TRAVcont (arg_node, arg_info);

        INFO_NIP_RESULT (arg_info)
          = INFO_NIP_RESULT (arg_info) || INFO_NIP_ARG (arg_info);
    }
#endif

    DBUG_RETURN (arg_node);
}

static node *
ATravNIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_NIP_ARG (arg_info) = INFO_NIP_ARG (arg_info)
                              || (!TYisScalar (AVIS_TYPE (ARG_AVIS (arg_node)))
                                  || TUisHidden (AVIS_TYPE (ARG_AVIS (arg_node))));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

static node *
ATravNIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#if 0
  if ( TYisAKS( AVIS_TYPE( IDS_AVIS( INFO_NIP_LHS( arg_info)))) ||
       TYisAKV( AVIS_TYPE( IDS_AVIS( INFO_NIP_LHS( arg_info))))){
    INFO_NIP_LHS( arg_info) = IDS_NEXT( INFO_NIP_LHS( arg_info));
    FOLD_NEXT( arg_node) = TRAVopt( FOLD_NEXT( arg_node), arg_info);
  } else {
    INFO_NIP_RESULT( arg_info) = TRUE;
  }
#else
    INFO_NIP_LHS (arg_info) = IDS_NEXT (INFO_NIP_LHS (arg_info));
    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);
#endif

    DBUG_RETURN (arg_node);
}

static bool
NotImplemented (node *with, info *arg_info)
{
    bool result;
    info *info;
    anontrav_t nip_trav[6]
      = {{N_genarray, &ATravNIgenarray}, {N_modarray, &ATravNImodarray},
         {N_fold, &ATravNIfold},         {N_break, &ATravNIfail},
         {N_propagate, &ATravNIfail},    {(nodetype)0, NULL}};
    anontrav_t nap_trav[3]
      = {{N_ap, &ATravNIap}, {N_arg, &ATravNIarg}, {(nodetype)0, NULL}};

    DBUG_ENTER ();

    info = InitialiseNipInfo (arg_info);

    TRAVpushAnonymous (nip_trav, &TRAVnone);
    /* check for genarray or AKS-modarray only */
    WITH2_WITHOP (with) = TRAVdo (WITH2_WITHOP (with), info);
    TRAVpop ();

    if (!INFO_NIP_RESULT (info)) {
        /* check for no ap in body due to memory management problems */
        TRAVpushAnonymous (nap_trav, &TRAVsons);
        WITH2_CODE (with) = TRAVdo (WITH2_CODE (with), info);
        TRAVpop ();
    }

    /* check that there is just one segment unless we are only folding */
    INFO_DENSE (info) = !(INFO_NIP_RESULT (info)
                          || ((TCcountWithops (WITH2_WITHOP (with))
                               == TCcountWithopsNeq (WITH2_WITHOP (with), N_fold))
                              && (TCcountWlseg (WITH2_SEGS (with)) != 1)));

    result = INFO_NIP_RESULT (info);

    info = ResetNipInfo (info);

    DBUG_RETURN (result);
}

static node *
ATravCOgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (
      TBmakeIds (GENARRAY_IDX (arg_node), TRAVopt (GENARRAY_NEXT (arg_node), arg_info)));
}

static node *
ATravCOmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_RETURN (
      TBmakeIds (MODARRAY_IDX (arg_node), TRAVopt (MODARRAY_NEXT (arg_node), arg_info)));
}

static node *
CreateWith2Offsets (node *withops)
{
    node *result;
    anontrav_t atrav[3] = {{N_genarray, &ATravCOgenarray},
                           {N_modarray, &ATravCOmodarray},
                           {(nodetype)0, NULL}};

    DBUG_ENTER ();
    TRAVpushAnonymous (atrav, &TRAVsons);
    result = TRAVdo (withops, NULL);
    TRAVpop ();

    DBUG_RETURN (result);
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
 * @fn node *WLSDfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_VARDECS (arg_info) == NULL, "leftover vardecs found.");

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        FUNDEF_VARDECS (arg_node)
          = TCappendVardec (FUNDEF_VARDECS (arg_node), INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

    INFO_FUNDEF (arg_info) = NULL;

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDassign(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * bottom-up traversal to ease inserting pre-assigns
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_ASSERT (INFO_PREASSIGNS (arg_info) == NULL, "left-over pre-assigns found.");

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDlet(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITH2_LHS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDblock (node *arg_node, info *arg_info)
{
    node *outerlhs, *outerpreassigns;

    DBUG_ENTER ();

    outerlhs = INFO_WITH2_LHS (arg_info);
    outerpreassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_PREASSIGNS (arg_info) = outerpreassigns;
    INFO_WITH2_LHS (arg_info) = outerlhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwith (node *arg_node, info *arg_info)
{
    bool old_incudawl;

    DBUG_ENTER ();

    old_incudawl = INFO_INCUDAWL (arg_info);
    INFO_INCUDAWL (arg_info) = WITH_CUDARIZABLE (arg_node);

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    INFO_INCUDAWL (arg_info) = old_incudawl;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwith2(node *arg_node, info *arg_info)
 *
 * @brief Transforms the given with2 node into a nesting of with3 nodes.
 *
 *****************************************************************************/
node *
WLSDwith2 (node *arg_node, info *arg_info)
{
    node *ranges, *withops;
    node *iv_avis;
    char *with_dist;

    DBUG_ENTER ();

    if (NotImplemented (arg_node, arg_info)) {
        CTInote (EMPTY_LOC, "Cannot transform with-loop due to unsupported operation");
    } else if ((global.backend == BE_mutc)
               || ((global.backend == BE_cuda || global.backend == BE_cudahybrid)
                   && INFO_INCUDAWL (arg_info))) {
        INFO_TRANSFORMED_W2_TO_W3 (arg_info) = TRUE;
        /*
         * First of all, we transform the code blocks. As we migth potentially
         * copy them into multiple with3 bodies later on, transforming them
         * first reduces complexity. Furthermore, this ensures that we never
         * nest transformations.
         */
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

        if (INFO_FRAME_INDICES (arg_info) != NULL) {
            INFO_FRAME_INDICES (arg_info)
              = FREEdoFreeTree (INFO_FRAME_INDICES (arg_info));
        }
        /*
         * we keep the withops as a template for later use in the new with3.
         */
        INFO_WITH2_WITHOPS (arg_info) = WITH2_WITHOP (arg_node);

        /*
         * We need to grab the indexvector, indexscalars and withloop offset
         * from the withid.
         */
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

        INFO_WITH2_OFFSETS (arg_info) = CreateWith2Offsets (WITH2_WITHOP (arg_node));

        /*
         * preset the indices and offset with 0 and compute the length of the
         * dimensions of the results.
         */
        iv_avis
          = MakeIntegerConst (0, &INFO_PREASSIGNS (arg_info), &INFO_VARDECS (arg_info));
        INFO_INDICES (arg_info) = TBmakeIds (iv_avis, NULL);

        INFO_WITH2_LENGTHS (arg_info)
          = ComputeLengths (INFO_WITH2_WITHOPS (arg_info), arg_info);

        INFO_OFFSETS (arg_info) = InitOffsets (INFO_WITH2_LENGTHS (arg_info), arg_info);

        /*
         * Finally, go off and transform :)
         *
         * Note that traversing the segments will return the ranges for the
         * outermost level of the new with3 stucture. Traversing the withops
         * creates new withops for the current dimension (should be 0).
         */
        ranges = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        withops = ComputeNewWithops (INFO_WITH2_WITHOPS (arg_info), arg_info);
        with_dist = STRcpy (WITH2_DIST (arg_node));
        /*
         * Build the new with-loop 3
         */
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TBmakeWith3 (ranges, withops);
        WITH3_DENSE (arg_node) = INFO_DENSE (arg_info);
        WITH3_ISTOPLEVEL (arg_node) = TRUE;
        WITH3_DIST (arg_node) = with_dist;

        /*
         * reset the info structure, mainly to ease finding bugs and to prevent
         * memory leaks.
         */
        INFO_INDICES (arg_info) = FREEdoFreeTree (INFO_INDICES (arg_info));
        if (INFO_OFFSETS (arg_info) != NULL) {
            INFO_OFFSETS (arg_info) = FREEdoFreeTree (INFO_OFFSETS (arg_info));
        }
        if (INFO_WITH2_LENGTHS (arg_info) != NULL) {
            INFO_WITH2_LENGTHS (arg_info)
              = FREEdoFreeTree (INFO_WITH2_LENGTHS (arg_info));
        }
        INFO_WITH2_WITHOPS (arg_info) = NULL;
        INFO_WITH2_IVECT (arg_info) = NULL;
        INFO_WITH2_ISCLS (arg_info) = NULL;
        INFO_WITH2_OFFSETS (arg_info) = NULL;

        DBUG_ASSERT (INFO_CURRENT_DIM (arg_info) == 0, "dimension counter out of sync.");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwithid(node *arg_node, info *arg_info)
 *
 * @brief Stores the avis of the index vector, the index scalars and
 *        the withloop offset in the info structure.
 *
 *****************************************************************************/
node *
WLSDwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITH2_IVECT (arg_info) = WITHID_VEC (arg_node);
    INFO_WITH2_ISCLS (arg_info) = WITHID_IDS (arg_node);
#if 0 /* need this list to be a one2one mapping to the genarray/modarray */
  INFO_WITH2_OFFSETS( arg_info) = WITHID_IDXS( arg_node);
#endif

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlseg(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlseg (node *arg_node, info *arg_info)
{
    node *ranges;

    DBUG_ENTER ();

    /*
     * A segment itself is just a wrapper around the strides/grids. Therefore,
     * it does not result in any additional structure. We just traverse the
     * content and then continue with the next one.
     */
    ranges = TRAVdo (WLSEG_CONTENTS (arg_node), arg_info);

    ranges = TCappendRange (ranges, TRAVopt (WLSEG_NEXT (arg_node), arg_info));

    DBUG_RETURN (ranges);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlstride(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlstride (node *arg_node, info *arg_info)
{
    node *result, *next = NULL;

    DBUG_ENTER ();

    arg_info = DeFrameDim (arg_info);

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        next = TRAVdo (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    result = ProcessStride (WLSTRIDE_LEVEL (arg_node), WLSTRIDE_DIM (arg_node),
                            WLSTRIDE_BOUND1 (arg_node), WLSTRIDE_BOUND2 (arg_node),
                            WLSTRIDE_STEP (arg_node), WLSTRIDE_CONTENTS (arg_node), next,
                            arg_info);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlblock(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlblock (node *arg_node, info *arg_info)
{
    node *result, *next = NULL;
    node *contents = NULL;

    DBUG_ENTER ();

    arg_info = FrameDim (arg_info);

    if (WLBLOCK_NEXT (arg_node) != NULL) {
        next = TRAVdo (WLBLOCK_NEXT (arg_node), arg_info);
    }

    contents = WLBLOCK_CONTENTS (arg_node);

    if (contents == NULL) {
        contents = WLBLOCK_NEXTDIM (arg_node);
    }

    result = ProcessBlock (WLBLOCK_LEVEL (arg_node), WLBLOCK_DIM (arg_node),
                           WLBLOCK_BOUND1 (arg_node), WLBLOCK_BOUND2 (arg_node),
                           WLBLOCK_STEP (arg_node), contents, next, arg_info);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlgrid(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlgrid (node *arg_node, info *arg_info)
{
    node *result;

    DBUG_ENTER ();

    result = TRAVopt (WLGRID_NEXT (arg_node), arg_info);

    if (!WLGRID_ISNOOP (arg_node)) {
        result = ProcessGrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node),
                              WLGRID_BOUND1 (arg_node), WLGRID_BOUND2 (arg_node),
                              WLGRID_NEXTDIM (arg_node), &WLGRID_CODE (arg_node), result,
                              arg_info);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With-loop Split Dimensions -->
 *****************************************************************************/

#undef DBUG_PREFIX
