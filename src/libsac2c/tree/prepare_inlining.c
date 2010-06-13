/*
 * $Id$
 */

/**
 *
 * @file prepare_inlining.c
 *
 * This file prepares a function definition for being inlined at a concrete
 * application position. This functionality is used by the optimization function
 * inlining, for the inlining of loop and conditional special functions and
 * last, but not least, for the preparation of fold functions to be
 * inserted into the synchronization barrier code in the case
 * of multithreaded code generation.
 *
 * The basic idea is to avoid the introduction of renaming assignments (a=b;)
 * as far as possible. Instead the "interface variables", i.e. the function's
 * formal parameters and the variables that occur in the return statement, are
 * renamed to match those variables that appear at the corresponding syntactic
 * positions of the function application. All other variables, i.e. those being
 * local to the function are consistently renamed to avoid name clashes.
 * This allows the final inlining step to be realized in an almost naive way.
 *
 * Preparation of a function definition requires making a copy of the function
 * body. This is exploited for an elegant implementation by carefully setting
 * up a look-up table beforehand, that controls all renaming activities as well
 * the establishment of back links appropriate in the inlining context.
 *
 * The function to be inlined is a LACFUN, so it has only one calling
 * point in the calling function. Assume the LACFUN function is this:
 *
 *   res1, res2, res3 = LACFUN( cond, parm1, parm2, parm3)
 *  {
 *     ...
 *     return( z1, z2, z2);
 *  }
 *
 *  and the calling site is:
 *
 *  callz1, callz2, callz3 = LACFUN( condarg, arg1, arg2, arg3);
 *
 * Several types of renaming take place within LACFUN:
 *
 *  a. References to formal parameters are replaced by the arguments
 *     in the LACFUN call:
 *
 *      z1, z2, z3 = LACFUN( cond, parm1, parm2, parm3)
 *
 *  b. The return values in the LACFUN are renamed to the
 *     names of calling site results, except where a name
 *     appears more than once in the return statement, in which
 *     case an additional assignment is created in the LACFUN.
 *     Hence, the following renames and assigns take place:
 *
 *      z1 --> callz1;
 *      z2 --> callz2;
 *
 *      callz3 = z2;
 *
 */

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "dbug.h"
#include "traverse.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "LookUpTable.h"
#include "phase.h"
#include "globals.h"

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

    result = MEMmalloc (sizeof (info));

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

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn node *PINLfundef( node *arg_node, info *arg_info)
 *
 * @brief controls inline preparation of a function definition
 *
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLfundef (node *arg_node, info *arg_info)
{
    node *keep_letids;

    DBUG_ENTER ("PINLfundef");

    DBUG_ASSERT ((FUNDEF_BODY (arg_node) != NULL),
                 "Prepare inlining started on function declaration.");

    /* Map formal argument names to calling-site names. */
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    DBUG_EXECUTE ("PINL_LUT", LUTprintLut (stderr, inline_lut););

    /* Rename some vardec names to calling-site names. */
    if (FUNDEF_VARDEC (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = DUPdoDupTreeLut (FUNDEF_VARDEC (arg_node), inline_lut);
    }

    DBUG_EXECUTE ("PINL_LUT", LUTprintLut (stderr, inline_lut););

    keep_letids = INFO_LETIDS (arg_info);

    /* Update return var names in LUT; generate new assigns, if needed */
    FUNDEF_RETURN (arg_node) = TRAVdo (FUNDEF_RETURN (arg_node), arg_info);

    INFO_LETIDS (arg_info) = keep_letids;

    DBUG_EXECUTE ("PINL_LUT", LUTprintLut (stderr, inline_lut););

    /* Perform renames of code in function body. */
    FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLarg( node *arg_node, info *arg_info)
 *
 * @brief puts pairs consisting of formal parameters and
 *        corresponding application arguments into
 *         a preconstructed look-up table.
 *
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLarg");

    DBUG_PRINT ("PINL", ("formal parameter %s linked to argument %s",
                         AVIS_NAME (ARG_AVIS (arg_node)),
                         AVIS_NAME (ID_AVIS (EXPRS_EXPR (INFO_APARGS (arg_info))))));

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

/**<!--***********************************************************************-->
 *
 * @fn node *PINLblock( node *arg_node, info *arg_info)
 *
 * @brief traversal function for N_block node
 *
 *  We first traverse into vardecs to complete setup of the look-up table.
 *  Then we copy the assignment chain with the look-up table in effect.
 *  Finally, the trailing return statement is eliminated from the copied
 *  assignment chain and it is stored in the info structure.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLblock");

    INFO_VARDECS (arg_info) = TRAVopt (INFO_VARDECS (arg_info), arg_info);

    INFO_ASSIGNS (arg_info) = DUPdoDupTreeLut (BLOCK_INSTR (arg_node), inline_lut);

    /*
     * Due to the construction of the LUT, all identifiers' AVIS nodes will
     * be redirected to either AVIS nodes in the calling context or to copies
     * created during copying the vardec chain.
     */

    INFO_ASSIGNS (arg_info) = TRAVdo (INFO_ASSIGNS (arg_info), arg_info);
    /*
     * Here, we merely traverse the assignment chain to eliminate the trailing
     * return statement and to append renaming assignments created beforehand.
     */

    INFO_LETIDS (arg_info) = TRAVopt (INFO_LETIDS (arg_info), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLvardec( node *arg_node, info *arg_info)
 *
 * @brief A variable declaration is eliminated if it belongs to a
 *        variable that appears in the return statement; otherwise,
 *        it is renamed by traversing into avis node.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLvardec");

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    if (AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
        /*
         * Since this identifier is marked dead, it must have occurred
         * in the return statement. This identifier is renamed to a let-bound
         * variable in the calling context. Hence, the vardec must be eliminated.
         */
        DBUG_PRINT ("PINL", ("Removing vardec %p avis %p (%s)", arg_node,
                             VARDEC_AVIS (arg_node), AVIS_NAME (VARDEC_AVIS (arg_node))));

        arg_node = FREEdoFreeNode (arg_node);
    } else {
        /*
         * This identifier was not found in the return statement. Hence,
         * it is simply renamed to a fresh name to avoid name clashes
         * in the calling context.
         */
        VARDEC_AVIS (arg_node) = TRAVdo (VARDEC_AVIS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLavis( node *arg_node, info *arg_info)
 *
 * @brief Variable name is replaced by a fresh identifier.
 *        We also traverse the sons and extrema of the AVIS, unless
 *        we are the same node.
 *
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLavis (node *arg_node, info *arg_info)
{
    char *name;

    DBUG_ENTER ("PINLavis");

    name = TRAVtmpVarName (AVIS_NAME (arg_node));

    DBUG_PRINT ("PINL", ("renaming %s to %s", AVIS_NAME (arg_node), name));

    MEMfree (AVIS_NAME (arg_node));
    AVIS_NAME (arg_node) = name;

#ifdef GARBAGE
    AVIS_DIM (arg_node) = TRAVopt (AVIS_DIM (arg_node), arg_info);
    AVIS_SHAPE (arg_node) = TRAVopt (AVIS_SHAPE (arg_node), arg_info);

    this is completely wrong.

      if ((NULL != AVIS_MINVAL (arg_node)) && (arg_node != AVIS_MINVAL (arg_node)))
    {
        AVIS_MINVAL (arg_node) = TRAVdo (AVIS_MINVAL (arg_node), arg_info);
    }
    if ((NULL != AVIS_MAXVAL (arg_node)) && (arg_node != AVIS_MAXVAL (arg_node))) {
        AVIS_MAXVAL (arg_node) = TRAVdo (AVIS_MAXVAL (arg_node), arg_info);
    }
#endif //  GARBAGE

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLassign( node *arg_node, info *arg_info)
 *
 * @brief traverses assignment chain until the trailing return statement
 *        and eliminates the latter.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("PINLassign");

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return) {
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = INFO_INSERT (arg_info);
        INFO_INSERT (arg_info) = NULL;
    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLid( node *arg_node, info *arg_info)
 *
 * @brief Handling of identifiers in return statement
 *
 *  Since we never traverse into expression positions otherwise, this code
 *  is a traversal into a functions's return statement.
 *
 *  Pairs consisting of the return identifier and the variable bound to it in
 *  the application context are put into the preconstructed look-up table.
 *  Additional renaming assignments are inserted if the same variable is
 *  multiply-returned by a function or if the returned variable is a formal
 *  parameter of the function.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLid (node *arg_node, info *arg_info)
{
    node *new_avis;
    node *old_id;

    DBUG_ENTER ("PINLid");

    DBUG_ASSERT ((INFO_LETIDS (arg_info) != NULL),
                 "Number of return expressions doesn't match "
                 "number of let-bound variables.");

    /*
     * Here, we are definitely inside a return statement.
     */

    if (NODE_TYPE (AVIS_DECL (ID_AVIS (arg_node))) == N_vardec) {

        DBUG_PRINT ("PINL",
                    ("Return id is local var: %s", AVIS_NAME (ID_AVIS (arg_node))));

        new_avis = LUTsearchInLutPp (inline_lut, ID_AVIS (arg_node));
        /*
         * new_avis points to copy of original avis.
         */

        if (AVIS_NAME (ID_AVIS (arg_node))
            == LUTsearchInLutPp (inline_lut, AVIS_NAME (ID_AVIS (arg_node)))) {
            /*
             * If AVIS_NAME is not in the LUT, it has not previously occured
             * in the return statement. Therefore, we simply "replace" the ID
             * with that used in the calling context.
             * Since the copied vardec is no longer needed
             * in this case, we mark it as dead and remove it later on.
             *
             * We insert AVIS_NAME into the LUT to keep
             * track of multiple occurrences of the same identifier in the
             * return statement. This is the only purpose for storing the name
             * string in the LUT.
             */

            DBUG_PRINT ("PINL", ("Return id not previously found in return"));

            DBUG_PRINT ("PINL", ("relinking return var %s to external let var %s",
                                 AVIS_NAME (ID_AVIS (arg_node)),
                                 AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info)))));

            inline_lut = LUTupdateLutP (inline_lut, ID_AVIS (arg_node),
                                        IDS_AVIS (INFO_LETIDS (arg_info)), NULL);
            AVIS_ISDEAD (new_avis) = TRUE;

            inline_lut
              = LUTinsertIntoLutP (inline_lut, AVIS_NAME (ID_AVIS (arg_node)),
                                   AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info))));

            AVIS_SSAASSIGN (IDS_AVIS (INFO_LETIDS (arg_info)))
              = AVIS_SSAASSIGN (ID_AVIS (arg_node));

            DBUG_PRINT ("PINL", ("Relinking SSA assign of avis %p (%s) to %p",
                                 IDS_AVIS (INFO_LETIDS (arg_info)),
                                 AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info))),
                                 AVIS_SSAASSIGN (ID_AVIS (arg_node))));

        } else {
            /*
             * This identifier has previously occurred in the return statement.
             * Therefore, we cannot simply rename it to the let-variable
             * in the calling context as that would overwrite
             * the previous binding. Instead, we create a corresponding assignment,
             * which will later on be appended to the assignment chain.
             */
            DBUG_PRINT ("PINL", ("Return id previously found in return"));

            old_id = TBmakeId (new_avis);
            INFO_INSERT (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (INFO_LETIDS (arg_info)),
                                                    NULL),
                                         old_id),
                              INFO_INSERT (arg_info));

            DBUG_PRINT ("PINL", ("Created new assignment to var %s",
                                 AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info)))));

            if (global.valid_ssaform) {
                AVIS_SSAASSIGN (IDS_AVIS (INFO_LETIDS (arg_info)))
                  = INFO_INSERT (arg_info);
            }

            DBUG_PRINT ("PINL", ("Relinking SSA assign of avis %p (%s) to %p",
                                 IDS_AVIS (INFO_LETIDS (arg_info)),
                                 AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info))),
                                 INFO_INSERT (arg_info)));
        }
    } else {
        /*
         * The return variable is also a parameter of the function to be inlined.
         * Therefore, we create an assignment renaming the argument variable to the
         * let variable both in the calling context. This assignment will later on
         * be appended to the assignment chain.
         */

        DBUG_PRINT ("PINL", ("Return id is parameter of inline function"));

        old_id = TBmakeId (LUTsearchInLutPp (inline_lut, ID_AVIS (arg_node)));
        INFO_INSERT (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (IDS_AVIS (INFO_LETIDS (arg_info)), NULL),
                                     old_id),
                          INFO_INSERT (arg_info));

        DBUG_PRINT ("PINL", ("Created new assignment to var %s",
                             AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info)))));

        if (global.valid_ssaform) {
            AVIS_SSAASSIGN (IDS_AVIS (INFO_LETIDS (arg_info))) = INFO_INSERT (arg_info);
        }

        DBUG_PRINT ("PINL", ("Relinking SSA assign of avis %p (%s) to %p",
                             IDS_AVIS (INFO_LETIDS (arg_info)),
                             AVIS_NAME (IDS_AVIS (INFO_LETIDS (arg_info))),
                             INFO_INSERT (arg_info)));
    }

    INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));

    DBUG_EXECUTE ("PINL_LUT", LUTprintLut (stderr, inline_lut););

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLids( node *arg_node, info *arg_info)
 *
 * @brief adjusts the SSAASSIGN pointers of external let ids to the copies
 *        of the assignments from the body of the current function.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/

