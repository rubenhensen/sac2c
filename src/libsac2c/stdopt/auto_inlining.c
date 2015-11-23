/**
 *
 * @file auto_inlining.c
 *
 * This file determines which functions should be automatically inlined.
 *
 * The heuristic that is used, determines likely candidates for inlining
 * by counting a few types of nodes - N_prf (primitive functions), N_ap
 * (application of user declared functions), and N_code (withloop bodies)
 * and counting the number of loop nestings - both LACfuns and withloops.
 *
 * We then select the fundef and mark it as 'inline' - the actual
 * inlining occurs in later traversals (INL, PINL, etc...)
 *
 * TODO: further additions/changes could be made to make the traversal
 *       more 'intelligent' - for example being more context aware and
 *       inlining functions at their application point selectively
 *       (not always as it is currently implemented). Also, the thresholds
 *       we use could be further improved, but this would need a lot more
 *       testing.
 *
 * FIXME: We avoid functions that have arguments passed by reference, meaning
 *        almost no inlining occurs in classses. See big #1172 for updates.
 */

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"

#define DBUG_PREFIX "AINL"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "ctinfo.h" /* for CTIitemName */
#include "str.h"
#include "memory.h"
#include "type_utils.h"
#include "prepare_inlining.h"
#include "group_local_funs.h"
#include "DupTree.h"

#include "auto_inlining.h"

// We could add this at the global level - might be used by others?
#define MAXNESTING(n, m) (((n) < (m)) ? (m) : (n))

/**
 * TODO: determine ideal metrics for inlining
 *
 * The current values seem to work fairly well - we get slight
 * reductions in application size...
 */
#define WLNESTING 2
#define LPNESTING 2
#define APCOUNT 5
#define PRFCOUNT 5

/*
 * INFO structure
 */
struct INFO {
    node *fundef; // stores the pointer to a N_fundef of a loop function
    bool isref;   // FIXME: this is to avoid inlining functions that have
                  // arguments as references. See bug #1172
    /* Metrics */
    int wl_nesting;  // stores current with-loop nesting level
    int wl_nest_max; // stores function-level max with-loop nesting level
    int lp_nesting;  // stores current loop nesting level
    int lp_nest_max; // stores function-level max loop nesting level
    int prf_count;   // stores the number of primitive function calls
                     // within a function
    int ap_count;    // stores the number of function applications within
                     // a function
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_ISREF(n) ((n)->isref)
#define INFO_WLNESTMAX(n) ((n)->wl_nest_max)
#define INFO_WLNESTING(n) ((n)->wl_nesting)
#define INFO_LPNESTMAX(n) ((n)->lp_nest_max)
#define INFO_LPNESTING(n) ((n)->lp_nesting)
#define INFO_PRFCOUNT(n) ((n)->prf_count)
#define INFO_APCOUNT(n) ((n)->ap_count)

/*
  INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ISREF (result) = FALSE;
    INFO_WLNESTMAX (result) = 0;
    INFO_WLNESTING (result) = 0;
    INFO_LPNESTMAX (result) = 0;
    INFO_LPNESTING (result) = 0;
    INFO_PRFCOUNT (result) = 0;
    INFO_APCOUNT (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLap( node *arg_node, info *arg_info)
 *
 * @brief Two things are done here, the first is that we count all function
 *        applications - excluding primitives. The second is that we count
 *        N_fundef nodes which are loop function, we traverse into them and
 *        thereby count the loop nesting level. After the traversal, we update
 *        the function-level max loop nesting counter.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("++ Processing application of %s", CTIitemName (AP_FUNDEF (arg_node)));

    // Count all N_ap nodes
    INFO_APCOUNT (arg_info)++;

    if (FUNDEF_ISCONDFUN (AP_FUNDEF (arg_node))) {
        DBUG_PRINT_TAG ("AINL-ALL", "Found conditional N_ap ->");
        AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);
    } else if (FUNDEF_ISLOOPFUN (AP_FUNDEF (arg_node))
               && (INFO_FUNDEF (arg_info) != AP_FUNDEF (arg_node))) {

        DBUG_PRINT_TAG ("AINL-ALL", "Found loop N_ap ->");

        // Increment the nesting counter
        INFO_LPNESTING (arg_info)++;

        AP_FUNDEF (arg_node) = TRAVopt (AP_FUNDEF (arg_node), arg_info);

        // Decriment the nesting counter
        INFO_LPNESTING (arg_info)--;

        // Update function global max nesting value
        INFO_LPNESTMAX (arg_info)
          = MAXNESTING (INFO_LPNESTMAX (arg_info), INFO_LPNESTING (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLprf( node *arg_node, info *arg_info)
 *
 * @brief We count all primitive function calls.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("AINL-ALL", "Processing call of PRF fun");

    INFO_PRFCOUNT (arg_info)++;

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLwith( node *arg_node, info *arg_info)
 *
 * @brief We traverse first into the partitions and then into the WITHOP node
 *        set - eventually we reach a N_code block.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("AINL-ALL", "Processing call of N_with node");

    // There might not be a partition...
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLpart( node *arg_node, info *arg_info)
 *
 * @brief We traverse into the N_code node of the withloop partition.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("AINL-ALL", "Processing call of N_part node");

    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLcode( node *arg_node, info *arg_info)
 *
 * @brief For every N_code node we encounter, we increment a counter, and traverse
 *        into the N_block nodes, thereby counting the number of nested withloops.
 *        After the traversal, we update our function-level max withloop nesting
 *        counter.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("=> Processing call of N_code node");

    // Increment nesting counter
    INFO_WLNESTING (arg_info)++;

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_PRINT ("<= Leaving N_code at nesting level %d", INFO_WLNESTING (arg_info) - 1);

    // Decrement nesting counter
    INFO_WLNESTING (arg_info)--;

    // Update global max nesting counter
    INFO_WLNESTMAX (arg_info)
      = MAXNESTING (INFO_WLNESTMAX (arg_info), INFO_WLNESTING (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLarg node *arg_node, info *arg_info)
 *
 * @brief Here we traverse through the `list' of args associated with the fundef
 *        looking for any args that are passed by reference - we do this because
 *        inling such functions can cause issues in later phases of the compiler.
 *        As it stands, inlining creates a situation where a reference passed
 *        variable is being directly assigned to, which is not supported by the
 *        compiler. See bug #1172.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (ARG_WASREFERENCE (arg_node)) {
        INFO_ISREF (arg_info) = TRUE;
        DBUG_PRINT ("Found referenced argument!");
    } else {
        ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLfundef( node *arg_node, info *arg_info)
 *
 * @brief We traverse through the body of each N_fundef, but treat LAC functions
 *        slightly differently. In the case of a conditional function, we just
 *        traverse into it's body - in the case of a loop function we store a
 *        pointer to the node in our INFO structure and traverse into its body.
 *        Once the traversal is done, we reset the INFO structure FUNDEF field.
 *        Through this, we can count the number of nested loops there are within
 *        a function. If we encounter a non-LAC function, we traverse through its
 *        body and when we return, we print out the counters and reset.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *****************************************************************************/
node *
AINLfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ();

