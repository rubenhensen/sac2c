/*
 *
 * $Log$
 * Revision 1.14  2004/12/13 18:54:49  ktr
 * Withids contain N_id/N_exprs of N_id after explicit allocation now.
 *
 * Revision 1.13  2004/12/13 13:55:59  ktr
 * added functionality to branch over with-loops (work in progress)
 *
 * Revision 1.12  2004/12/01 16:36:22  ktr
 * post DK bugfix
 *
 * ,
 *
 * Revision 1.11  2004/11/26 16:33:58  jhb
 * compile
 *
 * Revision 1.10  2004/11/25 18:04:36  jhb
 * INFDFMSdoInterInDFMAssignChain changed to INFDFMSdoInterInDfmAssignChain
 *
 * Revision 1.9  2004/11/24 14:06:43  ktr
 * MakeLet permutation.
 *
 * Revision 1.8  2004/11/24 12:57:12  ktr
 * COMPILES!!!
 *
 * Revision 1.7  2004/11/24 11:26:46  jhb
 * ismop
 *
 * Revision 1.6  2004/11/19 15:42:41  ktr
 * Support for F_alloc_or_reshape added.
 *
 * Revision 1.5  2004/11/18 17:05:18  ktr
 * ongoing work
 *
 * Revision 1.4  2004/11/17 11:38:14  ktr
 * ongoing implementation
 *
 * Revision 1.3  2004/11/17 09:06:01  ktr
 * Ongoing implementation
 *
 * Revision 1.2  2004/11/15 12:29:30  ktr
 * ongoing implementation
 *
 * Revision 1.1  2004/11/14 13:43:35  ktr
 * Initial revision
 *
 */

/**
 * @defgroup rb Reuse Branching
 * @ingroup emm
 *
 * @{
 */

/**
 *
 * @file reusebranching.c
 *
 *
 */
#include "reusebranching.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "internal_lib.h"
#include "dbug.h"
#include "print.h"
#include "lac2fun.h"
#include "InferDFMs.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "free.h"
#include "new_types.h"
#include "shape.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *branches;
    node *memvars;
    node *locals;
    dfmask_base_t *maskbase;
    dfmask_t *drcs;
    dfmask_t *localvars;
};

/*
 * INFO macros
 */
