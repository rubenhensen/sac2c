
/*
 *
 * $Log$
 * Revision 1.22  2004/11/28 22:14:30  ktr
 * added MMVblock
 *
 * Revision 1.21  2004/11/27 00:16:00  ktr
 * New barebones precompile.
 *
 * Revision 1.20  2004/11/25 11:38:37  ktr
 * COMPILES!
 *
 * Revision 1.19  2004/11/24 20:38:06  jhb
 * SacDevcamp 04
 *
 * Revision 1.18  2004/11/08 14:51:30  ktr
 * NWITHOP_MEM is now traversed, too.
 *
 * Revision 1.17  2004/09/24 12:53:41  ktr
 * Mask Base is now also updated after renaming args of SPMD-Functions.
 *
 * Revision 1.16  2004/09/23 21:54:13  ktr
 * MaskBase is now updated after inserting the subarray variable for A_sub.
 *
 * Revision 1.15  2004/09/23 16:37:32  ktr
 * DFM entries of variables no longer present are cleared now.
 *
 * Revision 1.14  2004/09/22 18:11:37  khf
 * moved renaming of cexprs from markmemvals
 * to third traversal of precompile
 *
 * Revision 1.13  2004/09/20 17:48:14  ktr
 * Removed unused variable.
 *
 * Revision 1.12  2004/09/18 16:07:23  ktr
 * SPMD blocks and functions are treated as well
 *
 * Revision 1.11  2004/09/07 20:55:45  khf
 * corrected creation of new assigns for renamed cexprs
 *
 * Revision 1.10  2004/08/05 16:11:24  ktr
 * Scalar with-loops are now treated as they always were. By using the
 * F_wl_assign abstraction we can now explicitly refcount this case.
 *
 * Revision 1.9  2004/08/04 12:04:58  ktr
 * substituted eacc by emm
 *
 * Revision 1.8  2004/08/03 10:05:54  ktr
 * All genarray/modarray results are named equally now.
 *
 * Revision 1.7  2004/08/01 13:18:36  ktr
 * added MMVwlsegx
 *
 * Revision 1.6  2004/07/31 21:29:15  ktr
 * moved treatment of F_fill, F_accu und F_suballoc into seperate functions.
 *
 * Revision 1.5  2004/07/28 12:26:24  khf
 * F_accu will be removed in compile.c
 *
 * Revision 1.4  2004/07/28 11:29:53  khf
 * insert N_empty node if BLOCK_INSTR is empty after removal
 * of F_accu
 *
 * Revision 1.3  2004/07/28 09:08:19  khf
 * support for ExplicitAccumulate added
 *
 * Revision 1.2  2004/07/22 14:13:50  ktr
 * a = fill( b, c) is now converted into c = b
 * which appears to be easier to compile.
 *
 * Revision 1.1  2004/07/21 16:52:24  ktr
 * Initial revision
 *
 *
 */

/**
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
#include "DataFlowMask.h"
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

#define INFO_MMV_LUT(n) (n->lut)
#define INFO_MMV_LHS(n) (n->lhs)
#define INFO_MMV_LHS_WL(n) (n->lhs_wl)
#define INFO_MMV_WITHOP(n) (n->withop)
#define INFO_MMV_FUNDEF(n) (n->fundef)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_MMV_LUT (result) = LUTgenerateLut ();
    INFO_MMV_LHS (result) = NULL;
    INFO_MMV_LHS_WL (result) = NULL;
    INFO_MMV_WITHOP (result) = NULL;
    INFO_MMV_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_MMV_LUT (info) = LUTremoveLut (INFO_MMV_LUT (info));
    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--******************************************************************-->
 *
 * @fn UpdateDFM
 *
 *  @brief Updates a DFM accoring to INFO_MMV_LUT
 *
 *  @param DFM
 *  @param arg_info
 *
 *  @return modified DFM
 *
 ***************************************************************************/
