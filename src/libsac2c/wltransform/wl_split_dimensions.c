/*
 * $Id$
 */

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
#include "dbug.h"
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
 *                             the unrolling lenght of each dimension for each
 *                             operator that has an index.
 * INFO_CURRENT_DIM:           Dimension currently being transformed.
 * INFO_CURRENT_SIZE:          Length of current subarray to be computed (used
 *                             for shape of genarray/modarray).
 * INFO_WITH3_ASSIGN:          Assignment node for current with3
 * INFO_INDICES:               New withloop indices in reverse order as a
 *                             chain of N_ids nodes.
 * INFO_OFFSETS:               Current withloop offset as N_ids chain. If the
 *                             corresponding operator does not have an offset,
 *                             this is NULL.
 * INFO_ACCUS:                 Stores the N_ids chain of accus for the
 *                             current with-loop 3.
 * INFO_VARDECS:               Stores vardecs that need to be joined into
 *                             the fundef vardec chain.
 * INFO_PREASSIGNS:            Holds assignments that need to be inserted before
 *                             the current assign.
 * INFO_LUT:                   lut used sharedly for renaming. Needs always to
 *                             be cleared before traversing on.
 *****************************************************************************/
struct INFO {
    node *with2_ivect;
    node *with2_iscls;
    node *with2_offsets;
    node *with2_withops;
    node *with2_lhs;
    node *with2_lengths;
    int current_dim;
    node *current_size;
    node *with3_assign;
    node *indices;
    node *offsets;
    node *accus;
    node *vardecs;
    node *preassigns;
    lut_t *lut;
};

#define INFO_WITH2_IVECT(n) ((n)->with2_ivect)
#define INFO_WITH2_ISCLS(n) ((n)->with2_iscls)
#define INFO_WITH2_OFFSETS(n) ((n)->with2_offsets)
#define INFO_WITH2_WITHOPS(n) ((n)->with2_withops)
#define INFO_WITH2_LHS(n) ((n)->with2_lhs)
#define INFO_WITH2_LENGTHS(n) ((n)->with2_lengths)
#define INFO_CURRENT_DIM(n) ((n)->current_dim)
#define INFO_CURRENT_SIZE(n) ((n)->current_size)
#define INFO_WITH3_ASSIGN(n) ((n)->with3_assign)
#define INFO_INDICES(n) ((n)->indices)
#define INFO_OFFSETS(n) ((n)->offsets)
#define INFO_ACCUS(n) ((n)->accus)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_LUT(n) ((n)->lut)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH2_IVECT (result) = NULL;
    INFO_WITH2_ISCLS (result) = NULL;
    INFO_WITH2_OFFSETS (result) = NULL;
    INFO_WITH2_WITHOPS (result) = NULL;
    INFO_WITH2_LHS (result) = NULL;
    INFO_WITH2_LENGTHS (result) = NULL;
    INFO_CURRENT_DIM (result) = 0;
    INFO_CURRENT_SIZE (result) = NULL;
    INFO_WITH3_ASSIGN (result) = NULL;
    INFO_INDICES (result) = NULL;
    INFO_OFFSETS (result) = NULL;
    INFO_ACCUS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_LUT (result) = LUTgenerateLut ();

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_LUT (info) = LUTremoveLut (INFO_LUT (info));

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
 * @fn node *WLSDdoWithLoopSplitDimensions( node *syntax_tree)
 *
 *****************************************************************************/
node *
WLSDdoWithLoopSplitDimensions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("WLSDdoWithLoopSplitDimensions");

    info = MakeInfo ();

    DBUG_PRINT ("WLSD", ("Starting to split with-loops by dimension."));

    TRAVpush (TR_wlsd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("WLSD", ("With-loop splitting complete."));

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

    DBUG_ENTER ("MakeIntegerVar");

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

    DBUG_ENTER ("AssignValue");

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), rhs), NULL);
    AVIS_SSAASSIGN (avis) = assign;

    *assigns = TCappendAssign (*assigns, assign);

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

    DBUG_ENTER ("IsNum");

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

    DBUG_ENTER ("GetNum");

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
 * @brief Increases the level of the dimension encoded in the info structure.
 *        This includes incrementing INFO_CURRENT_DIM and pushing an empty
 *        N_ids node on the withloop indices stack.
 *
 * @param arg_info info structure
 *
 * @return updated info structure
 ******************************************************************************/
