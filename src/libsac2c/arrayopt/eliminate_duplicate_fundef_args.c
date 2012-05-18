/*
 * $Id$
 */

/**<!--******************************************************************-->
 *
 * @file eliminate_duplicate_undef_args.c
 *
 * This traversal, eliminates duplicate
 * arguments from LACFUN calls. It does not, at present, perform
 * any other tasks theoretically performed by SISI.
 *
 * Overview of traversal:
 *
 *  We traverse all functions, including LACFUNs.
 *
 *  Whenever we see a call to a LACFUN, we set FUNDEF_NAP
 *  in the LACFUN to the N_ap node that invokes it,
 *  unless this is the recursive call from within a LOOPFUN.
 *
 *  If we are
 *
 *
 * ----------------------
 *  We traverse all functions, including LACFUNs.
 *  Whenever we see a call to a LACFUN, we check the
 *  parameter list to see if it contains duplicate entries.
 *
 *  The definition of a duplicate is a bit messier for LOOP-funs,
 *  as we have to check the recursive call within the LOOP-fun
 *  to ensure that said parameter is invariant.
 *
 *  If we do find a duplicate, we enter the LACFUN into a LUT.
 *  This is done using one LUT for all local functions hanging
 *  from the non-LACFUN. We can do this safely, because there
 *  is no sharing of N_avis nodes among functions.
 *  The rationale for this approach
 *
 *  We note that, if FUNDEF_CALLFN were properly maintained
 *  (or if I was willing to initialize it here), that all the
 *  code could happen within the scope of each LACFUN.
 *  However, that's not the case...
 *
 *  Eventually, we return to the non-LACFUN.
 *  At that point, we traverse the local functions, and
 *  replace each reference to the duplicated
 *  parameter with a reference to the original parameter by
 *  using the LUT.
 *
 *  With any luck, some other traversal will remove the now-unreferenced
 *  parameter and clean up the LACFUN header appropriately.
 *
 * Possible extensions:
 *
 *   1. Support duplicate argument elimination on non-exported, non-provided,
 *      and non-lac functions of a module.
 *
 *   2. Provide the other services of SISI. (Or, make SISI work again).
 *
 **************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "node_basic.h"
#include "new_types.h"

#define DBUG_PREFIX "EDFA"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "free.h"
#include "globals.h"
#include "constants.h"
#include "shape.h"
#include "tree_compound.h"
#include "LookUpTable.h"
#include "DupTree.h"
#include "eliminate_duplicate_fundef_args.h"

typedef enum { infer, simplify } travphases;

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    bool onefundef;
    lut_t *lutargs;
    lut_t *lutrenames;
};

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_ONEFUNDEF(n) (n->onefundef)
#define INFO_LUTARGS(n) (n->lutargs)
#define INFO_LUTRENAMES(n) (n->lutrenames)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();
    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_LUTARGS (result) = NULL;
    INFO_LUTRENAMES (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn bool IsLoopFunInvariant( node *arg_node, node *argavis,
 *                              node *rca)
 *
 * @brief
 *
 * @param
 *
 * @result true if arg_node is not a LOOPFUN.
 *         true if arg_node IS a LOOPFUN, and argavis (the current
 *         outer N_ap element) is the same as recursivecallavis
 *         (the current inner N_ap recursive call element).
 *
 ******************************************************************************/
static bool
IsLoopFunInvariant (node *arg_node, node *argavis, node *rca)
{
    bool z = TRUE;

    DBUG_ENTER ();

    if (FUNDEF_ISLOOPFUN (arg_node)) {
        z = argavis == ID_AVIS (EXPRS_EXPR (rca));
    }

    DBUG_RETURN (z);
}

/**<!--***********************************************************************-->
 *
 * @fn node *FindAndRenameDups( node *arg_node, info *arg_info)
 *
 * @brief Identify LACFUN FUNDEF_ARGS arguments that appear more than once
 *        in their N_ap function call, taking care to ignore
 *        non-loop-invariant arguments in a LOOPFUN.
 *
 * @param N_fundef arg_node to be traversed
 *
 * @result: renamed fundef node.
 *
 *         We populate two LUTs: LUTARGS to quickly(?) find
 *         duplicate N_ap arguments, and at the same time,
 *         map the argument name into the formal parameter (FUNDEF_ARG)
 *         name, and one to map the duplicated name in the
 *         FUNDEF_ARG to the first occurrence of that name.
 *
 *         E.g., if we have the following call:
 *
 *           z = Cond_1( A, B, A, D, A);
 *
 *         and Cond_1's function header is:
 *
 *           int Cond_1( int CA, int CB, int CC, int CD, int CE)
 *
 *        We put the following entries into LUTARGS:
 *
 *            A --> CA
 *            B --> CB
 *            D --> CD
 *
 *        The two latter values of A in the call are dups,
 *        as we find by looking at LUTARGS,
 *        so we put an entry into the LUTRENAMES to rename:
 *
 *           CC --> CA
 *           CE --> CA
 *
 *        The function header and call are not modified at
 *        this time, but the LACFUN body no longer references
 *        the duplicate parameters.
 *
 ******************************************************************************/
