/*
 *
 * $Log$
 * Revision 1.5  2005/07/21 12:03:21  ktr
 * removed AVIS_WITHID
 *
 * Revision 1.4  2005/07/03 17:18:17  ktr
 * Fixed a pointer type problem
 *
 * Revision 1.3  2005/06/06 14:01:07  ktr
 * Withloopification now builds copy with-loops for functions arguments as well
 *
 * Revision 1.2  2004/11/24 18:51:58  ktr
 * COMPILES!
 *
 * Revision 1.1  2004/10/07 12:36:03  ktr
 * Initial revision
 *
 */

/**
 * @defgroup wlsw WLSWithloopification
 * @ingroup wls
 *
 * @brief modifies a with-loop's codes in order to create the pattern of
 *        perfectly nested with-loops needed by WLSBuild.
 *
 * <pre>
 *
 * Withloopification of a with-loop's codes works in three steps:
 *
 * 1) Traverse inner code to check whether a copy with-loop is required
 *
 *    {
 *      ass;
 *      CEXPR = expr( ass, iv);
 *    } : CEXPR;
 *
 *    No copy with-loop is required if and only if expr(ass, iv) denotes
 *    a genarray/modarray with-loop whose generators/withop do not depend
 *    on the outer index vector iv.
 *
 * 2) Insert a copy with-loop
 *
 *    {
 *      ass;
 *      CEXPR = expr( ass, iv);
 *      copy  = with ( . <= jv <= . ) {
 *                res = CEXPR[jv];
 *              } : res;
 *              genarray( shape( CEXPR) )
 *    } : copy
 *
 * 3) Move code from before the inner with-loop inside the inner with-loop
 *
 *    {
 *      copy  = with ( . <= jv <= . ) {
 *                ass;
 *                CEXPR = expr( ass, iv);
 *                res = CEXPR[jv];
 *              } : res;
 *              genarray( shape( CEXPR) )
 *    } : copy
 *
 *    NOTE: This can only happen if -wls_aggressive or maxwls are specified!
 *
 * </pre>
 *
 * @{
 */

/**
 *
 * @file wlswithloopfication.c
 *
 * Implements a traversal to modify a with-loop's codes in order to create the
 * pattern of perfectly nested with-loops needed by WLSBuild.
 *
 */

#include "wls.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "new_types.h"
#include "print.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "internal_lib.h"
#include "shape.h"

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    int innerdims;
    node *outerwithid;
    bool innertrav;
    node *depstack;
    dfmask_t *depmask;
    node *cexpr;
    bool mustcopy;
};

/**
 * INFO macros
 */
#define INFO_WLSW_FUNDEF(n) (n->fundef)
#define INFO_WLSW_INNERDIMS(n) (n->innerdims)
#define INFO_WLSW_OUTERWITHID(n) (n->outerwithid)
#define INFO_WLSW_INNERTRAV(n) (n->innertrav)
#define INFO_WLSW_DEPSTACK(n) (n->depstack)
#define INFO_WLSW_DEPMASK(n) (n->depmask)
#define INFO_WLSW_CEXPR(n) (n->cexpr)
#define INFO_WLSW_MUSTCOPY(n) (n->mustcopy)

/**
 * INFO functions
 */
