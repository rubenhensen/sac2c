/*
 *
 * $Log$
 * Revision 1.6  2004/11/23 21:00:52  ktr
 * COMPILES!!!
 *
 * Revision 1.5  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.4  2004/11/04 18:45:38  ktr
 * All withops are traversed now.
 *
 * Revision 1.3  2004/11/02 14:34:59  ktr
 * Better support for conditionals.
 * AP_ARGS are now actually traversed.
 *
 * Revision 1.2  2004/10/12 12:18:01  ktr
 * moved declaration of local variables in EMFRCfundef in order to
 * please Stephan's compiler.
 *
 * Revision 1.1  2004/10/12 10:07:52  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup frc Filter reuse candidates
 * @ingroup rcp
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file filterrc.c
 *
 *
 */
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

/**
 * INFO structure
 */
struct INFO {
    dfmask_t *usemask;
    dfmask_t *oldmask;
    dfmask_t *thenmask;
    dfmask_t *elsemask;
    node *condargs;
};

/**
 * INFO macros
 */
#define INFO_FRC_USEMASK(n) (n->usemask)
#define INFO_FRC_OLDMASK(n) (n->oldmask)
#define INFO_FRC_THENMASK(n) (n->thenmask)
#define INFO_FRC_ELSEMASK(n) (n->elsemask)
#define INFO_FRC_CONDARGS(n) (n->condargs)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FRC_USEMASK (result) = NULL;
    INFO_FRC_OLDMASK (result) = NULL;
    INFO_FRC_CONDARGS (result) = NULL;
    INFO_FRC_THENMASK (result) = NULL;
    INFO_FRC_ELSEMASK (result) = NULL;

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
 * @fn node *EMFRCFilterReuseCandidates( node *syntax_tree)
 *
 * @brief starting point of filter reuse candidates traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMFRCFilterReuseCandidates (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMFRCFilterReuseCandidates");

    DBUG_PRINT ("EMFRC", ("Starting to filter reuse candidates..."));

    info = MakeInfo ();

    TRAVpush (TR_emfrc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("EMFRC", ("Filtering of reuse candidates complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *FilterTrav( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
static node *
FilterTrav (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FilterTrav");

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = FilterTrav (EXPRS_NEXT (arg_node), arg_info);
    }

    if (DFMtestMaskEntry (INFO_FRC_USEMASK (arg_info), NULL,
                          ID_AVIS (EXPRS_EXPR (arg_node)))) {
        DBUG_PRINT ("EMFRC", ("Invalid reuse candidate removed: %s",
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
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
static node *
FilterRCs (node *arg_node, info *arg_info)
{
    node *alloc;

    DBUG_ENTER ("FilterRCs");

    alloc = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (arg_node)));

    DBUG_EXECUTE ("EMFRC", PRTdoPrint (alloc););

    DBUG_ASSERT ((NODE_TYPE (alloc) == N_prf)
                   && ((PRF_PRF (alloc) == F_alloc)
                       || (PRF_PRF (alloc) == F_alloc_or_reuse)
                       || (PRF_PRF (alloc) == F_alloc_or_reshape)
                       || (PRF_PRF (alloc) == F_suballoc)),
                 "Illegal node type!");

    if (PRF_PRF (alloc) != F_suballoc) {

        if (PRF_EXPRS3 (alloc) != NULL) {
            PRF_EXPRS3 (alloc) = FilterTrav (PRF_EXPRS3 (alloc), arg_info);
        }

        if (PRF_EXPRS3 (alloc) == NULL) {
            PRF_PRF (alloc) = F_alloc;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Filter reuse candidates traversal (emfrc_tab)
 *
 * prefix: EMFRC
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMFRCap( node *arg_node, info *arg_info)
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
EMFRCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCap");

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_FRC_CONDARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCarg( node *arg_node, info *arg_info)
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
EMFRCarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCarg");

    if (DFMtestMaskEntry (INFO_FRC_OLDMASK (arg_info), NULL,
                          ID_AVIS (EXPRS_EXPR (INFO_FRC_CONDARGS (arg_info))))) {
        DBUG_PRINT ("EMFRC",
                    ("Variable used in calling context: %s", ARG_NAME (arg_node)));
        DFMsetMaskEntrySet (INFO_FRC_USEMASK (arg_info), NULL, ARG_AVIS (arg_node));
    }

    INFO_FRC_CONDARGS (arg_info) = EXPRS_NEXT (INFO_FRC_CONDARGS (arg_info));

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCassign( node *arg_node, info *arg_info)
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
EMFRCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCassign");

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
 * @fn node *EMFRCcond( node *arg_node, info *arg_info)
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
EMFRCcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCcond");

    DBUG_PRINT ("EMFRC", ("Filtering conditional"));

    INFO_FRC_USEMASK (arg_info) = INFO_FRC_THENMASK (arg_info);
    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);

    INFO_FRC_USEMASK (arg_info) = INFO_FRC_ELSEMASK (arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    DFMsetMaskOr (INFO_FRC_USEMASK (arg_info), INFO_FRC_THENMASK (arg_info));

    INFO_FRC_THENMASK (arg_info) = DFMremoveMask (INFO_FRC_THENMASK (arg_info));
    INFO_FRC_ELSEMASK (arg_info) = NULL;

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCfuncond( node *arg_node, info *arg_info)
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
EMFRCfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCfuncond");

    FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);

    if (INFO_FRC_THENMASK (arg_info) == NULL) {
        INFO_FRC_THENMASK (arg_info) = DFMgenMaskCopy (INFO_FRC_USEMASK (arg_info));
        INFO_FRC_ELSEMASK (arg_info) = INFO_FRC_USEMASK (arg_info);
    }

    INFO_FRC_USEMASK (arg_info) = INFO_FRC_THENMASK (arg_info);
    FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);

    INFO_FRC_USEMASK (arg_info) = INFO_FRC_ELSEMASK (arg_info);
    FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCfundef( node *arg_node, info *arg_info)
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
EMFRCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        if ((!FUNDEF_ISCONDFUN (arg_node)) || (INFO_FRC_USEMASK (arg_info) != NULL)) {

            dfmask_base_t *maskbase;
            dfmask_t *oldmask, *oldthen, *oldelse;

            DBUG_PRINT ("EMFRC", ("Filtering reuse candidates in function %s",
                                  FUNDEF_NAME (arg_node)));

            oldmask = INFO_FRC_USEMASK (arg_info);
            oldthen = INFO_FRC_THENMASK (arg_info);
            oldelse = INFO_FRC_ELSEMASK (arg_info);

            maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

            INFO_FRC_USEMASK (arg_info) = DFMgenMaskClear (maskbase);
            INFO_FRC_THENMASK (arg_info) = NULL;
            INFO_FRC_ELSEMASK (arg_info) = NULL;

            if (oldmask != NULL) {
                INFO_FRC_OLDMASK (arg_info) = oldmask;
                if (FUNDEF_ARGS (arg_node) != NULL) {
                    FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
                }
                INFO_FRC_OLDMASK (arg_info) = NULL;
            }

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            INFO_FRC_USEMASK (arg_info) = DFMremoveMask (INFO_FRC_USEMASK (arg_info));

            maskbase = DFMremoveMaskBase (maskbase);

            INFO_FRC_USEMASK (arg_info) = oldmask;
            INFO_FRC_THENMASK (arg_info) = oldthen;
            INFO_FRC_ELSEMASK (arg_info) = oldelse;

            DBUG_PRINT ("EMFRC", ("Filtering reuse candidates in function %s complete",
                                  FUNDEF_NAME (arg_node)));
        }
    }

    if ((INFO_FRC_USEMASK (arg_info) == NULL) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCid( node *arg_node, info *arg_info)
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
EMFRCid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCid");

    if (!DFMtestMaskEntry (INFO_FRC_USEMASK (arg_info), NULL, ID_AVIS (arg_node))) {

        DBUG_PRINT ("EMFRC", ("Used Variable: %s", ID_NAME (arg_node)););

        DFMsetMaskEntrySet (INFO_FRC_USEMASK (arg_info), NULL, ID_AVIS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCprf( node *arg_node, info *arg_info)
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
EMFRCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCprf");

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
 * @fn node *EMFRCwith( node *arg_node, info *arg_info)
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
EMFRCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCwith");

    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCwith2( node *arg_node, info *arg_info)
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
EMFRCwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCwith2");

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCfold( node *arg_node, info *arg_info)
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
EMFRCfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCfold");

    FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);

    if (FOLD_NEXT (arg_node) != NULL) {
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMFRCgenarray( node *arg_node, info *arg_info)
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
EMFRCgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCgenarray");

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
 * @fn node *EMFRCmodarray( node *arg_node, info *arg_info)
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
EMFRCmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMFRCmodarray");

    MODARRAY_MEM (arg_node) = FilterRCs (MODARRAY_MEM (arg_node), arg_info);
    MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