static node *
FindAndRenameDups (node *arg_node, info *arg_info)
{
    node *argavis;
    node *apargs;
    node *fundefargs = NULL;
    node *rca = NULL;
    bool lutnonempty = FALSE;
    node **lutitem = NULL;
    node *formalargavis = NULL;

    DBUG_ENTER ();

    DBUG_PRINT ("Examining LACFUN %s", FUNDEF_NAME (arg_node));

    LUTremoveContentLut (INFO_LUTARGS (arg_info));
    LUTremoveContentLut (INFO_LUTRENAMES (arg_info));

    apargs = AP_ARGS (FUNDEF_CALLAP (arg_node)); /* outer call */
    fundefargs = FUNDEF_ARGS (arg_node);         /* formal parameters */
    /* recursive callargs */
    rca = FUNDEF_LOOPRECURSIVEAP (arg_node);
    rca = (NULL != rca) ? AP_ARGS (rca) : NULL;

    while (NULL != apargs) {
        argavis = ID_AVIS (EXPRS_EXPR (apargs));
        if (IsLoopFunInvariant (arg_node, argavis, rca)) {
            lutitem = (node **)LUTsearchInLutP (INFO_LUTARGS (arg_info), argavis);
            if (NULL == lutitem) {
                /* Entry not in LUT. This is a new argument.
                 * Insert avis for set membership.
                 * Also, insert for renaming to the corresponding name inside the lacfun.
                 */
                INFO_LUTARGS (arg_info)
                  = LUTinsertIntoLutP (INFO_LUTARGS (arg_info), argavis,
                                       ARG_AVIS (fundefargs));
                DBUG_PRINT ("Non-duplicate argument %s found", AVIS_NAME (argavis));
            } else {
                /* This argument is a dup */
                formalargavis = *lutitem;
                DBUG_PRINT ("Duplicate arg %s renamed to %s in LACFUN %s",
                            AVIS_NAME (argavis), AVIS_NAME (formalargavis),
                            FUNDEF_NAME (arg_node));
                INFO_LUTRENAMES (arg_info)
                  = LUTinsertIntoLutP (INFO_LUTRENAMES (arg_info), ARG_AVIS (fundefargs),
                                       formalargavis);
                lutnonempty = TRUE;
            }
        }
        apargs = EXPRS_NEXT (apargs);
        fundefargs = ARG_NEXT (fundefargs);
        rca = (NULL != rca) ? ARG_NEXT (rca) : NULL;
    }

    if (lutnonempty) {
        DBUG_PRINT ("Performing renames for LACFUN %s", FUNDEF_NAME (arg_node));
        FUNDEF_ARGS (arg_node)
          = DUPdoDupTreeLut (FUNDEF_ARGS (arg_node), INFO_LUTRENAMES (arg_info));
        FUNDEF_BODY (arg_node)
          = DUPdoDupTreeLut (FUNDEF_BODY (arg_node), INFO_LUTRENAMES (arg_info));
    }
    LUTremoveContentLut (INFO_LUTARGS (arg_info));
    LUTremoveContentLut (INFO_LUTRENAMES (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAdoEliminateDuplicateFundefArgs(node *arg_node)
 *
 * @brief starting point of traversal
 *
 * @param N_fundef or N_module to be traversed
 *
 * @result
 *
 ******************************************************************************/
node *
EDFAdoEliminateDuplicateFundefArgs (node *arg_node)
{
    info *arg_info;
    DBUG_ENTER ();

    arg_info = MakeInfo ();
    INFO_ONEFUNDEF (arg_info) = (N_fundef == NODE_TYPE (arg_node));

    INFO_LUTARGS (arg_info) = LUTgenerateLut ();
    INFO_LUTRENAMES (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_edfa);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUTARGS (arg_info) = LUTremoveLut (INFO_LUTARGS (arg_info));
    INFO_LUTRENAMES (arg_info) = LUTremoveLut (INFO_LUTRENAMES (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAmodule(node *arg_node, info *arg_info)
 *
 * @brief traverses the functions of the module
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result
 *
 ******************************************************************************/
node *
EDFAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 * @param arg_node fundef to be traversed
 * @param arg_info
 *
 * @result Same old fundef, but LOCALFUNS may be updated.
 *
 ******************************************************************************/
node *
EDFAfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;

    DBUG_ENTER ();

    oldfundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("Looking at %s", FUNDEF_NAME (arg_node));
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    if (FUNDEF_ISLACFUN (arg_node)) {
        arg_node = FindAndRenameDups (arg_node, arg_info);
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    FUNDEF_CALLAP (arg_node) = NULL;
    INFO_FUNDEF (arg_info) = oldfundef;

    DBUG_RETURN (arg_node);
}

/**<!--***********************************************************************-->
 *
 * @fn node *EDFAap(node *arg_node, info *arg_info)
 *
 * @brief We have found a function call site.
 *        If the callee is a LACFUN, put the
 *        address of our N_ap into its fundef.
 *
 * @param arg_node node to be traversed
 * @param arg_info
 *
 * @result arg_node
 *
 ******************************************************************************/
node *
EDFAap (node *arg_node, info *arg_info)
{
    node *calledfn;

    DBUG_ENTER ();

    calledfn = AP_FUNDEF (arg_node);
    if ((FUNDEF_ISLACFUN (calledfn))
        && (calledfn != INFO_FUNDEF (arg_info))) { /* Ignore recursive call */
        DBUG_PRINT ("LACFUN: %s is called from: %s", FUNDEF_NAME (calledfn),
                    FUNDEF_NAME (INFO_FUNDEF (arg_info)));
        FUNDEF_CALLAP (calledfn) = arg_node; /* Point LACFUN at our N_ap */
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
