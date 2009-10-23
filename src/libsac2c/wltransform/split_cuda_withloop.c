
#include "dbug.h"

#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "WLPartitionGeneration.h"
#include "wldefaultpartition.h"

#include "split_cuda_withloop.h"

/**
 * This traversal transformes Multi-Generator With-Loops into sequences of
 * Single-Generator With-loops. To understand the basic principle, let us look
 * at the three base cases:
 * Expressions of the forms:
 *
 *    with {                    with {                    with {
 *       ( <p1> ) : e1;            ( <p1> ) : e1;            ( <p1> ) : e1;
 *       ( <p2> ) : e2;            ( <p2> ) : e2;            ( <p2> ) : e2;
 *       ( <p3> ) : e3;            ( <p3> ) : e3;            ( <p3> ) : e3;
 *     } genarray( shp, def)     } modarray( a)            } fold( fun, neutr)
 *
 * are semantically equivalent to
 *
 *     with {                    with {                    with {
 *       ( <p3> ) : e3;            ( <p3> ) : e3;            ( <p3> ) : e3;
 *     } modarray( tmp)          } modarray( tmp)          } fold( fun, tmp)
 *
 * provided the variable tmp is defined as
 *
 * tmp = with {                  tmp = with {              tmp = with {
 *         ( <p2> ) : e2;                ( <p2> ) : e2;            ( <p2> ) : e2;
 *       } modarray( tmp2);            } modarray( tmp2);        } fold( fun, tmp2);
 *
 * and tmp2 is defined as
 *
 * tmp2 = with {                 tmp2 = with {             tmp2 = with {
 *          ( <p1> ) : e1;                ( <p1> ) : e1;            ( <p1> ) : e1;
 *        } genarray( shp, def);        } modarray( a);           } fold( fun, neutr);
 *
 * Note here, that
 *   a) It does not matter where the original with-loop resides, i.e., it can be in any
 *      expression position! As a consequence, this traversal can be run prior to flatten!
 *      All we need to make sure is to place the freshly created tmp variables, more
 * precisely their definitions, correcly. We need to ensure that the With-Loop is in their
 * scope.
 *
 *   b) This transformation can also be applied if we are dealing with Multi-Operator
 *      With-Loops! In that case, we simply map the transformation as outlined above to
 *      all operation parts.
 *
 */

/**
 * INFO structure
 */
struct INFO {
    node *lastassign;
    node *lhs;
    node *withops;
    node *letids;
    node *fundef;
    node *withid;
};

/**
 * INFO macros
 */
