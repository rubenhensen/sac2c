/** <!--********************************************************************-->
 *
 * @defgroup swld Set With-Loop Depth
 *
 * @brief: This traversal sets AVIS_DEFDEPTH for each variable
 *         defined in a user-defined function to the number of WLs
 *         that surround each variable.
 *
 *****************************************************************************
 *
 * @ingroup swld
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file set_withloop_depth.c
 *
 * Prefix: SWLD
 *
 *****************************************************************************/
#include "set_withloop_depth.h"

#define DBUG_PREFIX "SWLD"
#include "debug.h"

#include "traverse.h"
#include "free.h"
#include "tree_basic.h"
#include "memory.h"
#include "new_types.h"
#include "constants.h"
#include "tree_compound.h"
#include "check.h"
#include "globals.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    int defdepth;
    bool checkmode;
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_DEFDEPTH(n) ((n)->defdepth)
#define INFO_CHECKMODE(n) ((n)->checkmode)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_DEFDEPTH (result) = 0;
    INFO_CHECKMODE (result) = FALSE;

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
 * @fn node *SWLDdoSetWithloopDepth( node *arg_node)
 *
 * @brief: Set with-loop depth (nesting level) for a function.
 *
 *****************************************************************************/
node *
SWLDdoSetWithloopDepth (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Expected N_fundef");

    DBUG_PRINT ("Starting set-withloop depth traversal");
    arg_info = MakeInfo ();

    TRAVpush (TR_swld);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DBUG_PRINT ("Completed set-withloop depth traversal.");

    arg_info = FreeInfo (arg_info);
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *SWLDvardec( node *arg_node, info *arg_info)
 *
 * description: This node is a paranoia check only
 *
 ******************************************************************************/
node *
SWLDvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CHECKMODE (arg_info)) {
        DBUG_PRINT ("Checking for non-negative vardec AVIS_DEFDEPTH( %s) = %d",
                    AVIS_NAME (VARDEC_AVIS (arg_node)),
                    AVIS_DEFDEPTH (VARDEC_AVIS (arg_node)));
        if (-1 != AVIS_DEFDEPTH (VARDEC_AVIS (arg_node))) {
            /* We could put a DBUG_ASSERT here, but we'd need a DCR call first */
            DBUG_PRINT ("SWLD vardec error for %s", AVIS_NAME (VARDEC_AVIS (arg_node)));
        }
    } else {
        AVIS_DEFDEPTH (VARDEC_AVIS (arg_node)) = -1;
        DBUG_PRINT ("Initializing vardec AVIS_DEFDEPTH( %s) = %d",
                    AVIS_NAME (VARDEC_AVIS (arg_node)),
                    AVIS_DEFDEPTH (VARDEC_AVIS (arg_node)));
    }

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDarg( node *arg_node, info *arg_info)
 *
 * description: This node is a paranoia check only
 *
 ******************************************************************************/