static info *
PushDim (info *arg_info)
{
    node *zero_avis;

    DBUG_ENTER ("PushDim");

    INFO_CURRENT_DIM (arg_info)++;
    zero_avis = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)), TBmakeNum (0),
                             &INFO_PREASSIGNS (arg_info));
    INFO_INDICES (arg_info) = TBmakeIds (zero_avis, INFO_INDICES (arg_info));

    DBUG_RETURN (arg_info);
}

/** <!-- ****************************************************************** -->
 * @fn info PopDim( info *arg_info)
 *
 * @brief Undoes the changes done by PushDim and removes on level of nesting
 *        from the info structure.
 *
 * @param arg_info info structure
 *
 * @return updated info structure
 ******************************************************************************/
static info *
PopDim (info *arg_info)
{
    DBUG_ENTER ("PopDim");

    INFO_INDICES (arg_info) = FREEdoFreeNode (INFO_INDICES (arg_info));
    INFO_CURRENT_DIM (arg_info)--;

    DBUG_RETURN (arg_info);
}

/** <!-- ****************************************************************** -->
 * @fn bool NeedsFitting( node *lower, node *upper, node *step)
 *
 * @brief Returns true if any of the arguments is not constant (i.e. not an
 *        N_num node) or if the range is not a multiple of the step.
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

    DBUG_ENTER ("NeedsFitting");

    result = !(IsNum (lower) && IsNum (upper) && IsNum (step)
               && (((GetNum (upper) - GetNum (lower)) % GetNum (step)) == 0));

    DBUG_RETURN (result);
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

    DBUG_ENTER ("NewBounds");

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

        nupAvis
          = AssignValue (nupAvis,
                         TCmakePrf2 (F_sub_SxS, DUPdoDupTree (upper), TBmakeId (ovlAvis)),
                         assigns);

        ovlAvis
          = AssignValue (ovlAvis, TCmakePrf2 (F_mod_SxS, length, DUPdoDupTree (step)),
                         assigns);

        newsize = TBmakeId (ovlAvis);
        *nupper = TBmakeId (nupAvis);
    }

    DBUG_RETURN (newsize);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param lower
 * @param upper
 * @param *assigns
 * @param arg_info
 *
 * @return
 ******************************************************************************/