static info *
MakeInfo (node *fundef, int innerdims)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_WLSW_FUNDEF (result) = fundef;
    INFO_WLSW_INNERDIMS (result) = innerdims;
    INFO_WLSW_OUTERWITHID (result) = NULL;
    INFO_WLSW_INNERTRAV (result) = FALSE;
    INFO_WLSW_DEPSTACK (result) = NULL;
    INFO_WLSW_DEPMASK (result) = NULL;
    INFO_WLSW_CEXPR (result) = NULL;
    INFO_WLSW_MUSTCOPY (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWdoWithloopify( node *arg_node, node *fundef, int innerdims)
 *
 * @brief starting point of WLSWithloopification
 *
 * @param arg_node with-loop to be withloopified
 * @param fundef current fundef node
 * @param innerdims number of inner dimensions to be scalarized
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWdoWithloopify (node *with, node *fundef, int innerdims)
{
    info *info;

    DBUG_ENTER ("WLSWdoWithloopify");

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "First parameter must be a with-loop");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "Second parameter must be a fundef node");

    info = MakeInfo (fundef, innerdims);

    TRAVpush (TR_wlsw);
    with = TRAVdo (with, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (with);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn shape *LowerBound( shape *unrshp, int index)
 *
 * @brief computes the lower bound of a generator in unrshp that only
 *        covers the element with number index
 *
 * @param unrshp
 * @param index
 *
 * @return lower bounds shape
 *
 *****************************************************************************/
static shape *
LowerBound (shape *unrshp, int index)
{
    shape *idx_shape;
    int i;

    DBUG_ENTER ("LowerBound");

    idx_shape = SHcopyShape (unrshp);

    for (i = SHgetDim (unrshp) - 1; i >= 0; i--) {
        SHsetExtent (idx_shape, i, index % SHgetExtent (unrshp, i));
        index = index / SHgetExtent (unrshp, i);
    }

    DBUG_RETURN (idx_shape);
}

/** <!--********************************************************************-->
 *
 * @fn shape *UpperBound( shape *unrshp, int index)
 *
 * @brief computes the upper bound of a generator in unrshp that only
 *        covers the element with number index
 *
 * @param unrshp
 * @param index
 *
 * @return upper bounds shape
 *
 *****************************************************************************/
static shape *
UpperBound (shape *unrshp, int index)
{
    shape *idx_shape;
    int i;

    DBUG_ENTER ("UpperBound");

    idx_shape = SHcopyShape (unrshp);

    for (i = SHgetDim (unrshp) - 1; i >= 0; i--) {
        SHsetExtent (idx_shape, i, (index % SHgetExtent (unrshp, i)) + 1);
        index = index / SHgetExtent (unrshp, i);
    }

    DBUG_RETURN (idx_shape);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSelCodes( node *parts, ids *iv, node *arr, node *fundef)
 *
 * @brief creates a CODE for each part in parts that does nothing but select
 *        arr[ iv ].
 *        The parts cannot simply share a code as the hope is to be able
 *        to do constant folding on these very small parts.
 *
 * @param parts
 * @param iv
 * @param arr
 * @param fundef
 *
 * @return chain of codes
 *
 *****************************************************************************/
static node *
MakeSelCodes (node *part, node *iv, node *arr, node *fundef)
{
    node *code = NULL;
    node *avis = NULL;
    node *vardecs = NULL;
    node *id;
    ntype *new_type;
    int dim;

    DBUG_ENTER ("MakeSelCodes");

    if (part != NULL) {
        dim = SHgetUnrLen (TYgetShape (AVIS_TYPE (IDS_AVIS (iv))));

        new_type = TYmakeAKS (TYcopyType (TYgetScalar (ID_NTYPE (arr))),
                              SHdropFromShape (dim, TYgetShape (ID_NTYPE (arr))));

        avis = TBmakeAvis (ILIBtmpVar (), new_type);

        vardecs = TBmakeVardec (avis, vardecs);

        fundef = TCaddVardecs (fundef, vardecs);

        id = TBmakeId (avis);

        code = TBmakeCode (TBmakeBlock (TCmakeAssignLet (avis,
                                                         TCmakePrf2 (F_sel,
                                                                     TBmakeId (
                                                                       IDS_AVIS (iv)),
                                                                     DUPdoDupNode (arr))),
                                        NULL),
                           TBmakeExprs (id, NULL));

        PART_CODE (part) = code;
        CODE_USED (code) = 1;

        CODE_NEXT (code) = MakeSelCodes (PART_NEXT (part), iv, arr, fundef);
    }

    DBUG_RETURN (code);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSelParts( shape *maxshp, int unrdim, node *withid)
 *
 * @brief creates a chain of parts whose generators each cover one element
 *        in the range of _0_ to take([unrdim], maxshp).
 *
 * @param maxshp
 * @param unrdim
 * @param withid
 *
 * @return chain of parts
 *
 *****************************************************************************/
static node *
MakeSelParts (shape *maxshp, int unrdim, node *withid)
{
    node *parts = NULL;
    shape *unrshp, *lower_tl, *upper_tl;
    int i;

    DBUG_ENTER ("MakeSelParts");

    unrshp = SHtakeFromShape (unrdim, maxshp);

    upper_tl = SHdropFromShape (unrdim, maxshp);

    lower_tl = SHcopyShape (upper_tl);
    for (i = 0; i < SHgetDim (lower_tl); i++) {
        SHsetExtent (lower_tl, i, 0);
    }

    for (i = 0; i < SHgetUnrLen (unrshp); i++) {
        node *newpart;
        shape *lower, *upper, *lower_hd, *upper_hd;

        lower_hd = LowerBound (unrshp, i);
        upper_hd = UpperBound (unrshp, i);

        lower = SHappendShapes (lower_hd, lower_tl);
        upper = SHappendShapes (upper_hd, upper_tl);

        newpart = TBmakePart (NULL, DUPdoDupNode (withid),
                              TBmakeGenerator (F_le, F_lt, SHshape2Array (lower),
                                               SHshape2Array (upper), NULL, NULL));

        lower_hd = SHfreeShape (lower_hd);
        upper_hd = SHfreeShape (upper_hd);
        lower = SHfreeShape (lower);
        upper = SHfreeShape (upper);

        PART_NEXT (newpart) = parts;
        parts = newpart;
    }

    lower_tl = SHfreeShape (lower_tl);
    upper_tl = SHfreeShape (upper_tl);
    unrshp = SHfreeShape (unrshp);

    DBUG_RETURN (parts);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateCopyWithloop( node *array, int dim, node *fundef)
 *
 * @brief creates a withloop copying the first dimensions from array array.
 *        Typically, this means there is one part containing a selection
 *        array[iv].
 *        However, if array is an N_array, its elements are copied elementwise
 *        by one part per array element.
 *
 * @param array id of the array to be copied
 * @param dim number of dimensions the copy-wl shall cover
 * @param fundef
 *
 * @return
 *
 *****************************************************************************/
static node *
CreateCopyWithloop (node *array, int dim, node *fundef)
{
    node *wl;
    node *avis;
    node *vardecs = NULL;
    node *parts;
    node *codes;
    node *withid;
    node *withop;
    node *vec_ids;
    node *scl_ids = NULL;
    int i;
    int unrdim = 0;
    shape *maxshp;

    DBUG_ENTER ("CreateCopyWithloop");

    avis = TBmakeAvis (ILIBtmpVar (),
                       TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, dim)));

    vardecs = TBmakeVardec (avis, vardecs);
    vec_ids = TBmakeIds (avis, NULL);

    for (i = 0; i < dim; i++) {
        avis = TBmakeAvis (ILIBtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)));

        vardecs = TBmakeVardec (avis, vardecs);
        scl_ids = TBmakeIds (avis, scl_ids);
    }

    if (AVIS_SSAASSIGN (ID_AVIS (array)) != NULL) {
        node *rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array)));

        if (NODE_TYPE (rhs) == N_array) {
            /*
             * array is given by an array, unroll at most the array dimensionality
             */
            int arraydim = SHgetDim (ARRAY_SHAPE (rhs));

            unrdim = (dim < arraydim) ? dim : arraydim;
        }
    }

    maxshp = SHtakeFromShape (dim, TYgetShape (ID_NTYPE (array)));

    withid = TBmakeWithid (vec_ids, scl_ids);

    parts = MakeSelParts (maxshp, unrdim, withid);

    codes = MakeSelCodes (parts, vec_ids, array, fundef);

    withop = TBmakeGenarray (SHshape2Array (
                               SHtakeFromShape (dim, TYgetShape (ID_NTYPE (array)))),
                             NULL);

    wl = TBmakeWith (parts, codes, withop);

    WITH_PARTS (wl) = TCcountParts (parts);

    fundef = TCaddVardecs (fundef, vardecs);

    /*
     * Clean up
     */
    maxshp = SHfreeShape (maxshp);
    withid = FREEdoFreeTree (withid);

    DBUG_RETURN (wl);
}

