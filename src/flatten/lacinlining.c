/*
 * $Log$
 * Revision 1.4  2005/07/21 12:00:58  sbs
 * moved freeing of lacinline functions into free
 *
 * Revision 1.3  2005/07/20 13:10:54  ktr
 * Functions markes as ISLACINLINE are now removed during the bottom-up
 * traversal
 *
 * Revision 1.2  2005/07/19 13:04:05  sah
 * moved DowngradeConcreteArgs from fun2lac to lacinlining
 * as the local knowledge is better during inlining
 *
 * Revision 1.1  2005/05/13 16:37:34  ktr
 * Initial revision
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
#include "new_types.h"
#include "internal_lib.h"
#include "prepare_inlining.h"

#include "lacinlining.h"

/*
 * INFO structure
 */
struct INFO {
    bool onefundef;
    node *fundef;
    node *letids;
    node *code;
    node *vardecs;
};

/*
 * INFO macros
 */
#define INFO_ONEFUNDEF(n) (n->onefundef)
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

    INFO_ONEFUNDEF (result) = FALSE;
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
 *   void DowngradeConcreteArgs( node *conc_arg, node *form_arg, node *fundef)
 *
 * Description:
 *   The type of each concrete lacfun argument is downgraded to the least
 *   upper bound of the types of the formal and concrete loop arguments.
 *
 ******************************************************************************/
void
DowngradeConcreteArgs (node *conc_arg, node *form_arg, node *fundef)
{
    ntype *ftype, *ctype;
    node *newavis;

    DBUG_ENTER ("DowngradeConcreteArgs");

    if (conc_arg != NULL) {
        DBUG_ASSERT (NODE_TYPE (conc_arg) == N_exprs,
                     "Concrete function arguments must be N_exprs");
        DBUG_ASSERT (form_arg != NULL,
                     "No correspondence between formal and concrete arguments");
        DBUG_ASSERT (NODE_TYPE (form_arg) == N_arg,
                     "Formal function arguments must be N_arg");

        DowngradeConcreteArgs (EXPRS_NEXT (conc_arg), ARG_NEXT (form_arg), fundef);

        DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (conc_arg)) == N_id,
                     "Concrete function argument must be N_id");

        ftype = AVIS_TYPE (ARG_AVIS (form_arg));
        ctype = AVIS_TYPE (ID_AVIS (EXPRS_EXPR (conc_arg)));

        if (!TYeqTypes (ftype, ctype)) {
            /*
             * Only formal args being more general than concrete args
             * are acceptable
             */
            DBUG_ASSERT (TYleTypes (ctype, ftype),
                         "Formal type is more special than concrete type!");

            if (TYisAKS (ftype)) {
                /*
                 * Concrete arg is AKV while formal arg is AKS:
                 * Downgrade concrete arg to AKS
                 */
                AVIS_TYPE (ID_AVIS (EXPRS_EXPR (conc_arg)))
                  = TYfreeType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (conc_arg))));
                AVIS_TYPE (ID_AVIS (EXPRS_EXPR (conc_arg))) = TYcopyType (ftype);
            } else {
                /*
                 * Formal arg is more general than AKS:
                 * Insert assignment
                 */
                newavis
                  = TBmakeAvis (ILIBtmpVarName (ARG_NAME (form_arg)), TYcopyType (ctype));

                FUNDEF_INSTR (fundef)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (ARG_AVIS (form_arg), NULL),
                                             TBmakeId (newavis)),
                                  FUNDEF_INSTR (fundef));

                FUNDEF_VARDEC (fundef)
                  = TBmakeVardec (ARG_AVIS (form_arg), FUNDEF_VARDEC (fundef));

                ARG_AVIS (form_arg) = newavis;
            }
        }

    } else {
        DBUG_ASSERT (form_arg == NULL,
                     "No correspondence between formal and concrete arguments");
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *LINLmodule( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses fundefs only.
 *
 ******************************************************************************/
node *
LINLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLfundef( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses function bodies
 *
 ******************************************************************************/
node *
LINLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
    }

    if ((!INFO_ONEFUNDEF (arg_info)) && (FUNDEF_NEXT (arg_node) != NULL)) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLassign( node *arg_node, info *arg_info)
 *
 * Description:
 *   Traverses RHS and replaces arg_node with inlined code if necessary
 *
 ******************************************************************************/
node *
LINLassign (node *arg_node, info *arg_info)
{
    bool inlined = FALSE;

    DBUG_ENTER ("LINLassign");

    if (ASSIGN_INSTR (arg_node) != NULL) {
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (INFO_CODE (arg_info) != NULL) {
        ASSIGN_NEXT (arg_node)
          = TCappendAssign (INFO_CODE (arg_info), ASSIGN_NEXT (arg_node));

        inlined = TRUE;
        INFO_CODE (arg_info) = NULL;
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

    if (inlined) {
        /*
         * you won't believe it, but this statement as well does
         * remove the lacfunction which it points to. This funny
         * sideeffect is not documented, but may be found in
         * free_attribs.c (FREEattribsExtLink)
         */

        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLlet( node *arg_node, info *arg_info)
 *
 * Description:
 *   Remembers LHS in INFO node and traverses RHS
 *
 ******************************************************************************/
node *
LINLlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLlet");

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLap( node *arg_node, info *arg_info)
 *
 * Description:
 *   Prepares inlining of applied former special functions
 *
 ******************************************************************************/
node *
LINLap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("LINLap");

    if (FUNDEF_ISLACINLINE (AP_FUNDEF (arg_node))) {
        /*
         * Downgrade types of the concrete loop/cond arguments to meet the
         * types of the potentially reassigned formal arguments.
         */
        DowngradeConcreteArgs (AP_ARGS (arg_node), FUNDEF_ARGS (AP_FUNDEF (arg_node)),
                               AP_FUNDEF (arg_node));

        INFO_CODE (arg_info)
          = PINLdoPrepareInlining (&INFO_VARDECS (arg_info), AP_FUNDEF (arg_node),
                                   INFO_LETIDS (arg_info), AP_ARGS (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLdoLACInlining( node *arg_node)
 *
 * Description:
 *   Starts function inlining of former loop and conditional functions.
 *
 ******************************************************************************/

node *
LINLdoLACInlining (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("LINLdoLACInlining");

    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    arg_info = MakeInfo ();

    TRAVpush (TR_linl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    FreeInfo (arg_info);

    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *LINLdoLACInliningOneFundef( node *arg_node)
 *
 * Description:
 *   Starts function inlining of former loop and conditional functions in one
 *   fundef.
 *
 ******************************************************************************/
node *
LINLdoLACInliningOneFundef (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("LINLdoLACInliningOneFundef");

    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_linl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    FreeInfo (arg_info);

    DBUG_PRINT ("OPTMEM",
                ("mem currently allocated: %d bytes", global.current_allocated_mem));

    DBUG_RETURN (arg_node);
}
