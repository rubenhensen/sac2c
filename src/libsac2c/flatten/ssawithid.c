/******************************************************************************
 *
 * @brief ssawithid performs conversion of N_withid nodes to and from
 *        SSA form.
 *
 *        When converting to SSA form, it also does SSA conversion
 *        of each N_part. This ensures that the resulting code
 *        is PURELY SSA. I.e., there is only ONE place
 *        in any WL where a name occurs as an LHS. This differs
 *        from "normal" sac2c behavior, where WITHID elements
 *        are duplicated across N_parts, and where assigns
 *        in N_part code blocks may contain names that
 *        are duplicated across N_part nodes.
 *
 *        Conversion to pure SSA greatly simplifies analysis and
 *        manipulation of the AST. The impetus for this traversal
 *        was a desire to simplify construction of maximal
 *        Affine Function Trees in PHUT/POGO/PWLF.
 *
 * This traversal is really only interested in looking at N_with nodes and
 * their children.
 *
 *****************************************************************************/
#include <limits.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "SSAW"
#include "debug.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "ssawithid.h"
#include "ctinfo.h"
#include "new_types.h"
#include "LookUpTable.h"
#include "shape.h"
#include "new_types.h"

struct INFO {
    node *fundef;
    node *vardecs;
    node *withid0;
    lut_t *lut;
    node *withcode;
    bool tossa;
};

/*
 * access macros:
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_WITHID0(n) (n->withid0)
#define INFO_LUT(n) (n->lut)
#define INFO_WITHCODE(n) (n->withcode)
#define INFO_TOSSA(n) (n->tossa)

/*
 * INFO functions:
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_WITHID0 (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_WITHCODE (result) = NULL;
    INFO_TOSSA (result) = 0;

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
 *
 * @fn node *SSAWfundef(node *arg_node, info *arg_info)
 *
 *
 ******************************************************************************/