static node *
ComputeSize (node *lower, node *upper, node **assigns, info *arg_info)
{
    node *size;

    DBUG_ENTER ("ComputeSize");

    if (IsNum (upper) && IsNum (lower)) {
        /*
         * static length
         */
        size = TBmakeNum (GetNum (upper) - GetNum (lower));

        DBUG_ASSERT ((NUM_VAL (size) >= 0), "negative size found");
    } else {
        /*
         * we have to compute the length of the range at runtime
         */
        node *savis = MakeIntegerVar (&INFO_VARDECS (arg_info));

        savis = AssignValue (savis,
                             TCmakePrf2 (F_sub_SxS, DUPdoDupNode (upper),
                                         DUPdoDupNode (lower)),
                             assigns);

        size = TBmakeId (savis);
    }

    DBUG_RETURN (size);
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

    DBUG_ENTER ("ComputeMax");

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
 * @fn node *ATravCNWgenarray( node *arg_node, info *arg_info)
 *
 * @brief Computes a new genarray withop from the current dimension, current
 *        size and the original with2 genarray withop. Used in an
 *        anonymous traversal by ComputeNewWithops.
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
    node *array;
    int sizeoffset;

    DBUG_ENTER ("ATravCNWgenarray");

    if (PM (PMarray (NULL, &array, GENARRAY_SHAPE (arg_node)))) {
        sizeoffset = (INFO_CURRENT_SIZE (arg_info) == NULL) ? 0 : 1;

        shape = DUPdoDupTree (
          TCgetNthExprs (INFO_CURRENT_DIM (arg_info) + sizeoffset, ARRAY_AELEMS (array)));

        if (INFO_CURRENT_SIZE (arg_info) != NULL) {
            shape = TBmakeExprs (DUPdoDupTree (INFO_CURRENT_SIZE (arg_info)), shape);
        }

        shape = TCmakeIntVector (shape);
    }

    DBUG_ASSERT ((shape != NULL), "no shape info for genarray constructed");

    new_node = TBmakeGenarray (shape, DUPdoDupTree (GENARRAY_DEFAULT (arg_node)));

    GENARRAY_NEXT (new_node) = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (new_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *ComputeNewWithops( node *withops, info *arg_info)
 *
 * @brief
 *
 * @param withops
 * @param arg_info
 *
 * @return
 ******************************************************************************/
static node *
ComputeNewWithops (node *withops, info *arg_info)
{
    anontrav_t cnw_trav[6]
      = {{N_genarray, &ATravCNWgenarray}, {N_modarray, &TRAVerror}, {N_fold, &TRAVerror},
         {N_propagate, &TRAVerror},       {N_break, &TRAVerror},    {0, NULL}};
    node *ops;

    DBUG_ENTER ("ComputeNewWithops");

    TRAVpushAnonymous (cnw_trav, &TRAVsons);

    ops = TRAVopt (withops, arg_info);

    TRAVpop ();

    DBUG_RETURN (ops);
}

static node *
ATravCNLgenarray (node *arg_node, info *arg_info)
{
    node *new_node;
    ntype *old_type, *new_type;
    node *avis;
    node *mylhs;

    DBUG_ENTER ("ATravCNLgenarray");

    mylhs = INFO_WITH2_LHS (arg_info);

    /*
     * go to next withop with next lhs. We have to reset the info structure
     * again as it will be needed multiple times.
     */
    INFO_WITH2_LHS (arg_info) = IDS_NEXT (INFO_WITH2_LHS (arg_info));
    new_node = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);
    INFO_WITH2_LHS (arg_info) = mylhs;

    old_type = AVIS_TYPE (IDS_AVIS (mylhs));

    DBUG_ASSERT ((!TYisAKV (old_type)), "lhs with known value?");

    if (TUshapeKnown (old_type)) {
        shape *shape;

        if ((INFO_CURRENT_SIZE (arg_info) == NULL)
            || (IsNum (INFO_CURRENT_SIZE (arg_info)))) {
            shape = SHdropFromShape (INFO_CURRENT_DIM (arg_info), TYgetShape (old_type));

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
            new_type
              = TYmakeAKD (TYcopyType (TYgetScalar (old_type)),
                           TYgetDim (old_type) - INFO_CURRENT_DIM (arg_info), NULL);
        }
    } else if (TUdimKnown (old_type)) {
        new_type = TYmakeAKD (TYcopyType (TYgetScalar (old_type)),
                              TYgetDim (old_type) - INFO_CURRENT_DIM (arg_info), NULL);
    } else {
        new_type = TYcopyType (old_type);
    }

    avis = TBmakeAvis (TRAVtmpVar (), new_type);
    AVIS_SSAASSIGN (avis) = INFO_WITH3_ASSIGN (arg_info);
    INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));

    new_node = TBmakeIds (avis, new_node);

    DBUG_RETURN (new_node);
}

static node *
ComputeNewLhs (node *withops, info *arg_info)
{
    anontrav_t cnw_trav[6]
      = {{N_genarray, &ATravCNLgenarray}, {N_modarray, &TRAVerror}, {N_fold, &TRAVerror},
         {N_propagate, &TRAVerror},       {N_break, &TRAVerror},    {0, NULL}};
    node *lhs;

    DBUG_ENTER ("ComputeNewLhs");

    TRAVpushAnonymous (cnw_trav, &TRAVsons);

    lhs = TRAVopt (withops, arg_info);

    TRAVpop ();

    DBUG_RETURN (lhs);
}

