#include "DupTree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "new_types.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "traverse.h"
#include "ctinfo.h"

#define DBUG_PREFIX "DLFR"
#include "debug.h"

#include "dead_local_function_removal.h"

// This traversal removes dead local functions from
// the AST, when sac2c is operating in GLF mode.
//
// See sacdev email from Sven-Bodo Scholz, et al.,
// from 2017-05-17 11:25:00EST for some discussion of the matter.
//
// It would be nice to be able to remove dead wrappers,
// dead non-localfns, etc., but I do not know exactly how
// to do this, despite the above email...
// Also, limiting the work to dead local functions simplifies the job
// somewhat.
//
// The immediate rationale for this was a wild goose chase in
// POGO unit test ipbb.sac, wherein some guards were not
// being removed. Eventually, I discovered that the LACFUNs
// involved were dead, and would not be removed until long
// after all optimization cycles were completed. Besides confusing
// the implementor, it also slows down compilation by some
// unknown amount.
//
// Traversal overview:
//  This traversal operates in three passes over the current fundef
//  and its local functions:
//
//    Pass 1: TS_markalldead: Marks all lacfuns in FUNDEF_LOCALFNS as dead.
//
//    Pass 2: TS_searchfordead: Traverses body of current fundef and
//             all FUNDEF_LOCALFNS, searching for LACFUN calls via an N_ap.
//             Those functions are marked alive, and then the LACFUN
//             is traversed, the same way.
//
//             The latter fundef traversal finds things such as loopfun
//             calls within condfuns.
//
//    Pass 3: TS_bringoutyourdead: Frees any FUNDEF_LOCALFNS functions
//            that are marked as dead, and updates the FUNDEF_LOCALFNS
//            chain.
//
// NB. This traversal strongly assumes that we are operating in GLF
//     mode.

enum traversaltype_t { TS_markalldead, TS_searchfordead, TS_bringoutyourdead };

// INFO structure
struct INFO {
    node *fundef;
    enum traversaltype_t traversaltype;
    bool iscall;
};

// INFO macros
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_TRAVERSALTYPE(n) ((n)->traversaltype)
#define INFO_ISCALL(n) ((n)->iscall)

// INFO functions
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_FUNDEF (result) = NULL;
    INFO_TRAVERSALTYPE (result) = TS_markalldead;
    INFO_ISCALL (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

// helper functions

/******************************************************************************
 *
 * Function:
 *   node *DLFRfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *  Operation here depends on which pass we are in:
 *
 *  In pass 1, we mark all lacfuns as dead.
 *  In pass 2, we traverse the body of the non-lacfun,
 *             and recurse into the body of any lacfun call we
 *             observe at an N_ap.
 *
 *             For lacfuns, we recurse as above.
 *             For any lacfun we encounter, we mark it as not dead.
 *
 *   In pass 3, we do a bottom-up traversal of the non-lacfun's
 *             FUNDEF_LOCALFUN chain, freeing any lacfuns that
 *             are dead, and updating the FUNDEF_LOCALFUN
 *             chain as appropriae.
 *
 ******************************************************************************/
node *
DLFRfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ();

    DBUG_PRINT ("Starting Dead Local Function Removal in %s: %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                CTIitemName (arg_node));
    oldfundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    switch (INFO_TRAVERSALTYPE (arg_info)) {

    default:
        DBUG_UNREACHABLE ("Bad enum type");
        break;

    case TS_markalldead:
        DBUG_ASSERT (FUNDEF_ISLACFUN (arg_node), "Saw non_lacfun: %s",
                     CTIitemName (arg_node));
        DBUG_PRINT ("Marking %s as dead", CTIitemName (arg_node));
        FUNDEF_ISNEEDED (arg_node) = FALSE;
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        break;

    case TS_searchfordead:
        // If non-lacfun, or if called from N_ap,
        // search the body of this function
        if ((!FUNDEF_ISLACFUN (arg_node)) || INFO_ISCALL (arg_info)) {
            FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        }
        break;

    case TS_bringoutyourdead:
        if (!FUNDEF_ISLACFUN (arg_node)) {
            // bottom-up traversal
            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

            // If function is dead, free it, and update fundef chain
            if (!FUNDEF_ISNEEDED (arg_node)) {
                DBUG_PRINT ("Freeing %s", CTIitemName (arg_node));
                FUNDEF_LOCALFUNS (arg_node) = FREEdoFreeNode (arg_node);
                global.optcounters.dead_lfun++;
            }
        }
        break;
    }

    INFO_FUNDEF (arg_info) = oldfundef;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DLFRap( node *arg_node, info *arg_info)
 *
 * Description:
 *   This node does a function call. If we are calling a LACFUN,
 *   mark it as alive, and then traverse it.
 *
 ******************************************************************************/
node *
DLFRap (node *arg_node, info *arg_info)
{
    node *calledfun;

    DBUG_ENTER ();

    calledfun = AP_FUNDEF (arg_node);

    DBUG_PRINT ("Looking at function %s call to lacfun  %s",
                FUNDEF_NAME (INFO_FUNDEF (arg_info)), FUNDEF_NAME (calledfun));
    if (FUNDEF_ISLACFUN (calledfun)) {
        DBUG_PRINT ("Marking called function %s as not dead", FUNDEF_NAME (calledfun));
        FUNDEF_ISNEEDED (calledfun) = TRUE;
        if (INFO_FUNDEF (arg_info) != calledfun) { // Ignore recursive call
            INFO_ISCALL (arg_info) = TRUE;
            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);
            INFO_ISCALL (arg_info) = FALSE;
        }
    }

    DBUG_PRINT ("Return from traversing N_ap for function %s", FUNDEF_NAME (calledfun));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DLFRdeadLocalFunctionRemoval( node *arg_node, info *arg_info)
 *
 * Description: Traversal to remove dead local functions
 * from the AST.
 *
 ******************************************************************************/
node *
DLFRdoDeadLocalFunctionRemoval (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();
    TRAVpush (TR_dlfr);

    // Pass 1: mark all local functions as dead
    INFO_TRAVERSALTYPE (arg_info) = TS_markalldead;
    DBUG_PRINT ("Start of pass to mark all local functions as dead");
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    // Pass 2: search for live local functions, and mark same as live
    // In this pass, local functions are traversed by N_ap calls
    INFO_TRAVERSALTYPE (arg_info) = TS_searchfordead;
    DBUG_PRINT ("Start of pass to find and mark local functions as live");
    arg_node = TRAVdo (arg_node, arg_info);

    // Pass 1: mark all local functions as dead
    INFO_TRAVERSALTYPE (arg_info) = TS_bringoutyourdead;
    DBUG_PRINT ("Start of pass to remove dead local functions");
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    TRAVpop ();
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
