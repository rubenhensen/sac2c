/*
 *
 * $Log$
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

#define NEW_INFO

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

/**
 * INFO structure
 */
struct INFO {
    node *fundef;
    int innerdims;
    node *outerwithid;
    bool innertrav;
    node *depstack;
    DFMmask_t depmask;
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

    result = Malloc (sizeof (info));

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

    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWithloopify( node *arg_node, node *fundef, int innerdims)
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
WLSWithloopify (node *with, node *fundef, int innerdims)
{
    info *info;
    funtab *old_tab;

    DBUG_ENTER ("WLSWithloopify");

    DBUG_ASSERT (NODE_TYPE (with) == N_Nwith, "First parameter must be a with-loop");

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "Second parameter must be a fundef node");

    info = MakeInfo (fundef, innerdims);

    old_tab = act_tab;
    act_tab = wlsw_tab;

    with = Trav (with, info);

    act_tab = old_tab;

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

    idx_shape = SHCopyShape (unrshp);

    for (i = SHGetDim (unrshp) - 1; i >= 0; i--) {
        SHSetExtent (idx_shape, i, index % SHGetExtent (unrshp, i));
        index = index / SHGetExtent (unrshp, i);
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

    idx_shape = SHCopyShape (unrshp);

    for (i = SHGetDim (unrshp) - 1; i >= 0; i--) {
        SHSetExtent (idx_shape, i, (index % SHGetExtent (unrshp, i)) + 1);
        index = index / SHGetExtent (unrshp, i);
    }

    DBUG_RETURN (idx_shape);
}

/** <!--********************************************************************-->
 *
 * @fn int CountParts( node *parts)
 *
 * @brief Counts the number of parts in a given chain of parts
 *
 * @param parts
 *
 * @return number of parts
 *
 *****************************************************************************/
static int
CountParts (node *parts)
{
    int counter = 0;

    DBUG_ENTER ("CountParts");

    while (parts != NULL) {
        counter += 1;
        parts = NPART_NEXT (parts);
    }

    DBUG_RETURN (counter);
}

/** <!--********************************************************************-->
 *
 * @fn node *MakeSelCodes( node *parts, ids *iv, node *arr, node *fundef)
 *
 * @brief creates a NCODE for each part in parts that does nothing but select
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
MakeSelCodes (node *part, ids *iv, node *arr, node *fundef)
{
    node *code = NULL;
    node *vardecs = NULL;
    node *id;
    ntype *new_type;
    int dim;

    DBUG_ENTER ("MakeSelCodes");

    if (part != NULL) {
        id = MakeId (TmpVar (), NULL, ST_regular);

        dim = SHGetUnrLen (TYGetShape (AVIS_TYPE (IDS_AVIS (iv))));

        new_type = TYMakeAKS (TYCopyType (TYGetScalar (ID_NTYPE (arr))),
                              SHDropFromShape (dim, TYGetShape (ID_NTYPE (arr))));

        vardecs
          = MakeVardec (StringCopy (ID_NAME (id)), TYType2OldType (new_type), vardecs);

        fundef = AddVardecs (fundef, vardecs);

        ID_VARDEC (id) = vardecs;
        ID_AVIS (id) = VARDEC_AVIS (vardecs);
        ID_NTYPE (id) = new_type;

        code = MakeNCode (MakeBlock (MakeAssignLet (StringCopy (ID_NAME (id)), vardecs,
                                                    MakePrf2 (F_sel, DupIds_Id (iv),
                                                              DupNode (arr))),
                                     NULL),
                          MakeExprs (id, NULL));

        NPART_CODE (part) = code;
        NCODE_USED (code) = 1;

        NCODE_NEXT (code) = MakeSelCodes (NPART_NEXT (part), iv, arr, fundef);
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

    unrshp = SHTakeFromShape (unrdim, maxshp);

    upper_tl = SHDropFromShape (unrdim, maxshp);

    lower_tl = SHCopyShape (upper_tl);
    for (i = 0; i < SHGetDim (lower_tl); i++) {
        SHSetExtent (lower_tl, i, 0);
    }

    for (i = 0; i < SHGetUnrLen (unrshp); i++) {
        node *newpart;
        shape *lower, *upper, *lower_hd, *upper_hd;

        lower_hd = LowerBound (unrshp, i);
        upper_hd = UpperBound (unrshp, i);

        lower = SHAppendShapes (lower_hd, lower_tl);
        upper = SHAppendShapes (upper_hd, upper_tl);

        newpart = MakeNPart (DupNode (withid),
                             MakeNGenerator (SHShape2Array (lower), SHShape2Array (upper),
                                             F_le, F_lt, NULL, NULL),
                             NULL);

        lower_hd = SHFreeShape (lower_hd);
        upper_hd = SHFreeShape (upper_hd);
        lower = SHFreeShape (lower);
        upper = SHFreeShape (upper);

        NPART_NEXT (newpart) = parts;
        parts = newpart;
    }

    lower_tl = SHFreeShape (lower_tl);
    upper_tl = SHFreeShape (upper_tl);
    unrshp = SHFreeShape (unrshp);

    DBUG_RETURN (parts);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateCopyWithloop( node *array, int dim, node *fundef)
 *
 * @brief creates a withloop copying the first dimensions from array array.
 *        Typically, this means there is one part containing a selection
 *        array[iv].
 *        However, if array is an N_array or an index vector, its elements
 *        are copied elementwise by one part per array element.
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
    node *vardecs = NULL;
    node *parts;
    node *codes;
    node *withid;
    node *withop;
    ids *vec_ids;
    ids *scl_ids = NULL;
    ids *tmp_ids;
    int i;
    int unrdim;
    shape *maxshp;
    ntype *new_type;

    DBUG_ENTER ("CreateCopyWithloop");

    vec_ids = MakeIds (TmpVar (), NULL, ST_regular);

    new_type = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, dim));

    vardecs
      = MakeVardec (StringCopy (IDS_NAME (vec_ids)), TYType2OldType (new_type), vardecs);
    IDS_VARDEC (vec_ids) = vardecs;
    IDS_AVIS (vec_ids) = VARDEC_AVIS (vardecs);
    AVIS_TYPE (IDS_AVIS (vec_ids)) = new_type;

    for (i = 0; i < dim; i++) {
        tmp_ids = MakeIds (TmpVar (), NULL, ST_regular);
        new_type = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (0));
        vardecs
          = MakeVardec (StringCopy (IDS_NAME (tmp_ids)), MakeTypes1 (T_int), vardecs);
        IDS_NEXT (tmp_ids) = scl_ids;
        scl_ids = tmp_ids;
        IDS_VARDEC (scl_ids) = vardecs;
        IDS_AVIS (scl_ids) = VARDEC_AVIS (vardecs);
        AVIS_TYPE (IDS_AVIS (scl_ids)) = new_type;
    }

    if (AVIS_WITHID (ID_AVIS (array)) != NULL) {
        /*
         * array is an index vector, unroll at most one dimenstion
         */
        unrdim = (dim < 1) ? dim : 1;
    } else {
        DBUG_ASSERT (AVIS_SSAASSIGN (ID_AVIS (array)) != NULL,
                     "AVIS_SSAASSIGN must not be zero!");

        if (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array)))) == N_array) {
            /*
             * array is given by an array, unroll at most the array dimensionality
             */
            int arraydim
              = SHGetDim (ARRAY_SHAPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array)))));

            unrdim = (dim < arraydim) ? dim : arraydim;
        } else {
            /*
             * In all other cases, build a plain copy with-loop
             */
            unrdim = 0;
        }
    }

    maxshp = SHTakeFromShape (dim, TYGetShape (ID_NTYPE (array)));

    withid = MakeNWithid (vec_ids, scl_ids);

    parts = MakeSelParts (maxshp, unrdim, withid);

    codes = MakeSelCodes (parts, vec_ids, array, fundef);

    withop = MakeNWithOp (WO_genarray,
                          SHShape2Array (
                            SHTakeFromShape (dim, TYGetShape (ID_NTYPE (array)))));

    wl = MakeNWith (parts, codes, withop);

    NWITH_PARTS (wl) = CountParts (parts);

    fundef = AddVardecs (fundef, vardecs);

    /*
     * Clean up
     */
    maxshp = SHFreeShape (maxshp);
    withid = FreeTree (withid);

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
    DFMmask_base_t maskbase;
    ids *_ids;

    DBUG_ENTER ("WLSWcode");

    if (!INFO_WLSW_INNERTRAV (arg_info)) {
        /*
         * 1. Traverse block to determine whether a copy with-loop is required
         */

        /*
         * Build a maskbase and DEPMASK
         */
        maskbase = DFMGenMaskBase (FUNDEF_ARGS (INFO_WLSW_FUNDEF (arg_info)),
                                   FUNDEF_VARDEC (INFO_WLSW_FUNDEF (arg_info)));
        INFO_WLSW_DEPMASK (arg_info) = DFMGenMaskClear (maskbase);

        /*
         * Mark all variables in OUTERWITHID as the inner generators
         * must not depend on them
         */
        DFMSetMaskEntrySet (INFO_WLSW_DEPMASK (arg_info), NULL,
                            IDS_VARDEC (NWITHID_VEC (INFO_WLSW_OUTERWITHID (arg_info))));

        _ids = NWITHID_IDS (INFO_WLSW_OUTERWITHID (arg_info));
        while (_ids != NULL) {
            DFMSetMaskEntrySet (INFO_WLSW_DEPMASK (arg_info), NULL, IDS_VARDEC (_ids));
            _ids = IDS_NEXT (_ids);
        }

        /*
         * Traverse into CBLOCK
         */
        INFO_WLSW_CEXPR (arg_info) = NCODE_CEXPR (arg_node);
        INFO_WLSW_MUSTCOPY (arg_info) = TRUE;
        INFO_WLSW_INNERTRAV (arg_info) = TRUE;
        NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        INFO_WLSW_INNERTRAV (arg_info) = FALSE;

        /*
         * Remove mask and maskbase
         */
        INFO_WLSW_DEPMASK (arg_info) = DFMRemoveMask (INFO_WLSW_DEPMASK (arg_info));
        maskbase = DFMRemoveMaskBase (maskbase);

        /*
         * 2. Insert copy with-loop (if required)
         */
        DBUG_EXECUTE ("WLS", PrintNode (arg_node););
        if (INFO_WLSW_MUSTCOPY (arg_info)) {
            node *id;
            ntype *new_type;
            node *vardecs = NULL;
            node *ass;

            DBUG_PRINT ("WLS", ("Copy with-loop required"));

            id = MakeId (TmpVar (), NULL, ST_regular);

            new_type = TYCopyType (ID_NTYPE (NCODE_CEXPR (arg_node)));

            vardecs = MakeVardec (StringCopy (ID_NAME (id)), TYType2OldType (new_type),
                                  vardecs);

            ID_VARDEC (id) = vardecs;
            ID_AVIS (id) = VARDEC_AVIS (vardecs);
            ID_NTYPE (id) = new_type;

            INFO_WLSW_FUNDEF (arg_info)
              = AddVardecs (INFO_WLSW_FUNDEF (arg_info), vardecs);

            ass = MakeAssignLet (StringCopy (ID_NAME (id)), vardecs,
                                 CreateCopyWithloop (NCODE_CEXPR (arg_node),
                                                     INFO_WLSW_INNERDIMS (arg_info),
                                                     INFO_WLSW_FUNDEF (arg_info)));

            AVIS_SSAASSIGN (ID_AVIS (id)) = ass;

            BLOCK_INSTR (NCODE_CBLOCK (arg_node))
              = AppendAssign (BLOCK_INSTR (NCODE_CBLOCK (arg_node)), ass);

            NCODE_CEXPRS (arg_node) = FreeTree (NCODE_CEXPRS (arg_node));
            NCODE_CEXPRS (arg_node) = MakeExprs (id, NULL);

            DBUG_PRINT ("WLS", ("New code after insertion of copy with-loop"));
            DBUG_EXECUTE ("WLS", PrintNode (arg_node););
        } else {
            DBUG_PRINT ("WLS", ("No copy with-loop required"));
        }

        /*
         * 3. Expand inner with-loops ( aggressive behaviour!)
         */
        if (BLOCK_INSTR (NCODE_CBLOCK (arg_node))
            != AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (arg_node)))) {

            node *first, *last;
            node *innercode;

            DBUG_PRINT ("WLS", ("Moving inter with-loop code into inner with-loop"));

            /*
             * Cut out code before the inner with-loop
             *
             * Remember it in first
             * the last assignment of that block is given by last
             */
            first = BLOCK_INSTR (NCODE_CBLOCK (arg_node));
            last = first;
            DBUG_EXECUTE ("WLS", PrintNode (last););
            DBUG_EXECUTE ("WLS",
                          PrintNode (AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (arg_node)))););
            while (ASSIGN_NEXT (last)
                   != AVIS_SSAASSIGN (ID_AVIS (NCODE_CEXPR (arg_node)))) {
                last = ASSIGN_NEXT (last);
                DBUG_EXECUTE ("WLS", PrintNode (last););
            }

            /*
             * Make the inner with-loop the first assignment in the current code
             */
            BLOCK_INSTR (NCODE_CBLOCK (arg_node)) = ASSIGN_NEXT (last);
            ASSIGN_NEXT (last) = NULL;

            DBUG_PRINT ("WLS", ("Intermediate code cut out"));
            DBUG_EXECUTE ("WLS", PrintNode (arg_node););
            DBUG_PRINT ("WLS", ("first"));
            DBUG_EXECUTE ("WLS", Print (first););
            DBUG_PRINT ("WLS", ("last"););
            DBUG_EXECUTE ("WLS", Print (last););

            /*
             * Insert the code fragment into all inner codes
             */
            innercode = NWITH_CODE (ASSIGN_RHS (BLOCK_INSTR (NCODE_CBLOCK (arg_node))));
            while (innercode != NULL) {
                node *newcode;

                /*
                 * Ensure there is no N_empty node in the inner with-loop
                 */
                if (NODE_TYPE (BLOCK_INSTR (NCODE_CBLOCK (innercode))) == N_empty) {
                    BLOCK_INSTR (NCODE_CBLOCK (innercode))
                      = FreeTree (BLOCK_INSTR (NCODE_CBLOCK (innercode)));
                }

                /*
                 * Prepend the inner code with the outer code and
                 * create a duplicate.
                 */
                ASSIGN_NEXT (last) = BLOCK_INSTR (NCODE_CBLOCK (innercode));
                BLOCK_INSTR (NCODE_CBLOCK (innercode)) = first;

                newcode = DupTreeSSA (innercode, INFO_WLSW_FUNDEF (arg_info));

                /*
                 * Restore old state of inner code
                 */
                BLOCK_INSTR (NCODE_CBLOCK (innercode)) = ASSIGN_NEXT (last);
                ASSIGN_NEXT (last) = NULL;

                /*
                 * Replace the old CBLOCK and CEXPRS with the new ones.
                 */
                NCODE_CBLOCK (innercode) = FreeNode (NCODE_CBLOCK (innercode));
                NCODE_CEXPRS (innercode) = FreeNode (NCODE_CEXPRS (innercode));
                NCODE_CBLOCK (innercode) = NCODE_CBLOCK (newcode);
                NCODE_CEXPRS (innercode) = NCODE_CEXPRS (newcode);
                NCODE_CBLOCK (newcode) = NULL;
                NCODE_CEXPRS (newcode) = NULL;

                newcode = FreeNode (newcode);

                innercode = NCODE_NEXT (innercode);
            }

            /*
             * Free the now obsolete code
             */
            last = FreeTree (last);
        }

        if (NCODE_NEXT (arg_node) != NULL) {
            NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
        }
    } else {
        /*
         * Traversal of inner code: TravSons
         */
        if (NCODE_CBLOCK (arg_node) != NULL) {
            NCODE_CBLOCK (arg_node) = Trav (NCODE_CBLOCK (arg_node), arg_info);
        }

        if (NCODE_NEXT (arg_node) != NULL) {
            NCODE_NEXT (arg_node) = Trav (NCODE_NEXT (arg_node), arg_info);
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
    if (DFMTestMaskEntry (INFO_WLSW_DEPMASK (arg_info), NULL, ID_VARDEC (arg_node))) {
        node *tmp;

        /*
         * Mark them as dependent
         */
        tmp = INFO_WLSW_DEPSTACK (arg_info);
        while (tmp != NULL) {
            DFMSetMaskEntrySet (INFO_WLSW_DEPMASK (arg_info), NULL,
                                ID_VARDEC (EXPRS_EXPR (tmp)));

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
    ids *_ids;

    DBUG_ENTER ("WLSWlet");

    /*
     * Push all the LHS identifiers onto DEPSTACK
     */
    _ids = LET_IDS (arg_node);
    while (_ids != NULL) {
        INFO_WLSW_DEPSTACK (arg_info)
          = MakeExprs (DupIds_Id (_ids), INFO_WLSW_DEPSTACK (arg_info));
        _ids = IDS_NEXT (_ids);
    }

    /*
     * Traverse RHS
     */
    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    /*
     * Pop LHS identifiers from stack
     */
    _ids = LET_IDS (arg_node);
    while (_ids != NULL) {
        INFO_WLSW_DEPSTACK (arg_info) = FreeNode (INFO_WLSW_DEPSTACK (arg_info));
        _ids = IDS_NEXT (_ids);
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
        NPART_GEN (arg_node) = Trav (NPART_GEN (arg_node), arg_info);

        if (NPART_NEXT (arg_node) != NULL) {
            NPART_NEXT (arg_node) = Trav (NPART_NEXT (arg_node), arg_info);
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
        NWITH_WITHID (arg_node) = Trav (NWITH_WITHID (arg_node), arg_info);

        /*
         * Traverse codes
         */
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);

        DBUG_PRINT ("WLS", ("Withloopified with-loop:"));
        DBUG_EXECUTE ("WLS", PrintNode (arg_node););
    } else {
        /*
         * Traversal of inner with-loop
         */

        /*
         * Traverse parts
         */
        NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

        /*
         * Traverse withop
         */
        NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

        if (ID_VARDEC (EXPRS_EXPR (INFO_WLSW_DEPSTACK (arg_info)))
            == ID_VARDEC (INFO_WLSW_CEXPR (arg_info))) {
            /*
             * No copy with-loop is required iff we have the base case:
             *  A perfect nesting of with-loops
             */
            if (((NWITH_TYPE (arg_node) == WO_genarray)
                 || (NWITH_TYPE (arg_node) == WO_modarray))
                && (!DFMTestMaskEntry (INFO_WLSW_DEPMASK (arg_info), NULL,
                                       ID_VARDEC (INFO_WLSW_CEXPR (arg_info))))) {
                INFO_WLSW_MUSTCOPY (arg_info) = FALSE;
            }
        } else {
            /*
             * Traverse codes
             */
            NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLSWwithid(node *arg_node, info *arg_info)
 *
 * @brief Initializes AVIS_WITHID of the index vector and
 *        remembers the first withid in the INFO structure.
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

    AVIS_WITHID (IDS_AVIS (NWITHID_VEC (arg_node))) = arg_node;
    INFO_WLSW_OUTERWITHID (arg_info) = arg_node;

    DBUG_RETURN (arg_node);
}

/*@}*/ /* defgroup wlsw */