#define INFO_LASTASSIGN(n) (n->lastassign)
#define INFO_LHS(n) (n->lhs)
#define INFO_NEW_WITHOPS(n) (n->withops)
#define INFO_LETIDS(n) (n->letids)
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_WITHID(n) (n->withid)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_LASTASSIGN (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEW_WITHOPS (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--**********************************************************************
 *
 * @fn node *HWLGdoHandleWithLoops( node *syntax_tree)
 *
 * @brief starts the splitting of mgen/mop With-Loops
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/

node *
SCUWLdoSplitCudaWithloops (node *arg_node)
{
    info *info_node;

    DBUG_ENTER ("SCUWLdoHandleWithLoops");

    info_node = MakeInfo ();

    TRAVpush (TR_scuwl);
    arg_node = TRAVdo (arg_node, info_node);
    TRAVpop ();

    info_node = FreeInfo (info_node);

    // arg_node = WLDPdoWlDefaultPartition( arg_node);

    // arg_node = WLPGdoWlPartitionGeneration( arg_node);

    DBUG_RETURN (arg_node);
}

node *
SCUWLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUWLfundef");

    INFO_FUNDEF (arg_info) = arg_node;
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *HWLGassign(node *arg_node, info *arg_info)
 *
 * @brief ensures that INFO_HWLG_LASTASSIGN always points to the last assignment
 *        in whose scope we are traversing. After traversing the rhs, assignments
 *        that might have been prepanded are inserted into the syntax tree.
 *        Note here, that such newly inserted assignments are NOT traversed here!
 *
 * @param arg_node N_assign to be traversed
 * @param arg_info INFO_HWLG_LASTASSIGN is preserved but modified during traversal
 *                 of the rhs ASSIGN_INSTR.
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
SCUWLassign (node *arg_node, info *arg_info)
{
    node *mem_last_assign, *return_node;

    DBUG_ENTER ("SCUWLassign");

    mem_last_assign = INFO_LASTASSIGN (arg_info);
    INFO_LASTASSIGN (arg_info) = arg_node;
    DBUG_PRINT ("SCUWL", ("LASTASSIGN set to %08x!", arg_node));

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    /*
     * newly inserted abstractions are prepanded in front of
     * INFO_HWLG_LASTASSIGN(arg_info). To properly insert these nodes,
     * that pointer has to be returned:
     */
    return_node = INFO_LASTASSIGN (arg_info);

    if (return_node != arg_node) {
        DBUG_PRINT ("SCUWL", ("node %08x will be inserted instead of %08x", return_node,
                              arg_node));
    }
    INFO_LASTASSIGN (arg_info) = mem_last_assign;
    DBUG_PRINT ("SCUWL", ("LASTASSIGN (re)set to %08x!", mem_last_assign));

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (return_node);
}

node *
SCUWLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SCUWLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SplitWith(node *arg_node, info *arg_info)
 *
 * @brief creates aa sequences of With-Loop assignments with Single-Generator
 *        With-Loops from Multi-Generator With-Loops.
 *
 * @param arg_node N_with to be potentially transformed
 * @param arg_info uses INFO_HWLG_LASTASSIGN for inserting new assignments
 *                 temporarily uses INFO_HWLG_NEW_WITHOPS and INFO_HWLG_LHS
 *                 for collecting the withops / lhs vars during traversal
 *                 of the withops. Exits with these being set to NULL
 *
 * @return arg_node
 *
 *****************************************************************************/

static node *
SplitWith (node *arg_node, info *arg_info)
{
    node *part, *withop, *first_wl, *first_let, *dup_code, *old_code;
    lut_t *lut;

    DBUG_ENTER ("SplitWith");

    if (WITH_PART (arg_node) == NULL) {

        DBUG_ASSERT ((WITH_CODE (arg_node) == NULL),
                     "found a WL w/o generators, but with code blocks!");
    } else if (PART_NEXT (WITH_PART (arg_node)) != NULL) {

        part = WITH_PART (arg_node);

        lut = LUTgenerateLut ();

        node *idxs = WITHID_IDXS (PART_WITHID (part));
        node *new_avis
          = TBmakeAvis (TRAVtmpVarName ("wlidx"),
                        TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0)));
        LUTinsertIntoLutP (lut, IDS_AVIS (idxs), new_avis);
        IDS_AVIS (idxs) = new_avis;

        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (IDS_AVIS (idxs), FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        old_code = PART_CODE (part);
        CODE_NEXT (old_code) = NULL;
        printf ("code count before dup: %d\n", CODE_USED (old_code));
        // dup_code = DUPdoDupNode( old_code);
        dup_code = DUPdoDupNodeLutSsa (old_code, lut, INFO_FUNDEF (arg_info));
        CODE_USED (dup_code) = 1;
        CODE_NEXT (dup_code) = NULL;

        lut = LUTremoveLut (lut);

        /**
         * pull out the first part!
         */

        PART_CODE (part) = dup_code;
        WITH_PART (arg_node) = PART_NEXT (part);
        PART_NEXT (part) = NULL;

        printf ("code count after dup: %d\n", CODE_USED (old_code));

        /**
         * steal the withop(s)!
         */
        withop = WITH_WITHOP (arg_node);
        L_WITHOP_IDX (withop, IDS_AVIS (idxs));

        /**
         * and create the new first WL:
         */
        first_wl = TBmakeWith (part, dup_code, withop);
        WITH_CUDARIZABLE (first_wl) = TRUE;

        /**
         * Traversing the withops yields:
         *   a modifed withop chain in INFO_NEW_WITHOPS and
         *   a list of lhs variables (N_spids) for the first WL in INFO_HWLG_LHS
         */
        if (WITH_WITHOP (arg_node) != NULL) {
            INFO_WITHID (arg_info) = PART_WITHID (WITH_PART (arg_node));
            WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
            INFO_WITHID (arg_info) = NULL;
        }

        WITH_WITHOP (arg_node) = INFO_NEW_WITHOPS (arg_info);
        INFO_NEW_WITHOPS (arg_info) = NULL;

        first_let = TBmakeLet (INFO_LHS (arg_info), first_wl);
        INFO_LHS (arg_info) = NULL;

        arg_node = SplitWith (arg_node, arg_info);

        INFO_LASTASSIGN (arg_info) = TBmakeAssign (first_let, INFO_LASTASSIGN (arg_info));
    } else {
        printf ("in last partition\n");
        old_code = PART_CODE (WITH_PART (arg_node));
        CODE_NEXT (old_code) = NULL;
        dup_code = DUPdoDupNode (old_code);
        CODE_USED (dup_code) = 1;
        CODE_NEXT (dup_code) = NULL;

        WITH_CODE (arg_node) = dup_code;
        PART_CODE (WITH_PART (arg_node)) = dup_code;

        L_WITHOP_IDX (WITH_WITHOP (arg_node),
                      IDS_AVIS (WITHID_IDXS (PART_WITHID (WITH_PART (arg_node)))));

        // WITH_CODE( arg_node) = PART_CODE( WITH_PART( arg_node));
    }

    DBUG_RETURN (arg_node);
}

node *
SCUWLwith (node *arg_node, info *arg_info)
{
    node *old_with_code;

    DBUG_ENTER ("SCUWLwith");

    /**
     * Now, we recursively split Multi-Generator With-Loops:
     */
    if (WITH_CUDARIZABLE (arg_node)) {
        if (PART_NEXT (WITH_PART (arg_node)) != NULL) {
            old_with_code = WITH_CODE (arg_node);
            arg_node = SplitWith (arg_node, arg_info);
            old_with_code = FREEdoFreeTree (old_with_code);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCUWLgenarray(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
SCUWLgenarray (node *arg_node, info *arg_info)
{
    node *new_withop;
    node *avis;

    printf ("in genarray\n");

    DBUG_ENTER ("SCUWLgenarray");

    if (GENARRAY_NEXT (arg_node) != NULL) {
        // GENARRAY_NEXT( arg_node) = TRAVdo( GENARRAY_NEXT( arg_node), arg_info);
        DBUG_ASSERT ((0), "Cudarizbale N_with with more than one operators!");
    }

    avis = TBmakeAvis (TRAVtmpVar (),
                       TYcopyType (AVIS_TYPE (IDS_AVIS (INFO_LETIDS (arg_info)))));

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    new_withop = TBmakeModarray (TBmakeId (avis));
    if (GENARRAY_RC (arg_node) != NULL) {
        // MODARRAY_RC( new_withop) = DUPdoDupTree( GENARRAY_RC( arg_node));
    }
    // MODARRAY_IDX( new_withop) = IDS_AVIS( WITHID_IDXS( INFO_WITHID( arg_info)));
    // MODARRAY_IDX( new_withop) = GENARRAY_IDX( arg_node);

    MODARRAY_NEXT (new_withop) = INFO_NEW_WITHOPS (arg_info);
    INFO_NEW_WITHOPS (arg_info) = new_withop;

    INFO_LHS (arg_info) = TBmakeIds (avis, INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SCUWLmodarray(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
SCUWLmodarray (node *arg_node, info *arg_info)
{
    node *new_withop;
    node *avis;

    DBUG_ENTER ("SCUWLmodarray");

    if (MODARRAY_NEXT (arg_node) != NULL) {
        // MODARRAY_NEXT( arg_node) = TRAVdo( MODARRAY_NEXT( arg_node), arg_info);
        DBUG_ASSERT ((0), "Cudarizbale N_with with more than one operators!");
    }

    avis = TBmakeAvis (TRAVtmpVar (),
                       TYcopyType (AVIS_TYPE (IDS_AVIS (INFO_LETIDS (arg_info)))));

    FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
      = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

    new_withop = TBmakeModarray (TBmakeId (avis));
    if (MODARRAY_RC (arg_node) != NULL) {
        // MODARRAY_RC( new_withop) = DUPdoDupTree( MODARRAY_RC( arg_node));
    }
    // MODARRAY_IDX( new_withop) = IDS_AVIS( WITHID_IDXS( INFO_WITHID( arg_info)));
    // MODARRAY_IDX( new_withop) = MODARRAY_IDX( arg_node);

    MODARRAY_NEXT (new_withop) = INFO_NEW_WITHOPS (arg_info);
    INFO_NEW_WITHOPS (arg_info) = new_withop;

    INFO_LHS (arg_info) = TBmakeIds (avis, INFO_LHS (arg_info));

    DBUG_RETURN (arg_node);
}
