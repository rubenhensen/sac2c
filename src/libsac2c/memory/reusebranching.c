/**
 * @defgroup rb Reuse Branching
 *
 * @ingroup mm
 *
 * @{
 */

/**
 * @file reusebranching.c
 *
 * Prefix: EMRB
 */
#include "reusebranching.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "EMRB"
#include "debug.h"

#include "print.h"
#include "infer_dfms.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "free.h"
#include "namespaces.h"
#include "new_types.h"
#include "shape.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *branches;
    node *memvars;
    dfmask_t *drcs;
    dfmask_t *localvars;
    node *precode;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_BRANCHES(n) ((n)->branches)
#define INFO_MEMVARS(n) ((n)->memvars)
#define INFO_DRCS(n) ((n)->drcs)
#define INFO_LOCALVARS(n) ((n)->localvars)
#define INFO_PRECODE(n) ((n)->precode)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_BRANCHES (result) = NULL;
    INFO_MEMVARS (result) = NULL;
    INFO_DRCS (result) = NULL;
    INFO_LOCALVARS (result) = NULL;
    INFO_PRECODE (result) = NULL;

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

    DBUG_ENTER ();

    DBUG_PRINT ("Starting reuse branching");

    info = MakeInfo ();

    TRAVpush (TR_emrb);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_PRINT ("Reuse branching complete.");

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   char *CreateLacFunName( char *suffix)
 *
 * description:
 *   Creates a new name for a LaC function. 'suffix' should be
 *   "Cond" or "Loop".
 *
 ******************************************************************************/

