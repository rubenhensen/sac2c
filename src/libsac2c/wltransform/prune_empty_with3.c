/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup prune_empty_with3 traversal
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |       |
 * can be called on N_fundef               |   -----   |  y  |       |
 * expects LaC funs                        |   -----   |  -  |       |
 * follows N_ap to LaC funs                |   -----   |  n  |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    |  y  |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |  y  |       |
 * utilises SAA annotations                |   -----   |  y  |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    |  y  |       |
 * tolerates flattened Generators          |    yes    |  y  |       |
 * tolerates flattened operation parts     |    yes    |  y  |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |  y  |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  ?  |       |
 * =============================================================================
 * </pre>
 *
 * Performs the following transformation untill it can nolonger be done.
 *
 * ... with3{
 *   ...
 *   ( ... ) {
 *     ...
 *     res0 = noop(...);
 *     resn = noop(...);
 *     ....
 *   } : res0, ..., resn;
 *   ...
 * } ...
 *
 * ... with3{
 *  ...
 *  ...
 * } ...
 *
 * and
 *
 * res0, ..., resn = with3{
 * } : ...(..., memvar0), ..., ...(..., memvarn);
 *
 * res0 = memvar0;
 * ...
 * resn = memvarn;
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file prune_empty_with3.c
 *
 * Prefix: PEW3
 *
 *****************************************************************************/
#include "prune_empty_with3.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "pattern_match.h"
#include "DupTree.h"
#include "free.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *
 * INFO_MEMVARS    exprs chain of memvars
 * INFO_CAN_REMOVE can remove this range
 *****************************************************************************/
struct INFO {
    node *memvars;
    bool do_range_remove;
    bool can_remove;
    node *replaceAssigns;
};

#define INFO_MEMVARS(n) (n->memvars)
#define INFO_DO_RANGE_REMOVE(n) (n->do_range_remove)
#define INFO_CAN_REMOVE(n) (n->can_remove)
#define INFO_REPLACE_ASSIGNS(n) (n->replaceAssigns)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_MEMVARS (result) = NULL;
    INFO_DO_RANGE_REMOVE (result) = FALSE;
    INFO_CAN_REMOVE (result) = FALSE;
    INFO_REPLACE_ASSIGNS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    DBUG_ASSERT (INFO_MEMVARS (info) == NULL, "Memory leak in info MEMVARS not empty");

    DBUG_ASSERT (INFO_REPLACE_ASSIGNS (info) == NULL,
                 "Memory leak in info REPLACE_ASSIGNS not empty");

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
 * @fn node *PEW3doPruneEmptyWith3( node *syntax_tree)
 *
 *****************************************************************************/
