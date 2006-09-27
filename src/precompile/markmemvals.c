/**
 * $Id$
 *
 * @file markmemvals.c
 *
 * 1. In this traversal all fill operations and unnecessary MemVal-variables
 *    are removed by means of a substitution traversal.
 *
 *    Ex.:
 *      c = alloc();
 *      a = fill( ..., c)
 *
 *    is transformed into
 *      c = alloc();
 *      c = ...;
 *
 *    All subsequent references to a are renamed into references to c.
 *
 *    As genarray-withloops are nothing but fancy fill-operations after
 *    the explicit alloc-operations are inserted, their result is treated
 *    exactly like the fill operation's result.
 *
 * 2. If ExplicitAccumulate was applied:
 *    All subsequent references to left hand side of accu operations
 *    in Withloops are renamed into references to the corresponding
 *    return values of the fold operators.
 *    The corresponding cexpr is also be renamed.
 *
 *
 *    Ex.:
 *      A,B,C = with(iv)
 *               gen:{ a,b   = accu( iv);
 *                     val1  = ...;
 *                     val2  = ...;
 *                     val3  = ...;
 *                     emal1 = alloc(...);
 *                     emal1 = op1( a, val1);
 *                     emal2 = alloc(...);
 *                     emal2 = op2( b, val2);
 *                   }: emal1, emal2, val3
 *              fold( op1, n1)
 *              fold( op2, n2)
 *              genarray( shp);
 *
 *    is transformed into
 *      A,B,C = with(iv)
 *               gen:{ A,B   = accu( iv);
 *                     val1  = ...;
 *                     val2  = ...;
 *                     val3  = ...;
 *                     emal1 = alloc(...);
 *                     emal1 = op1( A, val1);
 *                     emal2 = alloc(...);
 *                     emal2 = op2( B, val2);
 *                     A     = emal1;
 *                     B     = emal2;
 *                   }: A, B, val3
 *              fold( op1, n1)
 *              fold( op2, n2)
 *              genarray( shp);
 *
 *
 */
#include "markmemvals.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "LookUpTable.h"
#include "print.h"
#include "DupTree.h"
#include "free.h"
#include "scheduling.h"
#include "new_types.h"

#include <string.h>

/**
 * INFO structure
 */
struct INFO {
    lut_t *lut;
    node *lhs;
    node *lhs_wl;
    node *withop;
    node *fundef;
};

