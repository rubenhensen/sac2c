/*
 *
 * $Log$
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
 *   ...FUNS     chain of newly generated dummy functions
 *
 *****************************************************************************/

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
#include "InferDFMs.h"
#include "cleanup_decls.h"
#include "lac2fun.h"

/******************************************************************************
 *
 * function:
 *   char *GetDummyFunName( char *suffix)
 *
 * description:
 *   Creates a new name for a dummy function. 'suffix' should be one of the
 *   strings "Cond", "Do" or "While".
 *
 ******************************************************************************/

static char *
GetDummyFunName (char *suffix)
{
#define NAMLEN 100
    static int number = 0;
    static char funname[NAMLEN];

    DBUG_ENTER ("GetDummyFunName");

    DBUG_ASSERT (((strlen (suffix) + number / 10 + 4) <= NAMLEN),
                 "name of dummy function too long");
    sprintf (funname, "__%s%i", suffix, number);
    number++;

    DBUG_RETURN (funname);
}

/******************************************************************************
 *
 * function:
 *   node *MakeL2fFundef( char *funname, char *modname,
 *                        node *instr, node *funcall_let,
 *                        DFMmask_t in, DFMmask_t out, DFMmask_t local,
 *                        node *arg_info)
 *
 * description:
 *   Creates the fundef-node of a dummy function.
 *
 ******************************************************************************/

