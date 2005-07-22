/*
 *
 * $Log$
 * Revision 3.40  2005/07/22 15:05:40  sbs
 * errorneous Free on masks eliminated in DoLifting
 *
 * Revision 3.39  2005/07/21 11:58:46  sbs
 * used AP_ISRECURSIVEDOFUNCALL now
 *
 * Revision 3.38  2005/07/20 13:10:06  ktr
 * removed FUNDEF_INT_ASSIGN
 *
 * Revision 3.37  2005/07/19 13:03:29  sah
 * EXT/INT_ASSIGN is no longer maintained
 *
 * Revision 3.36  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.35  2005/04/12 11:06:38  sah
 * added DBUG_PRINT
 *
 * Revision 3.34  2005/03/04 21:21:42  cg
 * Naming og LaC functions improved and made static to module.
 * FUNDEF_USED counter etc removed.
 *
 * Revision 3.33  2004/11/30 21:58:09  ktr
 * Should work with new vardec/arg/avis constellation and ntypes now.
 *
 * Revision 3.32  2004/11/27 02:50:54  ktr
 * L2FgetLacFunName
 *
 * Revision 3.31  2004/11/25 11:19:01  mwe
 * use now correct fundef in TBmakeAp
 *
 * Revision 3.30  2004/11/24 21:01:53  mwe
 * SacDevCamp: compiles!!!
 * now: dinner!
 *
 * Revision 3.29  2004/11/17 09:05:08  ktr
 * GetLacFunName is now exported
 *
 * Revision 3.28  2004/09/30 15:14:39  sbs
 * eliminated FunTypes from ALL but wrapper functions
 * (memory concerns!)
 * Now, the function signatures of individual instances are
 * stored in the AVIS_TYPE and FUNDEF_RET_TYPE only!!!!!
 *
 * Revision 3.27  2004/09/27 19:06:18  sbs
 * FUNDEF_RET_TYPE is created as well; some DBUG_PRINTs added.
 *
 * Revision 3.26  2004/09/27 10:44:21  sah
 * added support for ntype types in MakeL2fFundef
 *
 * Revision 3.25  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.24  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.23  2003/09/11 08:37:44  sbs
 * DBUG_PRINTs OPT added.
 *
 * Revision 3.22  2002/10/16 12:09:47  dkr
 * code for RESOLVE_REFERENCES removed:
 * LaC functions no longer resolve reference parameters, but reproduce
 * them.
 *
 * Revision 3.21  2002/10/16 11:57:54  dkr
 * cpp flag RESOLVE_REFERENCES added
 *
 * Revision 3.20  2002/09/05 21:16:28  dkr
 * ooops... another bug in GetLacFunName() fixed...
 *
 * Revision 3.19  2002/09/05 21:02:35  dkr
 * GetLacFunName(): string is dynamically allocated now
 *
 * Revision 3.18  2002/09/05 19:38:45  dkr
 * L2Ffundef: ignore fundefs that are LaC-funs already!!
 *
 * Revision 3.17  2002/04/16 21:11:15  dkr
 * cpp-flag MAIN_HAS_MODNAME no longer needed
 *
 * Revision 3.16  2001/04/23 13:38:27  dkr
 * fixed a bug in Lac2Fun:
 * after lifting and clean-up InferDFMs() is called once more in order
 * to get correct DFMs!
 *
 * Revision 3.15  2001/04/19 07:42:40  dkr
 * macro F_PTR used as format string for pointers
 *
 * Revision 3.14  2001/04/02 11:06:25  nmw
 * MakeL2Ffundef inits counter/list for multiple used special fundefs.
 *
 * Revision 3.13  2001/03/22 19:21:19  dkr
 * include of tree.h eliminated
 *
 * Revision 3.12  2001/02/28 15:44:16  nmw
 * L2Fassign included in traversal to set correct FUNDEF_EXT_ASSIGN
 *
 * Revision 3.11  2001/02/14 14:39:24  dkr
 * some DFM2... functions renamed.
 * some ATTRIB/STATUS adjustment is done in DFM2... functions now.
 * ATTRIB/STATUS information of FUNDEF_TYPES is set correctly for
 * l2f-functions now.
 *
 * Revision 3.10  2001/02/13 17:36:36  dkr
 * act_tab is stacked now
 *
 * Revision 3.9  2001/02/12 21:22:58  dkr
 * FUNDEF_EXT_ASSIGN, FUNDEF_INT_ASSIGN added and set correctly
 *
 * Revision 3.8  2000/12/15 18:24:35  dkr
 * infer_dfms.h renamed into InferDFMs.h
 *
 * Revision 3.7  2000/12/15 10:43:34  dkr
 * signature of InferDFMs() modified
 *
 * Revision 3.6  2000/12/07 13:43:56  dkr
 * some includes added
 *
 * Revision 3.5  2000/12/07 11:00:35  dkr
 * DBUG-string LAC2FUN renamed into L2F
 *
 * Revision 3.4  2000/12/06 20:09:34  dkr
 * InferDFMs used
 *
 * Revision 3.3  2000/12/06 19:58:00  dkr
 * inference parts are moved into a new module infer_dfms.[ch]
 *
 * Revision 3.2  2000/11/28 09:44:40  sbs
 * compiler warning in InferMasks eliminated. For further info see "remarks"
 * section in function declaration comment.
 *
 * Revision 3.1  2000/11/20 17:59:23  sacbase
 * new release made
 *
 * Revision 1.25  2000/10/31 23:20:48  dkr
 * Trav: NWITH2_CODE might be NULL
 *
 * Revision 1.24  2000/10/17 17:03:43  dkr
 * define-flag MAIN_HAS_NO_MODNAME inverted
 *
 * Revision 1.23  2000/10/17 16:50:54  dkr
 * _MAIN replaced by macro MAIN_MOD_NAME
 *
 * Revision 1.22  2000/06/23 15:01:55  dkr
 * signature of DupTreeLUT changed
 *
 * Revision 1.21  2000/05/25 17:19:13  dkr
 * header added
 *
 * Revision 1.20  2000/03/24 15:21:25  dkr
 * fixed a bug in L2F_INFERap: varargs are handle correctly now
 *
 * Revision 1.19  2000/03/24 00:51:56  dkr
 * handling of reference parameters corrected
 *
 * Revision 1.18  2000/03/21 14:53:31  dkr
 * ASSERT added: For the time being Lac2fun() can be used after type
 * checking only
 *
 * Revision 1.17  2000/03/17 18:30:36  dkr
 * type lut_t* replaced by LUT_t
 *
 * Revision 1.16  2000/03/17 16:34:28  dkr
 * some superfluous local vars eliminated
 *
 * Revision 1.15  2000/03/17 16:00:49  dkr
 * include of cleanup_decls.h added
 *
 * Revision 1.14  2000/03/17 15:59:18  dkr
 * added call of CleanupDecls()
 *
 * Revision 1.13  2000/02/24 16:53:28  dkr
 * fixed a bug in InferMasks:
 * in case of do-loops the condition must be traversed *before* the body
 *
 * Revision 1.12  2000/02/24 15:06:07  dkr
 * some comments and dbug-output added
 *
 * Revision 1.11  2000/02/24 01:27:55  dkr
 * lac2fun completed now 8-))
 *
 * Revision 1.10  2000/02/24 00:27:55  dkr
 * lac2fun works now correct for conditionals and while-loops 8-))
 *
 * Revision 1.9  2000/02/17 11:36:07  dkr
 * FUNDEF_LAC_LET removed
 *
 * Revision 1.8  2000/02/09 16:38:00  dkr
 * Workaround for main function: For the time being the main function
 * has an empty module name. Therefore the module name for LAC functions
 * must be set by hand if the function is lifted from main().
 *
 * Revision 1.7  2000/02/09 15:06:05  dkr
 * the parts of a with-loop are traversed in the correct order now
 * the modul name of the lifted fundef is set correctly now
 *
 * Revision 1.6  2000/02/09 09:59:16  dkr
 * FUNDEF_LAC_LET added
 * global objects are handled correctly now
 *
 * Revision 1.5  2000/02/08 16:40:33  dkr
 * LAC2FUNwith() and LAC2FUNwith2() added
 *
 * Revision 1.4  2000/02/08 15:14:32  dkr
 * LAC2FUNwithid added
 * some bugs fixed
 *
 * Revision 1.3  2000/02/08 10:17:02  dkr
 * wrong include instruction removed
 *
 * Revision 1.2  2000/02/03 17:29:23  dkr
 * conditions are lifted now correctly :)
 *
 * Revision 1.1  2000/01/21 12:48:59  dkr
 * Initial revision
 *
 */

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
#include <string.h>