    DBUG_PRINT ("-> Traversing body of %s", CTIitemName (arg_node));

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        // Push N_fundef and store current N_fundef in INFO
        oldfundef = INFO_FUNDEF (arg_info);
        INFO_FUNDEF (arg_info) = arg_node;

        DBUG_PRINT ("%s (loop) has nesting %d", CTIitemName (arg_node),
                    INFO_LPNESTING (arg_info) - 1);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        // Pop previous N_fundef
        INFO_FUNDEF (arg_info) = oldfundef;

    } else if (FUNDEF_ISCONDFUN (arg_node)) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    } else if (FUNDEF_ISLOCAL (arg_node) && (!FUNDEF_WASUSED (arg_node))
               && (!FUNDEF_ISINLINECOMPLETED (arg_node))
               && (!FUNDEF_ISOBJECTWRAPPER (arg_node))
               && (!FUNDEF_ISWRAPPERFUN (arg_node)) && (!FUNDEF_ISLACFUN (arg_node))
               && (!FUNDEF_ISEXTERN (arg_node)) && (!FUNDEF_ISINLINE (arg_node))
               && (!FUNDEF_WASINLINED (arg_node))) {
        // Traverse into function arguments
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

        if (!(INFO_ISREF (arg_info))) {
            // Traverse into function body ->
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

            DBUG_PRINT ("## inlining metrics of %s ("
                        "WL: %d, "
                        "LPs: %d, "
                        "PRF: %d, "
                        "AP: %d"
                        ")",
                        CTIitemName (arg_node), INFO_WLNESTMAX (arg_info),
                        INFO_LPNESTMAX (arg_info), INFO_PRFCOUNT (arg_info),
                        INFO_APCOUNT (arg_info));

            if (INFO_WLNESTMAX (arg_info) < WLNESTING
                && INFO_LPNESTMAX (arg_info) < LPNESTING
                && INFO_PRFCOUNT (arg_info) < PRFCOUNT
                && INFO_APCOUNT (arg_info) < APCOUNT && (!FUNDEF_ISMAIN (arg_node))
                && (!FUNDEF_NOINLINE (arg_node))) {
                FUNDEF_ISINLINE (arg_node) = TRUE;
                FUNDEF_WASINLINED (arg_node) = TRUE;
                DBUG_PRINT ("%%%% marked %s as inline", CTIitemName (arg_node));
            }
        }
    } else {
        DBUG_PRINT ("!! skipping %s", CTIitemName (arg_node));
    }

    DBUG_PRINT ("<- Left body of %s", CTIitemName (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--***********************************************************************-->
 *
 * @fn node *AINLdoAutoInlining( node *arg_node)
 *
 * @brief initiates automatic function inlining as optimization phase
 *
 * @param arg_node
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
AINLdoAutoInlining (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "AINLdoAutoInlining called with wrong node type.");

    arg_info = MakeInfo ();

    TRAVpush (TR_ainl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}
