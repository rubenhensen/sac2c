/*
 *
 * $Log$
 * Revision 1.1  2005/09/27 17:23:16  sbs
 * Initial revision
 *
 *
 *
 */

#include "tree_basic.h"
#include "dbug.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "simd_infer.h"

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SUITABLE (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

static const bool SIMD_suitable[] = {
#define PRF_IF(a, b, c, d, e, f, g, h, i) i
#include "prf_node_info.mac"
#undef PRF_IF
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
    DBUG_ENTER ("SIMDap");

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
    DBUG_ENTER ("SIMDprf");

    INFO_SUITABLE (arg_info)
      = INFO_SUITABLE (arg_info) && SIMD_suitable[PRF_PRF (arg_node)];

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
    DBUG_ENTER ("SIMDarray");

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
    DBUG_ENTER ("SIMDwith2");

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
    DBUG_ENTER ("SIMDcode");

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
    DBUG_ENTER ("SIMDwlstride");

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
    DBUG_ENTER ("SIMDwlgrid");

    if (WLGRID_CODE (arg_node) != NULL) {
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
                                   && (WLGRID_BOUND1 (arg_node) == 0)
                                   && (WLGRID_BOUND2 (arg_node) == 1);
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

    DBUG_ENTER ("SIMDdoInferSIMD");

    TRAVpush (TR_simd);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);

    info = FreeInfo (info);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*@}*/
/*@}*/ /* defgroup ive */