#define INFO_LUT(n) (n->lut)
#define INFO_LHS(n) (n->lhs)
#define INFO_LHS_WL(n) (n->lhs_wl)
#define INFO_WITHOP(n) (n->withop)
#define INFO_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_LUT (result) = LUTgenerateLut ();
    INFO_LHS (result) = NULL;
    INFO_LHS_WL (result) = NULL;
    INFO_WITHOP (result) = NULL;
    INFO_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_LUT (info) = LUTremoveLut (INFO_LUT (info));
    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 *
 *  TRAVERSAL FUNCTIONS
 *
 * @{
 ****************************************************************************/
/** <!--******************************************************************-->
 *
 * @fn MMVblock
 *
 *  @brief Traverses a block's instructions and the vardec afterwards
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVblock");

    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);

    /*
     * Traverse into VARDECs in order to remove unneeded ones
     */
    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = TRAVdo (BLOCK_VARDEC (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVdo
 *
 *  @brief Traverses a do loop's bodies and the condition afterwards
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVdo");

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    if (DO_SKIP (arg_node) != NULL) {
        DO_SKIP (arg_node) = TRAVdo (DO_SKIP (arg_node), arg_info);
    }

    DO_COND (arg_node) = TRAVdo (DO_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVfundef
 *
 *  @brief Traverses a FUNDEF's body and clears the LUT afterwards.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return the given fundef with substituted identifiers.
 *
 ***************************************************************************/
node *
MMVfundef (node *arg_node, info *arg_info)
{
    info *info;

    DBUG_ENTER ("MMVfundef");

    /*
     * Regular functions are simply traversed
     */
    info = MakeInfo ();
    INFO_FUNDEF (info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);
    }

    INFO_LUT (info) = LUTremoveContentLut (INFO_LUT (info));
    info = FreeInfo (info);

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVlet
 *
 *  @brief Traverses right hand side and substitutes left hand side
 *         identifiers. If a substitution is made, the old vardec is marked
 *         ST_artificial as the new left hand side.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return modified let node.
 *
 ***************************************************************************/
node *
MMVlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MMVids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MMVids (node *arg_node, info *arg_info)
{
    node *newavis;

    DBUG_ENTER ("MMVids");

    newavis = LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));
    if (newavis != IDS_AVIS (arg_node)) {
        AVIS_ISDEAD (IDS_AVIS (arg_node)) = TRUE;
        IDS_AVIS (arg_node) = newavis;
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVid
 *
 *  @brief Substitutes the current reference with a reference from the
 *         LUT if possible
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return potentially, a new N_id node
 *
 ***************************************************************************/
node *
MMVid (node *arg_node, info *arg_info)
{
    node *newavis;

    DBUG_ENTER ("MMVid");

    newavis = LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

    if (newavis != ID_AVIS (arg_node)) {
        ID_AVIS (arg_node) = newavis;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MMVap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
MMVap (node *arg_node, info *arg_info)
{
    node *exprs, *args;

    DBUG_ENTER ("MMVap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    exprs = AP_ARGS (arg_node);
    args = FUNDEF_ARGS (AP_FUNDEF (arg_node));

    while (args != NULL) {
        if (ARG_HASLINKSIGNINFO (args)) {
            node *rets, *ids;

            rets = FUNDEF_RETS (AP_FUNDEF (arg_node));
            ids = INFO_LHS (arg_info);

            while (rets != NULL) {
                if (RET_HASLINKSIGNINFO (rets)
                    && (RET_LINKSIGN (rets) == ARG_LINKSIGN (args))) {
                    /*
                     * a = f( b)   where a === b
                     *
                     * rename: a -> b
                     */
                    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids),
                                       ID_NAME (EXPRS_EXPR (exprs)));

                    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids),
                                       ID_AVIS (EXPRS_EXPR (exprs)));
                }

                rets = RET_NEXT (rets);
                ids = IDS_NEXT (ids);
            }
        }

        args = ARG_NEXT (args);
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfFill
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfFill (node *arg_node, info *arg_info)
{
    node *temp;

    DBUG_ENTER ("MMVprfFill");

    /*
     * a = fill( ..., b')
     *
     * rename: b' -> b
     */
    PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);

    /*
     * a = fill( ..., b)
     *
     * rename: a -> b
     */
    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (PRF_ARG2 (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (PRF_ARG2 (arg_node)));

    /*
     * eliminate the fill operation
     *
     * b = ...;
     */
    temp = PRF_ARG1 (arg_node);
    PRF_ARG1 (arg_node) = NULL;
    arg_node = FREEdoFreeTree (arg_node);
    arg_node = temp;

    /*
     * Traverse the new rhs
     */
    if (arg_node != NULL) {
        arg_node = TRAVdo (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfAccu
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfAccu (node *arg_node, info *arg_info)
{
    node *withop;
    node *ids_assign, *ids_wl;

    DBUG_ENTER ("MMVprfAccu");

    /*
     * A,B = with(iv)
     *        gen:{
     *             a = accu( iv);
     *             ...
     *             }...
     *            fold( op1, n1)
     *            ...
     *
     *     rename: a -> A
     */

    ids_assign = INFO_LHS (arg_info);
    ids_wl = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);

    DBUG_ASSERT ((withop != NULL), "F_accu without withloop");

    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            DBUG_ASSERT ((ids_wl != NULL), "ids of wl is missing");
            DBUG_ASSERT ((ids_assign != NULL), "ids of assign is missing");

            LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids_assign),
                               IDS_NAME (ids_wl));

            LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids_assign),
                               IDS_AVIS (ids_wl));

            ids_assign = IDS_NEXT (ids_assign);
        }
        ids_wl = IDS_NEXT (ids_wl);
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfSuballoc
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfSuballoc (node *arg_node, info *arg_info)
{
    node *withop = NULL;
    node *ids_wl = NULL;
    node *avis = NULL;

    DBUG_ENTER ("MMVprfSuballoc");

    /*
     * a = suballoc( A, idx)
     */

    /*
     * rename RHS
     */
    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    /*
     * Find subarray identifier for this suballoc
     */
    ids_wl = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);

    while (ids_wl != NULL) {
        node *newavis = LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (ids_wl));

        if (newavis == ID_AVIS (PRF_ARG1 (arg_node))) {
            /*
             * Set a as new subarray identifier if none is set yet
             */
            if (WITHOP_SUB (withop) == NULL) {
                L_WITHOP_SUB (withop, TBmakeId (IDS_AVIS (INFO_LHS (arg_info))));
            }
            avis = ID_AVIS (WITHOP_SUB (withop));
            break;
        }
        withop = WITHOP_NEXT (withop);
        ids_wl = IDS_NEXT (ids_wl);
    }

    DBUG_ASSERT (avis != NULL, "No subarray identifier found!");

    /*
     * Insert pair (a, A_sub) into LUT
     */
    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       AVIS_NAME (avis));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)), avis);

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfWLAssign
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfWLAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVprfWLAssign");

    /*
     * a' = wl_assign( a, A, iv);
     *
     * 1. rename RHS
     * => a' = wl_assign( a, A)
     */
    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    /*
     * 2. insert pair ( a', a) into LUT
     */
    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (PRF_ARG1 (arg_node))),

      LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                         ID_AVIS (PRF_ARG1 (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprfPropObj
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
static node *
MMVprfPropObj (node *arg_node, info *arg_info)
{
    node *withop;
    node *ids_assign;
    node *args;

    DBUG_ENTER ("MMVprfPropObj");

    /*
     * A,B = with(iv)
     *        gen:{
     *             a',... = prop_obj( iv, a, ...);
     *             ...
     *             }...
     *            propagate( a)
     *            ...
     *
     *     rename: a' -> a
     */

    ids_assign = INFO_LHS (arg_info);
    args = EXPRS_NEXT (PRF_ARGS (arg_node));

    DBUG_ASSERT ((withop != NULL), "F_prop_obj without withloop");
    DBUG_ASSERT ((ids_assign != NULL), "ids of assign is missing");

    while (args != NULL) {
        LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (ids_assign),
                           ID_NAME (EXPRS_EXPR (args)));
        LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (ids_assign),
                           ID_AVIS (EXPRS_EXPR (args)));
        ids_assign = IDS_NEXT (ids_assign);
        args = EXPRS_NEXT (args);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVprf
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         prf is a F_fill.
 *         Adds the LHS and the corresponding LHS of current WL into LUT
 *         if this prf is a F_accu.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVprf");

    switch (PRF_PRF (arg_node)) {
    case F_fill:
        arg_node = MMVprfFill (arg_node, arg_info);
        break;

    case F_accu:
        arg_node = MMVprfAccu (arg_node, arg_info);
        break;

    case F_suballoc:
        arg_node = MMVprfSuballoc (arg_node, arg_info);
        break;

    case F_wl_assign:
        arg_node = MMVprfWLAssign (arg_node, arg_info);
        break;

    case F_prop_obj_in:
        arg_node = MMVprfPropObj (arg_node, arg_info);
        break;

    default:
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVvardec
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVvardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
    }

    if (AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith (node *arg_node, info *arg_info)
{
    node *withop;
    node *lhs;

    DBUG_ENTER ("MMVwith");

    /* stack lhs and withop of surrounding WL */
    lhs = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);

    INFO_LHS_WL (arg_info) = INFO_LHS (arg_info);
    INFO_WITHOP (arg_info) = WITH_WITHOP (arg_node);

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    INFO_WITHOP (arg_info) = withop;
    INFO_LHS_WL (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwith2
 *
 *  @brief
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVwith2 (node *arg_node, info *arg_info)
{
    node *withop;
    node *lhs;

    DBUG_ENTER ("MMVwith2");

    /*
     * stack lhs and withop of surrounding WL
     */
    lhs = INFO_LHS_WL (arg_info);
    withop = INFO_WITHOP (arg_info);

    INFO_LHS_WL (arg_info) = INFO_LHS (arg_info);
    INFO_WITHOP (arg_info) = WITH2_WITHOP (arg_node);

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    if (WITH2_SEGS (arg_node) != NULL) {
        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    }

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    INFO_WITHOP (arg_info) = withop;
    INFO_LHS_WL (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwlseg
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ***************************************************************************/
node *
MMVwlseg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVwlseg");

    if (WLSEG_SCHEDULING (arg_node) != NULL) {
        WLSEG_SCHEDULING (arg_node)
          = SCHmarkmemvalsScheduling (WLSEG_SCHEDULING (arg_node), INFO_LUT (arg_info));
        WLSEG_TASKSEL (arg_node)
          = SCHmarkmemvalsTasksel (WLSEG_TASKSEL (arg_node), INFO_LUT (arg_info));
    }

    WLSEG_CONTENTS (arg_node) = TRAVdo (WLSEG_CONTENTS (arg_node), arg_info);

    if (WLSEG_NEXT (arg_node) != NULL) {
        WLSEG_NEXT (arg_node) = TRAVdo (WLSEG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVwlsegvar
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 ***************************************************************************/
node *
MMVwlsegvar (node *arg_node, info *arg_info)
{
    int d;

    DBUG_ENTER ("MMVwlsegvar");

    if (WLSEGVAR_SCHEDULING (arg_node) != NULL) {
        WLSEGVAR_SCHEDULING (arg_node)
          = SCHmarkmemvalsScheduling (WLSEGVAR_SCHEDULING (arg_node),
                                      INFO_LUT (arg_info));
        WLSEGVAR_TASKSEL (arg_node)
          = SCHmarkmemvalsTasksel (WLSEGVAR_TASKSEL (arg_node), INFO_LUT (arg_info));
    }

    DBUG_ASSERT ((WLSEGVAR_IDX_MIN (arg_node) != NULL), "WLSEGVAR_IDX_MIN not found!");
    DBUG_ASSERT ((WLSEGVAR_IDX_MAX (arg_node) != NULL), "WLSEGVAR_IDX_MAX not found!");
    for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
        (WLSEGVAR_IDX_MIN (arg_node))[d]
          = TRAVdo ((WLSEGVAR_IDX_MIN (arg_node))[d], arg_info);
        (WLSEGVAR_IDX_MAX (arg_node))[d]
          = TRAVdo ((WLSEGVAR_IDX_MAX (arg_node))[d], arg_info);
    }

    WLSEGVAR_CONTENTS (arg_node) = TRAVdo (WLSEGVAR_CONTENTS (arg_node), arg_info);

    if (WLSEGVAR_NEXT (arg_node) != NULL) {
        WLSEGVAR_NEXT (arg_node) = TRAVdo (WLSEGVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVgenarray
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVgenarray");

    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (GENARRAY_MEM (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (GENARRAY_MEM (arg_node)));

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVmodarray
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVmodarray");

    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);

    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (MODARRAY_MEM (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (MODARRAY_MEM (arg_node)));

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVbreak
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVbreak");

    BREAK_MEM (arg_node) = TRAVdo (BREAK_MEM (arg_node), arg_info);

    LUTinsertIntoLutS (INFO_LUT (arg_info), IDS_NAME (INFO_LHS (arg_info)),
                       ID_NAME (BREAK_MEM (arg_node)));

    LUTinsertIntoLutP (INFO_LUT (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                       ID_AVIS (BREAK_MEM (arg_node)));

    if (BREAK_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVfold
 *
 *  @brief Adds the current LHS and the MEM-variable into LUT if this
 *         withop is either WO_genarray or WO_modarray.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVfold");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LHS (arg_info) = IDS_NEXT (INFO_LHS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVcode
 *
 *  @brief Substitutes a CEXPR reference with the corresponding reference
 *         on LHS of current withloop if the corresponding WL operation
 *         is fold.  OR  substitutes a CEXPR reference with the default
 *         element if corresponding WL operation is propagate.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVcode (node *arg_node, info *arg_info)
{
    node *wlids;
    node *cexprs;
    node *withop;

    DBUG_ENTER ("MMVcode");

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /*
     * if at least one WL-operator ist fold the accumulation results
     * must assigned back to the accumulation variable
     *
     * A,B = with(iv)
     *         gen:{ res1 = ...;
     *               res2 = ...;
     *               A    = res1   <---!!!
     *             } : A, res2
     *       fold( op1, n1)
     *       ...
     *
     */
    wlids = INFO_LHS_WL (arg_info);
    cexprs = CODE_CEXPRS (arg_node);
    withop = INFO_WITHOP (arg_info);

    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            BLOCK_INSTR (CODE_CBLOCK (arg_node))
              = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (arg_node)),
                                TBmakeAssign (TBmakeLet (DUPdoDupNode (wlids),
                                                         DUPdoDupNode (
                                                           EXPRS_EXPR (cexprs))),
                                              NULL));

            ID_AVIS (EXPRS_EXPR (cexprs)) = IDS_AVIS (wlids);
        } else if (NODE_TYPE (withop) == N_propagate) {
            BLOCK_INSTR (CODE_CBLOCK (arg_node))
              = TCappendAssign (BLOCK_INSTR (CODE_CBLOCK (arg_node)),
                                TBmakeAssign (TBmakeLet (TBmakeIds (ID_AVIS (
                                                                      PROPAGATE_DEFAULT (
                                                                        withop)),
                                                                    NULL),
                                                         DUPdoDupNode (
                                                           EXPRS_EXPR (cexprs))),
                                              NULL));
        }
        wlids = IDS_NEXT (wlids);
        cexprs = EXPRS_NEXT (cexprs);
        withop = WITHOP_NEXT (withop);
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * @}
 */

/** <!--******************************************************************-->
 *
 * @fn MarkMemVals
 *
 *  @brief Starting function of MarkMemVals traversal
 *
 *  @param syntax_tree
 *
 *  @return modified syntax tree
 *
 ***************************************************************************/
node *
MMVdoMarkMemVals (node *syntax_tree)
{
    DBUG_ENTER ("MMVdoMarkMemVals");

    TRAVpush (TR_mmv);

    syntax_tree = TRAVdo (syntax_tree, NULL);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
