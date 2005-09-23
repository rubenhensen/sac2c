/*
 *
 * $Log$
 * Revision 1.4  2005/09/23 14:03:22  sah
 * extended index_eliminate to drag index offsets across
 * lacfun boundaries.
 *
 * Revision 1.3  2005/09/15 17:13:56  ktr
 * removed IVE renaming function which was obsolete due to explicit
 * offset variables
 *
 * Revision 1.2  2005/09/15 12:46:05  sah
 * args and ids are now properly traversed
 *
 * Revision 1.1  2005/09/12 16:19:19  sah
 * Initial revision
 *
 */

#include "index_eliminate.h"

#include <string.h>
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "shape.h"
#include "new_types.h"
#include "dbug.h"
#include "traverse.h"
#include "DupTree.h"
#include "type_utils.h"
#include "free.h"

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
 * @defgroup ive IVE
 * @ingroup opt
 *
 * @brief The "index vector elimination" (IVE for short)
 *	does eliminate index vectors, at times.
 *	Specifically, it adds code that converts index
 *	vectors to array offsets. If all references to an
 *	index vector are sel or modarray operations, then
 *	dead code elimination in a later phase will, in fact,
 *	eliminate the index vector. If the index vector is
 *	referenced by some other operation, it will not be
 *	eliminated, but performance may still be improved,
 *	due to improved sel/modarray operation.
 *
 *      As of 2005-09-12, this works only for AKS arrays.
 *
 * <pre>
 * Example:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            iv = [2,3];
 *            z = a[iv];
 *
 * is transformed into:
 *
 *            a = reshape([4,4], [1,2,...,16]);
 *            iv = [2,3];
 *            __iv_4_4 = 11;
 *            z = idx_sel(a, __iv_4_4);
 * The iv =... will be deleted by DCR, if possible.
 * </pre>
 *
 * @{
 */

/**
 *
 * @file IndexVectorElimination.c
 *
 *  This file contains the implementation of IVE (index vector elimination).
 *
 *
 * <pre>
 * 1. Purported benefits of this optimization:
 *  a. Elimination of the need to allocate and initialize
 *     the index vector itself, if all references to the
 *     index vector are index operations.
 *
 *  b. Replacement of the one or more computations of the array
 *     offset [the inner product of index vector and array bounds]
 *     at each index site by a one-time computation of that
 *     offset, using vect2offset.
 *
 *  c. If the array shape and index vector values are
 *     statically known, the offset will be computed as a scalar
 *     by later optimization phases.
 *
 * 2. Potholes to watch for:
 *  a. With-loops do not have an index vector per se,
 *     so they are treated differently from normal index vectors.
 *  b. With-loops can have multiple bodies for different array
 *     sections; each body requires its own vect2offset calls.
 *
 * 3. Implementation strategy:
 *	Traverse all arguments and each vardec in each block.
 *
 *      For each of these, find its avis node, and generate
 *      vect2offset calls and assigns to scalar offset variables
 *	for each indexed array type defined in the avis.
 *	These calls are later appended to the index vector
 *	assigns.
 *
 *	Replace each sel or modarray reference to an index vector
 *	by a reference using the offset.
 *
 *	E.g:
 *		iv = [2,3]
 *              ...
 *              x = sel(iv,B);
 *	is changed to:
 *		iv = [2,3]
 *		iv_B = vect2offset(iv,shape(B));
 *              ...
 *		x = _idx_sel(iv,B,B);
 *
 * </pre>
 */

/**
 * INFO structure
 */
struct INFO {
    node *postassigns;
    node *vardecs;
    node *lhs;
    node *withid;
    node *fundef;
    node *lastap;
};

/**
 * INFO macros
 */
#define INFO_POSTASSIGNS(n) ((n)->postassigns)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_WITHID(n) ((n)->withid)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LASTAP(n) ((n)->lastap)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_POSTASSIGNS (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_WITHID (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_LASTAP (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 * counts the number of index vectors removed during optimization.
 */
static int ive_expr;

/**
 * helper functions
 */

/** <!-- ****************************************************************** -->
 * @brief Search iv's avis for array entry of "type"
 *
 * @param avis N_avis node of the selection index
 * @param type type of array potentially being indexed by the avis entry
 *
 * @return avis entry that matches type, or NULL, if no match is found
 ******************************************************************************/
static node *
GetAvis4Shape (node *avis, ntype *type)
{
    node *result = NULL;
    node *types;
    node *ids;

    DBUG_ENTER ("GetAvis4Shape");

    if (TUshapeKnown (type)) {
        types = AVIS_IDXTYPES (avis);
        ids = AVIS_IDXIDS (avis);

        while (types != NULL) {
            DBUG_ASSERT ((ids != NULL),
                         "number of IDXTYPES does not match number of IDXIDS!");

            if (SHcompareShapes (TYgetShape (type),
                                 TYgetShape (TYPE_TYPE (EXPRS_EXPR (types))))) {
                result = IDS_AVIS (ids);
                break;
            }

            types = EXPRS_NEXT (types);
            ids = IDS_NEXT (ids);
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

static node *
Type2IdxAssign (ntype *type, node *avis, info *info)
{
    node *result;
    node *assign;
    node *offset;

    DBUG_ENTER ("Type2IdxAssign");

    DBUG_ASSERT ((TYisAKS (type)), "Type2IdxAssign called with non-AKS type");

    /*
     * iv = id =>
     *
     * _vect2offset_( [ <shp> ], iv)
     */
    offset
      = TCmakePrf2 (F_vect2offset, SHshape2Array (TYgetShape (type)), TBmakeId (avis));

    result = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)),
                         TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

    INFO_VARDECS (info) = TBmakeVardec (result, INFO_VARDECS (info));

    assign = TBmakeAssign (TBmakeLet (TBmakeIds (result, NULL), offset), NULL);

    INFO_POSTASSIGNS (info) = TCappendAssign (assign, INFO_POSTASSIGNS (info));

    AVIS_SSAASSIGN (result) = assign;

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Generates vect2offset assignments for the given
 *        index variable and shapes.
 *
 * @param types N_exprs chain of N_type nodes containing all shapes
 *              an offset has to be built for.
 * @param avis N_avis node of the selection index
 * @param info info structure
 *
 * @return N_ids chain containing all created offset ids
 ******************************************************************************/
static node *
IdxTypes2IdxIds (node *types, node *avis, info *info)
{
    node *result = NULL;
    node *idxavis;

    DBUG_ENTER ("IdxTypes2IdxIds");

    if (types != NULL) {
        result = IdxTypes2IdxIds (EXPRS_NEXT (types), avis, info);

        idxavis = Type2IdxAssign (TYPE_TYPE (EXPRS_EXPR (types)), avis, info);

        result = TBmakeIds (idxavis, result);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief Generates new arguments for the given
 *        index variable and shapes.
 *
 * @param types N_exprs chain of N_type nodes containing all shapes
 *              an offset has to be built for.
 * @param avis N_avis node of the selection index
 * @param args stores the newly generated args chain
 * @param info info structure
 *
 * @return N_ids chain containing all created new offset args
 ******************************************************************************/
static node *
IdxTypes2IdxArgs (node *types, node *avis, node **args, info *info)
{
    node *result = NULL;
    node *idxavis;

    DBUG_ENTER ("IdxTypes2IdxArgs");

    if (types != NULL) {
        result = IdxTypes2IdxArgs (EXPRS_NEXT (types), avis, args, info);

        idxavis = TBmakeAvis (ILIBtmpVarName (AVIS_NAME (avis)),
                              TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));

        *args = TBmakeArg (idxavis, *args);
        result = TBmakeIds (idxavis, result);
    }

    DBUG_RETURN (result);
}

static
  /** <!-- ****************************************************************** -->
   * @brief Generates new concrete arguments for the given idxtypes and avis.
   *        The avis must contain matching idxids for all given types!
   *
   * @param types N_exprs chain of type nodes
   * @param avis avis node with idxids
   * @param args N_exprs chain of concrete args
   *
   * @return extended N_exprs arg chain
   ******************************************************************************/
  node *
  IdxTypes2ApArgs (node *types, node *avis, node *args)
{
    node *idxavis;

    DBUG_ENTER ("IdxTypes2ApArgs");

    if (types != NULL) {
        args = IdxTypes2ApArgs (EXPRS_NEXT (types), avis, args);

        idxavis = GetAvis4Shape (avis, TYPE_TYPE (EXPRS_EXPR (types)));

        DBUG_ASSERT ((idxavis != NULL), "no matching index for given arg found!");

        args = TBmakeExprs (TBmakeId (idxavis), args);
    }

    DBUG_RETURN (args);
}

node *
ExtendConcreteArgs (node *concargs, node *formargs)
{
    DBUG_ENTER ("ExtendConcreteArgs");

    if (concargs != NULL) {
        EXPRS_NEXT (concargs)
          = IdxTypes2ApArgs (AVIS_IDXTYPES (ARG_AVIS (formargs)),
                             ID_AVIS (EXPRS_EXPR (concargs)), EXPRS_NEXT (concargs));

        EXPRS_NEXT (concargs)
          = ExtendConcreteArgs (EXPRS_NEXT (concargs), ARG_NEXT (formargs));
    }

    DBUG_RETURN (concargs);
}

/** <!-- ****************************************************************** -->
 * @brief  Attempt to replace y = sel(iv,B) by  y = idx_sel(iv_B,B)
 *
 * @param prf prf entry for a sel(iv,B) operation
 * @param lhs - unused in this call
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceSel (node *prf, node *lhs)
{
    node *result;
    node *avis;

    DBUG_ENTER ("CheckAndReplaceSel");

    avis = GetAvis4Shape (ID_AVIS (PRF_ARG1 (prf)), ID_NTYPE (PRF_ARG2 (prf)));

    if (avis != NULL) {
        result = TCmakePrf2 (F_idx_sel, TBmakeId (avis), PRF_ARG2 (prf));

        PRF_ARG2 (prf) = NULL;
        prf = FREEdoFreeNode (prf);

        ive_expr++;
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @brief  Replace y = modarray(iv,B,val) with y = idx_modarray(iv_B,B,val)
 *
 * @param prf prf entry for a sel(iv,B) operation
 * @param lhs node of "y" in above. Required only because
 *            of _idx_modarray ICM restriction noted below.
 *
 * @return updated prf node
 ******************************************************************************/
static node *
CheckAndReplaceModarray (node *prf, node *lhs)
{
    node *result;
    node *avis;

    DBUG_ENTER ("CheckAndReplaceModarray");

    avis = GetAvis4Shape (ID_AVIS (PRF_ARG2 (prf)), ID_NTYPE (PRF_ARG1 (prf)));

    if ((avis != NULL) && (TUshapeKnown (IDS_NTYPE (lhs)))
        && (TUshapeKnown (ID_NTYPE (PRF_ARG1 (prf))))
        && (TUshapeKnown (ID_NTYPE (PRF_ARG2 (prf))))
        && ((NODE_TYPE (PRF_ARG3 (prf)) != N_id)
            || TUshapeKnown (ID_NTYPE (PRF_ARG3 (prf))))) {
        /*
         * TODO: sah
         * _idx_modarray_ is limited to args and return
         * values of at least AKS type, so we have to check
         * that here. A better idea would be to modify the
         * _idx_modarray_ prf to work on AKD as well...
         */
        result
          = TCmakePrf3 (F_idx_modarray, PRF_ARG1 (prf), TBmakeId (avis), PRF_ARG3 (prf));

        PRF_ARG1 (prf) = NULL;
        PRF_ARG3 (prf) = NULL;
        prf = FREEdoFreeNode (prf);

        ive_expr++;
    } else {
        result = prf;
    }

    DBUG_RETURN (result);
}

/**
 * traversal functions
 */

/** <!-- ****************************************************************** -->
 * @brief Starts index elimination in a given fundef node. On top-level,
 *        only non-lac functions are processed, as lac-funs are handeled
 *        when reaching their application.
 *
 * @param arg_node N_fundef node
 * @param arg_info info structure
 *
 * @return unchanged N_fundef node
 ******************************************************************************/
node *
IVEfundef (node *arg_node, info *arg_info)
{
    node *lastfun;
    node *oldvardecs;

    DBUG_ENTER ("IVEfundef");

    if ((INFO_FUNDEF (arg_info) != NULL) || (!FUNDEF_ISLACFUN (arg_node))) {
        DBUG_PRINT ("IVE", ("processing fundef %s", CTIitemName (arg_node)));

        /*
         * as we enter a new context here, we have to stack
         * the current fundef and the vardecs we have created
         * so far. This is to ensure that the vardecs are
         * appended to the correct fundef!
         */
        lastfun = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;
        oldvardecs = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;

        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        if (INFO_VARDECS (arg_info) != NULL) {
            FUNDEF_VARDEC (arg_node)
              = TCappendVardec (FUNDEF_VARDEC (arg_node), INFO_VARDECS (arg_info));
            INFO_VARDECS (arg_info) = NULL;
        }

        /*
         * unstack fundef and vardecs
         */

        INFO_FUNDEF (arg_info) = lastfun;
        INFO_VARDECS (arg_info) = oldvardecs;
    }

    if (INFO_FUNDEF (arg_info) == NULL) {
        /*
         * we are on top level, so continue with next fundef
         */
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
IVEarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEarg");

    if (INFO_LASTAP (arg_info) == NULL) {
        /*
         * top-level function
         */
        AVIS_IDXIDS (ARG_AVIS (arg_node))
          = IdxTypes2IdxIds (AVIS_IDXTYPES (ARG_AVIS (arg_node)), ARG_AVIS (arg_node),
                             arg_info);
    } else {
        /*
         * lac-function:
         *
         * here, we can try to reuse the offsets that have been
         * generated for the concrete args. to do so, we have to
         * extend the function signature.
         */
        AVIS_IDXIDS (ARG_AVIS (arg_node))
          = IdxTypes2IdxArgs (AVIS_IDXTYPES (ARG_AVIS (arg_node)), ARG_AVIS (arg_node),
                              &ARG_NEXT (arg_node), arg_info);
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Traverses into the assignments within the block to trigger
 *        the generation of idx2offset/vect2offset assignments. Prior to
 *        doing so, any leftover POSTASSIGNS are inserted at the beginning
 *        of the block. This is necessary, as the assignments generated
 *        for a WITHID ids need to be inserted at the beginning of the
 *        block of the corresponding CODE.
 *
 * @param arg_node N_block node
 * @param arg_info info structure
 *
 * @return unchanged N_block node
 ******************************************************************************/
node *
IVEblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEblock");

    if (INFO_POSTASSIGNS (arg_info) != NULL) {
        if (NODE_TYPE (BLOCK_INSTR (arg_node)) == N_empty) {
            BLOCK_INSTR (arg_node) = FREEdoFreeNode (BLOCK_INSTR (arg_node));
            BLOCK_INSTR (arg_node) = INFO_POSTASSIGNS (arg_info);
            INFO_POSTASSIGNS (arg_info) = NULL;
        } else {
            BLOCK_INSTR (arg_node)
              = TCappendAssign (INFO_POSTASSIGNS (arg_info), BLOCK_INSTR (arg_node));
            INFO_POSTASSIGNS (arg_info) = NULL;
        }
    }

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Generates idx2offset/vect2offset assignments and
 *        annotates the corresponding ids at the avis node.
 *
 * @param arg_node N_ids node
 * @param arg_info info structure
 *
 * @return unchanged N_ids node
 ******************************************************************************/
node *
IVEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEids");

    AVIS_IDXIDS (IDS_AVIS (arg_node))
      = IdxTypes2IdxIds (AVIS_IDXTYPES (IDS_AVIS (arg_node)), IDS_AVIS (arg_node),
                         arg_info);

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Replaces F_sel/F_modarray by F_idx_sel/F_idx_modarray whereever
 *        possible.
 *
 * @param arg_node N_prf node
 * @param arg_info info structure
 *
 * @return possibly replaced N_prf node
 ******************************************************************************/
node *
IVEprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEprf");

    switch (PRF_PRF (arg_node)) {
    case F_sel:
        arg_node = CheckAndReplaceSel (arg_node, INFO_LHS (arg_info));
        break;
    case F_modarray:
        arg_node = CheckAndReplaceModarray (arg_node, INFO_LHS (arg_info));
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Directs the traversal into the assignment instruction and inserts
 *        the INFO_POSTASSIGNS chain after the current node prior to
 *        traversing further down.
 *
 * @param arg_node N_assign node
 * @param arg_info info structure
 *
 * @return unchanged N_assign node
 ******************************************************************************/
node *
IVEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_POSTASSIGNS (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_POSTASSIGNS (arg_info), ASSIGN_NEXT (arg_node));
        INFO_POSTASSIGNS (arg_info) = NULL;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Stores the LET_IDS in INFO_LHS for further usage.
 *
 * @param arg_node N_let node
 * @param arg_info info structure
 *
 * @return unchanged N_let node
 ******************************************************************************/
node *
IVElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVElet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEwith (node *arg_node, info *arg_info)
{
    node *oldwithid;

    DBUG_ENTER ("IVEwith");

    oldwithid = INFO_WITHID (arg_info);
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    INFO_WITHID (arg_info) = oldwithid;

    DBUG_RETURN (arg_node);
}

node *
IVEcode (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ("IVEcode");

    avis = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));

    AVIS_IDXIDS (avis) = IdxTypes2IdxIds (AVIS_IDXTYPES (avis), avis, arg_info);

    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    if (AVIS_IDXIDS (avis) != NULL) {
        AVIS_IDXIDS (avis) = FREEdoFreeTree (AVIS_IDXIDS (avis));
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
IVEap (node *arg_node, info *arg_info)
{
    node *oldap;

    DBUG_ENTER ("IVEap");

    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        if (AP_FUNDEF (arg_node) != INFO_FUNDEF (arg_info)) {
            /* external application */
            oldap = INFO_LASTAP (arg_info);
            INFO_LASTAP (arg_info) = arg_node;

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            INFO_LASTAP (arg_info) = oldap;
        }

        /*
         * extend the arguments of the given application
         */

        AP_ARGS (arg_node)
          = ExtendConcreteArgs (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief function triggering the index vector elimination on the
 *        syntax tree. the traversal relies on the information
 *        infered by index vector elimination inference.
 *
 * @param syntax_tree N_module node
 *
 * @return transformed syntax tree
 ******************************************************************************/
node *
IVEdoIndexVectorElimination (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IndexVectorElimination");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "IVE is intended to run on the entire tree");

    info = MakeInfo ();

    TRAVpush (TR_ive);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/*@}*/ /* defgroup ive */