/******************************************************************************
 *
 * WLS withloopification traversal (wlsw_tab)
 *
 * prefix: WLSW
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLSWcode(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWcode (node *arg_node, info *arg_info)
{
    dfmask_base_t *maskbase;
    node *ids;

    DBUG_ENTER ("WLSWcode");

    if (!INFO_WLSW_INNERTRAV (arg_info)) {
        /*
         * 1. Traverse block to determine whether a copy with-loop is required
         */

        /*
         * Build a maskbase and DEPMASK
         */
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (INFO_WLSW_FUNDEF (arg_info)),
                                   FUNDEF_VARDEC (INFO_WLSW_FUNDEF (arg_info)));
        INFO_WLSW_DEPMASK (arg_info) = DFMgenMaskClear (maskbase);

        /*
         * Mark all variables in OUTERWITHID as the inner generators
         * must not depend on them
         */
        DFMsetMaskEntrySet (INFO_WLSW_DEPMASK (arg_info), NULL,
                            IDS_AVIS (WITHID_VEC (INFO_WLSW_OUTERWITHID (arg_info))));

        ids = WITHID_IDS (INFO_WLSW_OUTERWITHID (arg_info));
        while (ids != NULL) {
            DFMsetMaskEntrySet (INFO_WLSW_DEPMASK (arg_info), NULL, IDS_AVIS (ids));
            ids = IDS_NEXT (ids);
        }

        /*
         * Traverse into CBLOCK
         */
        INFO_WLSW_CEXPR (arg_info) = CODE_CEXPR (arg_node);
        INFO_WLSW_MUSTCOPY (arg_info) = TRUE;
        INFO_WLSW_INNERTRAV (arg_info) = TRUE;
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        INFO_WLSW_INNERTRAV (arg_info) = FALSE;

        /*
         * Remove mask and maskbase
         */
        INFO_WLSW_DEPMASK (arg_info) = DFMremoveMask (INFO_WLSW_DEPMASK (arg_info));
        maskbase = DFMremoveMaskBase (maskbase);

        /*
         * 2. Insert copy with-loop (if required)
         */
        DBUG_EXECUTE ("WLS", PRTdoPrintNode (arg_node););
        if (INFO_WLSW_MUSTCOPY (arg_info)) {
            node *id;
            node *avis;
            node *vardecs = NULL;
            node *ass;

            DBUG_PRINT ("WLS", ("Copy with-loop required"));

            avis
              = TBmakeAvis (ILIBtmpVar (), TYcopyType (ID_NTYPE (CODE_CEXPR (arg_node))));

            id = TBmakeId (avis);
            vardecs = TBmakeVardec (avis, vardecs);

            INFO_WLSW_FUNDEF (arg_info)
              = TCaddVardecs (INFO_WLSW_FUNDEF (arg_info), vardecs);

            ass
              = TCmakeAssignLet (avis, CreateCopyWithloop (CODE_CEXPR (arg_node),
                                                           INFO_WLSW_INNERDIMS (arg_info),
                                                           INFO_WLSW_FUNDEF (arg_info)));
            AVIS_SSAASSIGN (ID_AVIS (id)) = ass;

            BLOCK_INSTR (CODE_CBLOCK (arg_node))
              = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (arg_node)), ass);

            CODE_CEXPRS (arg_node) = FREEdoFreeTree (CODE_CEXPRS (arg_node));
            CODE_CEXPRS (arg_node) = TBmakeExprs (id, NULL);

            DBUG_PRINT ("WLS", ("New code after insertion of copy with-loop"));
            DBUG_EXECUTE ("WLS", PRTdoPrintNode (arg_node););
        } else {
            DBUG_PRINT ("WLS", ("No copy with-loop required"));
        }

        /*
         * 3. Expand inner with-loops ( aggressive behaviour!)
         */
        if (BLOCK_INSTR (CODE_CBLOCK (arg_node))
            != AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (arg_node)))) {

            node *first, *last;
            node *innercode;

            DBUG_PRINT ("WLS", ("Moving inter with-loop code into inner with-loop"));

            /*
             * Cut out code before the inner with-loop
             *
             * Remember it in first
             * the last assignment of that block is given by last
             */
            first = BLOCK_INSTR (CODE_CBLOCK (arg_node));
            last = first;
            DBUG_EXECUTE ("WLS", PRTdoPrintNode (last););
            DBUG_EXECUTE ("WLS", PRTdoPrintNode (
                                   AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (arg_node)))););

            while (ASSIGN_NEXT (last)
                   != AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (arg_node)))) {
                last = ASSIGN_NEXT (last);
                DBUG_EXECUTE ("WLS", PRTdoPrintNode (last););
            }

            /*
             * Make the inner with-loop the first assignment in the current code
             */
            BLOCK_INSTR (CODE_CBLOCK (arg_node)) = ASSIGN_NEXT (last);
            ASSIGN_NEXT (last) = NULL;

            DBUG_PRINT ("WLS", ("Intermediate code cut out"));
            DBUG_EXECUTE ("WLS", PRTdoPrintNode (arg_node););
            DBUG_PRINT ("WLS", ("first"));
            DBUG_EXECUTE ("WLS", PRTdoPrint (first););
            DBUG_PRINT ("WLS", ("last"););
            DBUG_EXECUTE ("WLS", PRTdoPrint (last););

            /*
             * Insert the code fragment into all inner codes
             */
            innercode = WITH_CODE (ASSIGN_RHS (BLOCK_INSTR (CODE_CBLOCK (arg_node))));
            while (innercode != NULL) {
                node *newcode;

                /*
                 * Ensure there is no N_empty node in the inner with-loop
                 */
                if (NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (innercode))) == N_empty) {
                    BLOCK_INSTR (CODE_CBLOCK (innercode))
                      = FREEdoFreeTree (BLOCK_INSTR (CODE_CBLOCK (innercode)));
                }

                /*
                 * Prepend the inner code with the outer code and
                 * create a duplicate.
                 */
                ASSIGN_NEXT (last) = BLOCK_INSTR (CODE_CBLOCK (innercode));
                BLOCK_INSTR (CODE_CBLOCK (innercode)) = first;

                newcode = DUPdoDupTreeSsa (innercode, INFO_WLSW_FUNDEF (arg_info));

                /*
                 * Restore old state of inner code
                 */
                BLOCK_INSTR (CODE_CBLOCK (innercode)) = ASSIGN_NEXT (last);
                ASSIGN_NEXT (last) = NULL;

                /*
                 * Replace the old CBLOCK and CEXPRS with the new ones.
                 */
                CODE_CBLOCK (innercode) = FREEdoFreeNode (CODE_CBLOCK (innercode));
                CODE_CEXPRS (innercode) = FREEdoFreeNode (CODE_CEXPRS (innercode));
                CODE_CBLOCK (innercode) = CODE_CBLOCK (newcode);
                CODE_CEXPRS (innercode) = CODE_CEXPRS (newcode);
                CODE_CBLOCK (newcode) = NULL;
                CODE_CEXPRS (newcode) = NULL;

                newcode = FREEdoFreeNode (newcode);

                innercode = CODE_NEXT (innercode);
            }

            /*
             * Free the now obsolete code
             */
            last = FREEdoFreeTree (last);
        }

        if (CODE_NEXT (arg_node) != NULL) {
            CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        }
    } else {
        /*
         * Traversal of inner code: TravSons
         */
        if (CODE_CBLOCK (arg_node) != NULL) {
            CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        }

        if (CODE_NEXT (arg_node) != NULL) {
            CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWid(node *arg_node, info *arg_info)
 *
 * @brief If the current id is marked in DEPMASK, all LHS identifiers are
 *        marked in depmask as well.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSWid");

    /*
     * If the current id is marked in DEPMASK, all LHS identifiers depend on it
     */
    if (DFMtestMaskEntry (INFO_WLSW_DEPMASK (arg_info), NULL, ID_AVIS (arg_node))) {
        node *tmp;

        /*
         * Mark them as dependent
         */
        tmp = INFO_WLSW_DEPSTACK (arg_info);
        while (tmp != NULL) {
            DFMsetMaskEntrySet (INFO_WLSW_DEPMASK (arg_info), NULL,
                                ID_AVIS (EXPRS_EXPR (tmp)));

            tmp = EXPRS_NEXT (tmp);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWlet(node *arg_node, info *arg_info)
 *
 * @brief pushes LHS identifier onto stack,
 *        traverses RHS and pops the identifier.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWlet (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ("WLSWlet");

    /*
     * Push all the LHS identifiers onto DEPSTACK
     */
    ids = LET_IDS (arg_node);
    while (ids != NULL) {
        INFO_WLSW_DEPSTACK (arg_info)
          = TBmakeExprs (TBmakeId (IDS_AVIS (ids)), INFO_WLSW_DEPSTACK (arg_info));
        ids = IDS_NEXT (ids);
    }

    /*
     * Traverse RHS
     */
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    /*
     * Pop LHS identifiers from stack
     */
    ids = LET_IDS (arg_node);
    while (ids != NULL) {
        INFO_WLSW_DEPSTACK (arg_info) = FREEdoFreeNode (INFO_WLSW_DEPSTACK (arg_info));
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWpart(node *arg_node, info *arg_info)
 *
 * @brief Traverses inner parts generators to check for dependencies
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSWpart");

    if (INFO_WLSW_INNERTRAV (arg_info)) {
        /*
         * Traversal of inner with-loop part
         */
        PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);

        if (PART_NEXT (arg_node) != NULL) {
            PART_NEXT (arg_node) = TRAVdo (PART_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWwith(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSWwith");

    if (!INFO_WLSW_INNERTRAV (arg_info)) {
        /*
         * Traversal of outer with-loop
         */

        /*
         * Traverse withid
         */
        WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);

        /*
         * Traverse codes
         */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        DBUG_PRINT ("WLS", ("Withloopified with-loop:"));
        DBUG_EXECUTE ("WLS", PRTdoPrintNode (arg_node););
    } else {
        /*
         * Traversal of inner with-loop
         */

        /*
         * Traverse parts
         */
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

        /*
         * Traverse withop
         */
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        if (ID_AVIS (EXPRS_EXPR (INFO_WLSW_DEPSTACK (arg_info)))
            == ID_AVIS (INFO_WLSW_CEXPR (arg_info))) {
            /*
             * No copy with-loop is required iff we have the base case:
             *  A perfect nesting of with-loops
             */
            if (((WITH_TYPE (arg_node) == N_genarray)
                 || (WITH_TYPE (arg_node) == N_modarray))
                && (!DFMtestMaskEntry (INFO_WLSW_DEPMASK (arg_info), NULL,
                                       ID_AVIS (INFO_WLSW_CEXPR (arg_info))))) {
                INFO_WLSW_MUSTCOPY (arg_info) = FALSE;
            }
        } else {
            /*
             * Traverse codes
             */
            WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWwithid(node *arg_node, info *arg_info)
 *
 * @brief remembers the first withid in the INFO structure.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
WLSWwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLSWwithid");

    INFO_WLSW_OUTERWITHID (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup wlsw */
