/** <!--********************************************************************-->
 *
 * @defgroup lva determine live variables for spawn and sync statements
 *
 *   Sets up a clear dataflow mask for the entire function and starts
 *   traversing. When traversing into a id or ids node, the corresponding bits
 *   are set in the dataflow.
 *
 *   When a spawn or sync is encountered, a new empty dataflow is created and
 *   traversal is continued from that point forwards. Again, encountering an
 *   id or ids means the corresponding bits are set. Other spawn and sync
 *   statements are ignored at this point. When traversal has reached the end,
 *   return to the original spawn or sync statement. We now have two data
 *   masks, one before and one after the statement. By ANDing those masks,
 *   live variables are found.
 *
 *   The dataflow mask from before the statement is put back and traversal
 *   continues until the next spawn or sync statement and the process is
 *   repeated.
 *
 *   TODO: It appears AssignIsSync and AssignIsSpawn are tunred around? Check!
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file live_variable_analysis.c
 *
 * Prefix: LVA
 *
 *****************************************************************************/
#include "live_variable_analysis.h"

#define DBUG_PREFIX "LVA"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"
#include "DataFlowMask.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    dfmask_base_t *base;
    dfmask_t *live;
    dfmask_t *func;
    bool analyse;
    dfmask_t *funion;
    bool inspawn;
};

#define INFO_BASE(n) ((n)->base)
#define INFO_LIVE(n) ((n)->live)
#define INFO_ANALYSE(n) ((n)->analyse)
#define INFO_FUNION(n) ((n)->funion)
#define INFO_INSPAWN(n) ((n)->inspawn)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_BASE (result) = NULL;
    INFO_LIVE (result) = NULL;
    INFO_ANALYSE (result) = FALSE;
    INFO_FUNION (result) = NULL;
    INFO_INSPAWN (result) = FALSE;

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
 * @fn node *LVAdoLiveVariableAnalysis( node *argnode)
 *
 *****************************************************************************/