node *
SWLDarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_CHECKMODE (arg_info)) {
        DBUG_PRINT ("Checking non-negative AVIS_DEFDEPTH( %s) = %d",
                    AVIS_NAME (ARG_AVIS (arg_node)), AVIS_DEFDEPTH (ARG_AVIS (arg_node)));
        if (-1 != AVIS_DEFDEPTH (ARG_AVIS (arg_node))) {
            DBUG_PRINT ("SWLD arg error for %s", AVIS_NAME (ARG_AVIS (arg_node)));
        }
    } else {
        AVIS_DEFDEPTH (ARG_AVIS (arg_node)) = -1;
        DBUG_PRINT ("Initializing arg AVIS_DEFDEPTH( %s) = %d",
                    AVIS_NAME (ARG_AVIS (arg_node)), AVIS_DEFDEPTH (ARG_AVIS (arg_node)));
    }

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDassign( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
SWLDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDwith( node *arg_node, info *arg_info)
 *
 * description: Increase DEFDEPTH for code within this WL.
 *
 *
 ******************************************************************************/
node *
SWLDwith (node *arg_node, info *arg_info)
{
    int olddepth;

    DBUG_ENTER ();

    olddepth = INFO_DEFDEPTH (arg_info);
    INFO_DEFDEPTH (arg_info) = INFO_DEFDEPTH (arg_info) + 1;

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    INFO_DEFDEPTH (arg_info) = olddepth;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDpart( node *arg_node, info *arg_info)
 *
 * description: Visit WITHID and N_code
 *
 *
 ******************************************************************************/
node *
SWLDpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    PART_WITHID (arg_node) = TRAVopt (PART_WITHID (arg_node), arg_info);
    PART_CODE (arg_node) = TRAVopt (PART_CODE (arg_node), arg_info);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDids( node *arg_node, info *arg_info)
 *
 * description: Mark each ids nodes with its depth
 *
 ******************************************************************************/
node *
SWLDids (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_DEFDEPTH (arg_info);
    DBUG_PRINT ("Setting DEFDEPTH(%s) = %d", AVIS_NAME (IDS_AVIS (arg_node)),
                AVIS_DEFDEPTH (IDS_AVIS (arg_node)));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDfundef( node *arg_node, info *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SWLDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("Begin SWLD in %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    INFO_FUNDEF (arg_info) = arg_node;

    if (NULL != FUNDEF_BODY (arg_node)) { /* No body? Then, no work */
        /* Set paranoia flags */
        INFO_CHECKMODE (arg_info) = FALSE;
        FUNDEF_VARDECS (arg_node) = TRAVopt (FUNDEF_VARDECS (arg_node), arg_info);
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

        /* Avoid re-traversal of vardecs */
        BLOCK_ASSIGNS (FUNDEF_BODY (arg_node))
          = TRAVopt (BLOCK_ASSIGNS (FUNDEF_BODY (arg_node)), arg_info);

        /* Test paranoia flags */
        INFO_CHECKMODE (arg_info) = TRUE;

        FUNDEF_VARDECS (arg_node) = TRAVopt (FUNDEF_VARDECS (arg_node), arg_info);
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_PRINT ("End SWLD in %s %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                FUNDEF_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLDwithid( node *arg_node, info *arg_info)
 *
 * description: Mark each ids nodes with its depth
 *
 ******************************************************************************/
node *
SWLDwithid (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    WITHID_VEC (arg_node) = TRAVopt (WITHID_VEC (arg_node), arg_info);
    WITHID_IDS (arg_node) = TRAVopt (WITHID_IDS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn bool SWLDisDefinedInThisBlock( node *avis, int wldepth)
 *
 * @brief: Predicate for checking that iv in sel( iv, PWL)
 *         is defined in this block.
 *
 * @param: avis: an N_avis
 * @param: wldepth: the current WL nesting level, i.e, 0 if not in a WL,
 *         1 if within one WL, etc.
 *
 * @result: TRUE if iv is defined in this block.
 *
 * @note: The requirement for constant iv means that
 *        a code sequence such as:
 *
 *           PWL =  with(...);
 *           s = _sel_VxA_( shape(PWL)-1, PWL);
 *
 *        will not be folded. I don't know a safe way to handle this,
 *        because in the more general case (e.g., iv is loop-carried
 *        in a FOR-loop), iv may vary in value,
 *        and the extrema on iv, if this sequence is in another WL,
 *        are not suitable for AWLF.
 *
 *        See Bug #11067.
 *
 *        TRUE here indicates a case of naked-consumer AWLF.
 *
 *
 *****************************************************************************/
bool
SWLDisDefinedInThisBlock (node *avis, int wldepth)
{
    bool z;

    DBUG_ENTER ();

    z = wldepth == AVIS_DEFDEPTH (avis);

    if (z) {
        DBUG_PRINT ("%s is defined in this block", AVIS_NAME (avis));
    } else {
        DBUG_PRINT ("%s is not defined in this block", AVIS_NAME (avis));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool SWLDisDefinedInNextOuterBlock( node *avis, int wldepth)
 *
 * @brief: Predicate for checking that iv in sel( iv, PWL)
 *         is defined in the block just outside this one.
 *         This is for standard AWLF, e.g.:
 *
 *          PWL = with(...);                DEFDEPTH n
 *          CWL = with( ...)
 *               el .= sel( iv, PWL);       DEFDEPTH n+1
 *               ...
 *
 *
 * @param: avis: an N_avis
 * @param: wldepth: the current WL nesting level, i.e, 0 if not in a WL,
 *         1 if within one WL, etc.
 *
 * @result: TRUE if iv is defined in this block.
 *
 *****************************************************************************/

#if 0
// I am commenting out this function for the time being, as it is not used
// anywhere in the compiler.  It has a weird bug in it that makes it evaluate
// to true all the time.  I'll leave the resolution to the person whp will
// actually want to use this.
bool
SWLDisDefinedInNextOuterBlock (node *avis, int wldepth)
{
    bool z;

    DBUG_ENTER ();

    // FIXME: I guess this expression should be
    // z = (1 + wldepth) == AVIS_DEPTH (avis), othewise it evaluates to
    // true all the time, which is weird.
    z = 1 + (wldepth == AVIS_DEFDEPTH (avis));

    if (z) {
        DBUG_PRINT ("%s is defined in next-outer block", AVIS_NAME (avis));
    } else {
        DBUG_PRINT ("%s is not defined in next-outer block", AVIS_NAME (avis));
    }

    DBUG_RETURN (z);
}
#endif

#undef DBUG_PREFIX