#define INFO_RB_FUNDEF(n) ((n)->fundef)
#define INFO_RB_BRANCHES(n) ((n)->branches)
#define INFO_RB_MEMVARS(n) ((n)->memvars)
#define INFO_RB_LOCALS(n) ((n)->locals)
#define INFO_RB_MASKBASE(n) ((n)->maskbase)
#define INFO_RB_DRCS(n) ((n)->drcs)
#define INFO_RB_LOCALVARS(n) ((n)->localvars)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_RB_FUNDEF (result) = NULL;
    INFO_RB_BRANCHES (result) = NULL;
    INFO_RB_MEMVARS (result) = NULL;
    INFO_RB_LOCALS (result) = NULL;
    INFO_RB_MASKBASE (result) = NULL;
    INFO_RB_DRCS (result) = NULL;
    INFO_RB_LOCALVARS (result) = NULL;

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
 * @fn node *EMRBReuseBranching( node *syntax_tree)
 *
 * @brief
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
EMRBdoReuseBranching (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMRBReuseBranching");

    DBUG_PRINT ("EMRB", ("Starting reuse branching"));

    info = MakeInfo ();

    TRAVpush (TR_emrb);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("EMRB", ("Reuse branching complete."));

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *GetReuseBranches( node *drcs, node *memop)
 *
 * @brief Returns sublist of reuse candidates that are data reuse candidates
 *
 * @param drcs Data Reuse Candidates mask
 * @param memop Memory operation
 *
 * @return sublist of reuse candidates that are also data reuse candidates
 *
 *****************************************************************************/
static node *
GetReuseBranches (dfmask_t *drcs, node *memop)
{
    node *res = NULL;

    DBUG_ENTER ("GetReuseBranches");

    if ((PRF_PRF (memop) == F_alloc_or_reuse)
        || (PRF_PRF (memop) == F_alloc_or_reshape)) {
        node *rcs = PRF_EXPRS3 (memop);

        while (rcs != NULL) {
            node *rc = EXPRS_EXPR (rcs);

            if (DFMtestMaskEntry (drcs, NULL, ID_AVIS (rc))) {
                res = TCappendExprs (res, TBmakeExprs (DUPdoDupNode (rc), NULL));
            }
            rcs = EXPRS_NEXT (rcs);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildCondTree( node *let, node *branches, node *locals)
 *
 * @brief Build the reuse decision tree
 *
 * @param ass The assignment chain to be copied
 * @param branches id list list of data reuse candidates
 * @param memvars id list of memory veriables
 * @param fundef Fundef to put the new vardecs into
 * @param inmask DFM containing the in parameters of the assignment chain
 * @param lut LUT
 * @return Assignment chain
 *
 *****************************************************************************/
static node *
BuildCondTree (node *ass, node *branches, node *memvars, node *fundef, dfmask_t *inmask,
               lut_t *lut)
{
    node *res = NULL;

    DBUG_ENTER ("BuildCondTree");

    if (branches == NULL) {
        res = DUPdoDupTreeLutSsa (ass, lut, fundef);
    } else {
        if (EXPRS_EXPR (branches) == NULL) {
            res = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars), fundef,
                                 inmask, lut);
        } else {
            node *condfun;
            node *cond;
            node *thenass;
            node *elseass;
            node *asslast;
            node *thenlast;
            node *elselast;
            node *ret;
            node *retexprs;
            node *rc;
            node *cfap;
            node *memavis, *valavis;
            node *memids, *valids;
            node *thenids, *elseids;
            node *assids;
            node *cfids;
            types *cftypes;
            node *cfrets;
            node *cfargs;
            lut_t *cflut, *tmplut;

            /*
             * Create condfun return types
             */
            cftypes = NULL;
            cfrets = NULL;
            asslast = ass;
            while (ASSIGN_NEXT (asslast) != NULL) {
                asslast = ASSIGN_NEXT (asslast);
            }
            assids = ASSIGN_LHS (asslast);
            while (assids != NULL) {
                cfrets = TCappendRet (cfrets, TBmakeRet (TYeliminateAKV (
                                                           AVIS_TYPE (IDS_AVIS (assids))),
                                                         NULL));

                cftypes = TCappendTypes (cftypes,
                                         TYtype2OldType (AVIS_TYPE (IDS_AVIS (assids))));
                assids = IDS_NEXT (assids);
            }

            /*
             * Create condfun
             */
            cflut = LUTgenerateLut ();
            cfargs = DFMUdfm2Args (inmask, cflut);
            cfargs = TBmakeArg (TBmakeAvis (ILIBtmpVar (),
                                            TYmakeAKS (TYmakeSimpleType (T_bool),
                                                       SHmakeShape (0))),
                                cfargs);

            condfun
              = TBmakeFundef (L2FgetLacFunName ("ReuseCond"), FUNDEF_MOD (fundef), cfrets,
                              cfargs, TBmakeBlock (NULL, NULL), FUNDEF_NEXT (fundef));

            FUNDEF_TYPES (condfun) = cftypes;

            FUNDEF_NEXT (fundef) = condfun;
            FUNDEF_ISCONDFUN (condfun) = TRUE;

            /*
             * Build all parts  of the conditional
             */
            cond = TBmakeId (ARG_AVIS (cfargs));

            tmplut = LUTduplicateLut (cflut);
            thenass = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars),
                                     condfun, inmask, tmplut);
            tmplut = LUTremoveLut (tmplut);

            /*
             * Save current reuse candidate for later usage
             */
            rc = EXPRS_EXPR (branches);
            EXPRS_EXPR (branches) = EXPRS_NEXT (EXPRS_EXPR (branches));
            elseass = BuildCondTree (ass, branches, memvars, condfun, inmask, cflut);
            EXPRS_EXPR (branches) = rc;
            cflut = LUTremoveLut (cflut);

            /*
             * Find last assignments of both blocks
             */
            thenlast = thenass;
            while (ASSIGN_NEXT (thenlast) != NULL) {
                thenlast = ASSIGN_NEXT (thenlast);
            }

            elselast = elseass;
            while (ASSIGN_NEXT (elselast) != NULL) {
                elselast = ASSIGN_NEXT (elselast);
            }

            /*
             * Set up thenids, elseids
             */
            thenids = ASSIGN_LHS (thenlast);
            elseids = ASSIGN_LHS (elselast);

            /*
             * Create funconds
             */
            retexprs = NULL;
            while (thenids != NULL) {
                node *cavis;
                node *cids;
                /*
                 * Create new lhs variable for FUNCOND
                 */
                cavis = TBmakeAvis (ILIBtmpVar (),
                                    TYcopyType (AVIS_TYPE (IDS_AVIS (thenids))));

                FUNDEF_VARDEC (condfun) = TBmakeVardec (cavis, FUNDEF_VARDEC (condfun));

                cids = TBmakeIds (cavis, NULL);

                /*
                 * create FUNCOND
                 */
                res
                  = TBmakeAssign (TBmakeLet (cids,
                                             TBmakeFuncond (DUPdoDupNode (cond),
                                                            TBmakeId (IDS_AVIS (thenids)),
                                                            TBmakeId (
                                                              IDS_AVIS (elseids)))),
                                  res);
                AVIS_SSAASSIGN (IDS_AVIS (cids)) = res;

                /*
                 * Put cids into retexprs
                 * TODO: Check order
                 */
                retexprs = TBmakeExprs (TBmakeId (cavis), retexprs);

                thenids = IDS_NEXT (thenids);
                elseids = IDS_NEXT (elseids);
            }

            /*
             * Append return( retexprs);
             */
            ret = TBmakeReturn (retexprs);
            res = TCappendAssign (res, TBmakeAssign (ret, NULL));

            /*
             * Create conditional
             */
            res = TBmakeAssign (TBmakeCond (cond, TBmakeBlock (thenass, NULL),
                                            TBmakeBlock (elseass, NULL)),
                                res);

            /*
             * Put Assignments into function body
             */
            FUNDEF_INSTR (condfun) = res;
            FUNDEF_RETURN (condfun) = ret;
            res = NULL;

            DBUG_EXECUTE ("EMRB", PRTdoPrintNode (condfun););

            /*
             * Create pattern:
             * c' = alloc( 0, []);
             * c  = fill( isreused( a, b), c');
             * r1, ... = reusecond( c, ...)
             */

            /*
             * Create variable c'
             */
            memavis = TBmakeAvis (ILIBtmpVarName ("mem"),
                                  TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

            FUNDEF_VARDEC (fundef) = TBmakeVardec (memavis, FUNDEF_VARDEC (fundef));

            memids = TBmakeIds (memavis, NULL);

            /*
             * Create variable c
             */
            valavis = TBmakeAvis (ILIBtmpVarName ("val"),
                                  TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

            FUNDEF_VARDEC (fundef) = TBmakeVardec (valavis, FUNDEF_VARDEC (fundef));

            valids = TBmakeIds (valavis, NULL);

            /*
             * Create variables returned by condfun
             */
            cfids = NULL;
            while (cfrets != NULL) {
                node *cfavis;

                cfavis = TBmakeAvis (ILIBtmpVar (), TYcopyType (RET_TYPE (cfrets)));

                FUNDEF_VARDEC (fundef) = TBmakeVardec (cfavis, FUNDEF_VARDEC (fundef));

                cfids = TCappendIds (cfids, TBmakeIds (cfavis, NULL));

                cfrets = RET_NEXT (cfrets);
            }

            /*
             * Create application of condfun
             */
            cfap = TBmakeAp (condfun, TBmakeExprs (TBmakeId (valavis),
                                                   DFMUdfm2ApArgs (inmask, lut)));

            AP_FUNDEF (cfap) = condfun;

            res = TBmakeAssign (TBmakeLet (cfids, cfap), res);

            while (cfids != NULL) {
                AVIS_SSAASSIGN (IDS_AVIS (cfids)) = res;
                cfids = IDS_NEXT (cfids);
            }

            FUNDEF_EXT_ASSIGNS (condfun) = TCnodeListAppend (NULL, res, NULL);
            FUNDEF_USED (condfun) = 1;

            /*
             * Create  c  = fill( isreused( a, b), c');
             */
            res
              = TBmakeAssign (TBmakeLet (valids,
                                         TCmakePrf2 (F_fill,
                                                     TCmakePrf2 (F_isreused,
                                                                 DUPdoDupNode (
                                                                   EXPRS_EXPR (memvars)),
                                                                 DUPdoDupNode (
                                                                   EXPRS_EXPR (rc))),
                                                     TBmakeId (memavis))),
                              res);
            AVIS_SSAASSIGN (IDS_AVIS (valids)) = res;

            /*
             * Create c' = alloc( 0, []);
             */
            res = TBmakeAssign (TBmakeLet (memids,
                                           TCmakePrf2 (F_alloc, TBmakeNum (0),
                                                       TCcreateZeroVector (0, T_int))),
                                res);
            AVIS_SSAASSIGN (IDS_AVIS (memids)) = res;

            DBUG_EXECUTE ("EMRB", PRTdoPrint (res););
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * Reuse branching traversal (emrc_tab)
 *
 * prefix: EMRB
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *EMRBassign( node *arg_node, info *arg_info)
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
EMRBassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_RB_BRANCHES (arg_info) != NULL) {
        node *next, *newass, *lastass;
        node *lhs;
        dfmask_t *inmask;
        lut_t *lut;

        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        /*
         * Determine in parameters of the assignment
         */
        lut = LUTgenerateLut ();
        inmask = INFDFMSdoInferInDfmAssignChain (arg_node, INFO_RB_FUNDEF (arg_info));

        newass = BuildCondTree (arg_node, INFO_RB_BRANCHES (arg_info),
                                INFO_RB_MEMVARS (arg_info), INFO_RB_FUNDEF (arg_info),
                                inmask, lut);

        INFO_RB_BRANCHES (arg_info) = FREEdoFreeTree (INFO_RB_BRANCHES (arg_info));
        INFO_RB_MEMVARS (arg_info) = FREEdoFreeTree (INFO_RB_MEMVARS (arg_info));

        FUNDEF_DFM_BASE (INFO_RB_FUNDEF (arg_info))
          = DFMremoveMaskBase (FUNDEF_DFM_BASE (INFO_RB_FUNDEF (arg_info)));
        inmask = DFMremoveMask (inmask);
        lut = LUTremoveLut (lut);

        /*
         * Replace LHS of new assignment in order to match current funcion
         */
        lastass = newass;
        while (ASSIGN_NEXT (lastass) != NULL) {
            lastass = ASSIGN_NEXT (lastass);
        }
        ASSIGN_LHS (lastass) = FREEdoFreeTree (ASSIGN_LHS (lastass));
        ASSIGN_LHS (lastass) = ASSIGN_LHS (arg_node);
        ASSIGN_LHS (arg_node) = NULL;

        lhs = ASSIGN_LHS (lastass);
        while (lhs != NULL) {
            AVIS_SSAASSIGN (IDS_AVIS (lhs)) = lastass;
            lhs = IDS_NEXT (lhs);
        }

        /*
         * Put new assignments into assignment chain
         */
        arg_node = FREEdoFreeNode (arg_node);
        arg_node = TCappendAssign (newass, arg_node);
        arg_node = TCappendAssign (arg_node, next);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBfundef( node *arg_node, info *arg_info)
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
EMRBfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("EMRB", ("Traversing function %s", FUNDEF_NAME (arg_node)));

        INFO_RB_FUNDEF (arg_info) = arg_node;
        INFO_RB_MASKBASE (arg_info)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_RB_MASKBASE (arg_info) = DFMremoveMaskBase (INFO_RB_MASKBASE (arg_info));

        DBUG_PRINT ("EMRB",
                    ("Traversal of function %s complete", FUNDEF_NAME (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBids( node *arg_node, info *arg_info)
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
EMRBids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBids");

    /*
     * Mark declared variables in LOCALMASK
     */
    if (INFO_RB_LOCALVARS (arg_info) != NULL) {
        DFMsetMaskEntrySet (INFO_RB_LOCALVARS (arg_info), NULL, IDS_AVIS (arg_node));
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBprf( node *arg_node, info *arg_info)
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
EMRBprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBprf");

    switch (PRF_PRF (arg_node)) {
    case F_fill:
        if (NODE_TYPE (PRF_ARG1 (arg_node)) == N_prf) {
            node *branches;
            node *prf;
            node *mem;
            dfmask_t *drcs;

            prf = PRF_ARG1 (arg_node);
            mem = PRF_ARG2 (arg_node);

            switch (PRF_PRF (prf)) {
            case F_copy:
                /*
                 * a = fill( copy( b), a');
                 *
                 * b is the only data reuse candidate
                 */
                drcs = DFMgenMaskClear (INFO_RB_MASKBASE (arg_info));
                DFMsetMaskEntrySet (drcs, NULL, ID_AVIS (PRF_ARG1 (prf)));

                branches
                  = GetReuseBranches (drcs, ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (mem))));

                drcs = DFMremoveMask (drcs);

                /*
                 * Add branches to INFO_RB_BRANCHES if there are any
                 */
                if (branches != NULL) {
                    INFO_RB_BRANCHES (arg_info)
                      = TBmakeExprs (branches, INFO_RB_BRANCHES (arg_info));

                    INFO_RB_MEMVARS (arg_info)
                      = TBmakeExprs (DUPdoDupNode (mem), INFO_RB_MEMVARS (arg_info));
                }
                break;

            default:
                /*
                 * No reuse branching is required as there cannot be any data reuse
                 */
                break;
            }
        }
        break;

    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBcode( node *arg_node, info *arg_info)
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
EMRBcode (node *arg_node, info *arg_info)
{
    dfmask_t *oldlocals;
    node *cexprs;

    DBUG_ENTER ("EMRBcode");

    /*
     * stack local indentifiers
     */
    oldlocals = INFO_RB_LOCALVARS (arg_info);
    INFO_RB_LOCALVARS (arg_info) = DFMgenMaskClear (INFO_RB_MASKBASE (arg_info));

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    cexprs = CODE_CEXPRS (arg_node);

    while (cexprs != NULL) {
        node *cid = EXPRS_EXPR (cexprs);
        node *wlass = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (cid))));
        node *mem;
        node *memop;
        node *val;
        node *cval;
        node *memval;

        switch (PRF_PRF (wlass)) {
        case F_fill:
            /*
             * Search for suballoc situation
             *
             *   a  = with ...
             *   m' = suballoc( A, iv);
             *   m  = fill( copy( a), m');
             * }: m
             */
            val = PRF_ARG1 (wlass);
            mem = PRF_ARG2 (wlass);
            DBUG_ASSERT ((NODE_TYPE (val) == N_prf) && (PRF_PRF (val) == F_copy),
                         "Illegal code situation");
            cval = PRF_ARG1 (val);
            memop = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (mem))));
            if ((PRF_PRF (memop) == F_suballoc)
                && (DFMtestMaskEntry (INFO_RB_LOCALVARS (arg_info), NULL, ID_AVIS (cval)))
                && ((NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (cval)))) == N_with)
                    || (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (cval))))
                        == N_with2))) {
                /*
                 * Full branch in order to be able to move suballoc upwards
                 */
                DFMsetMaskSet (INFO_RB_DRCS (arg_info));
            }
            break;

        case F_wl_assign:
            /*
             * Search for selection
             *
             *   a = fill( B[...], ...)
             *   r = wl_assign( a, ..., ...);
             * }: r
             *
             * OR
             *
             *   a = fill ( idx_sel( ..., B), ...);
             *   r = wl_assign( a, ..., ...);
             * }: r
             */
            memval = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (wlass))));
            if ((NODE_TYPE (memval) == N_prf) && (PRF_PRF (memval) = F_fill)
                && (NODE_TYPE (PRF_ARG1 (memval)) == N_prf)
                && ((PRF_PRF (PRF_ARG1 (memval)) == F_sel)
                    || (PRF_PRF (PRF_ARG1 (memval)) == F_idx_sel))) {
                DFMsetMaskEntrySet (INFO_RB_DRCS (arg_info), NULL,
                                    ID_AVIS (PRF_ARG2 (PRF_ARG1 (memval))));
            }
            break;

        default:
            DBUG_ASSERT ((0), "Illegal WITH-LOOP assignment found!");
            break;
        }

        cexprs = EXPRS_NEXT (cexprs);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBwith( node *arg_node, info *arg_info)
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
EMRBwith (node *arg_node, info *arg_info)
{
    dfmask_t *olddrcs;

    DBUG_ENTER ("EMRBwith");

    /*
     * Stack outer data reuse candidates
     */
    olddrcs = INFO_RB_DRCS (arg_info);
    INFO_RB_DRCS (arg_info) = DFMgenMaskClear (INFO_RB_MASKBASE (arg_info));

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * restore outer data reuse candidates
     */
    INFO_RB_DRCS (arg_info) = DFMremoveMask (INFO_RB_DRCS (arg_info));
    INFO_RB_DRCS (arg_info) = olddrcs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBwith2( node *arg_node, info *arg_info)
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
EMRBwith2 (node *arg_node, info *arg_info)
{
    dfmask_t *olddrcs;

    DBUG_ENTER ("EMRBwith2");

    /*
     * Stack outer data reuse candidates
     */
    olddrcs = INFO_RB_DRCS (arg_info);
    INFO_RB_DRCS (arg_info) = DFMgenMaskClear (INFO_RB_MASKBASE (arg_info));

    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * restore outer data reuse candidates
     */
    INFO_RB_DRCS (arg_info) = DFMremoveMask (INFO_RB_DRCS (arg_info));
    INFO_RB_DRCS (arg_info) = olddrcs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn void MakeWithopReuseBranches( node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
static void
MakeWithopReuseBranches (node *mem, info *arg_info)
{
    node *branches;

    DBUG_ENTER ("MakeWithopReuseBranches");

    branches = GetReuseBranches (INFO_RB_DRCS (arg_info),
                                 ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (mem))));

    /*
     * Add branches to INFO_RB_BRANCHES if there are any
     */
    if (branches != NULL) {
        INFO_RB_BRANCHES (arg_info) = TBmakeExprs (branches, INFO_RB_BRANCHES (arg_info));

        INFO_RB_MEMVARS (arg_info)
          = TBmakeExprs (DUPdoDupNode (mem), INFO_RB_MEMVARS (arg_info));
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBgenarray( node *arg_node, info *arg_info)
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
EMRBgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBgenarray");

    MakeWithopReuseBranches (GENARRAY_MEM (arg_node), arg_info);

    if (GENARRAY_NEXT (arg_node) != NULL) {
        TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBmodarray( node *arg_node, info *arg_info)
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
EMRBmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBmodarray");

    MakeWithopReuseBranches (MODARRAY_MEM (arg_node), arg_info);

    if (MODARRAY_NEXT (arg_node) != NULL) {
        TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*@}*/