node *
SSAWfundef (node *arg_node, info *arg_info)
{
    node *fundefold;
    DBUG_ENTER ();

    fundefold = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    if (!FUNDEF_ISWRAPPERFUN (arg_node)) {
        DBUG_PRINT ("Starting to traverse %s", FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    if (NULL != INFO_VARDECS (arg_info)) {
        FUNDEF_VARDECS (arg_node)
          = TCappendVardec (FUNDEF_VARDECS (arg_node), INFO_VARDECS (arg_info));
    }

    INFO_FUNDEF (arg_info) = fundefold;
    DBUG_PRINT ("leaving function %s", FUNDEF_NAME (arg_node));
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWblock(node *arg_node, info *arg_info)
 *
 ******************************************************************************/
node *
SSAWblock (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWassign(node *arg_node, info *arg_info)
 *
 *
 ******************************************************************************/
node *
SSAWassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWwith(node *arg_node, info *arg_info)
 *
 *
 *
 ******************************************************************************/
node *
SSAWwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITHID0 (arg_info) = NULL;
    INFO_WITHCODE (arg_info) = WITH_CODE (arg_node);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    INFO_WITHID0 (arg_info) = NULL;
    INFO_WITHCODE (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWpart(node *arg_node, info *arg_info)
 *
 * @brief The real work begins here. We save the first generator
 *        encountered, as generator0. This will serve as a model
 *        for the renaming in following partitions.
 *
 *        When we find a non-default partition, and we have
 *        a first generator, we do the SSA transformation.
 *
 ******************************************************************************/
node *
SSAWpart (node *arg_node, info *arg_info)
{
    node *cellexpr;
    node *pcode;
    node *pcodenew;

    DBUG_ENTER ();

    LUTremoveContentLut (INFO_LUT (arg_info));
    // Ignore N_default partition
    if (N_generator == NODE_TYPE (PART_GENERATOR (arg_node))) {
        if (NULL == INFO_WITHID0 (arg_info)) {
            INFO_WITHID0 (arg_info) = PART_WITHID (arg_node);
        } else {
            // This is some partition following a non-default one.
            // Rename this one.
            // We rename the N_withid elements in SSAWwithid,
            // and rename the references to those elements here.
            PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
        }
    }

    // Do renames in code block AND enforce SSA on all LHS
    // in the block.
    cellexpr = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (arg_node))));
    pcode = PART_CODE (arg_node);

    if (NULL != pcode) { // Do not dup code block if empty
        pcodenew
          = DUPdoDupTreeLutSsa (pcode, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));
        CODE_DEC_USED (PART_CODE (arg_node));
        CODE_INC_USED (pcodenew);
        INFO_WITHCODE (arg_info) = TCappendCode (INFO_WITHCODE (arg_info), pcodenew);
        PART_CODE (arg_node) = pcodenew;
    }

    // Rename CODE_CEXPRS
    cellexpr = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), cellexpr);
    ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (arg_node)))) = cellexpr;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *populateLut( node *arg_node, node *vardecs,
 *                    lut_t *lut, node *oldavis, bool tossa)
 *
 * @brief Generate a clone name for a WITHID element.
 *        Populate one element of a look up table with
 *        said name and its original, which we will use
 *        to do renames in the copied code block.
 *        Basically, we have a WL with generators of the form:
 *
 *    WL = with {
 *         ( lb0 <= iv=[i,j] <= ub0) : _sel_VxA_( iv, AAA); // Partn 0
 *         ...
 *         ( lb1 <= iv=[i,j] <= ub1) : _sel_VxA_( iv, AAA); // Partn 1
 *         ...
 *
 *        If renaming TO SSA, we do renames in the WL code block as follows:
 *
 *    WL' = with {
 *         ( lb0 <= iv=[i,j] <= ub0) : _sel_VxA_( iv, AAA); // Partn 0
 *         ...
 *         ( lb1 <= iv'=[i',j'] <= ub1) : _sel_VxA_( iv, AAA); // Partn 1
 *         ...
 *
 *        If renaming FROM SSA, we do renames in the WL' code block as follows:
 *
 *    WL'' = with {
 *         ( lb0 <= iv=[i,j] <= ub0) : _sel_VxA_( iv, AAA); // Partn 0
 *         ...
 *         ( lb1 <= iv=[i,j] <= ub1) : _sel_VxA_( iv, AAA); // Partn 1
 *         ...
 *
 *         I.e., WL and WL'' will have identical generators.
 *
 * @param: arg_node: one N_avis node of the PWL generator (e.g., iv),
 *                   to serve as iv for above assigns.
 *         vardecs:  N_vardec address for this fundef
 *         lut:      LUT to be populated
 *         oldavis:  N_avis from Partition 0, used if we are
 *                   renaming FROM SSA form.
 *         tossa:    true if going TO SSA form;
 *                   false if going from SSA form.
 *
 * @result: New N_avis node, e.g, iv'.
 *          Side effect: mapping iv -> iv' entry is now in LUT.
 *                       New vardec for iv'.
 *
 *****************************************************************************/
static node *
populateLut (node *arg_node, node *vardecs, lut_t *lut, node *oldavis, bool tossa)
{
    node *navis;

    DBUG_ENTER ();

    // Generate a new LHS name for WITHID_VEC/IDS, if doing TO SSA form,
    // but use N_part 0 name if going from SSA form.

    navis = tossa ? TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                                TYcopyType (AVIS_TYPE (arg_node)))
                  : oldavis;
    vardecs = TBmakeVardec (navis, vardecs);
    LUTinsertIntoLutP (lut, arg_node, navis);

    DBUG_PRINT ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                AVIS_NAME (arg_node), AVIS_NAME (navis));
    DBUG_RETURN (navis);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWwithid(node *arg_node, info *arg_info)
 *
 * @brief: Rename all identifiers in this node, populating
 *         the LUT with the old/new name pairs, so that
 *         we can then rename the N_part's code block.
 *
 ******************************************************************************/
node *
SSAWwithid (node *arg_node, info *arg_info)
{
    node *ids;
    node *oldids;
    node *lhsavis;

    DBUG_ENTER ();

    // Rename WITHID_VEC
    lhsavis
      = populateLut (IDS_AVIS (WITHID_VEC (arg_node)), INFO_VARDECS (arg_info),
                     INFO_LUT (arg_info), IDS_AVIS (WITHID_VEC (INFO_WITHID0 (arg_info))),
                     INFO_TOSSA (arg_info));
    IDS_AVIS (WITHID_VEC (arg_node)) = lhsavis;

    // Rename WITHID_IDXS, if it exists
    if (NULL != WITHID_IDXS (arg_node)) {
        lhsavis = populateLut (IDS_AVIS (WITHID_IDXS (arg_node)), INFO_VARDECS (arg_info),
                               INFO_LUT (arg_info),
                               IDS_AVIS (WITHID_IDXS (INFO_WITHID0 (arg_info))),
                               INFO_TOSSA (arg_info));
        IDS_AVIS (WITHID_IDXS (arg_node)) = lhsavis;
    }

    // Rename all WITHID_IDS elements
    oldids = WITHID_IDS (INFO_WITHID0 (arg_info));
    ids = WITHID_IDS (arg_node);
    while (NULL != ids) {
        lhsavis
          = populateLut (IDS_AVIS (ids), INFO_VARDECS (arg_info), INFO_LUT (arg_info),
                         IDS_AVIS (oldids), INFO_TOSSA (arg_info));
        IDS_AVIS (ids) = lhsavis;
        oldids = IDS_NEXT (oldids);
        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWids(node *arg_node, info *arg_info)
 *
 * @brief Rename all identifiers in the node, and
 *        populate the LUT with the old/new name pairs.
 *
 ******************************************************************************/

node *
SSAWids (node *arg_node, info *arg_info)
{
    node *navis;

    DBUG_ENTER ();

    /* Generate a new LHS name for WITHID_VEC/IDS */
    navis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (IDS_AVIS (arg_node))),
                        TYcopyType (AVIS_TYPE (IDS_AVIS (arg_node))));
    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (arg_node), navis);
    DBUG_PRINT ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                AVIS_NAME (IDS_AVIS (arg_node)), AVIS_NAME (navis));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWdoTransformToSSA(node *fundef)
 *
 * @brief Transform N_withid nodes into SSA form
 *        This works only in single-fundef mode
 *
 *
 ******************************************************************************/
node *
SSAWdoTransformToSSA (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo (arg_node);
    INFO_TOSSA (arg_info) = 1; // to SSA form
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_ssaw);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SSAWdoTransformFromSSA(node *arg_node)
 *
 * @brief Transform N_withid nodes from SSA form
 *        This works only in single-fundef mode
 *
 ******************************************************************************/
node *
SSAWdoTransformFromSSA (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo (arg_node);
    INFO_TOSSA (arg_info) = 0; // from SSA form
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_ssaw);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