static node *
MakeL2fFundef (char *funname, char *modname, node *instr, node *funcall_let, DFMmask_t in,
               DFMmask_t out, DFMmask_t local, node *arg_info)
{
    LUT_t lut;
    DFMmask_t tmp_mask;
    node *args, *vardecs, *ret, *fundef, *assigns, *new_body, *let, *ass;
    node *tmp;
    statustype status;

    DBUG_ENTER ("MakeL2fFundef");

    /*
     * Create a new LUT and store the old/new args and vardecs in it.
     * This is done to generate the right references in the function body.
     */
    lut = GenerateLUT ();
    args = DFM2Args (in, lut);
    tmp_mask = DFMGenMaskMinus (out, in);
    DFMSetMaskOr (tmp_mask, local);
    vardecs = DFM2Vardecs (tmp_mask, lut);
    tmp_mask = DFMRemoveMask (tmp_mask);

    /*
     * Convert parameters from call-by-reference into call-by-value
     *  because they have already been resolved!
     */
    tmp = args;
    while (tmp != NULL) {
        if ((ARG_ATTRIB (tmp) == ST_reference)
            || (ARG_ATTRIB (tmp) == ST_readonly_reference)) {
            ARG_ATTRIB (tmp) = ST_unique;

            DBUG_PRINT ("L2F", ("ARG_ATTRIB[ .. %s( .. %s .. ) { .. } ]: "
                                " ST_..reference -> ST_unique",
                                funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

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
     * is no longer true in the context of the dummy function '__Cond1' because
     * 'stack' is not an out-var of this function!!
     */
    tmp = args;
    while (tmp != NULL) {
        if (ARG_ATTRIB (tmp) == ST_was_reference) {
            /*
             * CAUTION: the arg-node is a new one not contained in the relevant
             * DFM-base! Therefore we must search for ARG_NAME instead of the pointer
             * itself!
             */
            if (!DFMTestMaskEntry (out, ARG_NAME (tmp), NULL)) {
                ARG_ATTRIB (tmp) = ST_unique;
            }

            DBUG_PRINT ("L2F", ("ARG_ATTRIB[ .. %s( .. %s ..) { .. } ]: "
                                " ST_was_reference -> ST_unique",
                                funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

    ret = MakeAssign (MakeReturn (DFM2ReturnExprs (out, lut)), NULL);

    fundef
      = MakeFundef (StringCopy (funname), StringCopy (modname), DFM2ReturnTypes (out),
                    args, NULL, /* the block is not complete yet */
                    NULL);
    FUNDEF_RETURN (fundef) = ASSIGN_INSTR (ret);
    FUNDEF_INT_ASSIGN (fundef) = NULL;
    FUNDEF_EXT_ASSIGNS (fundef) = NodeListAppend (NULL, INFO_L2F_ASSIGN (arg_info), NULL);
    FUNDEF_USED (fundef) = 1;
    DBUG_PRINT ("L2F",
                ("set link to external assignment: " F_PTR, INFO_L2F_ASSIGN (arg_info)));

    switch (NODE_TYPE (instr)) {
    case N_cond:
        status = ST_condfun;
        assigns = MakeAssign (DupTreeLUT (instr, lut), ret);
        break;

    case N_while:
        status = ST_whilefun;
        new_body = DupTreeLUT (WHILE_BODY (instr), lut);

        /*
         * append call of loop-dummy-function to body.
         */
        tmp = BLOCK_INSTR (new_body);
        if (tmp != NULL) {
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            let = DupTreeLUT (funcall_let, lut);
            AP_FUNDEF (LET_EXPR (let)) = fundef;
            ass = MakeAssign (let, NULL);
            FUNDEF_INT_ASSIGN (fundef) = ass;
            ASSIGN_NEXT (tmp) = ass;
        }

        assigns = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), lut), new_body,
                                        MakeBlock (MakeEmpty (), NULL)),
                              ret);
        break;

    case N_do:
        status = ST_dofun;
        assigns = DupTreeLUT (BLOCK_INSTR (WHILE_BODY (instr)), lut);

        /*
         * append conditional with call of loop-dummy-function to assignments.
         */
        tmp = assigns;
        if (tmp != NULL) {
            while (ASSIGN_NEXT (tmp) != NULL) {
                tmp = ASSIGN_NEXT (tmp);
            }
            let = DupTreeLUT (funcall_let, lut);
            AP_FUNDEF (LET_EXPR (let)) = fundef;
            ass = MakeAssign (let, NULL);
            FUNDEF_INT_ASSIGN (fundef) = ass;
            ASSIGN_NEXT (tmp)
              = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), lut),
                                      MakeBlock (ass, NULL),
                                      MakeBlock (MakeEmpty (), NULL)),
                            ret);
        }
        break;

    default:
        DBUG_ASSERT ((0), "illegal node type found!");
        assigns = NULL;
        status = ST_regular;
        break;
    }

    /*
     * now we can add the body to the fundef
     */
    FUNDEF_BODY (fundef) = MakeBlock (assigns, vardecs);
    FUNDEF_STATUS (fundef) = status;

    lut = RemoveLUT (lut);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *MakeL2fFunLet( char *funname,
 *                        DFMmask_t in, DFMmask_t out)
 *
 * description:
 *   Creates the let node containing the call of a dummy function.
 *
 ******************************************************************************/

static node *
MakeL2fFunLet (char *funname, char *modname, DFMmask_t in, DFMmask_t out)
{
    node *let;

    DBUG_ENTER ("MakeL2fFunLet");

    let = MakeLet (MakeAp (StringCopy (funname), StringCopy (modname),
                           DFM2ApArgs (in, NULL)),
                   DFM2LetIds (out, NULL));

    DBUG_RETURN (let);
}

/******************************************************************************
 *
 * function:
 *   node *DoLifting( char *prefix,
 *                    DFMmask_t in, DFMmask_t out, DFMmask_t local,
 *                    node *arg_node, node *arg_info)
 *
 * description:
 *   This function carries out the lifting of a conditional or loop.
 *
 ******************************************************************************/

static node *
DoLifting (char *prefix, DFMmask_t in, DFMmask_t out, DFMmask_t local, node *arg_node,
           node *arg_info)
{
    char *funname, *modname;
    node *fundef, *let;

    DBUG_ENTER ("DoLifting");

    /*
     * build call of the new dummy function
     */
    funname = GetDummyFunName (prefix);
    modname = FUNDEF_MOD (INFO_L2F_FUNDEF (arg_info));
    DBUG_ASSERT ((modname != NULL), "modul name for LAC function is NULL!");
    let = MakeL2fFunLet (funname, modname, in, out);

    /*
     * build new dummy function
     */
    fundef = MakeL2fFundef (funname, modname, arg_node, let, in, out, local, arg_info);

    /*
     * set back-references let <-> fundef
     */
    AP_FUNDEF (LET_EXPR (let)) = fundef;

    /*
     * insert new dummy function into INFO_L2F_FUNS
     */
    FUNDEF_NEXT (fundef) = INFO_L2F_FUNS (arg_info);
    INFO_L2F_FUNS (arg_info) = fundef;

    /*
     * replace the instruction by a call of the new dummy function
     */
    in = DFMRemoveMask (in);
    out = DFMRemoveMask (out);
    local = DFMRemoveMask (local);
    arg_node = FreeTree (arg_node);
    arg_node = let;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Ffundef( node *arg_node, node *arg_info)
 *
 * description:
 *   All dummy fundefs created during traversal of the body are inserted
 *   into the AST.
 *
 ******************************************************************************/

node *
L2Ffundef (node *arg_node, node *arg_info)
{
    node *ret, *tmp;

    DBUG_ENTER ("L2Ffundef");

    INFO_L2F_FUNDEF (arg_info) = arg_node;

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_L2F_FUNS (arg_info) = NULL;

        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

        /*
         * insert dummy fundefs into the AST
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
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (ret);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Bottom-up-traversal of the assignments.
 *
 ******************************************************************************/

node *
L2Fassign (node *arg_node, node *arg_info)
{
    node *old_assign;

    DBUG_ENTER ("L2Fassign");

    old_assign = INFO_L2F_ASSIGN (arg_info);

    INFO_L2F_ASSIGN (arg_info) = arg_node;

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    INFO_L2F_ASSIGN (arg_info) = old_assign;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fcond( node *arg_node, node *arg_info)
 *
 * description:
 *   Lifts the conditional and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2Fcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2Fcond");

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);
    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    arg_node = DoLifting ("Cond", COND_IN_MASK (arg_node), COND_OUT_MASK (arg_node),
                          COND_LOCAL_MASK (arg_node), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fwhile( node *arg_node, node *arg_info)
 *
 * description:
 *   Lifts the while-loop and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2Fwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2Fwhile");

    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);

    arg_node = DoLifting ("While", WHILE_IN_MASK (arg_node), WHILE_OUT_MASK (arg_node),
                          WHILE_LOCAL_MASK (arg_node), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *L2Fdo( node *arg_node, node *arg_info)
 *
 * description:
 *   Lifts the do-loop and inserts an equivalent function call instead.
 *
 ******************************************************************************/

node *
L2Fdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("L2Fdo");

    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);

    arg_node = DoLifting ("Do", DO_IN_MASK (arg_node), DO_OUT_MASK (arg_node),
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
Lac2Fun (node *syntax_tree)
{
    node *info_node;
    funtab *old_funtab;

    DBUG_ENTER ("Lac2Fun");

    /*
     * infer the in-, out- and local-masks of each conditional or loop
     * (via fixpoint iteration)
     */
    syntax_tree = InferDFMs (syntax_tree, HIDE_LOCALS_LAC);

    info_node = MakeInfo ();

    old_funtab = act_tab;
    act_tab = l2f_tab;
    syntax_tree = Trav (syntax_tree, info_node);
    act_tab = old_funtab;

    info_node = FreeNode (info_node);

    /*
     * cleanup declarations (remove unused vardecs, ...)
     */
    syntax_tree = CleanupDecls (syntax_tree);

    /*
     * after lifting and clean-up the current DFMs are not valid anymore!!!
     */
    syntax_tree = InferDFMs (syntax_tree, HIDE_LOCALS_NEVER);

    DBUG_RETURN (syntax_tree);
}
