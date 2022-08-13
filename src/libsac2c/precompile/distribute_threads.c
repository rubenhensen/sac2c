/** <!--********************************************************************-->
 *
 * @defgroup dst Distribute Threads
 *
 * Computes the blocksize to be used for creates. The current algorithm is
 * not tuned for performance but merely tries to prevent deadlock by
 * ensuring that enough threads are available to make progress on the
 * lowest level. The general idea is to equally distribute between
 * levels and put the overspill into the lowest level.
 *
 * There are a lot of problems with this approach. Here are two:
 *
 * 1) We loose a lot of threads as we partition the thread space
 *    equally for all ranges of a with3, regardless of their size.
 *    Not to speak of integer rounding errors.
 *
 * 2) The backend currently creates code to concurrently evaluate all
 *    ranges. However, this very quickly eats up the thread space for
 *    deep nestings. On the long run, we need a way to switch between
 *    concurrent and sequential range spawning.
 * 2b) To support the contingency plan mutc compiler, we for now use
 *     non-concurrent creates in all cases.
 *
 * @ingroup dst
 *
 * @{
 *
 *****************************************************************************/

#define FAMILYTABLESIZE 32

/** <!--********************************************************************-->
 *
 * @file distribute_threads.c
 *
 * Prefix: DST
 *
 *****************************************************************************/
#include "distribute_threads.h"

#define DBUG_PREFIX "DST"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "namespaces.h"
#include "str.h"
#include "globals.h"
#include "ctinfo.h"
#include "math_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
#define UNKNOWN 0
#define INFINITE -1
#define PROCESSING -2

#define MAX_HEIGHT(a, b) ((a == INFINITE) || (b == INFINITE)) ? a : ((a > b) ? a : b)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

enum travmode_t { DST_findmain, DST_follow, DST_clean };

struct INFO {
    int down;
    int up;
    enum travmode_t travmode;
    node *main;
    int width;
    int avail;
    int used;
    int throttle;
    bool failed;
    int globals;
};

/**
 * A template entry in the template info structure
 */
#define INFO_DOWN(n) ((n)->down)
#define INFO_UP(n) ((n)->up)
#define INFO_TRAVMODE(n) ((n)->travmode)
#define INFO_MAIN(n) ((n)->main)
#define INFO_WIDTH(n) ((n)->width)
#define INFO_AVAIL(n) ((n)->avail)
#define INFO_USED(n) ((n)->used)
#define INFO_THROTTLE(n) ((n)->throttle)
#define INFO_FAILED(n) ((n)->failed)
#define INFO_GLOBALS(n) ((n)->globals)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_DOWN (result) = UNKNOWN;
    INFO_UP (result) = UNKNOWN;
    INFO_TRAVMODE (result) = DST_findmain;
    INFO_MAIN (result) = NULL;
    INFO_WIDTH (result) = 0;
    INFO_AVAIL (result) = 0;
    INFO_USED (result) = 0;
    INFO_THROTTLE (result) = 1;
    INFO_FAILED (result) = FALSE;
    INFO_GLOBALS (result) = 1;

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
 * @fn node *DSTdoDistributeThreads( node *syntax_tree)
 *
 *****************************************************************************/
node *
DSTdoDistributeThreads (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_dst);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

#if 0
/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/


