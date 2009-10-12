
/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup as Add Sync In Sync Out prfs
 *
 * Add Sync ins after accu in fold funs
 * Add Sync outs at end of assigns
 *
 *
 * @ingroup ass
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file addSyncs.c
 *
 * Prefix: ASS
 *
 *****************************************************************************/
#include "addSyncs.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "new_types.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *postAssign;
    bool prfAccu;
    node *vardecs;
    node *lhsold;
    node *lhsnew;
    node *assign;
    node *shareds;
    node *withops;
    node *results;
};

/**
 *
 */

#define INFO_POSTASSIGN(n) ((n)->postAssign)
#define INFO_PRFACCU(n) ((n)->prfAccu)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_LHSOLD(n) ((n)->lhsold)
#define INFO_LHSNEW(n) ((n)->lhsnew)
#define INFO_ASSIGN(n) ((n)->assign)
#define INFO_SHAREDS(n) ((n)->shareds)
#define INFO_WITHOPS(n) ((n)->withops)
#define INFO_RESULTS(n) ((n)->results)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_POSTASSIGN (result) = NULL;
    INFO_PRFACCU (result) = FALSE;
    INFO_VARDECS (result) = NULL;
    INFO_LHSOLD (result) = NULL;
    INFO_LHSNEW (result) = NULL;
    INFO_ASSIGN (result) = NULL;
    INFO_SHAREDS (result) = NULL;
    INFO_WITHOPS (result) = NULL;
    INFO_RESULTS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    /*DBUG_ASSERT( ( INFO_POSTASSIGN( info) == NULL),
                 "Memory leaking");
    DBUG_ASSERT( ( INFO_VARDECS( info) == NULL),
                 "Memory leaking");
    DBUG_ASSERT( ( INFO_LHSOLD( info) == NULL),
        "Memory leaking");*/
    info = MEMfree (info);

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
 * @fn node *ASdoAddSyncs( node *syntax_tree)
 *
 *****************************************************************************/
node *
ASdoAddSyncs (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ASdoAddSyncs");

    info = MakeInfo ();

    DBUG_PRINT ("ASS", ("Starting Add Syncs traversal."));

    TRAVpush (TR_ass);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("ASS", ("Add Syncs traversal complete."));

    info = FreeInfo (info);

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

static node *
createIn (node *lhsnew, node *lhsold, node *next, info *arg_info)
{
    node *assign;
    DBUG_ENTER ("createIn");

    if (lhsnew == NULL) {
        assign = next;
        /*assert*/
    } else {
        /*assert*/
        node *avissh;
        avissh = TBmakeAvis (TRAVtmpVar (),
                             TYsetMutcScope (TYcopyType (AVIS_TYPE (IDS_AVIS (lhsold))),
                                             MUTC_SHARED));
        INFO_SHAREDS (arg_info) = TBmakeIds (avissh, INFO_SHAREDS (arg_info));
        INFO_VARDECS (arg_info) = TBmakeVardec (avissh, INFO_VARDECS (arg_info));
        assign
          = TBmakeAssign (TBmakeLet (DUPdoDupNode (lhsold),
                                     TBmakePrf (F_syncin,
                                                TBmakeExprs (TBmakeId (IDS_AVIS (lhsnew)),
                                                             TBmakeExprs (TBmakeId (
                                                                            avissh),
                                                                          NULL)))),
                          createIn (IDS_NEXT (lhsnew), IDS_NEXT (lhsold), next,
                                    arg_info));
    }

    DBUG_RETURN (assign);
}

static node *
ATravAssign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAssign");

    INFO_ASSIGN (arg_info) = arg_node;

    INFO_PRFACCU (arg_info) = FALSE;

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PRFACCU (arg_info)) {
        DBUG_ASSERT ((INFO_LHSOLD (arg_info) != NULL), "_accu without lhs?");
        ASSIGN_NEXT (arg_node) = createIn (INFO_LHSNEW (arg_info), INFO_LHSOLD (arg_info),
                                           ASSIGN_NEXT (arg_node), arg_info);
        INFO_LHSOLD (arg_info) = FREEdoFreeTree (INFO_LHSOLD (arg_info));
        INFO_LHSNEW (arg_info) = NULL;
    }
    INFO_PRFACCU (arg_info) = FALSE;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        ASSIGN_NEXT (arg_node) = INFO_POSTASSIGN (arg_info);
        INFO_POSTASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

