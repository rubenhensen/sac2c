#include "handle_with_loop_dots.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "free.h"
#include "ctinfo.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "namespaces.h"
#include "new_types.h"
#include "globals.h"
#include "tree_compound.h"

#include <strings.h>

/**
 * @file handle_with_loop_dots.c
 *
 * This file elides dots from With-Loop generators and it 
 * standardises all generator relations to (lb <= iv < ub ...)
 *
 * Dots at boundary positions within genarray/ modarray - withloops
 * are replaced by the minimal/maximal possible value, eg. 0s and
 * the shape vector. We also 'normalize' the relations to <= for
 * lower boundaries and < for the upper ones.
 */

/**
 * arg_info in this file:
 * DOTSHAPE:    this field is used in order to transport the generic shape
 *              from Nwithid (via Nwith) to Ngenerator, where it may be used
 *              in order to replace . generator boundaries.
 */

/* INFO structure */
struct INFO {
    node *dotshape;
};

/* access macros */
#define INFO_HWLD_DOTSHAPE(n) ((n)->dotshape)

/**
 * builds an info structure.
 *
 * @return new info structure
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_HWLD_DOTSHAPE (result) = NULL;

    DBUG_RETURN (result);
}

/**
 * frees an info structure.
 *
 * @param info the info structure to free
 */
static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}


/**
 * hook to start the handle dots traversal of the AST.
 *
 * @param arg_node current AST
 * @result transformed AST without dots and dot constructs
 */
node *
HWLDdoEliminateWithLoopDots (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_hwld);

    arg_node = TRAVdo (arg_node, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (arg_node);
}

/**
 * hook for with nodes. Needed to normalize dots within withloop
 * generators. At first, the withop node is traversed in order to
 * get the result shape of the withloop, needed to calculate the
 * replacements. The shape is stored in the arg_info structure.
 * Afterwards the rest is traversed in order to replace the dots.
 *
 * @param arg_node current node within the AST
 * @param arg_info info node
 * @result transformed AST
 */

node *
HWLDwith (node *arg_node, info *arg_info)
{
    /* INFO_HWLD_DOTSHAPE is used for '.'-substitution in WLgenerators */
    /* in order to handle nested WLs correct, olddotshape stores not */
    /* processed shapes until this (maybe inner) WL is processed.    */
    /* NOTE: We have to set it to NULL here, as it might be freed    */
    /*       in HWLDpart otherwise (this can happen as the DOTSHAPE is */
    /*       not collected in all traversal modes!                   */

    node *olddotshape = INFO_HWLD_DOTSHAPE (arg_info);
    INFO_HWLD_DOTSHAPE (arg_info) = NULL;

    DBUG_ENTER ();

    /*
     * by default (TravSons), withop would be traversed last, but
     * some information from withop is needed in order to traverse
     * the rest, so the withop is handeled first.
     */

    if (WITH_WITHOP (arg_node) != NULL) {
        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);
    }

    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_HWLD_DOTSHAPE (arg_info) = olddotshape;

    DBUG_RETURN (arg_node);
}

/**
 * scans the withop node for the shape of the current withloop and stores
 * it within the arg_info node. For fold withloops a null is stored as
 * there is no shape information in fold withop nodes.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HWLDgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_HWLD_DOTSHAPE (arg_info) = DUPdoDupTree (GENARRAY_SHAPE (arg_node));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * scans the withop node for the shape of the current withloop and stores
 * it within the arg_info node. For fold withloops a null is stored as
 * there is no shape information in fold withop nodes.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HWLDmodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_HWLD_DOTSHAPE (arg_info)
      = TCmakePrf1 (F_shape_A, DUPdoDupTree (MODARRAY_ARRAY (arg_node)));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * scans the withop node for the shape of the current withloop and stores
 * it within the arg_info node. For fold withloops a null is stored as
 * there is no shape information in fold withop nodes.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HWLDfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_HWLD_DOTSHAPE (arg_info) = NULL;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * removes the DOTINFO within the info structure, as it is no more needed
 * now.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HWLDpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_HWLD_DOTSHAPE (arg_info) != NULL) {
        /**
         * the shape info in INFO_HWLD_DOTSHAPE(arg_info) has been used now!
         * Note here, that it may not be consumed in HWLDgenerator, as there may
         * exist more than one generators for a single WL now!
         */
        INFO_HWLD_DOTSHAPE (arg_info) = FREEdoFreeTree (INFO_HWLD_DOTSHAPE (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/**
 * replaces dots in generators and normalizes the generator.
 * A dot as left boundary is replaced by 0 * shape, a right boundary
 * dot is replaced by shape. the left comparison operator is normalized
 * to <= by adding 1 to the left boundary if necessary, the right
 * boundary is normalized to < by decreasing the right boundary by 1 if
 * necessary.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return transformed AST
 */
node *
HWLDgenerator (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Dots are replaced by the "shape" expressions, that are imported via
     * INFO_HWLD_DOTSHAPE( arg_info)    (cf. HWLDWithop),
     * and the bounds are adjusted so that the operator can be
     * "normalized" to:   bound1 <= iv = [...] < bound2     .
     */

    if ((INFO_HWLD_DOTSHAPE (arg_info) == NULL)
        && (DOT_ISSINGLE (GENERATOR_BOUND1 (arg_node))
            || DOT_ISSINGLE (GENERATOR_BOUND2 (arg_node)))) {
        CTIabortLine (global.linenum, "Dot notation is only allowed in genarray and "
                                      "modarray with loops; ");
    }

    if (DOT_ISSINGLE (GENERATOR_BOUND1 (arg_node))) {
        /* replace "." by "0 * shp" */
        GENERATOR_BOUND1 (arg_node) = FREEdoFreeTree (GENERATOR_BOUND1 (arg_node));
        GENERATOR_BOUND1 (arg_node)
          = TCmakePrf2 (F_mul_SxV, TBmakeNum (0),
                        DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info)));
    }

    if (GENERATOR_OP1 (arg_node) == F_wl_lt) {
        /* make <= from < and add 1 to bound */
        GENERATOR_OP1 (arg_node) = F_wl_le;
        GENERATOR_BOUND1 (arg_node)
          = TCmakePrf2 (F_add_VxS, GENERATOR_BOUND1 (arg_node), TBmakeNum (1));
    }

    if (DOT_ISSINGLE (GENERATOR_BOUND2 (arg_node))) {
        if (GENERATOR_OP2 (arg_node) == F_wl_le) {
            /* make < from <= and replace "." by "shp"  */
            GENERATOR_OP2 (arg_node) = F_wl_lt;
            GENERATOR_BOUND2 (arg_node)
              = FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
            GENERATOR_BOUND2 (arg_node) = DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info));
        } else {
            /* replace "." by "shp - 1"  */
            GENERATOR_BOUND2 (arg_node)
              = FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
            GENERATOR_BOUND2 (arg_node)
              = TCmakePrf2 (F_sub_VxS, DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info)),
                            TBmakeNum (1));
        }
    } else {
        if (GENERATOR_OP2 (arg_node) == F_wl_le) {
            /* make < from <= and add 1 to bound */
            GENERATOR_OP2 (arg_node) = F_wl_lt;
            GENERATOR_BOUND2 (arg_node)
              = TCmakePrf2 (F_add_VxS, GENERATOR_BOUND2 (arg_node), TBmakeNum (1));
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