static dfmask_t *
UpdateDFM (dfmask_t *dfm, info *arg_info)
{
    node *vardec;

    DBUG_ENTER ("UpdateDFM");

    vardec = FUNDEF_VARDEC (INFO_MMV_FUNDEF (arg_info));

    while (vardec != NULL) {
        node *avis = VARDEC_AVIS (vardec);

        if (DFMtestMaskEntry (dfm, NULL, avis)) {
            DFMsetMaskEntryClear (dfm, NULL, avis);
            DFMsetMaskEntrySet (dfm, NULL,
                                LUTsearchInLutPp (INFO_MMV_LUT (arg_info), avis));
        }

        vardec = VARDEC_NEXT (vardec);
    }

    DBUG_RETURN (dfm);
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
    node *arg;
    char *newname;

    DBUG_ENTER ("MMVfundef");

    if (FUNDEF_ISSPMDFUN (arg_node)) {
        if (arg_info != NULL) {

            /*
             * SPMD-Functions: Arguments must be renamed
             */
            info = MakeInfo ();
            INFO_MMV_FUNDEF (info) = arg_node;

            arg = FUNDEF_ARGS (arg_node);
            while (arg != NULL) {
                if (ARG_NAME (arg) != NULL) {
                    newname = LUTsearchInLutSs (INFO_MMV_LUT (arg_info), ARG_NAME (arg));
                    if (newname != ARG_NAME (arg)) {
                        LUTinsertIntoLutS (INFO_MMV_LUT (info), ARG_NAME (arg),
                                           ILIBstringCopy (newname));

                        ARG_NAME (arg) = ILIBfree (ARG_NAME (arg));
                        ARG_NAME (arg) = ILIBstringCopy (newname);
                    }
                }
                arg = ARG_NEXT (arg);
            }

            /*
             * After renaming, DFM mask base must be updated
             */
            FUNDEF_DFM_BASE (arg_node)
              = DFMupdateMaskBaseAfterRenaming (FUNDEF_DFM_BASE (arg_node),
                                                FUNDEF_ARGS (arg_node),
                                                FUNDEF_VARDEC (arg_node));

            /*
             * Traverse function body
             */
            if (FUNDEF_BODY (arg_node) != NULL) {
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);
            }

            INFO_MMV_LUT (info) = LUTremoveContentLut (INFO_MMV_LUT (info));
            info = FreeInfo (info);
        } else {
            if (FUNDEF_NEXT (arg_node) != NULL) {
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        }
    } else {
        /*
         * Regular functions are simply traversed
         */
        info = MakeInfo ();
        INFO_MMV_FUNDEF (info) = arg_node;

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);
        }

        INFO_MMV_LUT (info) = LUTremoveContentLut (INFO_MMV_LUT (info));
        info = FreeInfo (info);

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        }
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

    newavis = LUTsearchInLutPp (INFO_MMV_LUT (arg_info), ID_AVIS (arg_node));

    if (newavis != ID_AVIS (arg_node)) {
        ID_AVIS (arg_node) = newavis;
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
    node *i;
    node *newavis;

    DBUG_ENTER ("MMVlet");

    INFO_MMV_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    i = LET_IDS (arg_node);

    while (i != NULL) {
        newavis = LUTsearchInLutPp (INFO_MMV_LUT (arg_info), IDS_AVIS (i));

        if (newavis != IDS_AVIS (i)) {
            /*
             * Mark the old lhs avis dead such that it will be removed
             */
            AVIS_ISDEAD (IDS_AVIS (i)) = TRUE;

            IDS_AVIS (i) = newavis;
        }

        i = IDS_NEXT (i);
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
    LUTinsertIntoLutS (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                       ID_NAME (PRF_ARG2 (arg_node)));

    LUTinsertIntoLutP (INFO_MMV_LUT (arg_info), IDS_AVIS (INFO_MMV_LHS (arg_info)),
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

    ids_assign = INFO_MMV_LHS (arg_info);
    ids_wl = INFO_MMV_LHS_WL (arg_info);
    withop = INFO_MMV_WITHOP (arg_info);

    DBUG_ASSERT ((withop != NULL), "F_accu without withloop");

    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            DBUG_ASSERT ((ids_wl != NULL), "ids of wl is missing");
            DBUG_ASSERT ((ids_assign != NULL), "ids of assign is missing");

            LUTinsertIntoLutS (INFO_MMV_LUT (arg_info), IDS_NAME (ids_assign),
                               IDS_NAME (ids_wl));

            LUTinsertIntoLutP (INFO_MMV_LUT (arg_info), IDS_AVIS (ids_assign),
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
    char *subname;
    node *vardec;
    node *avis;

    DBUG_ENTER ("MMVprfSuballoc");

    /*
     * a = suballoc( A, iv)
     *
     * 1. remove iv
     * => a = suballoc( A);
     */
    PRF_EXPRS2 (arg_node) = FREEdoFreeTree (PRF_EXPRS2 (arg_node));

    /*
     * rename RHS
     */
    PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

    /*
     * look for Vardec of A_sub
     */
    subname = ILIBstringConcat (ID_NAME (PRF_ARG1 (arg_node)), "_sub");

    vardec = FUNDEF_VARDEC (INFO_MMV_FUNDEF (arg_info));
    while ((vardec != NULL) && (0 != strcmp (VARDEC_NAME (vardec), subname))) {
        vardec = VARDEC_NEXT (vardec);
    }
    /*
     * if no vardec was found we need to create one
     */
    if (vardec == NULL) {
        avis = TBmakeAvis (subname, NULL);

        vardec = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_MMV_FUNDEF (arg_info)));

        VARDEC_TYPE (vardec) = DUPdupOneTypes (IDS_TYPE (INFO_MMV_LHS (arg_info)));
        FUNDEF_VARDEC (INFO_MMV_FUNDEF (arg_info)) = vardec;

        /*
         * Because of the new vardec, the mask base must be updated
         */
        if (FUNDEF_DFM_BASE (INFO_MMV_FUNDEF (arg_info)) != NULL) {
            FUNDEF_DFM_BASE (INFO_MMV_FUNDEF (arg_info))
              = DFMupdateMaskBase (FUNDEF_DFM_BASE (INFO_MMV_FUNDEF (arg_info)),
                                   FUNDEF_ARGS (INFO_MMV_FUNDEF (arg_info)),
                                   FUNDEF_VARDEC (INFO_MMV_FUNDEF (arg_info)));
        }
    } else {
        avis = VARDEC_AVIS (vardec);
        ILIBfree (subname);
    }

    /*
     * Insert pair (a, A_sub) into LUT
     */
    LUTinsertIntoLutS (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                       VARDEC_NAME (vardec));

    LUTinsertIntoLutP (INFO_MMV_LUT (arg_info), IDS_AVIS (INFO_MMV_LHS (arg_info)), avis);

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
    LUTinsertIntoLutS (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                       ID_NAME (PRF_ARG1 (arg_node))),

      LUTinsertIntoLutP (INFO_MMV_LUT (arg_info), IDS_AVIS (INFO_MMV_LHS (arg_info)),
                         ID_AVIS (PRF_ARG1 (arg_node)));

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

    default:
        if (PRF_ARGS (arg_node) != NULL) {
            PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn MMVspmd
 *
 *  @brief Corrects the dataflow masks in SPMD blocks
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return
 *
 ***************************************************************************/
node *
MMVspmd (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MMVspmd");

    SPMD_REGION (arg_node) = TRAVdo (SPMD_REGION (arg_node), arg_info);

    DBUG_EXECUTE ("MMV", PRTdoPrintNode (INFO_MMV_FUNDEF (arg_info)););

    SPMD_IN (arg_node) = UpdateDFM (SPMD_IN (arg_node), arg_info);
    SPMD_OUT (arg_node) = UpdateDFM (SPMD_OUT (arg_node), arg_info);
    SPMD_INOUT (arg_node) = UpdateDFM (SPMD_INOUT (arg_node), arg_info);
    SPMD_LOCAL (arg_node) = UpdateDFM (SPMD_LOCAL (arg_node), arg_info);
    SPMD_SHARED (arg_node) = UpdateDFM (SPMD_SHARED (arg_node), arg_info);

    SPMD_FUNDEF (arg_node) = TRAVdo (SPMD_FUNDEF (arg_node), arg_info);

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
    lhs = INFO_MMV_LHS_WL (arg_info);
    withop = INFO_MMV_WITHOP (arg_info);

    INFO_MMV_LHS_WL (arg_info) = INFO_MMV_LHS (arg_info);

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    if (WITH_PART (arg_node) != NULL) {
        WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    }

    if (WITH_CODE (arg_node) != NULL) {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    INFO_MMV_WITHOP (arg_info) = withop;
    INFO_MMV_LHS_WL (arg_info) = lhs;

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
    lhs = INFO_MMV_LHS_WL (arg_info);
    withop = INFO_MMV_WITHOP (arg_info);

    INFO_MMV_LHS_WL (arg_info) = INFO_MMV_LHS (arg_info);
    INFO_MMV_WITHOP (arg_info) = NULL;

    if (WITH2_WITHOP (arg_node) != NULL) {
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    }

    if (WITH2_SEGS (arg_node) != NULL) {
        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    }

    if (WITH2_CODE (arg_node) != NULL) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    }

    INFO_MMV_WITHOP (arg_info) = withop;
    INFO_MMV_LHS_WL (arg_info) = lhs;

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

    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        WLSEGX_SCHEDULING (arg_node)
          = SCHmarkmemvalsScheduling (WLSEGX_SCHEDULING (arg_node),
                                      INFO_MMV_LUT (arg_info));
        WLSEGX_TASKSEL (arg_node)
          = SCHmarkmemvalsTasksel (WLSEGX_TASKSEL (arg_node), INFO_MMV_LUT (arg_info));
    }

    WLSEGX_CONTENTS (arg_node) = TRAVdo (WLSEGX_CONTENTS (arg_node), arg_info);

    if (WLSEGX_NEXT (arg_node) != NULL) {
        WLSEGX_NEXT (arg_node) = TRAVdo (WLSEGX_NEXT (arg_node), arg_info);
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

    if (WLSEGX_SCHEDULING (arg_node) != NULL) {
        WLSEGX_SCHEDULING (arg_node)
          = SCHmarkmemvalsScheduling (WLSEGX_SCHEDULING (arg_node),
                                      INFO_MMV_LUT (arg_info));
        WLSEGX_TASKSEL (arg_node)
          = SCHmarkmemvalsTasksel (WLSEGX_TASKSEL (arg_node), INFO_MMV_LUT (arg_info));
    }

    DBUG_ASSERT ((WLSEGVAR_IDX_MIN (arg_node) != NULL), "WLSEGVAR_IDX_MIN not found!");
    DBUG_ASSERT ((WLSEGVAR_IDX_MAX (arg_node) != NULL), "WLSEGVAR_IDX_MAX not found!");
    for (d = 0; d < WLSEGVAR_DIMS (arg_node); d++) {
        (WLSEGVAR_IDX_MIN (arg_node))[d]
          = TRAVdo ((WLSEGVAR_IDX_MIN (arg_node))[d], arg_info);
        (WLSEGVAR_IDX_MAX (arg_node))[d]
          = TRAVdo ((WLSEGVAR_IDX_MAX (arg_node))[d], arg_info);
    }

    WLSEGX_CONTENTS (arg_node) = TRAVdo (WLSEGX_CONTENTS (arg_node), arg_info);

    if (WLSEGX_NEXT (arg_node) != NULL) {
        WLSEGX_NEXT (arg_node) = TRAVdo (WLSEGX_NEXT (arg_node), arg_info);
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

    if (INFO_MMV_WITHOP (arg_info) == NULL) {
        INFO_MMV_WITHOP (arg_info) = arg_node;
    }
    if (GENARRAY_SHAPE (arg_node) != NULL) {
        GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
    }
    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }
    if (GENARRAY_MEM (arg_node) != NULL) {
        GENARRAY_MEM (arg_node) = TRAVdo (GENARRAY_MEM (arg_node), arg_info);

        LUTinsertIntoLutS (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                           ID_NAME (GENARRAY_MEM (arg_node)));

        LUTinsertIntoLutP (INFO_MMV_LUT (arg_info), IDS_AVIS (INFO_MMV_LHS (arg_info)),
                           ID_AVIS (GENARRAY_MEM (arg_node)));
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_MMV_LHS (arg_info) = IDS_NEXT (INFO_MMV_LHS (arg_info));
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

    if (INFO_MMV_WITHOP (arg_info) == NULL) {
        INFO_MMV_WITHOP (arg_info) = arg_node;
    }

    if (MODARRAY_ARRAY (arg_node) != NULL) {
        MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
    }

    if (MODARRAY_MEM (arg_node) != NULL) {
        MODARRAY_MEM (arg_node) = TRAVdo (MODARRAY_MEM (arg_node), arg_info);

        LUTinsertIntoLutS (INFO_MMV_LUT (arg_info), IDS_NAME (INFO_MMV_LHS (arg_info)),
                           ID_NAME (MODARRAY_MEM (arg_node)));

        LUTinsertIntoLutP (INFO_MMV_LUT (arg_info), IDS_AVIS (INFO_MMV_LHS (arg_info)),
                           ID_AVIS (MODARRAY_MEM (arg_node)));
    }
    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_MMV_LHS (arg_info) = IDS_NEXT (INFO_MMV_LHS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
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

    if (INFO_MMV_WITHOP (arg_info) == NULL) {
        INFO_MMV_WITHOP (arg_info) = arg_node;
    }

    if (FOLD_NEUTRAL (arg_node) != NULL) {
        FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
    }

    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_MMV_LHS (arg_info) = IDS_NEXT (INFO_MMV_LHS (arg_info));
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
 *         is fold.
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

    DBUG_ENTER ("MMVcode");

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (CODE_CEXPRS (arg_node) != NULL) {
        CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);
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
