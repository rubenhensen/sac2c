/*****************************************************************************
 *
 * file:   rename.c
 *
 * prefix: REN
 *
 * description:
 *   This compiler module implements a simple renaming scheme for identifers.
 *   The desired renaming must be externally provided through a lookup table
 *   of N_avis nodes. This traversal then merely traverses all code including
 *   argument and vardec lists and replaces any avis node referred to in the
 *   lookup table by its specified replacement.
 *
 *****************************************************************************/

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "memory.h"
#include "traverse.h"
#include "LookUpTable.h"

#include "rename.h"

/*
 * INFO structure
 */

struct INFO {
    lut_t *lut;
};

/*
 * INFO macros
 */

#define INFO_LUT(n) ((n)->lut)

/*
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LUT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);
    /*
     * The lookup table is not de-allocated here because it is externally
     * provided must be taken care of in the calling context.
     */

    DBUG_RETURN (info);
}

/*
 * Traversal functions
 */

node *
RENarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_AVIS (arg_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ARG_AVIS (arg_node));

    ARG_NEXT (arg_node) = TRAVopt(ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RENvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    VARDEC_AVIS (arg_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), VARDEC_AVIS (arg_node));

    VARDEC_NEXT (arg_node) = TRAVopt(VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
RENid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ID_AVIS (arg_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), ID_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

node *
RENids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    IDS_AVIS (arg_node)
      = (node *)LUTsearchInLutPp (INFO_LUT (arg_info), IDS_AVIS (arg_node));

    IDS_NEXT (arg_node) = TRAVopt(IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 * Startup function
 */

node *
RENdoRenameLut (node *arg_node, lut_t *rename_lut)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (rename_lut != NULL, "RENdoRenameLut() called without lookup table");

    info = MakeInfo ();

    INFO_LUT (info) = rename_lut;

    TRAVpush (TR_ren);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    FreeInfo (info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
