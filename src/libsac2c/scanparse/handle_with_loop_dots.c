#include "handle_with_loop_dots.h"
#include "traverse.h"

#define DBUG_PREFIX "HWLD"
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
#include "print.h"

#include <strings.h>

/**
 * @file handle_with_loop_dots.c
 *
 * This file elides dots from With-Loop generators and it
 * standardises all generator relations to (lb <= iv < ub ...)
 *
 * Dots at boundary positions within genarray/ modarray - withloops
 * are replaced by the minimal/maximal possible value, eg. 0s and
 * the shape vector-1. We also 'normalize' the relations to <= for
 * lower boundaries and < for the upper ones.
 *
 * The semantics has been refined as of Nov 2018! Now, the dots no
 * longer are necessarily of maximum length. Instead, the length
 * adjusts to the length of the scalar index-vector (if present)
 * or the length of the non-dot bound (if present).
 * In case both are absent, the old behaviour is maintained.
 *
 * The reason for this extension is that it makes the resolution
 * of LHS-dots in set expressions much easier and, at the same time,
 * it enables the use an unconstrained(!) use of single dots in the
 * relations of set-expressions. An example for this is:
 *
 * Assume m to be of type int[10,10]. An expression
 *
 *   { [.,i] -> m[i] | [5] <= [i] < . }
 *
 * with the old semantics of the dot in the constraints would have meant
 *
 *   { [.,i] -> m[i] | [5] <= [i] < [9,9] }
 *
 * which is illegal. With the new, more flexible semantics of the dot in
 * relations we obtain:
 *
 *   { [.,i] -> m[i] | [5] <= [i] < [9] }
 *
 * which equates to:
 *
 *  with {
 *   ([0,5] <= [x,i] < [10,9]) : m[i][x];
 *  } : genarray( [10,9], 0 );
 *
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
    node *idxlen;
};

/* access macros */
#define INFO_HWLD_DOTSHAPE(n) ((n)->dotshape)
#define INFO_HWLD_IDXLEN(n) ((n)->idxlen)

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
    INFO_HWLD_IDXLEN (result) = NULL;

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
    node *oldidxlen = INFO_HWLD_IDXLEN (arg_info);
    INFO_HWLD_DOTSHAPE (arg_info) = NULL;
    INFO_HWLD_IDXLEN (arg_info) = NULL;

    DBUG_ENTER ();

    /*
     * get INFO_HWLD_DOTSHAPE from WITHOP:
     */
    WITH_WITHOP (arg_node) = TRAVopt (WITH_WITHOP (arg_node), arg_info);

    /*
     * trigger adjustments in the parts if needed:
     */
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    INFO_HWLD_DOTSHAPE (arg_info) = olddotshape;
    INFO_HWLD_IDXLEN (arg_info) = oldidxlen;

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
 * removes the DOTINFO and IDXLEN within the info structure, as it is no more needed
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

    /*
     * First, we collect INFO_HWLD_IDXLEN from WITHID_IDS:
     */
    DBUG_PRINT ("inferring IDXLEN");
    PART_WITHID (arg_node) = TRAVdo (PART_WITHID (arg_node), arg_info);
    /*
     * Now, we do the actual generator adjustment:
     */
    PART_GENERATOR (arg_node) = TRAVdo (PART_GENERATOR (arg_node), arg_info);
    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    /**
     * INFO_HWLD_DOTSHAPE(arg_info) and INFO_HWLD_IDXLEN(arg_info) have been
     * used now. While INFO_HWLD_IDXLEN is partition-specific and therefore
     * freed in HWLDgenerator, INFO_HWLD_DOTSHAPE is valid for all partitions
     * and, therefore, needs to be freed here.
     */
    INFO_HWLD_DOTSHAPE (arg_info) = FREEoptFreeTree(INFO_HWLD_DOTSHAPE (arg_info));

    DBUG_RETURN (arg_node);
}

/**
 * inserts INFO_HWLD_IDXLEN if WITHID_IDS exists.
 *
 * @param arg_node current node of the AST
 * @param arg_info info node
 * @return current node of the AST
 */