/** <!--********************************************************************-->
 *
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static
node *DummyStaticHelper(node *arg_node)
{
  DBUG_ENTER ();

  DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/
#endif

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *DSTmodule(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DSTmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TRAVMODE (arg_info) = DST_findmain;
    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_ASSERT (INFO_MAIN (arg_info) != NULL, "no main function found");

    INFO_AVAIL (arg_info) = global.max_threads;
    INFO_THROTTLE (arg_info) = global.max_threads / 2;
    do {
        INFO_THROTTLE (arg_info) = MATHmax (1, INFO_THROTTLE (arg_info));
        DBUG_PRINT ("!!! Trying a throttle of %d.", INFO_THROTTLE (arg_info));
        INFO_TRAVMODE (arg_info) = DST_follow;
        INFO_FAILED (arg_info) = FALSE;
        INFO_MAIN (arg_info) = TRAVdo (INFO_MAIN (arg_info), arg_info);

        INFO_TRAVMODE (arg_info) = DST_clean;
        MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);
        MODULE_FUNDECS (arg_node) = TRAVopt (MODULE_FUNDECS (arg_node), arg_info);
        MODULE_THREADFUNS (arg_node) = TRAVopt (MODULE_THREADFUNS (arg_node), arg_info);
        INFO_THROTTLE (arg_info) = INFO_THROTTLE (arg_info) / 2;
    } while (INFO_FAILED (arg_info) && INFO_THROTTLE (arg_info) > 0);

    if (INFO_FAILED (arg_info)) {
        CTIwarn (EMPTY_LOC, "Could not compute static thread distribution. The "
                 "program might deadlock due to lack of resources.");
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSTfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DSTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVMODE (arg_info) == DST_findmain) {
        if (NSequals (FUNDEF_NS (arg_node), NSgetRootNamespace ())
            && STReq (FUNDEF_NAME (arg_node), "main")) {
            INFO_MAIN (arg_info) = arg_node;
        } else {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }
    } else if (INFO_TRAVMODE (arg_info) == DST_follow) {
        DBUG_PRINT (">>> Entering function %s (%d/%d)...", CTIitemName (arg_node),
                    FUNDEF_DEPTH (arg_node), FUNDEF_HEIGHT (arg_node));

        if (FUNDEF_HEIGHT (arg_node) == PROCESSING) {
            /* recursive function, we don't know how deep it will go,
             * so we bump up the upwards level to infinite
             */
            DBUG_PRINT ("      recursive function!");
            INFO_UP (arg_info) = INFINITE;
        } else if ((FUNDEF_HEIGHT (arg_node) != UNKNOWN)
                   && (FUNDEF_DEPTH (arg_node) >= INFO_DOWN (arg_info))) {
            /*
             * we traversed the function before and have already
             * attributed resource information for a depth
             * higher than the current one. As the depth is now
             * smaller, we have more resources to use on the lower
             * levels, so we should be fine.
             * We reuse the height value.
             */
            DBUG_PRINT ("      seen function before!");
            INFO_UP (arg_info)
              = MAX_HEIGHT (FUNDEF_HEIGHT (arg_node), INFO_UP (arg_info));
        } else {
            /*
             * we have not seen it before, or we have a higher depth,
             * so we recompute.
             */
            if (FUNDEF_ISEXTERN (arg_node)) {
                /*
                 * we assume external functions spawn no further threads
                 * so we leave the level as is ( we compute MAX( current, 0)).
                 */
                DBUG_PRINT ("      external function!");
                INFO_UP (arg_info) = 0;
            } else if (FUNDEF_BODY (arg_node) == NULL) {
                /*
                 * we don't have the body. We cannot say anything here. To
                 * be safe we assume infinite.
                 */
                DBUG_PRINT ("      function without body!");
                INFO_UP (arg_info) = INFINITE;
            } else {
                /*
                 * we traverse the body and see.
                 */
                DBUG_PRINT ("      entering body to infer information.");
                FUNDEF_HEIGHT (arg_node) = PROCESSING;
                FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            }
            FUNDEF_DEPTH (arg_node) = INFO_DOWN (arg_info);
            FUNDEF_HEIGHT (arg_node) = INFO_UP (arg_info);
        }
        DBUG_PRINT ("<<< Leaving function %s (%d/%d)...", CTIitemName (arg_node),
                    FUNDEF_DEPTH (arg_node), FUNDEF_HEIGHT (arg_node));
    } else if (INFO_TRAVMODE (arg_info) == DST_clean) {
        FUNDEF_DEPTH (arg_node) = UNKNOWN;
        FUNDEF_HEIGHT (arg_node) = UNKNOWN;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSTap(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DSTap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSTwith3(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DSTwith3 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#ifdef USE_CONCURRENT_RANGES
    WITH3_USECONCURRENTRANGES (arg_node) = TRUE;
#else  /* USE_CONCURRENT_RANGES */
    WITH3_USECONCURRENTRANGES (arg_node) = FALSE;
#endif /* USE_CONCURRENT_RANGES */

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DSTrange(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
DSTrange (node *arg_node, info *arg_info)
{
    int previous_height, current_width, old_global, num_elements;
    int init_avail, level_avail, threshold;
    int level_used;

    DBUG_ENTER ();

    /* count width */
    INFO_WIDTH (arg_info)++;
    /* remember how many threads we can _conceptually_ use on this level */
    init_avail = INFO_AVAIL (arg_info);

    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    /* remember how many threads surrounding ranges use */
    level_used = INFO_USED (arg_info);
    /* remember how many threads are _actually_ left by concurrent ranges */
    level_avail = INFO_AVAIL (arg_info);
    INFO_AVAIL (arg_info) = init_avail;

    /*
     * manage thread distribution
     */

    old_global = INFO_GLOBALS (arg_info);

    switch (global.mutc_distribution_mode) {
    case MUTC_DMODE_default:
    case MUTC_DMODE_toplevel:
        RANGE_ISGLOBAL (arg_node) = (INFO_DOWN (arg_info) == 0);

        break;
    case MUTC_DMODE_bounded:
        threshold = global.mutc_distribution_mode_arg;
        DBUG_PRINT ("bounded mode: reached %d, max %d.", INFO_GLOBALS (arg_info),
                    threshold);

        if ((NODE_TYPE (RANGE_LOWERBOUND (arg_node)) == N_num)
            && (NODE_TYPE (RANGE_UPPERBOUND (arg_node)) == N_num)) {
            num_elements = NUM_VAL (RANGE_UPPERBOUND (arg_node))
                           - NUM_VAL (RANGE_LOWERBOUND (arg_node));

            if (num_elements <= 1) {
                /* do not distribute 1 element creates -> no point */
                RANGE_ISGLOBAL (arg_node) = FALSE;
                DBUG_PRINT ("singleton create --> local");
            } else if (INFO_GLOBALS (arg_info) * num_elements <= threshold) {
                RANGE_ISGLOBAL (arg_node) = TRUE;
                INFO_GLOBALS (arg_info) *= num_elements;
                DBUG_PRINT ("globals left, now using %d --> global",
                            INFO_GLOBALS (arg_info));
            } else {
                RANGE_ISGLOBAL (arg_node) = FALSE;
                DBUG_PRINT ("globals exhausted, cannot fit %d more --> local",
                            num_elements);
            }
        } else {
            if (INFO_GLOBALS (arg_info) <= threshold) {
                /*
                 * we don't know how this will change the distribution, so we
                 * just inhibit further global creates
                 */
                RANGE_ISGLOBAL (arg_node) = TRUE;
                INFO_GLOBALS (arg_info) = threshold;
                DBUG_PRINT ("unknown size, grab all --> global");
            } else {
                RANGE_ISGLOBAL (arg_node) = FALSE;
                DBUG_PRINT ("unknown size, exhausted --> local");
            }
        }
        break;

    default:
        DBUG_UNREACHABLE ("unknown distribution mode...");
        break;
    }

    /*
     * we go down a new branch of the concurrency tree, so we start
     * afresh with 0 for the height. However, when going back up we
     * have to use the maximum of this range's height and the height
     * of previous ranges.
     * Furthermore, we have to split the available threads between the
     * branches.
     */
    previous_height = INFO_UP (arg_info);
#ifdef USE_CONCURRENT_RANGES
    /*
     * split threads equally amongst ranges and reserve
     * at least THROTTLE many for us
     */
    INFO_AVAIL (arg_info)
      = INFO_AVAIL (arg_info) / INFO_WIDTH (arg_info) - INFO_THROTTLE (arg_info);
#else  /* USE_CONCURRENT_RANGES */
    /*
     * reserve at least one so that we can continue computation
     */
    INFO_AVAIL (arg_info) = INFO_AVAIL (arg_info) - 1;
#endif /* USE_CONCURRENT_RANGES */

    current_width = INFO_WIDTH (arg_info);
    INFO_WIDTH (arg_info) = UNKNOWN;
    INFO_USED (arg_info) = 1;

    INFO_UP (arg_info) = UNKNOWN;
    INFO_DOWN (arg_info)++;
    RANGE_RESULTS (arg_node) = TRAVdo (RANGE_RESULTS (arg_node), arg_info);
    INFO_DOWN (arg_info)--;
    INFO_UP (arg_info)++;

    /*
     * annotate resources
     */
    if (INFO_UP (arg_info) == INFINITE) {
        /* fall back, use the on we reserved */
        RANGE_BLOCKSIZE (arg_node) = 1;
#ifdef USE_CONCURRENT_RANGES
    } else if (INFO_UP (arg_info) == 1) {
        /* bottom level -> take it all */
        RANGE_BLOCKSIZE (arg_node) = init_avail / current_width;

        if (RANGE_BLOCKSIZE (arg_node) < INFO_THROTTLE (arg_info)) {
            RANGE_BLOCKSIZE (arg_node) = INFO_THROTTLE (arg_info);
            INFO_FAILED (arg_info) = TRUE;
        }
    } else {
        /* take the throttle level */
        RANGE_BLOCKSIZE (arg_node) = INFO_THROTTLE (arg_info);
#else  /* USE_CONCURRENT_RANGES */
    } else {
        /*
         * we could use all the ones available. But maybe that is too much.
         */
        /* first give back the one we reserved */
        INFO_AVAIL (arg_info)++;
        /* check how many we need */
        if ((NODE_TYPE (RANGE_LOWERBOUND (arg_node)) == N_num)
            && (NODE_TYPE (RANGE_UPPERBOUND (arg_node)) == N_num)) {
            num_elements = NUM_VAL (RANGE_UPPERBOUND (arg_node))
                           - NUM_VAL (RANGE_LOWERBOUND (arg_node));

            /*
             * Use no more than we can sustain (num_elements).
             * As a further constraint, we can only use so many that the threads
             * used by the tree below us still can compute!
             */
            RANGE_BLOCKSIZE (arg_node)
              = MIN (INFO_AVAIL (arg_info) / INFO_USED (arg_info), num_elements);

            INFO_AVAIL (arg_info) -= RANGE_BLOCKSIZE (arg_node);
        } else {
            /* We don't know. Let's use half of them. If this range turns out to
             * be small, we still have enough threads to use a BS of 2 above.
             */
            RANGE_BLOCKSIZE (arg_node)
              = MAX ((INFO_AVAIL (arg_info) / INFO_USED (arg_info) / 2), 1);
            INFO_AVAIL (arg_info) -= RANGE_BLOCKSIZE (arg_node);
        }

        /*
         * if this create is global, it may not use more than
         * THREADTABLESIZE / FAMILYTABLESIZE many threads to ensure that
         * there is enough space left for other creates to succeed!
         */
        if ((RANGE_ISGLOBAL (arg_node))
            && (RANGE_BLOCKSIZE (arg_node) > (global.max_threads / FAMILYTABLESIZE))) {
            /* give some back */
            INFO_AVAIL (arg_info)
              += RANGE_BLOCKSIZE (arg_node) - (global.max_threads / FAMILYTABLESIZE);
            RANGE_BLOCKSIZE (arg_node) = global.max_threads / FAMILYTABLESIZE;
        }
        INFO_USED (arg_info) *= RANGE_BLOCKSIZE (arg_node);
#endif /* USE_CONCURRENT_RANGES */
    }

    INFO_UP (arg_info) = MAX_HEIGHT (INFO_UP (arg_info), previous_height);
    INFO_WIDTH (arg_info) = current_width;
#ifdef USE_CONCURRENT_RANGES
    INFO_AVAIL (arg_info) = init_avail;
#else  /* USE_CONCURRENT_RANGES */
    INFO_AVAIL (arg_info) = MIN (INFO_AVAIL (arg_info), level_avail);
    INFO_USED (arg_info) = MAX (INFO_USED (arg_info), level_used);
#endif /* USE_CONCURRENT_RANGES */
    INFO_GLOBALS (arg_info) = old_global;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
