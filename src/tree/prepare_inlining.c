/*
 *
 * $Log$
 * Revision 1.2  2005/03/04 21:21:42  cg
 * Some minor bugs fixed.
 *
 * Revision 1.1  2005/02/14 11:15:17  cg
 * Initial revision
 *
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "LookUpTable.h"

#include "prepare_inlining.h"

static lut_t *inline_lut = NULL;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *letids;
    node *apargs;
    node *insert;
    node *assigns;
    node *vardecs;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_APARGS(n) (n->apargs)
#define INFO_INSERT(n) (n->insert)
#define INFO_ASSIGNS(n) (n->assigns)
#define INFO_VARDECS(n) (n->vardecs)

/*
 * INFO functions
 */
static info *
MakeInfo (node *fundef, node *letids, node *apargs)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_LETIDS (result) = letids;
    INFO_APARGS (result) = apargs;
    INFO_INSERT (result) = NULL;
    INFO_ASSIGNS (result) = NULL;
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

/**************************************************************************/

node *
PINLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLfundef");

    DBUG_ASSERT ((FUNDEF_BODY (arg_node) != NULL),
                 "Prepare inlining started on function declaration.");

    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_VARDEC (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = DUPdoDupTreeLut (FUNDEF_VARDEC (arg_node), inline_lut);
    }

    FUNDEF_RETURN (arg_node) = TRAVdo (FUNDEF_RETURN (arg_node), arg_info);

    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
PINLarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLarg");

    inline_lut = LUTinsertIntoLutP (inline_lut, ARG_AVIS (arg_node),
                                    ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))));

    if (ARG_NEXT (arg_node) != NULL) {
        DBUG_ASSERT ((EXPRS_NEXT (INFO_APARGS (arg_info)) != NULL),
                     "Number of arguments doesn't match number of parameters.");
        INFO_APARGS (arg_info) = EXPRS_NEXT (INFO_APARGS (arg_info));
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
PINLblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLblock");

    if (INFO_VARDECS (arg_info) != NULL) {
        INFO_VARDECS (arg_info) = TRAVdo (INFO_VARDECS (arg_info), arg_info);
    }

    INFO_ASSIGNS (arg_info) = DUPdoDupTreeLut (BLOCK_INSTR (arg_node), inline_lut);
    /*
     * Due to the construction of the LUT all identifiers' AVIS nodes will
     * be redirected to either AVIS nodes in the calling context or to copies
     * created during copying the vardec chain.
     */

    INFO_ASSIGNS (arg_info) = TRAVdo (INFO_ASSIGNS (arg_info), arg_info);
    /*
     * Here, we merely traverse the assignment chain to eliminate the trailing
     * return-statement and to append renaming assignments created beforehand.
     */

    DBUG_RETURN (arg_node);
}

