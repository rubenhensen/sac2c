/*
 *
 * $Log$
 * Revision 1.2  2005/03/04 21:21:42  cg
 * First working revision.
 *
 * Revision 1.1  2005/02/14 11:17:24  cg
 * Initial revision
 *
 *
 */

#include "globals.h"
#include "optimize.h"
/* for global optimization counter inl_fun */
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "internal_lib.h"
#include "prepare_inlining.h"

#include "inlining.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *letids;
    node *code;
    node *vardecs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_CODE(n) (n->code)
#define INFO_VARDECS(n) (n->vardecs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_CODE (result) = NULL;
    INFO_VARDECS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Function:
 *   node *INLmodule( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
INLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses instructons if function not inlined marked
 *
 ******************************************************************************/

node *
INLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLassign( node *arg_node, info *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
INLassign (node *arg_node, info *arg_info)
{
    node *assign_to_be_removed;

    DBUG_ENTER ("INLassign");

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_CODE (arg_info) != NULL) {
        assign_to_be_removed = arg_node;
        arg_node = TCappendAssign (INFO_CODE (arg_info), ASSIGN_NEXT (arg_node));
        INFO_CODE (arg_info) = NULL;
        assign_to_be_removed = FREEdoFreeNode (assign_to_be_removed);
        inl_fun++; /* global optimization counter */

        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info)))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDEC (FUNDEF_BODY (INFO_FUNDEF (arg_info))));
            INFO_VARDECS (arg_info) = NULL;
        }
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
INLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
INLap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("INLap");

    if ((FUNDEF_ISINLINE (AP_FUNDEF (arg_node)))
#if 0
      /*
       * We still need a solution for recursive functions marked inline.
       */
      && ((FUNDEF_FUNGROUP( AP_FUNDEF( arg_node)) == NULL)
          || (FUNGROUP_INLCOUNTER( FUNDEF_FUNGROUP( AP_FUNDEF( arg_node)))
              <= global.max_inl))
#endif
    ) {
        INFO_CODE (arg_info)
          = PINLdoPrepareInlining (&INFO_VARDECS (arg_info), AP_FUNDEF (arg_node),
                                   INFO_LETIDS (arg_info), AP_ARGS (arg_node));

        if (FUNDEF_EXT_ASSIGN (AP_FUNDEF (arg_node)) != NULL) {
            /*
             * Inlined function was loop or cond function.
             */
            AP_FUNDEF (arg_node) = FREEdoFreeNode (AP_FUNDEF (arg_node));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLdoInlining( node *arg_node)
 *
 * Description:
 *   Starts function inlining.
 *
 ******************************************************************************/

node *
INLdoInlining (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("INLdoInlining");

    DBUG_PRINT ("OPT", ("FUNCTION INLINING"));

    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    arg_info = MakeInfo ();

    TRAVpush (TR_inl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    FreeInfo (arg_info);

    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    DBUG_RETURN (arg_node);
}