node *
LVAdoLiveVariableAnalysis (node *argnode)
{
    info *info;
    DBUG_ENTER ();
    DBUG_PRINT ("Live Variable Analysis");

    info = MakeInfo ();

    TRAVpush (TR_lva);
    argnode = TRAVdo (argnode, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (argnode);
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
 * @fn static bool AssignIsSpawn( node *assign)
 *
 * @brief Returns whether or not the assignment node is a spawned ap node
 *
 * @param node an N_assign node
 *
 * @return is assign node a spawned ap
 *
 *****************************************************************************/
static bool
AssignIsSpawn (node *assign)
{
    bool result;
    node *instr;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign, "Node must be an N_assign node");

    instr = ASSIGN_STMT (assign);

    result = NODE_TYPE (instr) == N_let && NODE_TYPE (LET_EXPR (instr)) == N_ap
             && AP_ISSPAWNED (LET_EXPR (instr));

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static bool AssignIsSync( node *assign)
 *
 * @brief Returns whether or not the assignment node is sync statement
 *
 * @param node an N_assign node
 *
 * @return is assign node a sync statement
 *
 *****************************************************************************/
static bool
AssignIsSync (node *assign)
{
    bool result;
    node *instr;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign, "Node must be an N_assign node");

    instr = ASSIGN_STMT (assign);

    result = NODE_TYPE (instr) == N_let && NODE_TYPE (LET_EXPR (instr)) == N_prf
             && PRF_PRF (LET_EXPR (instr)) == F_sync;

    DBUG_RETURN (result);
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
 * @fn node *LVAfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverses into fundef local LAC functions, then function
 *        bodies and finally function next pointers. When traversing
 *        into a body a pointer in the info struct is maintained to
 *        the inner fundef.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
LVAfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t *base;
    node *avis, *livevars;

    DBUG_ENTER ();

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    if (FUNDEF_CONTAINSSPAWN (arg_node)) {
        // function contains spawn, do analysis

        // set up a base mask
        base = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDECS (arg_node));

        INFO_BASE (arg_info) = base;
        INFO_LIVE (arg_info) = DFMgenMaskClear (base);
        INFO_FUNION (arg_info) = DFMgenMaskClear (base);
        INFO_ANALYSE (arg_info) = FALSE;

        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("Union for fundef");
        DBUG_EXECUTE (DFMprintMaskDetailed (stdout, INFO_FUNION (arg_info)));

        avis = DFMgetMaskEntryAvisSet (INFO_FUNION (arg_info));
        livevars = NULL;

        while (avis != NULL) {
            DBUG_PRINT ("Live Var Found");
            livevars = TBmakeLivevars (avis, livevars);
            avis = DFMgetMaskEntryAvisSet (NULL);
        }

        FUNDEF_LIVEVARS (arg_node) = livevars;
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LVAassign(node *arg_node, info *arg_info)
 *
 * @brief Traverse assign nodes
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
LVAassign (node *arg_node, info *arg_info)
{
    dfmask_t *mask;
    dfmask_t *tmp;
    node *avis;
    node *livevars;

    DBUG_ENTER ();

    if (!INFO_ANALYSE (arg_info)
        && (AssignIsSpawn (arg_node) || AssignIsSync (arg_node))) {

        DBUG_PRINT ("Assign node is spawn or sync, analysing...");

        INFO_ANALYSE (arg_info) = TRUE;

        // save the mask until now and clear the mask used for traversal
        tmp = DFMgenMaskClear (INFO_BASE (arg_info));
        DFMsetMaskCopy (tmp, INFO_LIVE (arg_info));
        INFO_LIVE (arg_info) = DFMgenMaskClear (INFO_BASE (arg_info));

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        // AND the temporary mask with the one after this statement
        mask = DFMgenMaskAnd (tmp, INFO_LIVE (arg_info));
        DFMsetMaskCopy (INFO_LIVE (arg_info), tmp);

        INFO_ANALYSE (arg_info) = FALSE;

        DBUG_PRINT ("Done analysing");
        DBUG_EXECUTE (DFMprintMaskDetailed (stdout, mask));

        // create union mask for entire function
        DFMsetMaskOr (INFO_FUNION (arg_info), mask);

        // Add ids for spawned ap to funion as well
        // TODO: Use anonymous traversal
        if (AssignIsSync (arg_node)) {
            INFO_INSPAWN (arg_info) = TRUE;
            ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
            INFO_INSPAWN (arg_info) = FALSE;
        }

        // save live vars in the let node
        avis = DFMgetMaskEntryAvisSet (mask);
        livevars = NULL;

        while (avis != NULL) {
            DBUG_PRINT ("Live Var Found");
            livevars = TBmakeLivevars (avis, livevars);
            avis = DFMgetMaskEntryAvisSet (NULL);
        }

        LET_LIVEVARS (ASSIGN_STMT (arg_node)) = livevars;
    } else {
        // only analyse use of variables
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LVAlet(node *arg_node, info *arg_info)
 *
 * @brief Traverse let nodes and set spawned in info if it contains
 *        a spawned ap
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
LVAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Traversing Let node");

    if (INFO_INSPAWN (arg_info)) {
        DBUG_PRINT ("Found a spawned ap!");
    }

    LET_IDS (arg_node) = TRAVopt (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVopt (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LVAids(node *arg_node, info *arg_info)
 *
 * @brief Traverse ids nodes
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
LVAids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Traversing Ids node: %s", AVIS_NAME (IDS_AVIS (arg_node)));

    DFMsetMaskEntrySet (INFO_LIVE (arg_info), IDS_AVIS (arg_node));

    // Always add result from spawned ap to task frame
    if (INFO_INSPAWN (arg_info)) {
        DBUG_PRINT ("Adding Ids to funion");
        DFMsetMaskEntrySet (INFO_FUNION (arg_info), IDS_AVIS (arg_node));
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *LVAid(node *arg_node, info *arg_info)
 *
 * @brief Traverse id nodes
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
LVAid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
    DBUG_PRINT ("Traversing Id node: %s", AVIS_NAME (ID_AVIS (arg_node)));

    DFMsetMaskEntrySet (INFO_LIVE (arg_info), ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Live Variable Analysis -->
 *****************************************************************************/

#undef DBUG_PREFIX
