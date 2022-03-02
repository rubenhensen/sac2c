/*
 * The uniqueness checker will check whether unique user types are
 * consumed not more than once, with the exception that they may
 * be consumed in both the THEN- and ELSE-branches of a IF statement.
 *
 * PROBLEMS:
 *   - Generating meaningful error messages is difficult because the
 *     tree is in SSA form and has been mangled beyond recognition.
 *     Also, objects are seldomly used explicitly in user code, making
 *     an error message even more cryptic.
 *   - We intentionally skip the arguments expression of prop_in functions,
 *     to work around the fact that it is also used in the N_part of the
 *     with-loop. In actuality it will never be used at the same time,
 *     just like if-then-else, and so we should treat different with-ops
 *     as different code paths and allow simultaneous use.
 * BUGS:
 *   - None so far :)
 */

#include "check_uniqueness.h"

#include "ctinfo.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "type_utils.h"
#include "user_types.h"

/*
 * This enum is for INFO_MODE and is used to determine
 * whether we're currently traversing the normal function body,
 * an THEN part or an ELSE part of a N_cond.
 */
enum { CU_MODE_NORMAL = 0, CU_MODE_THEN = 1, CU_MODE_ELSE = 2 };

/*
 * INFO structure
 */
struct INFO {
    int mode;
    int withlooplevel;
};

/*
 * INFO macros
 */
#define INFO_MODE(n) ((n)->mode)
#define INFO_WITHLOOPLEVEL(n) ((n)->withlooplevel)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MODE (result) = CU_MODE_NORMAL;
    INFO_WITHLOOPLEVEL (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Traversal start function
 */
node *
CUdoCheckUniqueness (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_cu);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/*
 * Traversal functions
 */
node *
CUavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    bool used = AVIS_ISUNIQUECONSUMED (arg_node);
    bool thenused = AVIS_ISUNIQUECONSUMEDTHEN (arg_node);
    bool elseused = AVIS_ISUNIQUECONSUMEDELSE (arg_node);
    ntype *type = AVIS_TYPE (arg_node);

    /* Check whether this is a unique type */
    if (TYisArray (type) && TUisUniqueUserType (TYgetScalar (type))) {

        /* Check whether this declaration used at all */
        if (used == FALSE && thenused == FALSE && elseused == FALSE) {
            /*
             * This is no longer considered an error, because in some cases
             * an object may never be consumed (i.e. initializers, destructors)
             */
        }

        if (used == TRUE) {
            if (thenused == TRUE || elseused == TRUE) {
                CTIerrorLoc (NODE_LOCATION (arg_node),
                             "Unique type used in both main and then-else branch!");
            }
        }
    }

    /* Reset the flags and attributes */
    AVIS_ISUNIQUECONSUMED (arg_node) = FALSE;
    AVIS_ISUNIQUECONSUMEDTHEN (arg_node) = FALSE;
    AVIS_ISUNIQUECONSUMEDELSE (arg_node) = FALSE;
    AVIS_UNIQUEREF (arg_node) = NULL;

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * First traverse the ASSIGNs which does the actual
     * uniqueness checking. Then traverse the VARDECs to
     * reset the flags.
     */
    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    }
    if (BLOCK_VARDECS (arg_node) != NULL) {
        BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CUcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_WITHLOOPLEVEL (arg_info) = INFO_WITHLOOPLEVEL (arg_info) + 1;

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_WITHLOOPLEVEL (arg_info) = INFO_WITHLOOPLEVEL (arg_info) - 1;

    DBUG_RETURN (arg_node);
}

node *
CUcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (COND_COND (arg_node) != NULL) {
        COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    }

    /* Traverse down the THEN and ELSE parts in their respective modes. */

    INFO_MODE (arg_info) = CU_MODE_THEN;
    if (COND_THEN (arg_node) != NULL) {
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    }

    INFO_MODE (arg_info) = CU_MODE_ELSE;
    if (COND_ELSE (arg_node) != NULL) {
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    INFO_MODE (arg_info) = CU_MODE_NORMAL;

    DBUG_RETURN (arg_node);
}

node *
CUfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNCOND_IF (arg_node) != NULL) {
        FUNCOND_IF (arg_node) = TRAVdo (FUNCOND_IF (arg_node), arg_info);
    }

    /* Traverse down the THEN and ELSE parts in their respective modes. */

    INFO_MODE (arg_info) = CU_MODE_THEN;
    if (FUNCOND_THEN (arg_node) != NULL) {
        FUNCOND_THEN (arg_node) = TRAVdo (FUNCOND_THEN (arg_node), arg_info);
    }

    INFO_MODE (arg_info) = CU_MODE_ELSE;
    if (FUNCOND_ELSE (arg_node) != NULL) {
        FUNCOND_ELSE (arg_node) = TRAVdo (FUNCOND_ELSE (arg_node), arg_info);
    }

    INFO_MODE (arg_info) = CU_MODE_NORMAL;

    DBUG_RETURN (arg_node);
}

