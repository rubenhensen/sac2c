/*
 *
 * $Log$
 * Revision 1.2  2004/11/09 19:33:27  ktr
 * ongoing implementation
 *
 * Revision 1.1  2004/11/02 14:26:48  ktr
 * Initial revision
 *
 */

/**
 *
 * @defgroup lr Loop Reuse
 * @ingroup rcp
 *
 * <pre>
 * </pre>
 * @{
 */

/**
 *
 * @file loopreuseopt.c
 *
 *
 */
#define NEW_INFO

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "print.h"
#include "new_types.h"
#include "user_types.h"
#include "DataFlowMask.h"
#include "filterrc.h"
#include "aliasanalysis.h"
#include "DupTree.h"

/*
 * CONTEXT enumeration: lr_context_t
 */
typedef enum { LR_undef, LR_allocorreuse, LR_condargs, LR_doargs, LR_recap } lr_context_t;

/*
 * INFO structure
 */
struct INFO {
    lr_context_t context;
    node *preassign;
    node *fundef;
    node *apargs;
    DFMmask_t apmask;
    DFMmask_t reusemask;
};

/*
 * INFO macros
 */
#define INFO_EMLR_CONTEXT(n) (n->context)
#define INFO_EMLR_PREASSIGN(n) (n->preassign)
#define INFO_EMLR_FUNDEF(n) (n->fundef)
#define INFO_EMLR_REUSEMASK(n) (n->reusemask)
#define INFO_EMLR_APARGS(n) (n->apargs)
#define INFO_EMLR_APMASK(n) (n->apmask)

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_EMLR_CONTEXT (result) = LR_undef;
    INFO_EMLR_PREASSIGN (result) = NULL;
    INFO_EMLR_FUNDEF (result) = fundef;
    INFO_EMLR_REUSEMASK (result) = NULL;
    INFO_EMLR_APARGS (result) = NULL;
    INFO_EMLR_APMASK (result) = NULL;

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
 * @fn node *EMLRLoopReuseOptimization( node *arg_node)
 *
 * @brief starting point of Loop Reuse Optimization traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMLRLoopReuseOptimization (node *arg_node)
{
    DBUG_ENTER ("EMLRLoopReuseOptimization");

    DBUG_PRINT ("EMLR", ("Starting Loop Reuse Optimization..."));

    act_tab = emlr_tab;
    arg_node = Trav (arg_node, NULL);

    DBUG_PRINT ("EMLR", ("Loop Reuse Traversal complete."));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static bool
DFMEqualMasks (DFMmask_t mask1, DFMmask_t mask2)
{
    DFMmask_t d1, d2;
    int sum;

    DBUG_ENTER ("DFMEqualMasks");

    d1 = DFMGenMaskMinus (mask1, mask2);
    d2 = DFMGenMaskMinus (mask2, mask1);

    sum = DFMTestMask (d1) + DFMTestMask (d2);

    d1 = DFMRemoveMask (d1);
    d2 = DFMRemoveMask (d2);

    DBUG_RETURN (sum == 0);
}

/******************************************************************************
 *
 * Loop Reuse traversal (emlr_tab)
 *
 * prefix: EMLR
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node *EMLRap( node *arg_node, info *arg_info)
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
EMLRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLRap");

    /*
     * Traverse into special functions
     */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_EMLR_FUNDEF (arg_info))) {
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
    }

    /*
     * Optimize loops
     */
    if ((FUNDEF_IS_LOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_EMLR_FUNDEF (arg_info))) {
        node *doargs;
        node *apargs;

        /*
         * Perform loop optimization using seperate traversal
         */
        act_tab = emlro_tab;
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), NULL);
        act_tab = emlr_tab;

        /*
         * Copy all arguments that can be statically reused inside the loop
         */
        doargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        apargs = AP_ARGS (arg_node);

        while (doargs != NULL) {
            if (!AVIS_ALIAS (ARG_AVIS (doargs))) {
                /*
                 * Insert copy instructions
                 *
                 * b     = do( a);
                 *
                 * is converted into
                 *
                 * a_mem = alloc_or_reuse( dim(a), shape(a), a);
                 * a_val = fill( copy( a), a_mem);
                 *
                 * b      = do( a_val);
                 */
                node *memavis, *valavis, *oldarg, *oldavis;
                ids *lhs, *memarg;

                oldarg = EXPRS_EXPR (apargs);
                oldavis = ID_AVIS (oldarg);

                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info))
                  = MakeVardec (TmpVarName (ID_NAME (oldarg)),
                                DupOneTypes (VARDEC_TYPE (ID_VARDEC (oldarg))),
                                FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info)));

                memavis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info)));
                AVIS_TYPE (memavis) = TYCopyType (AVIS_TYPE (oldavis));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info))
                  = MakeVardec (TmpVarName (ID_NAME (oldarg)),
                                DupOneTypes (VARDEC_TYPE (ID_VARDEC (oldarg))),
                                FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info)));

                valavis = VARDEC_AVIS (FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info)));
                AVIS_TYPE (valavis) = TYCopyType (AVIS_TYPE (oldavis));

                /*
                 * Create fill operation
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 */
                lhs = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (valavis))),
                               NULL, ST_regular);
                IDS_AVIS (lhs) = valavis;
                IDS_VARDEC (lhs) = AVIS_VARDECORARG (valavis);

                memarg = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (memavis))),
                                  NULL, ST_regular);
                IDS_AVIS (memarg) = memavis;
                IDS_VARDEC (memarg) = AVIS_VARDECORARG (memavis);

                INFO_EMLR_PREASSIGN (arg_info)
                  = MakeAssign (MakeLet (MakePrf2 (F_fill,
                                                   MakePrf1 (F_copy, DupNode (oldarg)),
                                                   MakeIdFromIds (memarg)),
                                         lhs),
                                INFO_EMLR_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (IDS_AVIS (lhs)) = INFO_EMLR_PREASSIGN (arg_info);

                /*
                 * Substitute application argument
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                EXPRS_EXPR (apargs) = MakeIdFromIds (DupOneIds (lhs));

                /*
                 * Create allor_or_reuse assignment
                 *
                 * Ex:
                 *   a_mem = allor_or_reuse( dim( a), shape( a), a);
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                lhs = DupOneIds (memarg);

                INFO_EMLR_PREASSIGN (arg_info)
                  = MakeAssign (MakeLet (MakePrf3 (F_alloc_or_reuse,
                                                   MakePrf1 (F_dim, DupNode (oldarg)),
                                                   MakePrf1 (F_shape, DupNode (oldarg)),
                                                   oldarg),
                                         lhs),
                                INFO_EMLR_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (IDS_AVIS (lhs)) = INFO_EMLR_PREASSIGN (arg_info);
            }

            doargs = ARG_NEXT (doargs);
            apargs = EXPRS_NEXT (apargs);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLRassign( node *arg_node, info *arg_info)
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
EMLRassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLRap");

    /*
     * Bottom-up traversal
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_EMLR_PREASSIGN (arg_info) != NULL) {
        INFO_EMLR_PREASSIGN (arg_info)
          = AppendAssign (INFO_EMLR_PREASSIGN (arg_info), arg_node);

        arg_node = INFO_EMLR_PREASSIGN (arg_info);
        INFO_EMLR_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLRfundef( node *arg_node, info *arg_info)
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
EMLRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLRfundef");

    if ((!FUNDEF_IS_LACFUN (arg_node)) || (arg_info != NULL)) {

        DBUG_PRINT ("EMLR", ("Traversing function %s", FUNDEF_NAME (arg_node)));
        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            info = MakeInfo (arg_node);

            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Loop Reuse Optimization traversal (emlro_tab)
 *
 * prefix: EMLRO
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @node *EMLROap( node *arg_node, info *arg_info)
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
EMLROap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLROap");

    if (FUNDEF_IS_CONDFUN (AP_FUNDEF (arg_node))) {
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), arg_info);
    }

    if ((FUNDEF_IS_LOOPFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) == INFO_EMLR_FUNDEF (arg_info))) {
        /*
         * Clean up reusable arguments
         */
        if (FUNDEF_ARGS (INFO_EMLR_FUNDEF (arg_info)) != NULL) {
            INFO_EMLR_CONTEXT (arg_info) = LR_recap;
            INFO_EMLR_APARGS (arg_info) = AP_ARGS (arg_node);

            FUNDEF_ARGS (INFO_EMLR_FUNDEF (arg_info))
              = Trav (FUNDEF_ARGS (INFO_EMLR_FUNDEF (arg_info)), arg_info);

            INFO_EMLR_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROarg( node *arg_node, info *arg_info)
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
EMLROarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLROarg");

    switch (INFO_EMLR_CONTEXT (arg_info)) {
    case LR_doargs:
        if (INFO_EMLR_REUSEMASK (arg_info) == NULL) {
            AVIS_ALIAS (ARG_AVIS (arg_node)) = FALSE;
        } else {
            AVIS_ALIAS (ARG_AVIS (arg_node))
              = !DFMTestMaskEntry (INFO_EMLR_REUSEMASK (arg_info), NULL, arg_node);
        }
        break;

    case LR_condargs:
        if (DFMTestMaskEntry (INFO_EMLR_REUSEMASK (arg_info), NULL, arg_node)) {
            DFMSetMaskEntrySet (INFO_EMLR_APMASK (arg_info), NULL,
                                ID_VARDEC (EXPRS_EXPR (INFO_EMLR_APARGS (arg_info))));
        }

        INFO_EMLR_APARGS (arg_info) = EXPRS_NEXT (INFO_EMLR_APARGS (arg_info));
        break;

    case LR_recap:
        if (AVIS_ALIAS (ID_AVIS (EXPRS_EXPR (INFO_EMLR_APARGS (arg_info))))) {
            DFMSetMaskEntryClear (INFO_EMLR_REUSEMASK (arg_info), NULL, arg_node);
        }

        INFO_EMLR_APARGS (arg_info) = EXPRS_NEXT (INFO_EMLR_APARGS (arg_info));
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context!");
        break;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROfundef( node *arg_node, info *arg_info)
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
EMLROfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLROfundef");

    DBUG_ASSERT (FUNDEF_IS_LACFUN (arg_node),
                 "EMLROfundef is only applicable for LAC-functions");

    if (FUNDEF_IS_LOOPFUN (arg_node)) {
        info *info;
        node *fundef_next;
        DFMmask_base_t maskbase;
        DFMmask_t oldmask;

        DBUG_PRINT ("EMLR", ("Optimizing %s", FUNDEF_NAME (arg_node)));

        /*
         * Traversal of DOFUN
         */

        info = MakeInfo (arg_node);
        maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        /*
         * rescue FUNDEF_NEXT
         */
        fundef_next = FUNDEF_NEXT (arg_node);

        /*
         * Filter reuse candidates
         */
        FUNDEF_NEXT (arg_node) = NULL;
        arg_node = EMFRCFilterReuseCandidates (arg_node);

        /*
         * Initialize arguments' AVIS_ALIAS with values from REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_EMLR_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
            INFO_EMLR_CONTEXT (info) = LR_undef;
        }

        /*
         * Perform alias analysis
         */
        arg_node = EMAAAliasAnalysis (arg_node);

        INFO_EMLR_REUSEMASK (info) = DFMGenMaskClear (maskbase);
        oldmask = DFMGenMaskClear (maskbase);

        while (TRUE) {
            DFMSetMaskCopy (oldmask, INFO_EMLR_REUSEMASK (info));

            /*
             * Traverse body in order to determine REUSEMASK
             */
            FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

            if (DFMEqualMasks (oldmask, INFO_EMLR_REUSEMASK (info))) {
                break;
            }

            /*
             * Perform alias analysis
             */
            arg_node = EMAAAliasAnalysis (arg_node);
        }

        /*
         * Restore FUNDEF_NEXT
         */
        FUNDEF_NEXT (arg_node) = fundef_next;

        oldmask = DFMRemoveMask (oldmask);
        INFO_EMLR_REUSEMASK (info) = DFMRemoveMask (INFO_EMLR_REUSEMASK (info));
        maskbase = DFMRemoveMaskBase (maskbase);
        info = FreeInfo (info);
    } else {
        info *info;
        DFMmask_base_t maskbase;

        /*
         * Traversal of inner CONDFUN
         */
        info = MakeInfo (arg_node);
        maskbase = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        INFO_EMLR_REUSEMASK (info) = DFMGenMaskClear (maskbase);

        /*
         * Traverse function body
         */
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), info);

        /*
         * Transcribe REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_EMLR_APMASK (info) = INFO_EMLR_REUSEMASK (arg_info);
            INFO_EMLR_CONTEXT (info) = LR_condargs;
            FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), info);
            INFO_EMLR_CONTEXT (info) = LR_undef;
        }

        INFO_EMLR_REUSEMASK (info) = DFMRemoveMask (INFO_EMLR_REUSEMASK (info));
        maskbase = DFMRemoveMaskBase (maskbase);
        info = FreeInfo (info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROid( node *arg_node, info *arg_info)
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
EMLROid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLROid");

    switch (INFO_EMLR_CONTEXT (arg_info)) {
    case LR_allocorreuse:
        if (!AVIS_ALIAS (ID_AVIS (arg_node))) {
            DFMSetMaskEntrySet (INFO_EMLR_REUSEMASK (arg_info), NULL,
                                ID_VARDEC (arg_node));
        }
        break;

    case LR_undef:
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context!");
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @node *EMLROprf( node *arg_node, info *arg_info)
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
EMLROprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMLROprf");

    if (PRF_PRF (arg_node) == F_alloc_or_reuse) {
        if (PRF_EXPRS3 (arg_node) != NULL) {
            INFO_EMLR_CONTEXT (arg_info) = LR_allocorreuse;
            PRF_EXPRS3 (arg_node) = Trav (PRF_EXPRS3 (arg_node), arg_info);
            INFO_EMLR_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/* @} */