node *
HWLDwithid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (WITHID_IDS (arg_node) != NULL) {
        INFO_HWLD_IDXLEN (arg_info) = TBmakeNum (TCcountSpids (WITHID_IDS (arg_node)));
        DBUG_PRINT ("IDXLEN set to index-length %d!", TCcountSpids (WITHID_IDS (arg_node)));
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
        CTIabort (LINE_TO_LOC (global.linenum),
                  "Dot notation is only allowed in genarray and modarray with loops; ");
    }

    /*
     * Before we can use the shape we potentially have to shorten them
     * to the length of the scalar index vector or other, non-dot
     * generator components. Therefore, we make sure INFO_HWLD_IDXLEN
     * is set appropriately whenever some length info is given.
     * Note here, that we CANNOT put this info into INFO_HWLD_DOTSHAPE
     * as that info is shared across multiple partitions while
     * INFO_HWLD_IDXLEN is partition specific.
     */
    if (INFO_HWLD_IDXLEN (arg_info) == NULL) {
        if (!DOT_ISSINGLE (GENERATOR_BOUND1 (arg_node))) {
            INFO_HWLD_IDXLEN (arg_info)
                = TCmakePrf2 (F_sel_VxA,
                              TCcreateIntVector (1, 0, 1),
                              TCmakePrf1 (F_shape_A,
                                          DUPdoDupTree (GENERATOR_BOUND1 (arg_node))));
            DBUG_PRINT ("IDXLEN set to lower-bound length!");
        } else if (!DOT_ISSINGLE (GENERATOR_BOUND2 (arg_node))) {
            INFO_HWLD_IDXLEN (arg_info)
                = TCmakePrf2 (F_sel_VxA,
                              TCcreateIntVector (1, 0, 1),
                              TCmakePrf1 (F_shape_A,
                                          DUPdoDupTree (GENERATOR_BOUND2 (arg_node))));
            DBUG_PRINT ("IDXLEN set to upper-bound length!");
        } else if (GENERATOR_STEP (arg_node) != NULL) {
            INFO_HWLD_IDXLEN (arg_info)
                = TCmakePrf2 (F_sel_VxA,
                              TCcreateIntVector (1, 0, 1),
                              TCmakePrf1 (F_shape_A,
                                          DUPdoDupTree (GENERATOR_STEP (arg_node))));
            DBUG_PRINT ("IDXLEN set to step length!");
        } else if (GENERATOR_WIDTH (arg_node) != NULL) {
            INFO_HWLD_IDXLEN (arg_info)
                = TCmakePrf2 (F_sel_VxA,
                              TCcreateIntVector (1, 0, 1),
                              TCmakePrf1 (F_shape_A,
                                          DUPdoDupTree (GENERATOR_WIDTH (arg_node))));
            DBUG_PRINT ("IDXLEN set to width length!");
        }
    }
    DBUG_PRINT ("final IDXLEN %s", ((INFO_HWLD_IDXLEN (arg_info) == NULL)?
                                    "not set - full length!" :
                                    "set to"));
    DBUG_EXECUTE ( if (INFO_HWLD_IDXLEN (arg_info) != NULL) {
                       PRTdoPrintFile (stderr, INFO_HWLD_IDXLEN (arg_info));
                   });

    if (DOT_ISSINGLE (GENERATOR_BOUND1 (arg_node))) {
        /* replace "." by "0 * shp" */
        GENERATOR_BOUND1 (arg_node) = FREEdoFreeTree (GENERATOR_BOUND1 (arg_node));
        GENERATOR_BOUND1 (arg_node)
          = TCmakePrf2 (F_mul_SxV, TBmakeNum (0),
                        (INFO_HWLD_IDXLEN (arg_info) != NULL?
                         TCmakePrf2 (F_take_SxV,
                                     DUPdoDupTree (INFO_HWLD_IDXLEN (arg_info)),
                                     DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info))) :
                         DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info))));
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
            GENERATOR_BOUND2 (arg_node)
              = (INFO_HWLD_IDXLEN (arg_info) != NULL?
                 TCmakePrf2 (F_take_SxV,
                             DUPdoDupTree (INFO_HWLD_IDXLEN (arg_info)),
                             DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info))) :
                 DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info)));
        } else {
            /* replace "." by "shp - 1"  */
            GENERATOR_BOUND2 (arg_node)
              = FREEdoFreeTree (GENERATOR_BOUND2 (arg_node));
            GENERATOR_BOUND2 (arg_node)
              = TCmakePrf2 (F_sub_VxS,
                            (INFO_HWLD_IDXLEN (arg_info) != NULL?
                             TCmakePrf2 (F_take_SxV,
                                         DUPdoDupTree (INFO_HWLD_IDXLEN (arg_info)),
                                         DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info))) :
                             DUPdoDupTree (INFO_HWLD_DOTSHAPE (arg_info))),
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

    INFO_HWLD_IDXLEN (arg_info) = FREEoptFreeTree(INFO_HWLD_IDXLEN (arg_info));

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