node *
CUfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * First traverse the body, which sets the UniqueConsumed flags
     * on the AVIS nodes and does the uniqueness checking. The BODY
     * traversal resets these flags for the VARDECs. The ARGS traverse
     * reset the flags in that part as well.
     */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* Only traverse the arguments if we have a body
         * (otherwise external functions break) */
        if (FUNDEF_ARGS (arg_node) != NULL) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }
    }

    /* Continue to the next function. */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
CUid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    node *avis = ID_AVIS (arg_node);
    ntype *type = AVIS_TYPE (avis);

    /*
     * Check whether this is a unique type, and if so, whether it has been
     * used before.
     */
    if (TYisArray (type) && TUisUniqueUserType (TYgetScalar (type))) {
        switch (INFO_MODE (arg_info)) {

        case CU_MODE_NORMAL: /* normal mode */
            if (AVIS_ISUNIQUECONSUMED (avis) == TRUE
                || AVIS_ISUNIQUECONSUMEDTHEN (avis) == TRUE
                || AVIS_ISUNIQUECONSUMEDELSE (avis) == TRUE) {
                CTIerror (NODE_LOCATION (arg_node),
                             "Unique var %s of type %s referenced more than once",
                             AVIS_NAME (avis),
                             UTgetName (TYgetUserType (TYgetScalar (type))));
                CTIerror (NODE_LOCATION (AVIS_UNIQUEREF (avis)),
                          "Previous reference was here");
            } else {
                AVIS_ISUNIQUECONSUMED (avis) = TRUE;
                AVIS_UNIQUEREF (avis) = arg_node;
            }
            break;

        case CU_MODE_THEN: /* then-mode */
            if (AVIS_ISUNIQUECONSUMEDTHEN (avis) == TRUE
                || AVIS_ISUNIQUECONSUMED (avis) == TRUE) {
                CTIerror (NODE_LOCATION (arg_node),
                          "Unique var %s of type %s referenced more than once",
                          AVIS_NAME (avis),
                          UTgetName (TYgetUserType (TYgetScalar (type))));
                CTIerror (NODE_LOCATION (AVIS_UNIQUEREF (avis)),
                          "Previous reference was here");
            } else {
                AVIS_ISUNIQUECONSUMEDTHEN (avis) = TRUE;
                AVIS_UNIQUEREF (avis) = arg_node;
            }
            break;

        case CU_MODE_ELSE: /* else-mode */
            if (AVIS_ISUNIQUECONSUMEDELSE (avis) == TRUE
                || AVIS_ISUNIQUECONSUMED (avis) == TRUE) {
                CTIerror (NODE_LOCATION (arg_node),
                             "Unique var %s of type %s referenced more than once",
                             AVIS_NAME (avis),
                             UTgetName (TYgetUserType (TYgetScalar (type))));
                CTIerror (NODE_LOCATION (AVIS_UNIQUEREF (avis)),
                              "Previous reference was here");
            } else {
                AVIS_ISUNIQUECONSUMEDELSE (avis) = TRUE;
                AVIS_UNIQUEREF (avis) = arg_node;
            }
            break;
        }

        if (AVIS_WITHLOOPLEVEL (avis) != INFO_WITHLOOPLEVEL (arg_info)) {
            CTIerrorLoc (NODE_LOCATION (arg_node),
                         "Unique var %s of type %s not propagated into this WithLoop at level %d",
                         AVIS_NAME (avis), UTgetName (TYgetUserType (TYgetScalar (type))),
                         INFO_WITHLOOPLEVEL (arg_info));
        }
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_WITHLOOPLEVEL (IDS_AVIS (arg_node)) = INFO_WITHLOOPLEVEL (arg_info);

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
CUprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) != F_prop_obj_in) {
        arg_node = TRAVcont (arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Skipped because of the decicion to ignore the arguments in prop_in functions
 * instead of ignoring the Object in N_propagate

node *CUpropagate( node *arg_node, info *arg_info)
{
  DBUG_ENTER("CUpropagate");

  * Skip the 'default element' son, see PROBLEMS at the top of this
  * file.

  if ( PROPAGATE_NEXT( arg_node) != NULL) {
    PROPAGATE_NEXT( arg_node) = TRAVdo( PROPAGATE_NEXT( arg_node), arg_info);
  }

  DBUG_RETURN( arg_node);
}
*/

#undef DBUG_PREFIX