static char *
CreateLacFunName (char *root_funname)
{
    static int number = 0;
    char *name;

    DBUG_ENTER ();

    name = (char *)MEMmalloc ((STRlen (root_funname) + 10 + 20 + 3) * sizeof (char));
    sprintf (name, "%s__ReuseCond_%i", root_funname, number);
    number++;

    DBUG_RETURN (name);
}

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

    DBUG_ENTER ();

    if ((PRF_PRF (memop) == F_alloc_or_reuse) || (PRF_PRF (memop) == F_alloc_or_reshape)
        || (PRF_PRF (memop) == F_alloc_or_resize)) {
        node *rcs = PRF_EXPRS3 (memop);

        while (rcs != NULL) {
            node *rc = EXPRS_EXPR (rcs);

            if (DFMtestMaskEntry (drcs, ID_AVIS (rc))) {
                res = TCappendExprs (res, TBmakeExprs (DUPdoDupNode (rc), NULL));
            }
            rcs = EXPRS_NEXT (rcs);
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildCondTree( node *ass,
 *                                 node *branches,
 *                                 node *memvars,
 *                                 node *fundef,
 *                                 char *root_funname,
 *                                 dfmask_t *inmask,
 *                                 lut_t *lut)
 *
 * @brief Build the reuse decision tree
 *
 * @param ass The assignment chain to be copied
 * @param branches id list list of data reuse candidates
 * @param memvars id list of memory veriables
 * @param fundef Fundef to put the new vardecs into
 * @param root_funname Name of root function
 * @param inmask DFM containing the in parameters of the assignment chain
 * @param lut LUT
 * @return Assignment chain
 *
 *****************************************************************************/
static node *
BuildCondTree (node *ass, node *branches, node *memvars, node *fundef, char *root_funname,
               dfmask_t *inmask, lut_t *lut)
{
    node *res = NULL;

    DBUG_ENTER ();

    if (branches == NULL) {
        DBUG_PRINT ("No branches to process, duplicating the tree...");
        res = DUPdoDupTreeLutSsa (ass, lut, fundef);
    } else {
        if (EXPRS_EXPR (branches) == NULL) {
            res = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars), fundef,
                                 root_funname, inmask, lut);
            DBUG_PRINT ("Finished processing branches...");
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
            node *cfrets;
            node *cfargs;
            lut_t *cflut, *tmplut;

            DBUG_PRINT ("Building conditional tree...");

            /*
             * Create condfun return types
             */
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
                assids = IDS_NEXT (assids);
            }

            /*
             * Create condfun
             */
            cflut = LUTgenerateLut ();
            cfargs = DFMUdfm2Args (inmask, cflut);
            cfargs = TBmakeArg (TBmakeAvis (TRAVtmpVar (),
                                            TYmakeAKS (TYmakeSimpleType (T_bool),
                                                       SHmakeShape (0))),
                                cfargs);

            condfun = TBmakeFundef (CreateLacFunName (root_funname),
                                    NSdupNamespace (FUNDEF_NS (fundef)), cfrets, cfargs,
                                    TBmakeBlock (NULL, NULL), FUNDEF_NEXT (fundef));

            FUNDEF_NEXT (fundef) = condfun;
            FUNDEF_ISCONDFUN (condfun) = TRUE;

            /*
             * Build all parts  of the conditional
             */
            cond = TBmakeId (ARG_AVIS (cfargs));

            tmplut = LUTduplicateLut (cflut);
            thenass = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars),
                                     condfun, root_funname, inmask, tmplut);
            tmplut = LUTremoveLut (tmplut);

            /*
             * Save current reuse candidate for later usage
             */
            rc = EXPRS_EXPR (branches);
            EXPRS_EXPR (branches) = EXPRS_NEXT (EXPRS_EXPR (branches));
            elseass = BuildCondTree (ass, branches, memvars, condfun, root_funname,
                                     inmask, cflut);
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
                cavis = TBmakeAvis (TRAVtmpVar (),
                                    TYcopyType (AVIS_TYPE (IDS_AVIS (thenids))));

                FUNDEF_VARDECS (condfun) = TBmakeVardec (cavis, FUNDEF_VARDECS (condfun));

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
            FUNDEF_ASSIGNS (condfun) = res;
            FUNDEF_RETURN (condfun) = ret;
            res = NULL;

            DBUG_PRINT ("Created conditional function:");
            DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, condfun));

            /*
             * Create pattern:
             * c' = alloc( 0, []);
             * c  = fill( isreused( a, b), c');
             * r1, ... = reusecond( c, ...)
             */

            /*
             * Create variable c'
             */
            memavis = TBmakeAvis (TRAVtmpVarName ("mem"),
                                  TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

            FUNDEF_VARDECS (fundef) = TBmakeVardec (memavis, FUNDEF_VARDECS (fundef));

            memids = TBmakeIds (memavis, NULL);

            /*
             * Create variable c
             */
            valavis = TBmakeAvis (TRAVtmpVarName ("val"),
                                  TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));

            FUNDEF_VARDECS (fundef) = TBmakeVardec (valavis, FUNDEF_VARDECS (fundef));

            valids = TBmakeIds (valavis, NULL);

            /*
             * Create variables returned by condfun
             */
            cfids = NULL;
            while (cfrets != NULL) {
                node *cfavis;

                cfavis = TBmakeAvis (TRAVtmpVar (), TYcopyType (RET_TYPE (cfrets)));

                FUNDEF_VARDECS (fundef) = TBmakeVardec (cfavis, FUNDEF_VARDECS (fundef));

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

            DBUG_PRINT ("Created N_assign for conditional");

            while (cfids != NULL) {
                AVIS_SSAASSIGN (IDS_AVIS (cfids)) = res;
                cfids = IDS_NEXT (cfids);
            }

            /*
             * Create  c  = fill( isreused( a, b), c');
             */
            res = TBmakeAssign (
              TBmakeLet (valids,
                         TCmakePrf2 (F_fill,
                                     TCmakePrf2 (F_isreused,
                                                 DUPdoDupTreeLutSsa (EXPRS_EXPR (memvars),
                                                                     lut, fundef),
                                                 DUPdoDupTreeLutSsa (EXPRS_EXPR (rc), lut,
                                                                     fundef)),
                                     TBmakeId (memavis))),
              res);
            AVIS_SSAASSIGN (IDS_AVIS (valids)) = res;

            DBUG_PRINT ("Created N_assign for fill N_prt");

            /*
             * Create c' = alloc( 0, []);
             */
            res = TBmakeAssign (TBmakeLet (memids,
                                           TCmakePrf2 (F_alloc, TBmakeNum (0),
                                                       TCcreateZeroVector (0, T_int))),
                                res);
            AVIS_SSAASSIGN (IDS_AVIS (memids)) = res;

            DBUG_PRINT ("Created N_assign for alloc N_prt");
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
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    /*
     * In case this assigment was holding some memory operations for a
     * with-loop, ASSIGN_STMT can be empty
     */
    if (ASSIGN_STMT (arg_node) == NULL) {
        DBUG_PRINT ("Encountered empty N_assign statement, freeing N_assign...");
        arg_node = FREEdoFreeNode (arg_node);
    } else {
        DBUG_PRINT ("Inspecting assign statement...");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, ASSIGN_STMT (arg_node)););
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
        DBUG_PRINT ("Done inspecting assign statement...");

        if (INFO_BRANCHES (arg_info) != NULL) {
            node *next, *newass, *lastass;
            node *lhs;
            dfmask_t *inmask;
            lut_t *lut;

            DBUG_PRINT ("Found reuse branches, building a cond tree...");

            next = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (arg_node) = NULL;

            INFO_PRECODE (arg_info) = TCappendAssign (INFO_PRECODE (arg_info), arg_node);

            /*
             * Determine in parameters of the assignment
             */
            lut = LUTgenerateLut ();
            inmask = INFDFMSdoInferInDfmAssignChain (INFO_PRECODE (arg_info),
                                                     INFO_FUNDEF (arg_info));

            newass = BuildCondTree (INFO_PRECODE (arg_info), INFO_BRANCHES (arg_info),
                                    INFO_MEMVARS (arg_info), INFO_FUNDEF (arg_info),
                                    FUNDEF_NAME (INFO_FUNDEF (arg_info)), inmask, lut);

            INFO_BRANCHES (arg_info) = FREEdoFreeTree (INFO_BRANCHES (arg_info));
            INFO_MEMVARS (arg_info) = FREEdoFreeTree (INFO_MEMVARS (arg_info));

            inmask = DFMremoveMask (inmask);
            lut = LUTremoveLut (lut);

            /*
             * Replace LHS of new assignment in order to match current funcion
             */
            DBUG_PRINT ("New assignments: ");
            DBUG_EXECUTE (PRTdoPrintFile (stderr, newass));

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
            INFO_PRECODE (arg_info) = FREEdoFreeTree (INFO_PRECODE (arg_info));
            arg_node = TCappendAssign (newass, next);
        }
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
    DBUG_ENTER ();

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("Traversing function %s", FUNDEF_NAME (arg_node));

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_DFM_BASE (arg_node)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));

        DBUG_PRINT ("Traversal of function %s complete", FUNDEF_NAME (arg_node));
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
    DBUG_ENTER ();

    /*
     * Mark declared variables in LOCALMASK
     */
    if (INFO_LOCALVARS (arg_info) != NULL) {
        DFMsetMaskEntrySet (INFO_LOCALVARS (arg_info), IDS_AVIS (arg_node));
    }

    IDS_NEXT (arg_node) = TRAVopt(IDS_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

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
                drcs = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));
                DFMsetMaskEntrySet (drcs, ID_AVIS (PRF_ARG1 (prf)));

                branches
                  = GetReuseBranches (drcs, ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (mem))));

                drcs = DFMremoveMask (drcs);

                /*
                 * Add branches to INFO_BRANCHES if there are any
                 */
                if (branches != NULL) {
                    INFO_BRANCHES (arg_info)
                      = TBmakeExprs (branches, INFO_BRANCHES (arg_info));

                    INFO_MEMVARS (arg_info)
                      = TBmakeExprs (DUPdoDupNode (mem), INFO_MEMVARS (arg_info));
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

static void
handleCodeBlock (node *cexprs, info *arg_info)
{
    while (cexprs != NULL) {
        node *cid;
        node *wlass;
        node *rhs;
        node *mem;
        node *memop;
        node *val;
        node *cval;
        node *memval;

        cid = EXPRS_EXPR (cexprs);
        wlass = AVIS_SSAASSIGN (ID_AVIS (cid));

        if (wlass != NULL) {
            rhs = ASSIGN_RHS (wlass);
            if (NODE_TYPE (rhs) == N_prf) {
                switch (PRF_PRF (rhs)) {
                case F_fill:
                    /*
                     * Search for suballoc situation
                     *
                     *   a  = with ...
                     *   m' = suballoc( A, iv);
                     *   m  = fill( copy( a), m');
                     * }: m
                     */
                    val = PRF_ARG1 (rhs);
                    mem = PRF_ARG2 (rhs);
                    if ((NODE_TYPE (val) == N_prf) && (PRF_PRF (val) == F_copy)) {
                        cval = PRF_ARG1 (val);
                        memop = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (ID_AVIS (mem))));
                        if ((PRF_PRF (memop) == F_suballoc)
                            && (DFMtestMaskEntry (INFO_LOCALVARS (arg_info),
                                                  ID_AVIS (cval)))
                            && ((NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (cval))))
                                 == N_with)
                                || (NODE_TYPE (
                                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (cval))))
                                    == N_with2)
                                || (NODE_TYPE (
                                      ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (cval))))
                                    == N_with3))) {
                            /*
                             * Full branch in order to be able to move suballoc upwards
                             */
                            DFMsetMaskSet (INFO_DRCS (arg_info));
                        }
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
                    if (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (rhs))) != NULL) {
                        memval = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (rhs))));
                        if ((NODE_TYPE (memval) == N_prf) && (PRF_PRF (memval) == F_fill)
                            && (NODE_TYPE (PRF_ARG1 (memval)) == N_prf)
                            && ((PRF_PRF (PRF_ARG1 (memval)) == F_sel_VxA)
                                || (PRF_PRF (PRF_ARG1 (memval)) == F_idx_sel))) {
                            DFMsetMaskEntrySet (INFO_DRCS (arg_info),
                                                ID_AVIS (PRF_ARG2 (PRF_ARG1 (memval))));
                        }
                    }
                    break;

                default:
                    DBUG_PRINT ("No suballoc or wl_assign found: Fold-Withop?");
                    break;
                }
            }
        }
        cexprs = EXPRS_NEXT (cexprs);
    }
}