node *
PINLvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLvardec");

    if (LUTsearchInLutPp (inline_lut, VARDEC_AVIS (arg_node)) == VARDEC_AVIS (arg_node)) {
        /*
         * This identifier was not found in the return-statement. Hence,
         * it is simply renamed to a fresh name to avoid name clashes
         * in the calling context.
         */
        VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);
        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
        }
    } else {
        /*
         * Since this identifier is already in the LUT, it must have occurred
         * in the return-statement. This identifier is renamed to a let-bound
         * variable in the calling context. Hence, the vardec must be eliminated.
         */
        if (VARDEC_NEXT (arg_node) != NULL) {
            VARDEC_NEXT (arg_node) = TRAVdo (VARDEC_NEXT (arg_node), arg_info);
        }
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
PINLavis (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("PINLavis");

    name = ILIBtmpVarName (AVIS_NAME (arg_node));
    ILIBfree (AVIS_NAME (arg_node));
    AVIS_NAME (arg_node) = name;

    DBUG_RETURN (arg_node);
}

node *
PINLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLassign");

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return) {
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = INFO_INSERT (arg_info);
        INFO_INSERT (arg_info) = NULL;
    } else {
        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

node *
PINLid (node *arg_node, info *arg_info)
{
    node *new_avis;

    DBUG_ENTER ("PINLid");

    DBUG_ASSERT ((INFO_LETIDS (arg_info) != NULL),
                 "Number of return expressions doesn't match "
                 "number of let-bound variables.");

    /*
     * Here, we are definitely inside a return-statement.
     */

    if (NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node))) == N_vardec) {

        new_avis = LUTsearchInLutPp (inline_lut, ID_AVIS (arg_node));
        /*
         * new_avis points to copy of original avis.
         */

        if (AVIS_NAME (ID_AVIS (arg_node))
            == LUTsearchInLutPp (inline_lut, AVIS_NAME (ID_AVIS (arg_node)))) {
            /*
             * If AVIS_NAME is not in the LUT, it has not previously occured
             * in the return-statement. Therefore, we simply "replace" the ID with that
             * used in the calling context. We insert AVIS_NAME into the LUT to keep
             * track of multiple occurrences of the same identifier in the
             * return-statement. This is the only purpose for storing the name
             * string in the LUT.
             */

            inline_lut = LUTupdateLutP (inline_lut, ID_AVIS (arg_node),
                                        IDS_AVIS (INFO_LETIDS (arg_info)), NULL);
            inline_lut
              = LUTinsertIntoLutP (inline_lut, AVIS_NAME (ID_AVIS (arg_node)),
                                   AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info))));

            AVIS_SSAASSIGN (IDS_AVIS (INFO_LETIDS (arg_info)))
              = AVIS_SSAASSIGN (ID_AVIS (arg_node));
        } else {
            /*
             * This identifier has previously occurred in the return-statement.
             * Therefore, we cannot simply rename it to the let-variable in the calling
             * context as that would overwrite the previous binding. Instead, we
             * create a corresponding assignment, which will later on be appended to
             * the assignment chain.
             */
            INFO_INSERT (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (INFO_LETIDS (arg_info)),
                                                    NULL),
                                         TBmakeId (new_avis)),
                              INFO_INSERT (arg_info));

            AVIS_SSAASSIGN (IDS_AVIS (INFO_LETIDS (arg_info))) = INFO_INSERT (arg_info);

            DBUG_PRINT ("PINL", ("Created new assignment to var %s",
                                 AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info)))));
        }
    } else {
        /*
         * The return variable is in fact a parameter of the function to be inlined.
         * Therefore, we create an assignment renaming the argument variable to the
         * let variable both in the calling context. This assignment will later on
         * be appended to the assignment chain.
         */

        INFO_INSERT (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (INFO_LETIDS (arg_info)), NULL),
                                     TBmakeId (LUTsearchInLutPp (inline_lut,
                                                                 ID_AVIS (arg_node)))),
                          INFO_INSERT (arg_info));

        AVIS_SSAASSIGN (IDS_AVIS (INFO_LETIDS (arg_info))) = INFO_INSERT (arg_info);

        DBUG_PRINT ("PINL", ("Created new assignment to var %s",
                             AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info)))));
    }

    INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));

    DBUG_RETURN (arg_node);
}

node *
PINLdoPrepareInlining (node **vardecs, node *fundef, node *letids, node *apargs)
{
    node *code;
    info *arg_info;

    DBUG_ENTER ("PINLdoPrepareInlining");

    arg_info = MakeInfo (fundef, letids, apargs);

    if (inline_lut == NULL) {
        inline_lut = LUTgenerateLut ();
    }

    DBUG_PRINT ("PINL", ("Inline preparing function %s", FUNDEF_NAME (fundef)));

    TRAVpush (TR_pinl);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    *vardecs = INFO_VARDECS (arg_info);
    code = INFO_ASSIGNS (arg_info);

    arg_info = FreeInfo (arg_info);
    inline_lut = LUTremoveContentLut (inline_lut);

    DBUG_RETURN (code);
}