node *
PINLids (node *arg_node, info *arg_info)
{
    node *new_ssaassign, *old_ssaassign;

    DBUG_ENTER ("PINLids");

    if (global.valid_ssaform) {
        old_ssaassign = AVIS_SSAASSIGN (IDS_AVIS (arg_node));
        new_ssaassign = LUTsearchInLutPp (inline_lut, old_ssaassign);

        if (new_ssaassign != old_ssaassign) {
            DBUG_PRINT ("PINL", ("AVIS_SSAASSIGN corrected for %s.\n",
                                 AVIS_NAME (IDS_AVIS (arg_node))));
        } else {
            DBUG_PRINT ("PINL", ("AVIS_SSAASSIGN not corrected for %s.\n",
                                 AVIS_NAME (IDS_AVIS (arg_node))));
        }

        AVIS_SSAASSIGN (IDS_AVIS (arg_node)) = new_ssaassign;
    }

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *PINLdoPrepareInlining( node **vardecs,
 *                                  node *fundef,
 *                                  node *letids,
 *                                  node *apargs)
 *
 * @brief initiates inline preparation
 *
 *
 * @param vardecs  return (out) parameter which yields vardecs to be added
 *                 in calling function.
 * @param fundef   fundef node to be prepared for inlining.
 * @param letids   bound variables of function application
 * @param apargs   current arguments of function application
 *
 * @return  assignment chain ready to be inlined naively.
 *
 *****************************************************************************/

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
    fundef = PINLfundef (fundef, arg_info);
    TRAVpop ();
    /*
     * It may seem very odd to call PINLfundef right away instead of properly
     * using the traversal mechanism, but a subtle dependency between the
     * traversal mechanism, the LaC function hook mechanism, and the
     * traversal order in inlining make this design necessary.
     */

    *vardecs = INFO_VARDECS (arg_info);
    code = INFO_ASSIGNS (arg_info);

    arg_info = FreeInfo (arg_info);
    inline_lut = LUTremoveLut (inline_lut);

    DBUG_RETURN (code);
}