node *
PEW3doPruneEmptyWith3 (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("PEW3doPruneEmptyWith3");

    info = MakeInfo ();

    DBUG_PRINT ("PEW3", ("Starting prune empty with3 traversal."));

    TRAVpush (TR_pew3);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("PEW3", ("Prune empty with3 traversal complete."));

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

/** <!--********************************************************************-->
 *
 * @fn node *createAssigns(node *ids, node *exprs)
 *
 * @brief convert ids exprs into assign->let chain
 *
 *****************************************************************************/
static node *
createAssignChain (node *arg_ids, node *exprs)
{
    node *assign, *ids, *rhs;
    DBUG_ENTER ("createAssignChain");

    DBUG_ASSERT ((arg_ids != NULL), "ids missing");
    DBUG_ASSERT ((exprs != NULL), "exprs missing");

    if (IDS_NEXT (arg_ids) == NULL) {
        assign = NULL;
    } else {
        assign = createAssignChain (IDS_NEXT (arg_ids), EXPRS_NEXT (exprs));
    }
    ids = DUPdoDupNode (arg_ids);
    rhs = DUPdoDupNode (EXPRS_EXPR (exprs));

    assign = TBmakeAssign (TBmakeLet (ids, rhs), assign);

    /* avis->ssaassign needed to keep the AST legal */
    AVIS_SSAASSIGN (IDS_AVIS (ids)) = assign;

    DBUG_RETURN (assign);
}
/** <!--********************************************************************-->
 *
 * @fn node *ATRAVwithop(node *ids, node *exprs)
 *
 * @brief Save withop mem var
 *
 *****************************************************************************/
static node *
ATRAVwithop (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATRAVwithop");

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_MEMVARS (arg_info)
      = TBmakeExprs (WITHOP_MEM (arg_node), INFO_MEMVARS (arg_info));

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *getMemvars(node *ids, node *exprs)
 *
 * @brief put the memvars from a chain of withops into info
 *
 *****************************************************************************/
static node *
getMemvars (node *withops, info *arg_info)
{ // N_genarrayN_modarrayN_spfoldN_foldN_breakN_propagate
    anontrav_t trav[] = {{N_modarray, &ATRAVwithop},
                         {N_genarray, &ATRAVwithop},
                         {N_spfold, &TRAVerror},
                         {N_fold, &TRAVerror},
                         {N_break, &TRAVerror},
                         {N_propagate, &TRAVerror},
                         {0, NULL}};
    DBUG_ENTER ("getMemvars");

    TRAVpushAnonymous (trav, &TRAVsons);
    withops = TRAVopt (withops, arg_info);
    TRAVpop ();

    DBUG_RETURN (withops);
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
 * @fn node *PEW3with3(node *arg_node, info *arg_info)
 *
 * @brief Disable doing transformation if this is top level
 *
 *****************************************************************************/
node *
PEW3with3 (node *arg_node, info *arg_info)
{
    bool stack;
    DBUG_ENTER ("PEW3with3");

    stack = INFO_DO_RANGE_REMOVE (arg_info);
    INFO_DO_RANGE_REMOVE (arg_info) = !WITH3_ISTOPLEVEL (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_DO_RANGE_REMOVE (arg_info) = stack;

    if ((!WITH3_ISTOPLEVEL (arg_node))
        && (TCcountRanges (WITH3_RANGES (arg_node)) == 0)) {
        INFO_MEMVARS (arg_info) = getMemvars (WITH3_OPERATIONS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEW3range(node *arg_node, info *arg_info)
 *
 * @brief Remove this range if meets conditions
 *
 *****************************************************************************/
node *
PEW3range (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PEW3range");

    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), arg_info);

    INFO_CAN_REMOVE (arg_info) = TRUE;
    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);

    if (INFO_CAN_REMOVE (arg_info)) {
        node *del;
        del = arg_node;
        arg_node = RANGE_NEXT (del);
        RANGE_NEXT (del) = NULL;
        del = FREEdoFreeTree (del);
    } else {
        RANGE_BODY (arg_node) = TRAVdo (RANGE_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *PEW3id(node *arg_node, info *arg_info)
 *
 * @brief is this id created from F_noop
 *        can be used every where but only valid when done on range result
 *
 *****************************************************************************/
node *
PEW3id (node *arg_node, info *arg_info)
{
    pattern *pat;
    DBUG_ENTER ("PEW3range");

    pat = PMprf (1, PMAisPrf (F_noop), 0);
    INFO_CAN_REMOVE (arg_info)
      = (INFO_CAN_REMOVE (arg_info) && PMmatchFlat (pat, arg_node));
    pat = PMfree (pat);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEW3assign(node *arg_node, info *arg_info)
 *
 * @brief If replace_assign replace current assign with replace assign
 *
 *****************************************************************************/
node *
PEW3assign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PEW3assign");

    DBUG_ASSERT ((INFO_REPLACE_ASSIGNS (arg_info) == NULL),
                 "Should not have any replace assigns until traved assign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_REPLACE_ASSIGNS (arg_info) != NULL) {
        arg_node
          = TCappendAssign (INFO_REPLACE_ASSIGNS (arg_info), FREEdoFreeNode (arg_node));
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *PEW3let(node *arg_node, info *arg_info)
 *
 * @brief If removed with3 on rhs create a replacment chain of assigns
 *        put in replace assign
 *
 *****************************************************************************/
node *
PEW3let (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PEW3let");

    DBUG_ASSERT ((INFO_MEMVARS (arg_info) == NULL),
                 "Should not have any memvars at this point");

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_MEMVARS (arg_info) != NULL) {
        INFO_REPLACE_ASSIGNS (arg_info)
          = createAssignChain (LET_IDS (arg_node), INFO_MEMVARS (arg_info));
        INFO_MEMVARS (arg_info) = FREEdoFreeTree (INFO_MEMVARS (arg_info));
    }

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
