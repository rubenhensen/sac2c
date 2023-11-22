/*****************************************************************************
 *
 * file:   lac2fun.c
 *
 * prefix: L2F
 *
 * description:
 *
 *   This compiler module implements the conversion of conditionals and
 *   loops into their true functional representation.
 *
 * usage of arg_info (INFO_L2F_...):
 *
 *   ...FUNDEF   pointer to the current fundef
 *   ...ASSIGN   pointer to the current assign
 *
 *   ...FUNS     chain of newly generated LaC functions
 *
 *****************************************************************************/

#include <stdlib.h>

#include "LookUpTable.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"

#define DBUG_PREFIX "L2F"
#include "debug.h"

#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "infer_dfms.h"
#include "remove_dfms.h"
#include "cleanup_decls.h"
#include "namespaces.h"
#include "lac2fun.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *funs;
    node *lastcond;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_FUNS(n) (n->funs)
#define INFO_LASTCOND(n) (n->lastcond)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_FUNS (result) = NULL;
    INFO_LASTCOND (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *   char *CreateLacFunName( char *suffix)
 *
 * description:
 *   Creates a new name for a LaC function. 'suffix' should be
 *   "Cond" or "Loop".
 *
 ******************************************************************************/

static char *
CreateLacFunName (char *funname, char *suffix)
{
    static int number = 0;
    char *name;

    DBUG_ENTER ();

    name
      = (char *)MEMmalloc ((STRlen (funname) + STRlen (suffix) + 20 + 3) * sizeof (char));
    sprintf (name, "%s__%s_%i", funname, suffix, number);
    number++;

    DBUG_RETURN (name);
}

/******************************************************************************
 *
 * function:
 *   node *MakeL2fFundef( char *funname, namespace_t *ns,
 *                        node *instr, node *funcall_let,
 *                        DFMmask_t in, DFMmask_t out, DFMmask_t local,
 *                        info *arg_info)
 *
 * description:
 *   Creates the fundef-node of a LaC function.
 *
 ******************************************************************************/

static node *
MakeL2fFundef (char *funname, namespace_t *ns, node *instr, node *funcall_let,
               dfmask_t *in, dfmask_t *out, dfmask_t *local, info *arg_info)
{
    lut_t *lut;
    dfmask_t *tmp_mask;
    node *args, *vardecs, *ret, *fundef, *assigns, *let, *ass;
    node *tmp;

    DBUG_ENTER ();

    /*
     * Create a new LUT and store the old/new args and vardecs in it.
     * This is done to generate the right references in the function body.
     */
    lut = LUTgenerateLut ();
    args = DFMUdfm2Args (in, lut);
    tmp_mask = DFMgenMaskMinus (out, in);
    DFMsetMaskOr (tmp_mask, local);
    vardecs = DFMUdfm2Vardecs (tmp_mask, lut);
    tmp_mask = DFMremoveMask (tmp_mask);

    /*
     * NOTE:
     * InferDFMs() marks reference parameters as IN-parameters only, therefore
     * a ST_(readonly_)reference attribute (ARG_ATTRIB) must be preserved here!
     */

    /*
     * All args with attrib 'ST_was_reference' which are no out-vars must have
     *  the attrib 'ST_unique' instead.
     *
     * Example:
     *
     *    IntStack fun( IntStack &stack)
     *    {
     *      new_stack = create_stack();
     *      _flat_3 = !( is_empty( stack));
     *      if (_flat_3) {
     *        push( new_stack, top( stack));
     *      }
     *      stack = create_stack();
     *      return (new_stack);
     *    }
     *
     * With resolved reference parameters:
     *
     *    IntStack, IntStack fun( IntStack stack)
     *    {
     *      new_stack = create_stack();
     *      _flat_3 = !( is_empty( stack));
     *      if (_flat_3) {
     *        new_stack = push( new_stack, top( stack));
     *      }
     *      stack = create_stack();
     *      return (stack, new_stack);
     *    }
     *
     * After L2F transformation:
     *
     *    IntStack __Cond1( IntStack new_stack, bool _flat_3, IntStack stack)
     *    {
     *      if (_flat_3) {
     *        new_stack = push( new_stack, top( stack));
     *      }
     *      return (new_stack);         // 'stack' is not needed in outer block!!
     *    }
     *
     *    IntStack, IntStack fun( IntStack stack)
     *    {
     *      new_stack = create_stack();
     *      _flat_3 = !( is_empty( stack));
     *      new_stack = __Cond1( new_stack, _flat_3, stack);
     *      stack = create_stack();     // redefinition of 'stack'
     *      return (stack, new_stack);
     *    }
     *
     * Although 'stack' was marked as 'ST_was_reference' in function 'fun' that
     * is no longer true in the context of the LaC function '__Cond1' because
     * 'stack' is not an out-var of this function!!
     */
    tmp = args;

#if 0
  /*
   * as we do not annotate uniqueness to avises anymore,
   * this code is no longer needed. It just remains here for the
   * case that it becomes necessary again...
   */
  while (tmp != NULL) {
    if (ARG_ISREFERENCE( tmp)) {
      /*
       * CAUTION:
       * the arg-node is a new one not contained in the relevant DFM-base!
       * Therefore we must search for ARG_NAME instead of the pointer itself!
       */
       // NOTE: The functionality to search for the name has been removed
       //       for performance reasons. If this code is ever to be revived,
       //       the avis has to be passed instead of the name
      if (! DFMtestMaskEntry( out, ARG_NAME( tmp), NULL)) {
        AVIS_ISUNIQUE( ARG_AVIS( tmp)) = TRUE;
      }

      DBUG_PRINT ("ARG_ATTRIB[ .. %s( .. %s ..) { .. } ]: "
                          " ST_was_reference -> ST_unique",
                          funname, ARG_NAME( tmp));
    }
    tmp = ARG_NEXT( tmp);
  }
#endif

    ret = TBmakeAssign (TBmakeReturn (DFMUdfm2ReturnExprs (out, lut)), NULL);

    fundef = TBmakeFundef (STRcpy (funname), NSdupNamespace (ns), DFMUdfm2Rets (out),
                           args, NULL, /* the block is not complete yet */
                           NULL);

    FUNDEF_RETURN (fundef) = ASSIGN_STMT (ret);

    /*
     * construct the new type for the created function
     */

    switch (NODE_TYPE (instr)) {
    case N_cond:
        FUNDEF_ISCONDFUN (fundef) = TRUE;
        assigns = TBmakeAssign (DUPdoDupTreeLut (instr, lut), ret);
        break;

    case N_do:
        FUNDEF_ISLOOPFUN (fundef) = TRUE;
        FUNDEF_ISCUDALACFUN (fundef) = DO_ISCUDARIZABLE (instr);
        FUNDEF_ISFORLOOP (fundef) = DO_ISFORLOOP (instr);
        assigns = DUPdoDupTreeLut (BLOCK_ASSIGNS (DO_BODY (instr)), lut);

        /*
         * append conditional with call of loop-function to assignments.
         */
        tmp = assigns;
        if (tmp != NULL) {
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            let = DUPdoDupTreeLut (funcall_let, lut);
            AP_FUNDEF (LET_EXPR (let)) = fundef;
            AP_ISRECURSIVEDOFUNCALL (LET_EXPR (let)) = TRUE;
            FUNDEF_LOOPRECURSIVEAP (fundef) = LET_EXPR (let);
            ass = TBmakeAssign (let, NULL);
            ASSIGN_NEXT (tmp)
              = TBmakeAssign (TBmakeCond (DUPdoDupTreeLut (DO_COND (instr), lut),
                                          TBmakeBlock (ass, NULL),
                                          TBmakeBlock (NULL, NULL)),
                              ret);
        }
        break;

    default:
        DBUG_UNREACHABLE ("illegal node type found!");
        assigns = NULL;
        break;
    }

    /*
     * now we can add the body to the fundef
     */
    FUNDEF_BODY (fundef) = TBmakeBlock (assigns, vardecs);

    lut = LUTremoveLut (lut);

    DBUG_PRINT ("created function '%s:%s'", NSgetName (FUNDEF_NS (fundef)),
                FUNDEF_NAME (fundef));

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *MakeL2fFunLet( char *funname,
 *                        DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Creates the let node containing the call of a LaC function.
 *
 ******************************************************************************/

static node *
MakeL2fFunLet (node *fundef, dfmask_t *in, dfmask_t *out)
{
    node *let;

    DBUG_ENTER ();

    let = TBmakeLet (DFMUdfm2LetIds (out, NULL),
                     TBmakeAp (fundef, DFMUdfm2ApArgs (in, NULL)));

    DBUG_RETURN (let);
}

/******************************************************************************
 *
 * function:
 *   node *DoLifting( char *prefix,
 *                    DFMmask_t in, DFMmask_t out, DFMmask_t local,
 *                    node *arg_node, info *arg_info)
 *
 * description:
 *   This function carries out the lifting of a conditional or loop.
 *
 ******************************************************************************/

static node *
DoLifting (char *suffix, dfmask_t *in, dfmask_t *out, dfmask_t *local, node *arg_node,
           info *arg_info)
{
    char *funname;
    namespace_t *funns;
    node *fundef, *let;

    DBUG_ENTER ();

    /*
     * build call of the new LaC function
     */
    funname = CreateLacFunName (FUNDEF_NAME (INFO_FUNDEF (arg_info)), suffix);
    funns = FUNDEF_NS (INFO_FUNDEF (arg_info));

    DBUG_ASSERT (funns != NULL, "modul name for LAC function is NULL!");

    let = MakeL2fFunLet (NULL, in, out);

    /*
     * build new LaC function
     */
    fundef = MakeL2fFundef (funname, funns, arg_node, let, in, out, local, arg_info);

    DBUG_ASSERT (NODE_TYPE (LET_EXPR (let)) == N_ap, "N_ap expected!");

    funname = MEMfree (funname);

    /*
     * set back-references let <-> fundef
     */
    AP_FUNDEF (LET_EXPR (let)) = fundef;

    /*
     * insert new LaC function into INFO_FUNS
     */
    FUNDEF_NEXT (fundef) = INFO_FUNS (arg_info);
    INFO_FUNS (arg_info) = fundef;

    /*
     * replace the instruction by a call of the new LaC function
     */
    arg_node = FREEdoFreeTree (arg_node);
    arg_node = let;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Ffundef( node *arg_node, info *arg_info)
 *
 * description:
 *   All LaC fundefs created during traversal of the body are inserted
 *   into the AST.
 *
 ******************************************************************************/

node *
L2Ffundef (node *arg_node, info *arg_info)
{
    node *ret, *tmp;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {

        INFO_FUNDEF (arg_info) = arg_node;
        INFO_FUNS (arg_info) = NULL;
        INFO_LASTCOND (arg_info) = NULL;

        DBUG_PRINT ("processing body of `%s'", FUNDEF_NAME (arg_node));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("finished body of `%s'", FUNDEF_NAME (arg_node));
        /*
         * insert LaC fundefs into the AST
         */
        tmp = INFO_FUNS (arg_info);
        if (tmp != NULL) {
            while (FUNDEF_NEXT (tmp) != NULL) {
                tmp = FUNDEF_NEXT (tmp);
            }
            FUNDEF_NEXT (tmp) = arg_node;
            ret = INFO_FUNS (arg_info);
            INFO_FUNS (arg_info) = NULL;
        } else {
            ret = arg_node;
        }
    } else {
        ret = arg_node;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fassign( node *arg_node, info *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *
 ******************************************************************************/

node *
L2Fassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fcond( node *arg_node, info *arg_info)
 *
 * description:
 *   Lifts the conditional and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2Fcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_LASTCOND (arg_info) == NULL) {
        INFO_LASTCOND (arg_info) = arg_node;
    }

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    if ((!FUNDEF_ISLACFUN (INFO_FUNDEF (arg_info)))
        || (INFO_LASTCOND (arg_info) != arg_node)) {
        arg_node = DoLifting ("Cond", COND_IN_MASK (arg_node), COND_OUT_MASK (arg_node),
                              COND_LOCAL_MASK (arg_node), arg_node, arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fdo( node *arg_node, info *arg_info)
 *
 * description:
 *   Lifts the do-loop and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2Fdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);

    arg_node = DoLifting ("Loop", DO_IN_MASK (arg_node), DO_OUT_MASK (arg_node),
                          DO_LOCAL_MASK (arg_node), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Lac2Fun( node *syntax_tree)
 *
 * description:
 *   Converts all loops and conditions into (special) functions.
 *
 ******************************************************************************/

node *
L2FdoLac2Fun (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    /*
     * infer the in-, out- and local-masks of each conditional or loop
     * (via fixpoint iteration)
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_LAC);

    info = MakeInfo ();

    TRAVpush (TR_l2f);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    /*
     * cleanup declarations (remove unused vardecs, ...)
     */
    syntax_tree = CUDdoCleanupDecls (syntax_tree);

    /*
     * so we do not need the DFMs anymore
     */
    syntax_tree = RDFMSdoRemoveDfms (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
