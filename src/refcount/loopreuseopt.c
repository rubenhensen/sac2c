/*
 *
 * $Log$
 * Revision 1.5  2004/11/24 14:04:08  ktr
 * MakeLet permutation.
 *
 * Revision 1.4  2004/11/23 22:20:18  ktr
 * COMPILES!!!
 *
 * Revision 1.3  2004/11/09 19:38:11  ktr
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
#include "loopreuseopt.h"

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
#include "internal_lib.h"

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
    dfmask_t *apmask;
    dfmask_t *reusemask;
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

    result = ILIBmalloc (sizeof (info));

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

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMLRdoLoopReuseOptimization( node *arg_node)
 *
 * @brief starting point of Loop Reuse Optimization traversal
 *
 * @param arg_node
 *
 * @return
 *
 *****************************************************************************/
node *
EMLRdoLoopReuseOptimization (node *arg_node)
{
    DBUG_ENTER ("EMLRdoLoopReuseOptimization");

    DBUG_PRINT ("EMLR", ("Starting Loop Reuse Optimization..."));

    TRAVpush (TR_emlr);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("EMLR", ("Loop Reuse Traversal complete."));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static bool
DFMequalMasks (dfmask_t *mask1, dfmask_t *mask2)
{
    dfmask_t *d1, *d2;
    int sum;

    DBUG_ENTER ("DFMEqualMasks");

    d1 = DFMgenMaskMinus (mask1, mask2);
    d2 = DFMgenMaskMinus (mask2, mask1);

    sum = DFMtestMask (d1) + DFMtestMask (d2);

    d1 = DFMremoveMask (d1);
    d2 = DFMremoveMask (d2);

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
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_EMLR_FUNDEF (arg_info))) {
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    /*
     * Optimize loops
     */
    if ((FUNDEF_ISDOFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_EMLR_FUNDEF (arg_info))) {
        node *doargs;
        node *apargs;

        /*
         * Perform loop optimization using seperate traversal
         */
        TRAVpush (TR_emlro);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), NULL);
        TRAVpop ();

        /*
         * Copy all arguments that can be statically reused inside the loop
         */
        doargs = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        apargs = AP_ARGS (arg_node);

        while (doargs != NULL) {
            if (!AVIS_ISALIAS (ARG_AVIS (doargs))) {
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

                oldarg = EXPRS_EXPR (apargs);
                oldavis = ID_AVIS (oldarg);

                /*
                 * Create a new memory variable
                 * Ex: a_mem
                 */
                memavis = TBmakeAvis (ILIBtmpVarName (ID_NAME (oldarg)),
                                      TYcopyType (AVIS_TYPE (ID_AVIS (oldavis))));

                FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info))
                  = TBmakeVardec (memavis, FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info)));

                /*
                 * Create a new value variable
                 * Ex: a_val
                 */
                valavis = TBmakeAvis (ILIBtmpVarName (ID_NAME (oldarg)),
                                      TYcopyType (AVIS_TYPE (oldavis)));

                FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info))
                  = TBmakeVardec (valavis, FUNDEF_VARDEC (INFO_EMLR_FUNDEF (arg_info)));

                /*
                 * Create fill operation
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 */
                INFO_EMLR_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (valavis, NULL),
                                             TCmakePrf2 (F_fill,
                                                         TCmakePrf1 (F_copy,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         TBmakeId (memavis))),
                                  INFO_EMLR_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (valavis) = INFO_EMLR_PREASSIGN (arg_info);

                /*
                 * Substitute application argument
                 *
                 * Ex:
                 *   ...
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                EXPRS_EXPR (apargs) = TBmakeId (valavis);

                /*
                 * Create allor_or_reuse assignment
                 *
                 * Ex:
                 *   a_mem = allor_or_reuse( dim( a), shape( a), a);
                 *   a_val = fill( copy( a), a_mem);
                 *   b     = do( a_val);
                 */
                INFO_EMLR_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (memavis, NULL),
                                             TCmakePrf3 (F_alloc_or_reuse,
                                                         TCmakePrf1 (F_dim, DUPdoDupNode (
                                                                              oldarg)),
                                                         TCmakePrf1 (F_shape,
                                                                     DUPdoDupNode (
                                                                       oldarg)),
                                                         oldarg)),
                                  INFO_EMLR_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (memavis) = INFO_EMLR_PREASSIGN (arg_info);
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
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_EMLR_PREASSIGN (arg_info) != NULL) {
        INFO_EMLR_PREASSIGN (arg_info)
          = TCappendAssign (INFO_EMLR_PREASSIGN (arg_info), arg_node);

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

    if ((!FUNDEF_ISLACFUN (arg_node)) || (arg_info != NULL)) {

        DBUG_PRINT ("EMLR", ("Traversing function %s", FUNDEF_NAME (arg_node)));
        if (FUNDEF_BODY (arg_node) != NULL) {
            info *info;
            info = MakeInfo (arg_node);

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            info = FreeInfo (info);
        }
    }

    if (arg_info == NULL) {
        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
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

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        INFO_EMLR_APARGS (arg_info) = AP_ARGS (arg_node);
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
    }

    if ((FUNDEF_ISDOFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) == INFO_EMLR_FUNDEF (arg_info))) {
        /*
         * Clean up reusable arguments
         */
        if (FUNDEF_ARGS (INFO_EMLR_FUNDEF (arg_info)) != NULL) {
            INFO_EMLR_CONTEXT (arg_info) = LR_recap;
            INFO_EMLR_APARGS (arg_info) = AP_ARGS (arg_node);

            FUNDEF_ARGS (INFO_EMLR_FUNDEF (arg_info))
              = TRAVdo (FUNDEF_ARGS (INFO_EMLR_FUNDEF (arg_info)), arg_info);

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
            AVIS_ISALIAS (ARG_AVIS (arg_node)) = FALSE;
        } else {
            AVIS_ISALIAS (ARG_AVIS (arg_node))
              = !DFMtestMaskEntry (INFO_EMLR_REUSEMASK (arg_info), NULL,
                                   ARG_AVIS (arg_node));
        }
        break;

    case LR_condargs:
        if (DFMtestMaskEntry (INFO_EMLR_REUSEMASK (arg_info), NULL,
                              ARG_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_EMLR_APMASK (arg_info), NULL,
                                ID_AVIS (EXPRS_EXPR (INFO_EMLR_APARGS (arg_info))));
        }

        INFO_EMLR_APARGS (arg_info) = EXPRS_NEXT (INFO_EMLR_APARGS (arg_info));
        break;

    case LR_recap:
        if (AVIS_ISALIAS (ID_AVIS (EXPRS_EXPR (INFO_EMLR_APARGS (arg_info))))) {
            DFMsetMaskEntryClear (INFO_EMLR_REUSEMASK (arg_info), NULL,
                                  ARG_AVIS (arg_node));
        }

        INFO_EMLR_APARGS (arg_info) = EXPRS_NEXT (INFO_EMLR_APARGS (arg_info));
        break;

    default:
        DBUG_ASSERT ((0), "Illegal context!");
        break;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
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

    DBUG_ASSERT (FUNDEF_ISLACFUN (arg_node),
                 "EMLROfundef is only applicable for LAC-functions");

    if (FUNDEF_ISDOFUN (arg_node)) {
        info *info;
        node *fundef_next;
        dfmask_base_t *maskbase;
        dfmask_t *oldmask;

        DBUG_PRINT ("EMLR", ("Optimizing %s", FUNDEF_NAME (arg_node)));

        /*
         * Traversal of DOFUN
         */

        info = MakeInfo (arg_node);
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        /*
         * rescue FUNDEF_NEXT
         */
        fundef_next = FUNDEF_NEXT (arg_node);

        /*
         * Filter reuse candidates
         */
        FUNDEF_NEXT (arg_node) = NULL;
        arg_node = EMFRCdoFilterReuseCandidates (arg_node);

        /*
         * Initialize arguments' AVIS_ALIAS with values from REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_EMLR_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_EMLR_CONTEXT (info) = LR_undef;
        }

        /*
         * Perform alias analysis
         */
        arg_node = EMAAdoAliasAnalysis (arg_node);

        INFO_EMLR_REUSEMASK (info) = DFMgenMaskClear (maskbase);
        oldmask = DFMgenMaskClear (maskbase);

        while (TRUE) {
            DFMsetMaskCopy (oldmask, INFO_EMLR_REUSEMASK (info));

            /*
             * Traverse body in order to determine REUSEMASK
             */
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

            if (DFMequalMasks (oldmask, INFO_EMLR_REUSEMASK (info))) {
                break;
            }

            /*
             * Perform alias analysis
             */
            arg_node = EMAAdoAliasAnalysis (arg_node);
        }

        /*
         * Print statically resusable variables
         */
        DBUG_PRINT ("EMLR", ("The following variables can be statically reused:"));
        DBUG_EXECUTE ("EMLR",
                      DFMprintMask (global.outfile, "%s\n", INFO_EMLR_REUSEMASK (info)););

        /*
         * Initialize arguments' AVIS_ALIAS with values from REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_EMLR_CONTEXT (info) = LR_doargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_EMLR_CONTEXT (info) = LR_undef;
        }

        /*
         * Restore FUNDEF_NEXT
         */
        FUNDEF_NEXT (arg_node) = fundef_next;

        oldmask = DFMremoveMask (oldmask);
        INFO_EMLR_REUSEMASK (info) = DFMremoveMask (INFO_EMLR_REUSEMASK (info));
        maskbase = DFMremoveMaskBase (maskbase);
        info = FreeInfo (info);
    } else {
        info *info;
        dfmask_base_t *maskbase;

        /*
         * Traversal of inner CONDFUN
         */
        info = MakeInfo (arg_node);
        maskbase = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
        INFO_EMLR_REUSEMASK (info) = DFMgenMaskClear (maskbase);

        /*
         * Traverse function body
         */
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);

        /*
         * Transcribe REUSEMASK
         */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            INFO_EMLR_APMASK (info) = INFO_EMLR_REUSEMASK (arg_info);
            INFO_EMLR_APARGS (info) = INFO_EMLR_APARGS (arg_info);
            INFO_EMLR_APARGS (arg_info) = NULL;
            INFO_EMLR_CONTEXT (info) = LR_condargs;
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), info);
            INFO_EMLR_CONTEXT (info) = LR_undef;
        }

        INFO_EMLR_REUSEMASK (info) = DFMremoveMask (INFO_EMLR_REUSEMASK (info));
        maskbase = DFMremoveMaskBase (maskbase);
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
        if (!AVIS_ISALIAS (ID_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_EMLR_REUSEMASK (arg_info), NULL, ID_AVIS (arg_node));
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
            PRF_EXPRS3 (arg_node) = TRAVdo (PRF_EXPRS3 (arg_node), arg_info);
            INFO_EMLR_CONTEXT (arg_info) = LR_undef;
        }
    }

    DBUG_RETURN (arg_node);
}

/* @} */