static node *
InitOffsets (node *lengths, info *arg_info)
{
    node *zeroavis;
    node *result = NULL;

    DBUG_ENTER ("InitOffsets");

    if (lengths != NULL) {
        zeroavis = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)), TBmakeNum (0),
                                &INFO_PREASSIGNS (arg_info));

        do {
            result = TBmakeIds (zeroavis, result);
            lengths = SET_NEXT (lengths);
        } while (lengths != NULL);
    }

    DBUG_RETURN (result);
}

static node *
UpdateOffsets (node *index, node *offsets, int dim, node *lengths, node **assigns,
               info *arg_info)
{
    node *oavis, *tavis;
    node *new_offsets;
    node *len;

    DBUG_ENTER ("UpdateOffsets");

    if (lengths != NULL) {
        new_offsets = UpdateOffsets (index, IDS_NEXT (offsets), dim, SET_NEXT (lengths),
                                     assigns, arg_info);

        len = TCgetNthExprsExpr (dim, SET_MEMBER (lengths));
        DBUG_ASSERT ((len != NULL), "no length found");

        tavis = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)),
                             TCmakePrf2 (F_mul_SxS, TBmakeId (index), DUPdoDupNode (len)),
                             assigns);

        oavis = AssignValue (MakeIntegerVar (&INFO_VARDECS (arg_info)),
                             TCmakePrf2 (F_add_SxS, TBmakeId (tavis),
                                         TBmakeId (IDS_AVIS (offsets))),
                             assigns);

        new_offsets = TBmakeIds (oavis, new_offsets);
    } else {
        new_offsets = NULL;
    }

    DBUG_RETURN (new_offsets);
}

static lut_t *
InsertIndicesIntoLut (lut_t *lut, node **w2ind, node *w3ind)
{
    DBUG_ENTER ("InsertIndicesIntoLut");

    if (w3ind != NULL) {
        lut = InsertIndicesIntoLut (lut, w2ind, IDS_NEXT (w3ind));

        lut = LUTinsertIntoLutP (lut, IDS_AVIS (*w2ind), IDS_AVIS (w3ind));

        *w2ind = IDS_NEXT (*w2ind);
    }

    DBUG_RETURN (lut);
}

static node *
CreateIndexVectorExprs (node *index)
{
    node *result = NULL;

    DBUG_ENTER ("CreateIndexVectorExprs");

    if (index != NULL) {
        if (IDS_NEXT (index) != NULL) {
            result = CreateIndexVectorExprs (IDS_NEXT (index));
        }
        result = TCappendExprs (result, TBmakeExprs (TBmakeId (IDS_AVIS (index)), NULL));
    }

    DBUG_RETURN (result);
}

