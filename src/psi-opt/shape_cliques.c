/*
 *
 * $Id$
 *
 */

#include "tree_basic.h"
#include "dbug.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape_cliques.h"
#include "DataFlowMask.h"
#include "ctinfo.h"

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/**
 *
 * @defgroup sci SCI
 * @ingroup opt
 *
 * @{
 */

/**
 *
 * @file shape_cliques.c
 *
 *  This file contains the implementation of the inference of shape cliques
 *  for IVE (index vector elimination) and potentially other optimizations.
 *
 */
/**
 *
 * @name Some utility functions:
 *
 * @{
 */

/*
 *  * INFO structure
 *   */
struct INFO {
    node *lhs;
    node *fundef;
    dfmask_t **cliquemasks;
};

/*
 *  * INFO macros
 *   */
#define INFO_LHS(n) ((n)->lhs)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_CLIQUEMASKS(n) ((n)->cliquemasks)

/*
 *  * INFO functions
 *   */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*@}*/

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for SCI:
 *
 * @{
 ****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *SCIid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SCIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCIid");

    AVIS_NEEDCOUNT (ID_AVIS (arg_node))++;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCIprf( node *arg_node, info *arg_info )
 *
 *****************************************************************************/

node *
SCIprf (node *arg_node, info *arg_info)
{
    int junk;
    node *lhs, *arg1, *arg2, *lhsavis, *arg1avis, *arg2avis;

    DBUG_ENTER ("SCIprf");

    switch (PRF_PRF (arg_node)) {
    case F_add_SxA: /* Scalar-left dyadic scalar functions */
    case F_sub_SxA:
    case F_mul_SxA:
    case F_div_SxA:
        /* Place lhs and arg2 in same shape clique */
        lhs = INFO_LHS (arg_info);
        arg2 = PRF_ARG2 (arg_node);
        lhsavis = IDS_AVIS (lhs);
        arg2avis = ID_AVIS (arg2);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE),
                     "PRF SxA shape clique lhs not NONE");
        SCIAppendAvisToShapeClique (lhsavis, arg2avis, arg_info);
        break;

    case F_add_AxS: /* Scalar-right dyadic scalar functions */
    case F_sub_AxS:
    case F_mul_AxS:
    case F_div_AxS:
    case F_modarray: /* the modarray primitive function */
    case F_mod:
    case F_max:
    case F_min:
    case F_and:
    case F_or:
    case F_neg: /* Monadic scalar functions */
    case F_abs:
    case F_not:
    case F_toi_A: /* Monadic type coercion functions */
    case F_tof_A:
    case F_tod_A:
        /* Place lhs id and arg1 in same shape clique */
        lhs = INFO_LHS (arg_info);
        arg1 = PRF_ARG1 (arg_node);
        lhsavis = IDS_AVIS (lhs);
        arg1avis = ID_AVIS (arg1);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE),
                     "PRF AxSshape clique lhs not NONE");
        SCIAppendAvisToShapeClique (lhsavis, arg1avis, arg_info);
        break;
    case F_add_AxA: /* Dyadic scalar functions */
    case F_sub_AxA:
    case F_mul_AxA:
    case F_div_AxA:
        junk = 4;
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *  *
 *   * @fn node *SCImodarray(node *arg_node, info *arg_info)
 *    *
 *     * @brief  FIXME
 *      *
 *       * @param arg_node
 *        * @param arg_info
 *         *
 *          * @return
 *           *
 *            *****************************************************************************/
