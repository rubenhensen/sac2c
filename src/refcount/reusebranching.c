/*
 *
 * $Log$
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
#include "DupTree.h"
#include "LookUpTable.h"
#include "DataFlowMask.h"

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
 * @param ass The assignment to be copied
 * @param branches id list list of data reuse candidates
 * @param memvars id list of memory veriables
 * @param fundef Fundef to put the new vardecs into
 *
 * @return Assignment chain
 *
 *****************************************************************************/
static node *
BuildCondTree (node *ass, node *branches, node *memvars, node *fundef)
{
    node *res;

    DBUG_ENTER ("BuildCondTree");

    if (branches == NULL) {
        res = DupNode (ass);
    } else {
        if (EXPRS_EXPR (branches) == NULL) {
            res
              = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars), fundef);
        } else {
            node *cond;
            node *thenass;
            node *elseass;
            node *rc, *stack;
            node *memavis, *valavis;
            ids *memids, *valids;

            /*
             * Create pattern:
             * c' = alloc( 0, []);
             * c  = fill( isreused( a, b), c');
             * if ( c) ...
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
            IDS_VARDEC (memids) = AVIS_VARDECORARG (IDS_AVIS (memids));

            res = MakeAssign (MakeLet (MakePrf2 (F_alloc, MakeNum (0),
                                                 MakeFlatArray (NULL)),
                                       memids),
                              NULL);

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
            IDS_VARDEC (valids) = AVIS_VARDECORARG (IDS_AVIS (valids));

            /*
             * Build all parts  of the conditional
             */
            cond = MakeIdFromIds (DupOneIds (valids));

            thenass
              = BuildCondTree (ass, EXPRS_NEXT (branches), EXPRS_NEXT (memvars), fundef);

            /*
             * Save current reuse candidate for later usage
             */
            rc = EXPRS_EXPR (branches);
            EXPRS_EXPR (branches) = EXPRS_NEXT (EXPRS_EXPR (branches));
            elseass = BuildCondTree (ass, branches, memvars, fundef);
            EXPRS_EXPR (branches) = rc;

            res = MakeAssign (MakeCond (cond, MakeBlock (thenass, NULL),
                                        MakeBlock (elseass, NULL)),
                              NULL);

            res
              = MakeAssign (MakeLet (MakePrf2 (F_fill,
                                               MakePrf2 (F_isreused,
                                                         DupNode (EXPRS_EXPR (rc)),
                                                         DupNode (EXPRS_EXPR (memvars))),
                                               MakeIdFromIds (DupOneIds (memids))),
                                     valids),
                            res);
            AVIS_SSAASSIGN (IDS_AVIS (valids)) = res;

            res = MakeAssign (MakeLet (MakePrf2 (F_alloc, MakeNum (0),
                                                 MakeFlatArray (NULL)),
                                       memids),
                              res);
            AVIS_SSAASSIGN (IDS_AVIS (memids)) = res;
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
        node *newass
          = BuildCondTree (arg_node, INFO_RB_BRANCHES (arg_info),
                           INFO_RB_MEMVARS (arg_info), INFO_RB_FUNDEF (arg_info));

        INFO_RB_BRANCHES (arg_info) = FreeTree (INFO_RB_BRANCHES (arg_info));
        INFO_RB_MEMVARS (arg_info) = FreeTree (INFO_RB_MEMVARS (arg_info));

        arg_node = FreeNode (arg_node);
        arg_node = AppendAssign (newass, arg_node);
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