static lut_t *
PrepareCopyLut (lut_t *lut, node *offsets, node **assigns, info *arg_info)
{
    node *w2off, *w3off;
    node *w2ind;
    node *ivavis, *ivarray, *ivelems, *ivassign;

    DBUG_ENTER ("PrepareCopyLut");

    /* insert offsets */
    w2off = INFO_WITH2_OFFSETS (arg_info);
    w3off = offsets;
    while (w3off != NULL) {
        DBUG_ASSERT ((w2off != NULL), "less with2 offsets than with3 offsets");

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
 * @fn node *MakeRangeBody( node *outerindex, node *contents, node *size,
 *                          node **results, info *arg_info)
 *
 * @brief
 *
 * @param outerindex
 * @param contents
 * @param size chunksize of this level. NULL denotes producing scalars wrt.
 *             the outermost dimension, i.e., a grid.
 * @param newdim this body is the last one in this dimension
 * @param *results
 * @param arg_info
 *
 * @return
 ******************************************************************************/
static node *
MakeRangeBody (node *outerindex, node *contents, node *size, bool newdim, node **results,
               info *arg_info)
{
    node *body, *ranges, *ops, *lhs, *with3;
    node *assigns = NULL;
    node *old_size, *old_preassigns;
    node *old_offsets;
    node *iv_avis, *old_iv_avis;
    node *iv_assigns = NULL;

    DBUG_ENTER ("MakeRangeBody");

    /*
     * compute current offset
     */
    old_offsets = INFO_OFFSETS (arg_info);
    INFO_OFFSETS (arg_info)
      = UpdateOffsets (outerindex, INFO_OFFSETS (arg_info), INFO_CURRENT_DIM (arg_info),
                       INFO_WITH2_LENGTHS (arg_info), &iv_assigns, arg_info);
    /*
     * compute current index vector
     */
    DBUG_ASSERT ((INFO_INDICES (arg_info) != NULL), "no wl indices found");

    iv_avis = MakeIntegerVar (&INFO_VARDECS (arg_info));
    iv_avis
      = AssignValue (iv_avis,
                     TCmakePrf2 (F_add_SxS, TBmakeId (IDS_AVIS (INFO_INDICES (arg_info))),
                                 TBmakeId (outerindex)),
                     &iv_assigns);

    /*
     * compute body: setup new state and traverse one level below
     */
    old_iv_avis = IDS_AVIS (INFO_INDICES (arg_info));
    IDS_AVIS (INFO_INDICES (arg_info)) = iv_avis;
    old_size = INFO_CURRENT_SIZE (arg_info);
    INFO_CURRENT_SIZE (arg_info) = size;
    old_preassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;

    if (newdim) {
        arg_info = PushDim (arg_info);
    }
    ranges = TRAVdo (contents, arg_info);
    if (newdim) {
        arg_info = PopDim (arg_info);
    }

    DBUG_ASSERT ((INFO_WITH3_ASSIGN (arg_info) == NULL), "left-over with3 assign");

    /*
     * produce with3
     */
    /*
     * create the assign node empty to set SSA_ASSIGN properly
     */
    INFO_WITH3_ASSIGN (arg_info) = TBmakeAssign (NULL, NULL);

    ops = ComputeNewWithops (INFO_WITH2_WITHOPS (arg_info), arg_info);
    lhs = ComputeNewLhs (INFO_WITH2_WITHOPS (arg_info), arg_info);

    with3 = TBmakeWith3 (ranges, ops);

    ASSIGN_INSTR (INFO_WITH3_ASSIGN (arg_info)) = TBmakeLet (lhs, with3);
    ASSIGN_NEXT (INFO_WITH3_ASSIGN (arg_info)) = assigns;
    assigns = INFO_WITH3_ASSIGN (arg_info);
    INFO_WITH3_ASSIGN (arg_info) = NULL;

    /*
     * glue it all together
     */
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
    INFO_OFFSETS (arg_info) = FREEdoFreeTree (INFO_OFFSETS (arg_info));
    INFO_OFFSETS (arg_info) = old_offsets;

    DBUG_RETURN (body);
}

/** <!-- ****************************************************************** -->
 * @fn node *ProcessStride( int level, int dim, node *lower, node *upper,
 *                          node *step, node *contents, node *next,
 *                          info *arg_info)
 *
 * @brief Computes a range node corresponding to the given stride node.
 *
 * @param level
 * @param dim
 * @param lower CONSUMED!
 * @param upper CONSUMED!
 * @param step CONSUMED!
 * @param contents
 * @param next
 * @param arg_info
 *
 * @return
 ******************************************************************************/
static node *
ProcessStride (int level, int dim, node *lower, node *upper, node *step, node *contents,
               node *next, info *arg_info)
{
    node *index, *body, *results;

    DBUG_ENTER ("ProcessStride");

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
        node *nupper, *over, *body, *index, *results;

        index = MakeIntegerVar (&INFO_VARDECS (arg_info));
        over = ComputeNewBounds (lower, upper, step, &nupper, &INFO_PREASSIGNS (arg_info),
                                 arg_info);
        body = MakeRangeBody (index, contents, over, FALSE, &results, arg_info);

        next = TBmakeRange (TBmakeIds (index, NULL), DUPdoDupTree (nupper), upper, over,
                            body, results, next);

        /*
         * replace old bounds
         */
        upper = nupper;
    }

    index = MakeIntegerVar (&INFO_VARDECS (arg_info));
    body = MakeRangeBody (index, contents, step, FALSE, &results, arg_info);

    next = TBmakeRange (TBmakeIds (index, NULL), lower, upper, step, body, results, next);

    DBUG_RETURN (next);
}

/** <!-- ****************************************************************** -->
 * @brief
 *
 * @param level
 * @param dim
 * @param lower CONSUMED!
 * @param upper CONSUMED!
 * @param nextdim
 * @param code
 * @param next
 * @param arg_info
 *
 * @return
 ******************************************************************************/
static node *
ProcessGrid (int level, int dim, node *lower, node *upper, node *nextdim, node *code,
             node *next, info *arg_info)
{
    node *index, *max, *body, *res, *result;

    DBUG_ENTER ("ProcessGrid");

    /*
     * first process all remaining grids on this level
     */
    next = TRAVopt (next, arg_info);

    /*
     * now build the range for this level
     */
    index = MakeIntegerVar (&INFO_VARDECS (arg_info));
    max = ComputeMax (lower, upper, &INFO_PREASSIGNS (arg_info), arg_info);

    if (code != NULL) {
        node *preassigns = NULL;
        node *iv_avis, *old_iv_avis, *final_offsets;
        lut_t *lut;

        DBUG_ASSERT ((nextdim == NULL), "code and nextdim?");

        if (NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (code))) == N_empty) {
            /*
             * nothing much to do here
             */
            body = DUPdoDupTree (CODE_CBLOCK (code));
        } else {
            /*
             * compute current offset
             */
            final_offsets
              = UpdateOffsets (index, INFO_OFFSETS (arg_info),
                               INFO_CURRENT_DIM (arg_info), INFO_WITH2_LENGTHS (arg_info),
                               &preassigns, arg_info);
            /*
             * compute current index vector
             */
            DBUG_ASSERT ((INFO_INDICES (arg_info) != NULL), "no wl indices found");

            iv_avis = MakeIntegerVar (&INFO_VARDECS (arg_info));
            iv_avis
              = AssignValue (iv_avis,
                             TCmakePrf2 (F_add_SxS,
                                         TBmakeId (IDS_AVIS (INFO_INDICES (arg_info))),
                                         TBmakeId (index)),
                             &preassigns);
            old_iv_avis = IDS_AVIS (INFO_INDICES (arg_info));
            IDS_AVIS (INFO_INDICES (arg_info)) = iv_avis;
            lut = PrepareCopyLut (INFO_LUT (arg_info), final_offsets, &preassigns,
                                  arg_info);
            body = DUPdoDupTreeLut (CODE_CBLOCK (code), lut);
            BLOCK_INSTR (body) = TCappendAssign (preassigns, BLOCK_INSTR (body));
            /* only remove contents! the lut is reused! */
            lut = LUTremoveContentLut (lut);
            final_offsets = FREEdoFreeTree (final_offsets);
            IDS_AVIS (INFO_INDICES (arg_info)) = old_iv_avis;
        }

        res = DUPdoDupTree (CODE_CEXPRS (code));
    } else {
        DBUG_ASSERT ((nextdim != NULL), "neither code nor nextdim?");

        body = MakeRangeBody (index, nextdim, NULL, TRUE, &res, arg_info);
    }

    result = TBmakeRange (TBmakeIds (index, NULL), lower, max,
                          NULL, /* grids have no chunksize */
                          body, res, next);

    /*
     * consume unuseds
     */
    upper = FREEdoFreeNode (upper);

    DBUG_RETURN (result);
}