node *
SCImodarray (node *arg_node, info *arg_info)
{
    node *lhs, *rhs, *lhsavis, *rhsavis;

    DBUG_ENTER ("SCImodarray");

    /* For a WL of the form:
     *           *    C = with() modarray( B, iv, val),
     *                * place C in same shape clique as B.
     */

    rhs = MODARRAY_ARRAY (arg_node);
    if (N_id == NODE_TYPE (rhs)) {
        DBUG_PRINT ("SCI", ("found modarray N_id"));
        /* Place C in same shape clique as B */
        lhs = INFO_LHS (arg_info);
        lhsavis = IDS_AVIS (lhs);
        rhsavis = ID_AVIS (rhs);
        DBUG_ASSERT ((AVIS_SHAPECLIQUEID (lhsavis) == SHAPECLIQUEIDNONE),
                     "modarray shape clique lhs not NONE");
        SCIAppendAvisToShapeClique (lhsavis, rhsavis, arg_info);
    } else {
        DBUG_PRINT ("SCI", ("found modarray non- N_id"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *  *
 *   * @fn node *SCIgenarray(node *arg_node, info *arg_info)
 *    *
 *     * @brief  node *SCIgenarray( node *arg_node, info *arg_info)
 *     * With-loop genarray shape clique inference
 *      *
 *       * @param arg_node
 *        * @param arg_info
 *         *
 *          * @return
 *           *
 *            *****************************************************************************/
node *
SCIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCIgenarray");
    /* For a WL of the form:
     *    S = shape(B);
     *    C = with() genarray( S, def),
     * place C in same shape clique as B, IFF def is a scalar.
     * It may be possible to get fancier with non-scalar def, but
     * you gotta start somewhere.
     */

    node *lhs, *shp;
    int junk;
    shp = GENARRAY_SHAPE (arg_node);
    if (N_id == NODE_TYPE (shp)) { /* S may be an N_id or an N_array */
        junk = 4;
        DBUG_PRINT ("SCI", ("found genarray shape N_id"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *SCIdoShapeCliqueInference( node *syntax_tree)
 *
 *   @brief this function infers shape clique membership for all arrays.
 *   @param part of the AST (usually the entire tree) i is to be applied on.
 *   @return modified AST.
 *
 *****************************************************************************/
node *
SCIdoShapeCliqueInference (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SCIdoShapeCliqueInference");

    DBUG_PRINT ("OPT", ("Starting shape clique inference..."));

    TRAVpush (TR_sci);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_PRINT ("OPT", ("shape clique inference complete!"));

    DBUG_RETURN (syntax_tree);
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn node *SCIlet( node *arg_node, info *arg_info)
 *   @brief This function handles assignments of values to names.
 *          The lhs and rhs are placed into the same shape clique via
 *          a set join operation.
 *    *
 *     ****************************************************************************/
node *
SCIlet (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SCIlet");
    /* Place lhs in same clique as rhs. If rhs is not in a clique already,
     * make a clique from lhs and rhs.
     * We tuck the lhs into the arg_info and pass it down to whoever
     * finds the rhs avis; they will do the dirty work. */

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn node *SCIfundef( node *arg_node, info *arg_info)
 *    *
 *     *****************************************************************************/
node *
SCIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCIfundef");

    /* Do nothing if wrapper function, external function, do-fun or cond-fun,
     * or if fn does not have a body */
    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISWRAPPERFUN (arg_node))
        && (!FUNDEF_ISCONDFUN (arg_node)) && (!FUNDEF_ISDOFUN (arg_node))) {

        SCIInitializeShapeCliques (arg_node, arg_info);
        SCIShapeCliquePrintIDs (arg_node, arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        SCIDestroyShapeCliques (arg_node, arg_info);
        SCIShapeCliquePrintIDs (arg_node, arg_info);
    }

    if (NULL != FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn int SCIAppendAvisToShapeClique( node *avis1, node *avis2, info *arg_info)
 *   @brief This function appends avis1 to the shape clique of avis2.
 *          If avis2 is not in a shape clique, a new shape clique will be built,
 *          containing avis1 and avis2.
 *          The result is the shape clique id (it happens to be an address,
 *          but it's basically any unique integer).
 *    *
 *     ****************************************************************************/
int
SCIAppendAvisToShapeClique (node *avis1, node *avis2, info *arg_info)
{
    dfmask_t *CliquePtr;

    CliquePtr = (dfmask_t *)SCICreateShapeClique (avis2, arg_info);
    /* join avis1 to avis2 shape clique */
    DFMsetMaskEntrySet (CliquePtr, NULL, avis1);
    DBUG_PRINT ("SCI", ("Adding entry to shape clique #%d", AVIS_SHAPECLIQUEID (avis2)));
    AVIS_SHAPECLIQUEID (avis1) = AVIS_SHAPECLIQUEID (avis2);

    return (AVIS_SHAPECLIQUEID (avis1));
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn dfmask_t *SCICreateShapeClique( node *avis1, info *arg_info)
 *   @brief This function creates a shape clique containing avis1.
 *          If avis1 is already in a shape clique, it does nothing.
 *          The result is the shape clique id (it happens to be an address,
 *          but it's basically any unique integer).
 *    *
 *     ****************************************************************************/
dfmask_t *
SCICreateShapeClique (node *avis1, info *arg_info)
{
    node *fundef;
    dfmask_base_t *BasePtr;
    dfmask_t *CliquePtr, **CliquePtrs;
    int numids, i;

    if (AVIS_SHAPECLIQUEID (avis1) == SHAPECLIQUEIDNONE) {
        /* build new shape clique for avis1 */
        fundef = INFO_FUNDEF (arg_info);
        BasePtr = FUNDEF_DFM_BASE (fundef);
        CliquePtr = DFMgenMaskClear (BasePtr);
        DFMsetMaskEntrySet (CliquePtr, NULL, avis1);
        numids = DFMnumIds (BasePtr);
        CliquePtrs = INFO_CLIQUEMASKS (arg_info);
        /* Append new clique pointer to the list of shape cliques */
        for (i = 0; i < numids; i++) {
            if (CliquePtrs[i] == NULL) {
                CliquePtrs[i] = CliquePtr;
                DBUG_PRINT ("SCI", ("Created new shape clique #%d", i));
                AVIS_SHAPECLIQUEID (avis1) = i;
                i = numids;
            }
        }
    }
    DBUG_PRINT ("SCI",
                ("Adding new element to shape clique #%d", AVIS_SHAPECLIQUEID (avis1)));
    CliquePtr = SCIShapeCliqueIDToCliquePtr (AVIS_SHAPECLIQUEID (avis1), arg_info);

    return (CliquePtr);
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn void *SCIInitializeShapeCliques( node *avis1, info *arg_info)
 *   @brief This function creates the dataflow mask data base for
 *          shape clique inference use.
 *    *
 *     ****************************************************************************/
void
SCIInitializeShapeCliques (node *arg_node, info *arg_info)
{
    dfmask_base_t *BasePtr;
    dfmask_t **CliquePtrs;
    int numids;

    INFO_FUNDEF (arg_info) = arg_node;
    DBUG_PRINT ("SCI",
                ("Initializing shape cliques for function: %s", CTIitemName (arg_node)));
    /* Shape clique sets are represented as dataflow masks */
    BasePtr = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
    FUNDEF_DFM_BASE (arg_node) = BasePtr;
    /* Allocate and clear an array of pointers to the maximum number of
     * sets we may require.
     */
    numids = DFMnumIds (BasePtr);
    DBUG_PRINT ("SCI", ("Inferring %d identifiers in function %s", numids,
                        CTIitemName (arg_node)));
    CliquePtrs = (dfmask_t **)ILIBmalloc (numids * sizeof (dfmask_t *));
    INFO_CLIQUEMASKS (arg_info) = CliquePtrs;
    {
        int i;
        for (i = 0; i < numids; i++)
            CliquePtrs[i] = NULL;
    }
    return;
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn void *SCIDestroyShapeCliques( node *avis1, info *arg_info)
 *   @brief This function destroys the dataflow mask data base used by
 *          shape clique inference use.
 *    *
 *     ****************************************************************************/
void
SCIDestroyShapeCliques (node *arg_node, info *arg_info)
{
    node *fundef;
    dfmask_base_t *BasePtr;
    dfmask_t **CliquePtrs;
    dfmask_t *CliquePtr;
    int numids;
    int i;

    /*
     *      remove the dataflowmasks - they don't survive well over opt phases.
     *      Also remove the dataflow mask base.
     */
    fundef = INFO_FUNDEF (arg_info);
    BasePtr = FUNDEF_DFM_BASE (fundef);
    DBUG_PRINT ("SCI",
                ("Destroying shape cliques for function: %s", CTIitemName (arg_node)));
    numids = DFMnumIds (BasePtr);
    CliquePtrs = INFO_CLIQUEMASKS (arg_info);
    for (i = 0; i < numids; i++) { /* Print and remove each clique */
        CliquePtr = CliquePtrs[i];
        if (NULL != CliquePtr) {
            DBUG_PRINT ("SCI", ("Shape clique %d:", i));
            DBUG_EXECUTE ("SCI", DFMprintMask (stderr, "%s ", CliquePtr);
                          fprintf (stderr, "\n"););
            DFMremoveMask (CliquePtr);
        }
    }
    if (INFO_CLIQUEMASKS (arg_info) != NULL)
        ILIBfree (INFO_CLIQUEMASKS (arg_info));
    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }
    return;
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn dfmask_t *SCIShapeCliqueIDToCliquePtrs( int ShapeCliqueID, info *arg_info)
 *   @brief This function computes a pointer to a shape clique's dataflow mask
 *          entry from a shape clique ID.
 *    *
 *     ****************************************************************************/
dfmask_t *
SCIShapeCliqueIDToCliquePtr (int ShapeCliqueID, info *arg_info)
{
    dfmask_t **CliquePtrs;
    dfmask_t *CliquePtr;

    CliquePtrs = INFO_CLIQUEMASKS (arg_info);
    CliquePtr = CliquePtrs[ShapeCliqueID];
    return (CliquePtr);
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn void SCIShapeCliquePrintIDs(  node *arg_node, info *arg_info)
 *   @brief This function is a debugging function, which is intended to print
 *          all names and ShapeCliqueIDs for this function.
 *    *
 *     ****************************************************************************/
void
SCIShapeCliquePrintIDs (node *arg_node, info *arg_info)
{
    node *arg, *argavis;

    /* print all arguments */
    arg = FUNDEF_ARGS (arg_node);
    while (arg != NULL) {
        argavis = ARG_AVIS (arg);
        DBUG_PRINT ("SCI", ("ShapeCliqueID %d for Argument %s",
                            AVIS_SHAPECLIQUEID (argavis), AVIS_NAME (argavis)));
        arg = ARG_NEXT (arg);
    }

    /* print all locals */
    arg = FUNDEF_BODY (arg_node); /* maybe N_block */
    if (NULL != arg) {
        arg = BLOCK_VARDEC (arg);
        while (arg != NULL) {
            argavis = VARDEC_AVIS (arg);
            DBUG_PRINT ("SCI", ("ShapeCliqueID %d for Local %s",
                                AVIS_SHAPECLIQUEID (argavis), AVIS_NAME (argavis)));
            arg = VARDEC_NEXT (arg);
        }
    }
    return;
}

/** <!--*******************************************************************-->
 *  *
 *   * @fn bool SCIAreInSameShapeClique( node *avis1, node *avis2)
 *   @brief Predicate for determining if two avis nodes are in same
 *          shape clique. The main reason for having a function to do this
 *          today is that I am contemplating a very different representation
 *          for shape clique membership.
 *    *
 *     ****************************************************************************/
bool
SCIAreInSameShapeClique (node *avis1, node *avis2)
{
    return (AVIS_SHAPECLIQUEID (avis1) == AVIS_SHAPECLIQUEID (avis2));
}

/*@}*/
/*@}*/ /* defgroup SCI */