node *
EMRBrange (node *arg_node, info *arg_info)
{
    dfmask_t *oldlocals;
    node *cexprs;

    DBUG_ENTER ();

    /*
     * stack local indentifiers
     */
    oldlocals = INFO_LOCALVARS (arg_info);
    INFO_LOCALVARS (arg_info) =
        DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    cexprs = RANGE_RESULTS (arg_node);

    handleCodeBlock (cexprs, arg_info);

    /*
     * restore local identifiers
     */
    INFO_LOCALVARS (arg_info) = DFMremoveMask (INFO_LOCALVARS (arg_info));
    INFO_LOCALVARS (arg_info) = oldlocals;

    /*
     * Traverse next
     */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

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

    DBUG_ENTER ();

    /*
     * stack local indentifiers
     */
    oldlocals = INFO_LOCALVARS (arg_info);
    INFO_LOCALVARS (arg_info) =
        DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    CODE_CBLOCK (arg_node) = TRAVopt(CODE_CBLOCK (arg_node), arg_info);

    cexprs = CODE_CEXPRS (arg_node);

    handleCodeBlock (cexprs, arg_info);

    /*
     * restore local identifiers
     */
    INFO_LOCALVARS (arg_info) = DFMremoveMask (INFO_LOCALVARS (arg_info));
    INFO_LOCALVARS (arg_info) = oldlocals;

    /*
     * Traverse next code
     */
    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

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

    DBUG_ENTER ();

    /*
     * Stack outer data reuse candidates
     */
    olddrcs = INFO_DRCS (arg_info);
    INFO_DRCS (arg_info) =
        DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    /*
     * restore outer data reuse candidates
     */
    INFO_DRCS (arg_info) = DFMremoveMask (INFO_DRCS (arg_info));
    INFO_DRCS (arg_info) = olddrcs;

    if (INFO_BRANCHES (arg_info) != NULL) {
        WITH_WITHID (arg_node) = TRAVdo (WITH_WITHID (arg_node), arg_info);
    }

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

    DBUG_ENTER ();

    /*
     * Stack outer data reuse candidates
     */
    olddrcs = INFO_DRCS (arg_info);
    INFO_DRCS (arg_info) =
        DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    /*
     * restore outer data reuse candidates
     */
    INFO_DRCS (arg_info) = DFMremoveMask (INFO_DRCS (arg_info));
    INFO_DRCS (arg_info) = olddrcs;

    if (INFO_BRANCHES (arg_info) != NULL) {
        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *EMRBwith3( node *arg_node, info *arg_info)
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
EMRBwith3 (node *arg_node, info *arg_info)
{
    dfmask_t *olddrcs;

    DBUG_ENTER ();

    /*
     * Stack outer data reuse candidates
     */
    olddrcs = INFO_DRCS (arg_info);
    INFO_DRCS (arg_info) =
        DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

    WITH3_RANGES (arg_node) = TRAVdo (WITH3_RANGES (arg_node), arg_info);
    WITH3_OPERATIONS (arg_node) = TRAVdo (WITH3_OPERATIONS (arg_node), arg_info);

    /*
     * restore outer data reuse candidates
     */
    INFO_DRCS (arg_info) = DFMremoveMask (INFO_DRCS (arg_info));
    INFO_DRCS (arg_info) = olddrcs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *StealLet( node *assign)
 *
 * @brief
 *
 * @param assign
 *
 * @return arg_node
 *
 *****************************************************************************/
static node *
StealLet (node *assign)
{
    node *let;
    node *res;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (ASSIGN_STMT (assign)) == N_let, "N_assign does not contain N_let!");

    let = ASSIGN_STMT (assign);
    ASSIGN_STMT (assign) = NULL;

    res = TBmakeAssign (let, NULL);
    AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (let))) = res;

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBwithid( node *arg_node, info *arg_info)
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
EMRBwithid (node *arg_node, info *arg_info)
{
    node *id;
    node *exprs;

    DBUG_ENTER ();

    id = WITHID_VEC (arg_node);
    INFO_PRECODE (arg_info) = TCappendAssign (INFO_PRECODE (arg_info),
                                              StealLet (AVIS_SSAASSIGN (ID_AVIS (id))));

    exprs = WITHID_IDS (arg_node);
    while (exprs != NULL) {
        id = EXPRS_EXPR (exprs);
        INFO_PRECODE (arg_info)
          = TCappendAssign (INFO_PRECODE (arg_info),
                            StealLet (AVIS_SSAASSIGN (ID_AVIS (id))));

        exprs = EXPRS_NEXT (exprs);
    }

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

    DBUG_ENTER ();

    branches = GetReuseBranches (INFO_DRCS (arg_info),
                                 ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (mem))));

    /*
     * Add branches to INFO_BRANCHES if there are any
     */
    if (branches != NULL) {
        INFO_BRANCHES (arg_info) = TBmakeExprs (branches, INFO_BRANCHES (arg_info));

        INFO_MEMVARS (arg_info)
          = TBmakeExprs (DUPdoDupNode (mem), INFO_MEMVARS (arg_info));
    }

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    MakeWithopReuseBranches (GENARRAY_MEM (arg_node), arg_info);

    GENARRAY_NEXT (arg_node) = TRAVopt(GENARRAY_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    MakeWithopReuseBranches (MODARRAY_MEM (arg_node), arg_info);

    MODARRAY_NEXT (arg_node) = TRAVopt(MODARRAY_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*@}*/

#undef DBUG_PREFIX
