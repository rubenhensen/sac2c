/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup frc Filter Reuse Candidates
 *
 * @ingroup mm
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file filterrc.c
 *
 * Prefix: FRC
 *
 *****************************************************************************/
#include "filterrc.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "DataFlowMask.h"
#include "internal_lib.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    dfmask_t *usemask;
    dfmask_t *oldmask;
    dfmask_t *thenmask;
    dfmask_t *elsemask;
    node *condargs;
};

#define INFO_USEMASK(n) (n->usemask)
#define INFO_OLDMASK(n) (n->oldmask)
#define INFO_THENMASK(n) (n->thenmask)
#define INFO_ELSEMASK(n) (n->elsemask)
#define INFO_CONDARGS(n) (n->condargs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_USEMASK (result) = NULL;
    INFO_OLDMASK (result) = NULL;
    INFO_CONDARGS (result) = NULL;
    INFO_THENMASK (result) = NULL;
    INFO_ELSEMASK (result) = NULL;

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
 * @fn node *FRCdoFilterReuseCandidates( node *syntax_tree)
 *
 * @brief starting point of filter reuse candidates traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
FRCdoFilterReuseCandidates (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("FRCdoFilterReuseCandidates");

    DBUG_PRINT ("FRC", ("Starting to filter reuse candidates..."));

    info = MakeInfo ();

    TRAVpush (TR_frc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("FRC", ("Filtering of reuse candidates complete."));

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

/** <!--********************************************************************-->
 *
 * @fn node *FilterTrav( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static node *
FilterTrav (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FilterTrav");

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = FilterTrav (EXPRS_NEXT (arg_node), arg_info);
    }

    if (DFMtestMaskEntry (INFO_USEMASK (arg_info), NULL,
                          ID_AVIS (EXPRS_EXPR (arg_node)))) {
        DBUG_PRINT ("FRC", ("Invalid reuse candidate removed: %s",
                            ID_NAME (EXPRS_EXPR (arg_node))));
        arg_node = FREEdoFreeNode (arg_node);
    } else {
        EXPRS_EXPR (arg_node) = TRAVdo (EXPRS_EXPR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FilterRCs( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
static node *
FilterRCs (node *arg_node, info *arg_info)
{
    node *alloc;

    DBUG_ENTER ("FilterRCs");

    alloc = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node)));

    DBUG_EXECUTE ("FRC", PRTdoPrint (alloc););

    DBUG_ASSERT ((NODE_TYPE (alloc) == N_prf)
                   && ((PRF_PRF (alloc) == F_alloc)
                       || (PRF_PRF (alloc) == F_alloc_or_reuse)
                       || (PRF_PRF (alloc) == F_reuse)
                       || (PRF_PRF (alloc) == F_alloc_or_reshape)
                       || (PRF_PRF (alloc) == F_suballoc)),
                 "Illegal node type!");

    if ((PRF_PRF (alloc) != F_suballoc) && (PRF_PRF (alloc) != F_reuse)) {

        if (PRF_EXPRS3 (alloc) != NULL) {
            PRF_EXPRS3 (alloc) = FilterTrav (PRF_EXPRS3 (alloc), arg_info);
        }

        if (PRF_EXPRS3 (alloc) == NULL) {
            PRF_PRF (alloc) = F_alloc;
        }
    }

    DBUG_RETURN (arg_node);
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
 * @fn node *FRCap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCap");

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_CONDARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCarg( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCarg");

    if (DFMtestMaskEntry (INFO_OLDMASK (arg_info), NULL,
                          ID_AVIS (EXPRS_EXPR (INFO_CONDARGS (arg_info))))) {
        DBUG_PRINT ("FRC", ("Variable used in calling context: %s", ARG_NAME (arg_node)));
        DFMsetMaskEntrySet (INFO_USEMASK (arg_info), NULL, ARG_AVIS (arg_node));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_CONDARGS (arg_info) = EXPRS_NEXT (INFO_CONDARGS (arg_info));

        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCassign");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCcond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCcond");

    DBUG_PRINT ("FRC", ("Filtering conditional"));

    if (INFO_THENMASK (arg_info) == NULL) {
        /*
         * If a loop or cond fun has no return values, it also has no funcond.
         * So, here we must still care about non-existant INFO_THENMASK and
         * INFO_ELSEMASK.
         */
        INFO_THENMASK (arg_info) = DFMgenMaskCopy (INFO_USEMASK (arg_info));
        INFO_ELSEMASK (arg_info) = INFO_USEMASK (arg_info);
    }

    INFO_USEMASK (arg_info) = INFO_THENMASK (arg_info);
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_USEMASK (arg_info) = INFO_ELSEMASK (arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DFMsetMaskOr (INFO_USEMASK (arg_info), INFO_THENMASK (arg_info));

    INFO_THENMASK (arg_info) = DFMremoveMask (INFO_THENMASK (arg_info));
    INFO_ELSEMASK (arg_info) = NULL;

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCfuncond( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCfuncond");

    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);

    if (INFO_THENMASK (arg_info) == NULL) {
        INFO_THENMASK (arg_info) = DFMgenMaskCopy (INFO_USEMASK (arg_info));
        INFO_ELSEMASK (arg_info) = INFO_USEMASK (arg_info);
    }

    INFO_USEMASK (arg_info) = INFO_THENMASK (arg_info);
    FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);

    INFO_USEMASK (arg_info) = INFO_ELSEMASK (arg_info);
    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCfundef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        if ((!FUNDEF_ISCONDFUN (arg_node)) || (INFO_USEMASK (arg_info) != NULL)) {

            dfmask_base_t *maskbase;
            dfmask_t *oldmask, *oldthen, *oldelse;

            DBUG_PRINT ("FRC", ("Filtering reuse candidates in function %s",
                                FUNDEF_NAME (arg_node)));

            oldmask = INFO_USEMASK (arg_info);
            oldthen = INFO_THENMASK (arg_info);
            oldelse = INFO_ELSEMASK (arg_info);

            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_USEMASK (arg_info) = DFMgenMaskClear (maskbase);
            INFO_THENMASK (arg_info) = NULL;
            INFO_ELSEMASK (arg_info) = NULL;

            if (oldmask != NULL) {
                INFO_OLDMASK (arg_info) = oldmask;
                if (FUNDEF_ARGS (arg_node) != NULL) {
                    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
                }
                INFO_OLDMASK (arg_info) = NULL;
            }

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_USEMASK (arg_info) = DFMremoveMask (INFO_USEMASK (arg_info));

            maskbase = DFMremoveMaskBase (maskbase);

            INFO_USEMASK (arg_info) = oldmask;
            INFO_THENMASK (arg_info) = oldthen;
            INFO_ELSEMASK (arg_info) = oldelse;

            DBUG_PRINT ("FRC", ("Filtering reuse candidates in function %s complete",
                                FUNDEF_NAME (arg_node)));
        }
    }

    if ((INFO_USEMASK (arg_info) == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCid( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCid");

    if (!DFMtestMaskEntry (INFO_USEMASK (arg_info), NULL, ID_AVIS (arg_node))) {

        DBUG_PRINT ("FRC", ("Used Variable: %s", ID_NAME (arg_node)););

        DFMsetMaskEntrySet (INFO_USEMASK (arg_info), NULL, ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCprf");

    if (PRF_PRF (arg_node) == F_fill) {
        PRF_ARG2 (arg_node) = FilterRCs (PRF_ARG2 (arg_node), arg_info);
    }

    if (PRF_ARGS (arg_node) != NULL) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCwith( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCwith");

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCwith2( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCwith2");

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCbreak( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCbreak (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCbreak");

    BREAK_MEM (arg_node) = TRAVdo (BREAK_MEM (arg_node), arg_info);

    if (BREAK_NEXT (arg_node) != NULL) {
        BREAK_NEXT (arg_node) = TRAVdo (BREAK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCfold( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCfold");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCgenarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCgenarray");

    GENARRAY_MEM (arg_node) = FilterRCs (GENARRAY_MEM (arg_node), arg_info);
    GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);

    if (GENARRAY_DEFAULT (arg_node) != NULL) {
        GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCmodarray( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
FRCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCmodarray");

    MODARRAY_MEM (arg_node) = FilterRCs (MODARRAY_MEM (arg_node), arg_info);
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FRCcode( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
FRCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FRCcode");

    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Filter Reuse Candidates -->
 *****************************************************************************/
