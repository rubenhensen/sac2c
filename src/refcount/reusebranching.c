/*
 *
 * $Log$
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
#define NEW_INFO

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

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *branches;
    node *memvars;
    node *locals;
};

/*
 * INFO macros
 */
#define INFO_RB_FUNDEF(n) (n->fundef)
#define INFO_RB_BRANCHES(n) (n->branches)
#define INFO_RB_MEMVARS(n) (n->memvars)
#define INFO_RB_LOCALS(n) (n->locals)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_RB_FUNDEF (result) = NULL;
    INFO_RB_BRANCHES (result) = NULL;
    INFO_RB_MEMVARS (result) = NULL;
    INFO_RB_LOCALS (result) = NULL;

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
EMRBReuseBranching (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("EMRBReuseBranching");

    DBUG_PRINT ("EMRB", ("Starting reuse branching"));

    info = MakeInfo ();

    act_tab = emrb_tab;

    syntax_tree = Trav (syntax_tree, info);

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
 * @fn bool IsIdInList( node *id, node *exprs)
 *
 * @param id
 * @param exprs
 *
 * @return Returns whether id is found in exprs.
 *
 *****************************************************************************/
bool
IsIdInList (node *id, node *exprs)
{
    bool res = FALSE;

    DBUG_ENTER ("IsIdInList");

    while (exprs != NULL) {
        if (ID_VARDEC (EXPRS_EXPR (exprs)) == ID_VARDEC (id)) {
            res = TRUE;
            break;
        }
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *GetReuseBranches( node *drcs, node *memop)
 *
 * @brief Returns sublist of reuse candidates that are data reuse candidates
 *
 * @param drcs Data Reuse Candidates ( will be consumed )
 * @param memop Memory operation
 *
 * @return sublist of reuse candidates that are also data reuse candidates
 *
 *****************************************************************************/
static node *
GetReuseBranches (node *drcs, node *memop)
{
    node *res = NULL;

    DBUG_ENTER ("GetReuseBranches");

    if (PRF_PRF (memop) == F_alloc_or_reuse) {
        node *rcs = PRF_EXPRS3 (memop);

        while (rcs != NULL) {
            node *rc = EXPRS_EXPR (rcs);

            if (IsIdInList (rc, drcs)) {
                res = AppendExprs (res, MakeExprs (DupNode (rc), NULL));
            }
            rcs = EXPRS_NEXT (rcs);
        }
    }

    if (drcs != NULL) {
        drcs = FreeTree (drcs);
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
BuildCondTree (node *ass, node *branches, node *memvars, node *fundef, DFMmask_t inmask,
               LUT_t lut)
{
    node *res = NULL;

    DBUG_ENTER ("BuildCondTree");

    if (branches == NULL) {
        res = DupTreeLUTSSA (ass, lut, fundef);
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
            ids *condids;
            ids *memids, *valids;
            ids *thenids, *elseids;
            ids *assids;
            ids *cfids;
            types *cftypes;
            node *cfargs;
            LUT_t cflut, tmplut;
            int i;

            /*
             * Create condfun return types
             */
            cftypes = NULL;
            asslast = ass;
            while (ASSIGN_NEXT (asslast) != NULL) {
                asslast = ASSIGN_NEXT (asslast);
            }
            assids = ASSIGN_LHS (asslast);
            while (assids != NULL) {
                cftypes = AppendTypes (cftypes, DupOneTypes (IDS_TYPE (assids)));
                assids = IDS_NEXT (assids);
            }

            /*
             * Create condfun
             */
            cflut = GenerateLUT ();
            cfargs = DFM2Args (inmask, cflut);
            cfargs
              = MakeArg (TmpVar (), MakeTypes1 (T_bool), ST_regular, ST_regular, cfargs);

            AVIS_TYPE (ARG_AVIS (cfargs))
              = TYMakeAKS (TYMakeSimpleType (T_bool), SHMakeShape (0));

            condfun
              = MakeFundef (GetLacFunName ("ReuseCond"), FUNDEF_MOD (fundef), cftypes,
                            cfargs, MakeBlock (NULL, NULL), FUNDEF_NEXT (fundef));

            FUNDEF_NEXT (fundef) = condfun;
            FUNDEF_RET_TYPE (condfun) = TYOldTypes2ProdType (FUNDEF_TYPES (condfun));
            FUNDEF_STATUS (condfun) = ST_condfun;

            /*
             * Build all parts  of the conditional
             */
            condids = MakeIds (StringCopy (ARG_NAME (cfargs)), NULL, ST_regular);
            cond = MakeIdFromIds (condids);
            ID_VARDEC (cond) = cfargs;
            ID_AVIS (cond) = VARDEC_AVIS (ID_VARDEC (cond));

            tmplut = DuplicateLUT (cflut);
            thenass = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars),
                                     condfun, inmask, tmplut);
            tmplut = RemoveLUT (tmplut);

            /*
             * Save current reuse candidate for later usage
             */
            rc = EXPRS_EXPR (branches);
            EXPRS_EXPR (branches) = EXPRS_NEXT (EXPRS_EXPR (branches));
            elseass = BuildCondTree (ass, branches, memvars, condfun, inmask, cflut);
            EXPRS_EXPR (branches) = rc;
            cflut = RemoveLUT (cflut);

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
                ids *cids;
                /*
                 * Create new lhs variable for FUNCOND
                 */
                FUNDEF_VARDEC (condfun)
                  = MakeVardec (TmpVar (),
                                DupOneTypes (VARDEC_TYPE (IDS_VARDEC (thenids))),
                                FUNDEF_VARDEC (condfun));

                cavis = VARDEC_AVIS (FUNDEF_VARDEC (condfun));
                AVIS_TYPE (cavis) = TYCopyType (AVIS_TYPE (IDS_AVIS (thenids)));

                cids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (cavis))), NULL,
                                ST_regular);
                IDS_AVIS (cids) = cavis;
                IDS_VARDEC (cids) = AVIS_VARDECORARG (IDS_AVIS (cids));

                /*
                 * create FUNCOND
                 */
                res = MakeAssign (MakeLet (MakeFuncond (MakeExprs (DupNode (cond), NULL),
                                                        MakeExprs (MakeIdFromIds (
                                                                     DupOneIds (thenids)),
                                                                   NULL),
                                                        MakeExprs (MakeIdFromIds (
                                                                     DupOneIds (elseids)),
                                                                   NULL)),
                                           cids),
                                  res);
                AVIS_SSAASSIGN (IDS_AVIS (cids)) = res;

                /*
                 * Put cids into retexprs
                 */
                retexprs = MakeExprs (MakeIdFromIds (DupOneIds (cids)), NULL);

                thenids = IDS_NEXT (thenids);
                elseids = IDS_NEXT (elseids);
            }

            /*
             * Append return( retexprs);
             */
            ret = MakeReturn (retexprs);
            res = AppendAssign (res, MakeAssign (ret, NULL));

            /*
             * Create conditional
             */
            res = MakeAssign (MakeCond (cond, MakeBlock (thenass, NULL),
                                        MakeBlock (elseass, NULL)),
                              res);

            /*
             * Put Assignments into function body
             */
            FUNDEF_INSTR (condfun) = res;
            FUNDEF_RETURN (condfun) = ret;
            res = NULL;

            DBUG_EXECUTE ("EMRB", PrintNode (condfun););

            /*
             * Create pattern:
             * c' = alloc( 0, []);
             * c  = fill( isreused( a, b), c');
             * r1, ... = reusecond( c, ...)
             */

            /*
             * Create variable c'
             */
            FUNDEF_VARDEC (fundef)
              = MakeVardec (TmpVar (), MakeTypes1 (T_bool), FUNDEF_VARDEC (fundef));

            memavis = VARDEC_AVIS (FUNDEF_VARDEC (fundef));
            AVIS_TYPE (memavis) = TYMakeAKS (TYMakeSimpleType (T_bool), SHMakeShape (0));

            memids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (memavis))), NULL,
                              ST_regular);
            IDS_AVIS (memids) = memavis;
            IDS_VARDEC (memids) = AVIS_VARDECORARG (memavis);

            /*
             * Create variable c
             */
            FUNDEF_VARDEC (fundef)
              = MakeVardec (TmpVar (), MakeTypes1 (T_bool), FUNDEF_VARDEC (fundef));

            valavis = VARDEC_AVIS (FUNDEF_VARDEC (fundef));
            AVIS_TYPE (valavis) = TYMakeAKS (TYMakeSimpleType (T_bool), SHMakeShape (0));

            valids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (valavis))), NULL,
                              ST_regular);
            IDS_AVIS (valids) = valavis;
            IDS_VARDEC (valids) = AVIS_VARDECORARG (valavis);

            /*
             * Create variables returned by condfun
             */
            cfids = NULL;
            i = 0;
            while (cftypes != NULL) {
                node *cfavis;
                ids *newids;

                FUNDEF_VARDEC (fundef)
                  = MakeVardec (TmpVar (), DupOneTypes (cftypes), FUNDEF_VARDEC (fundef));

                cfavis = VARDEC_AVIS (FUNDEF_VARDEC (fundef));
                AVIS_TYPE (cfavis)
                  = TYCopyType (TYGetProductMember (FUNDEF_RET_TYPE (condfun), i));

                newids = MakeIds (StringCopy (VARDEC_NAME (AVIS_VARDECORARG (cfavis))),
                                  NULL, ST_regular);
                IDS_AVIS (newids) = cfavis;
                IDS_VARDEC (newids) = AVIS_VARDECORARG (cfavis);

                cfids = AppendIds (cfids, newids);

                cftypes = TYPES_NEXT (cftypes);
                i += 1;
            }

            /*
             * Create application of condfun
             */
            cfap = MakeAp (StringCopy (FUNDEF_NAME (condfun)),
                           StringCopy (FUNDEF_MOD (condfun)),
                           MakeExprs (MakeIdFromIds (DupOneIds (valids)),
                                      DFM2ApArgs (inmask, lut)));

            AP_FUNDEF (cfap) = condfun;

            res = MakeAssign (MakeLet (cfap, cfids), res);

            while (cfids != NULL) {
                AVIS_SSAASSIGN (IDS_AVIS (cfids)) = res;
                cfids = IDS_NEXT (cfids);
            }

            FUNDEF_EXT_ASSIGNS (condfun) = NodeListAppend (NULL, res, NULL);
            FUNDEF_USED (condfun) = 1;

            /*
             * Create  c  = fill( isreused( a, b), c');
             */
            res
              = MakeAssign (MakeLet (MakePrf2 (F_fill,
                                               MakePrf2 (F_isreused,
                                                         DupNode (EXPRS_EXPR (rc)),
                                                         DupNode (EXPRS_EXPR (memvars))),
                                               MakeIdFromIds (DupOneIds (memids))),
                                     valids),
                            res);
            AVIS_SSAASSIGN (IDS_AVIS (valids)) = res;

            /*
             * Create c" = alloc( 0, []);
             */
            res = MakeAssign (MakeLet (MakePrf2 (F_alloc, MakeNum (0),
                                                 CreateZeroVector (0, T_int)),
                                       memids),
                              res);
            AVIS_SSAASSIGN (IDS_AVIS (memids)) = res;

            DBUG_EXECUTE ("EMRB", Print (res););
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
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_RB_BRANCHES (arg_info) != NULL) {
        node *next, *newass, *lastass;
        ids *lhs;
        DFMmask_t inmask;
        LUT_t lut;

        next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        /*
         * Determine in parameters of the assignment
         */
        lut = GenerateLUT ();
        inmask = InferInDFMAssignChain (arg_node, INFO_RB_FUNDEF (arg_info));

        newass = BuildCondTree (arg_node, INFO_RB_BRANCHES (arg_info),
                                INFO_RB_MEMVARS (arg_info), INFO_RB_FUNDEF (arg_info),
                                inmask, lut);

        INFO_RB_BRANCHES (arg_info) = FreeTree (INFO_RB_BRANCHES (arg_info));
        INFO_RB_MEMVARS (arg_info) = FreeTree (INFO_RB_MEMVARS (arg_info));

        FUNDEF_DFM_BASE (INFO_RB_FUNDEF (arg_info))
          = DFMRemoveMaskBase (FUNDEF_DFM_BASE (INFO_RB_FUNDEF (arg_info)));
        inmask = DFMRemoveMask (inmask);
        lut = RemoveLUT (lut);

        /*
         * Replace LHS of new assignment in order to match current funcion
         */
        lastass = newass;
        while (ASSIGN_NEXT (lastass) != NULL) {
            lastass = ASSIGN_NEXT (lastass);
        }
        ASSIGN_LHS (lastass) = FreeAllIds (ASSIGN_LHS (lastass));
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
        arg_node = FreeNode (arg_node);
        arg_node = AppendAssign (newass, arg_node);
        arg_node = AppendAssign (arg_node, next);
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
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("EMRB", ("Traversing function %s", FUNDEF_NAME (arg_node)));

        INFO_RB_FUNDEF (arg_info) = arg_node;

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("EMRB",
                    ("Traversal of function %s complete", FUNDEF_NAME (arg_node)));
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
            node *prf = PRF_ARG1 (arg_node);
            node *mem = PRF_ARG2 (arg_node);

            switch (PRF_PRF (prf)) {
            case F_copy:
            case F_add_AxA:
                /*
                 * a = fill( copy( b), a');
                 *
                 * b is the only data reuse candidate
                 */
                branches = GetReuseBranches (DupTree (PRF_ARGS (prf)),
                                             ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (mem))));

                /*
                 * Add branches to INFO_RB_BRANCHES if there are any
                 */
                if (branches != NULL) {
                    INFO_RB_BRANCHES (arg_info)
                      = MakeExprs (branches, INFO_RB_BRANCHES (arg_info));

                    INFO_RB_MEMVARS (arg_info)
                      = MakeExprs (DupNode (mem), INFO_RB_MEMVARS (arg_info));
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
    DBUG_ENTER ("EMRBwith");

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
    DBUG_ENTER ("EMRBwith2");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *EMRBwithop( node *arg_node, info *arg_info)
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
EMRBwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMRBwithop");

    DBUG_RETURN (arg_node);
}

/*@}*/
