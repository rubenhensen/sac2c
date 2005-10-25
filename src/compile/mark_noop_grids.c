/**
 * $Id$
 */

#include "mark_noop_grids.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"

/**
 * INFO structure
 */
struct INFO {
    bool isnoop;
};

/**
 * INFO macros
 */
#define INFO_ISNOOP(n) ((n)->isnoop)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_ISNOOP (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 * traversal functions
 */

node *
MNGwlblock (node *arg_node, info *arg_info)
{
    bool oldinfo;

    DBUG_ENTER ("MNGwlblock");

    /*
     * traverse next dim first
     */
    if (WLBLOCK_NEXTDIM (arg_node) != NULL) {
        WLBLOCK_NEXTDIM (arg_node) = TRAVdo (WLBLOCK_NEXTDIM (arg_node), arg_info);
    }

    /*
     * check whether this dim is a noop
     */
    oldinfo = INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = TRUE;

    if (WLBLOCK_CONTENTS (arg_node) != NULL) {
        WLBLOCK_CONTENTS (arg_node) = TRAVdo (WLBLOCK_CONTENTS (arg_node), arg_info);
    }

    /*
     * transform this node in a NOOP if possible
     */
    if (INFO_ISNOOP (arg_info) && (WLBLOCK_NEXTDIM (arg_node) == NULL)) {
        if (WLBLOCK_CONTENTS (arg_node) != NULL) {
            WLBLOCK_CONTENTS (arg_node) = FREEdoFreeTree (WLBLOCK_CONTENTS (arg_node));
        }
    }

    /*
     * propagate information up
     */
    INFO_ISNOOP (arg_info) = oldinfo && INFO_ISNOOP (arg_info);

    /*
     * continue with next block in this dim
     */
    if (WLBLOCK_NEXT (arg_node) != NULL) {
        WLBLOCK_NEXT (arg_node) = TRAVdo (WLBLOCK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MNGwlublock (node *arg_node, info *arg_info)
{
    bool oldinfo;

    DBUG_ENTER ("MNGwlublock");

    /*
     * traverse next dim first
     */
    if (WLUBLOCK_NEXTDIM (arg_node) != NULL) {
        WLUBLOCK_NEXTDIM (arg_node) = TRAVdo (WLUBLOCK_NEXTDIM (arg_node), arg_info);
    }

    /*
     * check whether this dim is a noop
     */
    oldinfo = INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = TRUE;

    if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
        WLUBLOCK_CONTENTS (arg_node) = TRAVdo (WLUBLOCK_CONTENTS (arg_node), arg_info);
    }

    /*
     * transform this node in a NOOP if possible
     */
    if (INFO_ISNOOP (arg_info) && (WLUBLOCK_NEXTDIM (arg_node) == NULL)) {
        if (WLUBLOCK_CONTENTS (arg_node) != NULL) {
            WLUBLOCK_CONTENTS (arg_node) = FREEdoFreeTree (WLUBLOCK_CONTENTS (arg_node));
        }
    }

    /*
     * propagate information up
     */
    INFO_ISNOOP (arg_info) = oldinfo && INFO_ISNOOP (arg_info);

    /*
     * continue with next block in this dim
     */
    if (WLUBLOCK_NEXT (arg_node) != NULL) {
        WLUBLOCK_NEXT (arg_node) = TRAVdo (WLUBLOCK_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MNGwlstride (node *arg_node, info *arg_info)
{
    bool oldinfo;

    DBUG_ENTER ("MNGwlstride");

    /*
     * check whether this stride is a noop
     */
    oldinfo = INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = TRUE;

    if (WLSTRIDE_CONTENTS (arg_node) != NULL) {
        WLSTRIDE_CONTENTS (arg_node) = TRAVdo (WLSTRIDE_CONTENTS (arg_node), arg_info);
    }

    /*
     * transform this stride into a NOOP if it is one
     */
    if (INFO_ISNOOP (arg_info)) {
        DBUG_PRINT ("MNG", ("tagging wlstride as noop"));

        if (WLSTRIDE_CONTENTS (arg_node) != NULL) {
            WLSTRIDE_CONTENTS (arg_node) = FREEdoFreeTree (WLSTRIDE_CONTENTS (arg_node));
        }
    }

    /*
     * propagate result up
     */
    oldinfo = oldinfo && INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = oldinfo;

    /*
     * continue with next stride
     */
    if (WLSTRIDE_NEXT (arg_node) != NULL) {
        WLSTRIDE_NEXT (arg_node) = TRAVdo (WLSTRIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MNGwlstridevar (node *arg_node, info *arg_info)
{
    bool oldinfo;

    DBUG_ENTER ("MNGwlstridevar");

    /*
     * check whether this stride is a noop
     */
    oldinfo = INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = TRUE;

    if (WLSTRIDEVAR_CONTENTS (arg_node) != NULL) {
        WLSTRIDEVAR_CONTENTS (arg_node)
          = TRAVdo (WLSTRIDEVAR_CONTENTS (arg_node), arg_info);
    }

    /*
     * transform this stride into a NOOP if it is one
     */
    if (INFO_ISNOOP (arg_info)) {
        DBUG_PRINT ("MNG", ("tagging wlstridevar as noop"));

        if (WLSTRIDEVAR_CONTENTS (arg_node) != NULL) {
            WLSTRIDEVAR_CONTENTS (arg_node)
              = FREEdoFreeTree (WLSTRIDEVAR_CONTENTS (arg_node));
        }
    }

    /*
     * propagate result up
     */
    oldinfo = oldinfo && INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = oldinfo;

    /*
     * continue with next stride
     */
    if (WLSTRIDEVAR_NEXT (arg_node) != NULL) {
        WLSTRIDEVAR_NEXT (arg_node) = TRAVdo (WLSTRIDEVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MNGwlgrid (node *arg_node, info *arg_info)
{
    bool oldinfo;

    DBUG_ENTER ("MNGwlgrid");

    /*
     * check whether this grid is a noop
     */
    oldinfo = INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = TRUE;

    /*
     * there are two possibilities here:
     * - CODE != NULL: innermost dimension. check the code
     * - NEXTDIM != NULL: check inner dimensions
     */
    if (WLGRID_CODE (arg_node) != NULL) {
        WLGRID_CODE (arg_node) = TRAVdo (WLGRID_CODE (arg_node), arg_info);
    } else {
        WLGRID_NEXTDIM (arg_node) = TRAVdo (WLGRID_NEXTDIM (arg_node), arg_info);
    }

    /*
     * transform it into a NOOP
     */
#ifndef DBUG_OFF
    if (INFO_ISNOOP (arg_info)) {
        DBUG_PRINT ("MNG", ("tagging wlgrid as noop"));
    }
#endif

    WLGRID_ISNOOP (arg_node) = WLGRID_ISNOOP (arg_node) || INFO_ISNOOP (arg_info);

    if (WLGRID_ISNOOP (arg_node) && (WLGRID_NEXTDIM (arg_node) != NULL)) {
        WLGRID_NEXTDIM (arg_node) = FREEdoFreeTree (WLGRID_NEXTDIM (arg_node));
    }

    /*
     * propagate the result up
     */
    INFO_ISNOOP (arg_info) = oldinfo;
    INFO_ISNOOP (arg_info) = INFO_ISNOOP (arg_info) && WLGRID_ISNOOP (arg_node);

    /*
     * continue with next
     */
    if (WLGRID_NEXT (arg_node) != NULL) {
        WLGRID_NEXT (arg_node) = TRAVdo (WLGRID_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MNGwlgridvar (node *arg_node, info *arg_info)
{
    bool oldinfo;

    DBUG_ENTER ("MNGwlgridvar");

    /*
     * check whether this gridvar is a noop
     */
    oldinfo = INFO_ISNOOP (arg_info);
    INFO_ISNOOP (arg_info) = TRUE;

    /*
     * there are two possibilities here:
     * - CODE != NULL: innermost dimension. check the code
     * - NEXTDIM != NULL: check inner dimensions
     */
    if (WLGRIDVAR_CODE (arg_node) != NULL) {
        WLGRIDVAR_CODE (arg_node) = TRAVdo (WLGRIDVAR_CODE (arg_node), arg_info);
    } else {
        WLGRIDVAR_NEXTDIM (arg_node) = TRAVdo (WLGRIDVAR_NEXTDIM (arg_node), arg_info);
    }

    /*
     * transform it into a NOOP
     */
#ifndef DBUG_OFF
    if (INFO_ISNOOP (arg_info)) {
        DBUG_PRINT ("MNG", ("tagging wlgridvar as noop"));
    }
#endif

    WLGRIDVAR_ISNOOP (arg_node) = WLGRIDVAR_ISNOOP (arg_node) || INFO_ISNOOP (arg_info);

    if (WLGRIDVAR_ISNOOP (arg_node) && (WLGRIDVAR_NEXTDIM (arg_node) != NULL)) {
        WLGRIDVAR_NEXTDIM (arg_node) = FREEdoFreeTree (WLGRIDVAR_NEXTDIM (arg_node));
    }

    /*
     * propagate the result up
     */
    INFO_ISNOOP (arg_info) = oldinfo;
    INFO_ISNOOP (arg_info) = INFO_ISNOOP (arg_info) && WLGRIDVAR_ISNOOP (arg_node);

    /*
     * continue with next
     */
    if (WLGRIDVAR_NEXT (arg_node) != NULL) {
        WLGRIDVAR_NEXT (arg_node) = TRAVdo (WLGRIDVAR_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
MNGcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MNGcode");

    /*
     * only traverse this block and do not continue
     * with the other code blocks (NEXT) as we want
     * to infer whether this block is a noop!
     */
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
MNGlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MNGlet");

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_prf) {
        if (PRF_PRF (LET_EXPR (arg_node)) != F_noop) {
            INFO_ISNOOP (arg_info) = FALSE;
        }
    } else {
        INFO_ISNOOP (arg_info) = FALSE;
    }

    /*
     * we have to continue the traversal here as we
     * want to walk through the entire tree. Otherwise
     * we would miss all with nodes ;)
     */
    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * entry function
 */
node *
MNGdoMarkNoopGrids (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("MNGdoMarkNoopGrids");

    info = MakeInfo ();

    TRAVpush (TR_mng);

    syntax_tree = TRAVdo (syntax_tree, info);

    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}