static node *
createIds (node *lhs, node *assign, info *arg_info)
{
    node *ids;
    DBUG_ENTER ("createIds");

    if (lhs == NULL) {
        ids = NULL;
    } else {
        node *avis;
        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (IDS_AVIS (lhs))));

        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
        AVIS_SSAASSIGN (avis) = assign;
        ids = TBmakeIds (avis, createIds (IDS_NEXT (lhs), assign, arg_info));
    }
    DBUG_RETURN (ids);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravLet(node *arg_node)
 *
 * @brief Save lhs of let into info struct
 *
 *****************************************************************************/
static node *
ATravLet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAssign");

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    if (INFO_PRFACCU (arg_info)) {
        node *lhs;
        INFO_LHSOLD (arg_info) = LET_IDS (arg_node);
        lhs = createIds (LET_IDS (arg_node), INFO_ASSIGN (arg_info), arg_info);
        LET_IDS (arg_node) = lhs;
        INFO_LHSNEW (arg_info) = lhs;
    }

    DBUG_RETURN (arg_node);
}

static node *
ATravPrf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAssign");

    INFO_PRFACCU (arg_info) = (PRF_PRF (arg_node) == F_accu);

    DBUG_RETURN (arg_node);
}

static node *
createSyncOut (node *rets, node *shareds, node *ops, info *arg_info)
{
    node *res;
    DBUG_ENTER ("createSyncOut");

    if (rets == NULL) {
        res = NULL;
    } else {
        node *next;

        if (NODE_TYPE (ops) == N_fold) {
            node *avis
              = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (IDS_AVIS (shareds))));

            AVIS_WITH3FOLD (IDS_AVIS (shareds)) = ops;

            next = createSyncOut (EXPRS_NEXT (rets), IDS_NEXT (shareds),
                                  WITHOP_NEXT (ops), arg_info);

            INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
            INFO_POSTASSIGN (arg_info) = TBmakeAssign (
              TBmakeLet (TBmakeIds (avis, NULL),
                         TBmakePrf (F_syncout,
                                    TBmakeExprs (DUPdoDupNode (EXPRS_EXPR (rets)),
                                                 TBmakeExprs (TBmakeId (
                                                                IDS_AVIS (shareds)),
                                                              NULL)))),
              INFO_POSTASSIGN (arg_info));
            res = TBmakeExprs (TBmakeId (avis), next);
        } else {
            EXPRS_NEXT (rets)
              = createSyncOut (EXPRS_NEXT (rets), shareds, WITHOP_NEXT (ops), arg_info);
            res = rets;
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *AddSyncs(node *arg_node)
 *
 * @brief
 *
 *****************************************************************************/
static node *
AddSyncs (node *assign, node *rets, info *arg_info)
{
    anontrav_t insert[7] = {{N_assign, &ATravAssign},
                            {N_let, &ATravLet},
                            {N_prf, &ATravPrf},
                            {N_with, &TRAVnone},
                            {N_with2, &TRAVnone},
                            {N_with3, &TRAVnone},
                            {0, NULL}};
    DBUG_ENTER ("AddSyncs");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "Node not an assign");

    TRAVpushAnonymous (insert, &TRAVsons);

    assign = TRAVopt (assign, arg_info);

    TRAVpop ();

    INFO_RESULTS (arg_info)
      = createSyncOut (rets, INFO_SHAREDS (arg_info), INFO_WITHOPS (arg_info), arg_info);

    assign = TCappendAssign (assign, INFO_POSTASSIGN (arg_info));
    INFO_POSTASSIGN (arg_info) = NULL;

    DBUG_RETURN (assign);
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
 * @fn node *ASvardec(node *arg_node, info *arg_info)
 *
 * @brief Append vardecs
 *
 *****************************************************************************/
node *
ASSvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSvardec");

    arg_node = TRAVcont (arg_node, arg_info);

    if (VARDEC_NEXT (arg_node) == NULL) {
        VARDEC_NEXT (arg_node) = INFO_VARDECS (arg_info);
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

node *
ASSrange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ASSrange");

    arg_node = TRAVcont (arg_node, arg_info);

    BLOCK_INSTR (RANGE_BODY (arg_node)) = AddSyncs (BLOCK_INSTR (RANGE_BODY (arg_node)),
                                                    RANGE_RESULTS (arg_node), arg_info);

    RANGE_RESULTS (arg_node) = INFO_RESULTS (arg_info);
    INFO_RESULTS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
ASSwith3 (node *arg_node, info *arg_info)
{
    node *stack;
    DBUG_ENTER ("ASSwith3");

    stack = INFO_WITHOPS (arg_info);

    INFO_WITHOPS (arg_info) = WITH3_OPERATIONS (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITHOPS (arg_info) = stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