#include "LookUpTable.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "InferDFMs.h"
#include "cleanup_decls.h"
#include "namespaces.h"
#include "lac2fun.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *funs;
};

/*
 * INFO macros
 */
#define INFO_L2F_FUNDEF(n) (n->fundef)
#define INFO_L2F_FUNS(n) (n->funs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_L2F_FUNDEF (result) = NULL;
    INFO_L2F_FUNS (result) = NULL;

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

    DBUG_ENTER ("CreateLacFunName");

    name = (char *)ILIBmalloc ((strlen (funname) + strlen (suffix) + 20 + 3)
                               * sizeof (char));
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

    DBUG_ENTER ("MakeL2fFundef");

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
    while (tmp != NULL) {
        if (ARG_ISREFERENCE (tmp)) {
            /*
             * CAUTION:
             * the arg-node is a new one not contained in the relevant DFM-base!
             * Therefore we must search for ARG_NAME instead of the pointer itself!
             */
            if (!DFMtestMaskEntry (out, ARG_NAME (tmp), NULL)) {
                AVIS_ISUNIQUE (ARG_AVIS (tmp)) = ST_unique;
            }

            DBUG_PRINT ("L2F", ("ARG_ATTRIB[ .. %s( .. %s ..) { .. } ]: "
                                " ST_was_reference -> ST_unique",
                                funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

    ret = TBmakeAssign (TBmakeReturn (DFMUdfm2ReturnExprs (out, lut)), NULL);

    fundef
      = TBmakeFundef (ILIBstringCopy (funname), NSdupNamespace (ns), DFMUdfm2Rets (out),
                      args, NULL, /* the block is not complete yet */
                      NULL);

    FUNDEF_TYPES (fundef) = DFMUdfm2ReturnTypes (out);
    FUNDEF_RETURN (fundef) = ASSIGN_INSTR (ret);

    /*
     * construct the new type for the created function
     */

    switch (NODE_TYPE (instr)) {
    case N_cond:
        FUNDEF_ISCONDFUN (fundef) = TRUE;
        assigns = TBmakeAssign (DUPdoDupTreeLut (instr, lut), ret);
        break;

    case N_do:
        FUNDEF_ISDOFUN (fundef) = TRUE;
        assigns = DUPdoDupTreeLut (BLOCK_INSTR (DO_BODY (instr)), lut);

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
            ass = TBmakeAssign (let, NULL);
            ASSIGN_NEXT (tmp)
              = TBmakeAssign (TBmakeCond (DUPdoDupTreeLut (DO_COND (instr), lut),
                                          TBmakeBlock (ass, NULL),
                                          TBmakeBlock (TBmakeEmpty (), NULL)),
                              ret);
        }
        break;

    default:
        DBUG_ASSERT ((0), "illegal node type found!");
        assigns = NULL;
        break;
    }

    /*
     * now we can add the body to the fundef
     */
    FUNDEF_BODY (fundef) = TBmakeBlock (assigns, vardecs);

    lut = LUTremoveLut (lut);

    DBUG_PRINT ("L2F", ("created function '%s:%s'", NSgetName (FUNDEF_NS (fundef)),
                        FUNDEF_NAME (fundef)));

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

    DBUG_ENTER ("MakeL2fFunLet");

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

    DBUG_ENTER ("DoLifting");

    /*
     * build call of the new LaC function
     */
    funname = CreateLacFunName (FUNDEF_NAME (INFO_L2F_FUNDEF (arg_info)), suffix);
    funns = FUNDEF_NS (INFO_L2F_FUNDEF (arg_info));

    DBUG_ASSERT ((funns != NULL), "modul name for LAC function is NULL!");

    let = MakeL2fFunLet (NULL, in, out);

    /*
     * build new LaC function
     */
    fundef = MakeL2fFundef (funname, funns, arg_node, let, in, out, local, arg_info);

    DBUG_ASSERT ((NODE_TYPE (LET_EXPR (let)) == N_ap), "N_ap expected!");

    funname = ILIBfree (funname);

    /*
     * set back-references let <-> fundef
     */
    AP_FUNDEF (LET_EXPR (let)) = fundef;

    /*
     * insert new LaC function into INFO_L2F_FUNS
     */
    FUNDEF_NEXT (fundef) = INFO_L2F_FUNS (arg_info);
    INFO_L2F_FUNS (arg_info) = fundef;

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

    DBUG_ENTER ("L2Ffundef");

    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISCONDFUN (arg_node))
        && (!FUNDEF_ISDOFUN (arg_node))) {
        INFO_L2F_FUNDEF (arg_info) = arg_node;
        INFO_L2F_FUNS (arg_info) = NULL;

        DBUG_PRINT ("L2F", ("processing body of `%s'", FUNDEF_NAME (arg_node)));

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("L2F", ("finished body of `%s'", FUNDEF_NAME (arg_node)));
        /*
         * insert LaC fundefs into the AST
         */
        tmp = INFO_L2F_FUNS (arg_info);
        if (tmp != NULL) {
            while (FUNDEF_NEXT (tmp) != NULL) {
                tmp = FUNDEF_NEXT (tmp);
            }
            FUNDEF_NEXT (tmp) = arg_node;
            ret = INFO_L2F_FUNS (arg_info);
            INFO_L2F_FUNS (arg_info) = NULL;
        } else {
            ret = arg_node;
        }
    } else {
        ret = arg_node;
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ("L2Fassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

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
    DBUG_ENTER ("L2Fcond");

    COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

    arg_node = DoLifting ("Cond", COND_IN_MASK (arg_node), COND_OUT_MASK (arg_node),
                          COND_LOCAL_MASK (arg_node), arg_node, arg_info);

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
    DBUG_ENTER ("L2Fdo");

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

    DBUG_ENTER ("Lac2Fun");

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("LAC2FUN"));
        DBUG_PRINT ("OPT", ("  inferring DFMs"));
    }
#endif

    /*
     * infer the in-, out- and local-masks of each conditional or loop
     * (via fixpoint iteration)
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_LAC);

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("  transforming loops and conditionals"));
    }
#endif

    info = MakeInfo ();

    TRAVpush (TR_l2f);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    /*
     * cleanup declarations (remove unused vardecs, ...)
     */
    syntax_tree = CUDdoCleanupDecls (syntax_tree);

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("  inferring DFMs"));
    }
#endif

    /*
     * after lifting and clean-up the current DFMs are not valid anymore!!!
     */
    syntax_tree = INFDFMSdoInferDfms (syntax_tree, HIDE_LOCALS_NEVER);

#ifndef DBUG_OFF
    if (global.compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("                        done"));
    }
#endif

    DBUG_RETURN (syntax_tree);
}
