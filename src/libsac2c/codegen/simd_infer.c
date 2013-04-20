#include "tree_basic.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "simd_infer.h"

/**
 *
 * @file simd_infer.c
 *
 * This file contains a simple inference which tags N_code blocks
 * either as SIMD suitable or non-suitable.
 *
 * The inference mechanism is rather crude. When called via
 *       SIMDdoInferSIMD( syntax_tree)
 *
 * it traverses syntax_tree and tags all those N_code nodes <code>
 * contained in it as  CODE_ISSIMDSUITABLE( <code> ) = TRUE
 * which do not contain:
 *
 *   a) any N_ap nodes
 *   b) any N_prf nodes whose prf is marked as NOSIMD
 *   c) any N_array node
 *
 * Then, it tags all N_WLstride nodes <wlstride> as
 * WLSTRIDE_ISSIMDSUITABLE( <wlstride>) = TRUE, iff.
 *
 *   a) The innermost N_Wlgrid is a singleton and has suitable <code>
 *   b) and the stride is bigger or equal to 2.
 *
 *
 * In compile, WLSTRIDE_ISSIMDSUITABLE is being used to create extra code!
 *
 */

/*
 * INFO structure
 */
struct INFO {
    bool suitable;
};

/*
 * INFO macros
 */
#define INFO_SUITABLE(n) ((n)->suitable)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_SUITABLE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

static const bool simd_suitable[] = {
#define PRFsimd(simd) simd
#include "prf_info.mac"
};

/*
 * OPEN PROBLEMS:
 *
 * I) not yet solved:
 *
 * II) to be fixed here:
 *
 * III) to be fixed somewhere else:
 *
 */

/**
 *
 * @defgroup
 * @ingroup
 *
 * @{
 */

/**
 *
 * @file simd_infer.c
 *
 *  This file contains the implementation of the SIMD inference
 *
 */
/**
 *
 * @name Some utility functions:
 *
 * @{
 */

/*@}*/

/** <!--*******************************************************************-->
 *
 * @name Traversal Functions for SIMD:
 *
 * @{
 ****************************************************************************/

/** <!--*******************************************************************-->
 *
 * @fn node *SIMDap( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SIMDap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SUITABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SIMDprf( node *arg_node, info *arg_info )
 *
 *****************************************************************************/
node *
SIMDprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SUITABLE (arg_info)
      = INFO_SUITABLE (arg_info) && simd_suitable[PRF_PRF (arg_node)];

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SIMDarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SIMDarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SUITABLE (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SIMDwith2( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
SIMDwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (WITH2_CODE (arg_node) != NULL, "N_with2 without code found!");

    WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);

    DBUG_ASSERT (WITH2_SEGS (arg_node) != NULL, "N_with2 wo segs found!");
    WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SIMDcode( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
SIMDcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SUITABLE (arg_info) = TRUE;

    if (CODE_CBLOCK (arg_node) != NULL) {
        CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
    }

    CODE_ISSIMDSUITABLE (arg_node) = INFO_SUITABLE (arg_info);

    INFO_SUITABLE (arg_info) = FALSE;

    if (CODE_NEXT (arg_node) != NULL) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SIMDwlstride( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
SIMDwlstride (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (WLSTRIDE_CONTENTS (arg_node) != NULL, "N_wlstride wo contents found!");
    WLSTRIDE_CONTENTS (arg_node) = TRAVdo (WLSTRIDE_CONTENTS (arg_node), arg_info);

    /**
     * if the content of this wlstride node is suitable, i.e., if
     * it consists of one wlgrid node only and that node points to suitable code
     * AND the stride range is bigger or equal to 2, we mark it accordingly.
     */
    WLSTRIDE_ISSIMDSUITABLE (arg_node)
      = INFO_SUITABLE (arg_info)
        && (WLSTRIDE_BOUND2 (arg_node) - WLSTRIDE_BOUND1 (arg_node) >= 2);

    INFO_SUITABLE (arg_info) = FALSE;

    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        WLSTRIDE_NEXT (arg_node) = TRAVdo (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--*******************************************************************-->
 *
 * @fn node *SIMDwlgrid( node *arg_node, info *arg_info)
 *
 ****************************************************************************/
node *
SIMDwlgrid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((!WLGRID_ISDYNAMIC (arg_node)) && (WLGRID_CODE (arg_node) != NULL)) {
        DBUG_ASSERT (WLGRID_NEXTDIM (arg_node) == NULL,
                     "N_wlgrid with non-NULL NEXTDIM and CODE found!");
        /**
         * we look for the following pattern:
         *  - code is marked as SIMDsuitable
         *  - there is only one grid!
         *  - the grid is one element only, i.e., no further loop!
         */
        INFO_SUITABLE (arg_info) = CODE_ISSIMDSUITABLE (WLGRID_CODE (arg_node))
                                   && (WLGRID_NEXT (arg_node) == NULL)
                                   && (NUM_VAL (WLGRID_BOUND1 (arg_node)) == 0)
                                   && (NUM_VAL (WLGRID_BOUND2 (arg_node)) == 1);
    } else {
        DBUG_ASSERT (WLGRID_NEXTDIM (arg_node) != NULL,
                     "N_wlgrid with NULL NEXTDIM and CODE found!");
        WLGRID_NEXTDIM (arg_node) = TRAVdo (WLGRID_NEXTDIM (arg_node), arg_info);

        if (WLGRID_NEXT (arg_node) != NULL) {
            WLGRID_NEXT (arg_node) = TRAVdo (WLGRID_NEXT (arg_node), arg_info);
        }

        INFO_SUITABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn  node *SIMDdoInferSIMD( node *syntax_tree)
 *
 *   @brief call this function for inferring suitability for SIMD code generation.
 *   @param part of the AST (usually the entire tree) IVE is to be applied on.
 *   @return modified AST.
 *
 *****************************************************************************/
node *
SIMDdoInferSIMD (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_simd);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*@}*/
/*@}*/ /* defgroup ive */

#undef DBUG_PREFIX