static node *
ComputeOneLengthVector (node *aelems, info *arg_info)
{
    node *exprs;
    node *lavis;
    node *len;

    DBUG_ENTER ("ComputeOneLengthVector");

    if (EXPRS_NEXT (aelems) != NULL) {
        exprs = ComputeOneLengthVector (EXPRS_NEXT (aelems), arg_info);

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
        }
        exprs = TBmakeExprs (len, exprs);
    } else {
        exprs = TBmakeExprs (TBmakeNum (1), NULL);
    }

    DBUG_RETURN (exprs);
}

static node *
ATravCDLgenarray (node *arg_node, info *arg_info)
{
    node *set, *exprs, *sarray;
    bool match;

    DBUG_ENTER ("ATravCDLgenarray");

    set = TRAVopt (GENARRAY_NEXT (arg_node), arg_info);

    match = PM (PMarray (NULL, &sarray, GENARRAY_SHAPE (arg_node)));
    DBUG_ASSERT (match, "shape not defined as vector");

    exprs = ComputeOneLengthVector (ARRAY_AELEMS (sarray), arg_info);

    set = TBmakeSet (exprs, set);

    DBUG_RETURN (set);
}

static node *
ComputeLengths (node *withops, info *arg_info)
{
    node *result;

    anontrav_t len_trav[3]
      = {{N_genarray, &ATravCDLgenarray}, {N_modarray, &TRAVerror}, {0, NULL}};

    DBUG_ENTER ("ComputeLengths");

    TRAVpushAnonymous (len_trav, &TRAVsons);
    result = TRAVopt (withops, arg_info);
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
    DBUG_ENTER ("WLSDfundef");

    DBUG_ASSERT ((INFO_VARDECS (arg_info) == NULL), "leftover vardecs found.");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (FUNDEF_VARDEC (arg_node), INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

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
    DBUG_ENTER ("WLSDassign");

    /*
     * bottom-up traversal to ease inserting pre-assigns
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_ASSERT ((INFO_PREASSIGNS (arg_info) == NULL), "left-over pre-assigns found.");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

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
    DBUG_ENTER ("WLSDlet");

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

    DBUG_ENTER ("WLSDblock");

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
 * @fn node *WLSDwith2(node *arg_node, info *arg_info)
 *
 * @brief Transforms the given with2 node into a nesting of with3 nodes.
 *
 *****************************************************************************/
node *
WLSDwith2 (node *arg_node, info *arg_info)
{
    node *ranges, *withops, *with3;
    node *iv_avis;

    DBUG_ENTER ("WLSDwith2");

    /*
     * First of all, we transform the code blocks. As we migth potentially
     * copy them into multiple with3 bodies later on, transforming them
     * first reduces complexity. Furthermore, this ensures that we never
     * nest transformations.
     */
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    /*
     * we keep the withops as a template for later use in the new with3.
     */
    INFO_WITH2_WITHOPS (arg_info) = WITH2_WITHOP (arg_node);

    /*
     * We need to grab the indexvector, indexscalars and withloop offset
     * from the withid.
     */
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);

    /*
     * preset the indices and offset with 0 and compute the length of the
     * dimensions of the results.
     */
    iv_avis = MakeIntegerVar (&INFO_VARDECS (arg_info));
    iv_avis = AssignValue (iv_avis, TBmakeNum (0), &INFO_PREASSIGNS (arg_info));
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

    /*
     * Build the new with-loop 3
     */
    with3 = TBmakeWith3 (ranges, withops);

    arg_node = FREEdoFreeNode (arg_node);

    /*
     * reset the info structure, mainly to ease finding bugs
     */
    INFO_INDICES (arg_info) = FREEdoFreeTree (INFO_INDICES (arg_info));
    INFO_OFFSETS (arg_info) = FREEdoFreeTree (INFO_OFFSETS (arg_info));
    INFO_WITH2_LENGTHS (arg_info) = FREEdoFreeTree (INFO_WITH2_LENGTHS (arg_info));
    INFO_WITH2_WITHOPS (arg_info) = NULL;
    INFO_WITH2_IVECT (arg_info) = NULL;
    INFO_WITH2_ISCLS (arg_info) = NULL;
    INFO_WITH2_OFFSETS (arg_info) = NULL;

    DBUG_ASSERT ((INFO_CURRENT_DIM (arg_info) == 0), "dimension counter out of sync.");

    DBUG_RETURN (with3);
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
    DBUG_ENTER ("WLSDwithid");

    INFO_WITH2_IVECT (arg_info) = WITHID_VEC (arg_node);
    INFO_WITH2_ISCLS (arg_info) = WITHID_IDS (arg_node);
    INFO_WITH2_OFFSETS (arg_info) = WITHID_IDXS (arg_node);

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

    DBUG_ENTER ("WLSDwlseg");

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
 * @fn node *WLSDwlsegvar(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlsegvar (node *arg_node, info *arg_info)
{
    node *ranges;

    DBUG_ENTER ("WLSDwlsegvar");

    /*
     * A segment itself is just a wrapper around the strides/grids. Therefore,
     * it does not result in any additional structure. We just traverse the
     * content and then continue with the next one.
     */
    ranges = TRAVdo (WLSEGVAR_CONTENTS (arg_node), arg_info);

    ranges = TCappendRange (ranges, TRAVopt (WLSEGVAR_NEXT (arg_node), arg_info));

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
    node *bound1, *bound2, *step;

    DBUG_ENTER ("WLSDwlstride");

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        next = TRAVdo (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    bound1 = TBmakeNum (WLSTRIDE_BOUND1 (arg_node));
    bound2 = TBmakeNum (WLSTRIDE_BOUND2 (arg_node));
    step = TBmakeNum (WLSTRIDE_STEP (arg_node));

    result = ProcessStride (WLSTRIDE_LEVEL (arg_node), WLSTRIDE_DIM (arg_node), bound1,
                            bound2, step, WLSTRIDE_CONTENTS (arg_node), next, arg_info);

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlstridevar(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlstridevar (node *arg_node, info *arg_info)
{
    node *result, *next = NULL;
    node *bound1, *bound2, *step;

    DBUG_ENTER ("WLSDwlstridevar");

    if (WLSTRIDEVAR_NEXT (arg_node) != NULL) {
        next = TRAVdo (WLSTRIDEVAR_NEXT (arg_node), arg_info);
    }

    bound1 = DUPdoDupNode (WLSTRIDEVAR_BOUND1 (arg_node));
    bound2 = DUPdoDupNode (WLSTRIDEVAR_BOUND2 (arg_node));
    step = DUPdoDupNode (WLSTRIDEVAR_STEP (arg_node));

    result
      = ProcessStride (WLSTRIDEVAR_LEVEL (arg_node), WLSTRIDEVAR_DIM (arg_node), bound1,
                       bound2, step, WLSTRIDEVAR_CONTENTS (arg_node), next, arg_info);

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
    node *bound1, *bound2;

    DBUG_ENTER ("WLSDwlgrid");

    result = TRAVopt (WLGRID_NEXT (arg_node), arg_info);

    if (!WLGRID_ISNOOP (arg_node)) {
        bound1 = TBmakeNum (WLGRID_BOUND1 (arg_node));
        bound2 = TBmakeNum (WLGRID_BOUND2 (arg_node));

        result = ProcessGrid (WLGRID_LEVEL (arg_node), WLGRID_DIM (arg_node), bound1,
                              bound2, WLGRID_NEXTDIM (arg_node), WLGRID_CODE (arg_node),
                              result, arg_info);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSDwlgridvar(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLSDwlgridvar (node *arg_node, info *arg_info)
{
    node *result;
    node *bound1, *bound2;

    DBUG_ENTER ("WLSDwlgridvar");

    result = TRAVopt (WLGRIDVAR_NEXT (arg_node), arg_info);

    if (!WLGRIDVAR_ISNOOP (arg_node)) {
        bound1 = DUPdoDupNode (WLGRIDVAR_BOUND1 (arg_node));
        bound2 = DUPdoDupNode (WLGRIDVAR_BOUND2 (arg_node));

        result = ProcessGrid (WLGRIDVAR_LEVEL (arg_node), WLGRIDVAR_DIM (arg_node),
                              bound1, bound2, WLGRIDVAR_NEXTDIM (arg_node),
                              WLGRIDVAR_CODE (arg_node), result, arg_info);
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With-loop Split Dimensions -->
 *****************************************************************************/
