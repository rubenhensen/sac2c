/*
 *
 * $Log$
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
 *
 *   ...ISTRANS  flag: has the current assignment been modified?
 *   ...FUNS     chain of newly generated dummy functions
 *
 *****************************************************************************/

#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "dbug.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "DataFlowMaskUtils.h"
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
 *   node *MakeL2fFundef( char *funname, char *modname, status status,
 *                        node *instr, node *funcall_let,
 *                        DFMmask_t in, DFMmask_t out, DFMmask_t local)
 *
 * description:
 *   Creates the fundef-node of a dummy function.
 *
 ******************************************************************************/

static node *
MakeL2fFundef (char *funname, char *modname, statustype status, node *instr,
               node *funcall_let, DFMmask_t in, DFMmask_t out, DFMmask_t local)
{
    LUT_t lut;
    DFMmask_t tmp_mask;
    node *args, *vardecs, *ret, *fundef, *assigns, *new_body, *let, *tmp;

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

            DBUG_PRINT ("LAC2FUN", ("ATTRIB[ .. %s( .. %s .. ) { .. } ]: "
                                    " ST_..reference -> ST_unique",
                                    funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

    ret = MakeAssign (MakeReturn (DFM2Exprs (out, lut)), NULL);

    /*
     * All return ids with attrib 'ST_was_reference' must have the status
     *  'ST_artificial'
     */
    tmp = RETURN_EXPRS (ASSIGN_INSTR (ret));
    while (tmp != NULL) {
        if (ID_ATTRIB (EXPRS_EXPR (tmp)) == ST_was_reference) {
            ID_ATTRIB (EXPRS_EXPR (tmp)) = ST_unique;
            ID_STATUS (EXPRS_EXPR (tmp)) = ST_artificial;

            DBUG_PRINT ("LAC2FUN", ("%s():  ATTRIB/STATUS[ return( %s) ] "
                                    " .. -> ST_unique/ST_artificial",
                                    funname, ID_NAME (EXPRS_EXPR (tmp))));
        }
        tmp = EXPRS_NEXT (tmp);
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
     *    IntStack __Cond1( IntStack new_stack, bool _flat_3, IntStack:IntStack stack)
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
     * Although 'stack' was marked as 'ST_was_reference' in function 'fun' that is
     * no longer true in the context of the dummy function '__Cond1' because 'stack'
     * is not an out-var of this function!!
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

            DBUG_PRINT ("LAC2FUN", ("ATTRIB[ .. %s( .. %s ..) { .. } ]: "
                                    " ST_was_reference -> ST_unique",
                                    funname, ARG_NAME (tmp)));
        }
        tmp = ARG_NEXT (tmp);
    }

    fundef
      = MakeFundef (StringCopy (funname), StringCopy (modname), DFM2ReturnTypes (out),
                    args, NULL, /* the block is not complete yet */
                    NULL);
    FUNDEF_STATUS (fundef) = status;
    FUNDEF_RETURN (fundef) = ASSIGN_INSTR (ret);

    switch (status) {
    case ST_condfun:
        assigns = MakeAssign (DupTreeLUT (instr, lut), ret);
        break;

    case ST_whilefun:
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
            ASSIGN_NEXT (tmp) = MakeAssign (let, NULL);
        }

        assigns = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), lut), new_body,
                                        MakeBlock (MakeEmpty (), NULL)),
                              ret);
        break;

    case ST_dofun:
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
            ASSIGN_NEXT (tmp)
              = MakeAssign (MakeCond (DupTreeLUT (WHILE_COND (instr), lut),
                                      MakeBlock (MakeAssign (let, NULL), NULL),
                                      MakeBlock (MakeEmpty (), NULL)),
                            ret);
        }
        break;

    default:
        assigns = NULL;
        break;
    }
    DBUG_ASSERT ((assigns != NULL), "wrong status -> no assigns created");

    /*
     * now we can add the body to the fundef
     */
    FUNDEF_BODY (fundef) = MakeBlock (assigns, vardecs);

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
    ids *tmp;

    DBUG_ENTER ("MakeL2fFunLet");

    let = MakeLet (MakeAp (StringCopy (funname), StringCopy (modname),
                           DFM2Exprs (in, NULL)),
                   DFM2Ids (out, NULL));

    /*
     * All left hand side ids with attrib 'ST_was_reference' must have the status
     *  'ST_artificial'
     */
    tmp = LET_IDS (let);
    while (tmp != NULL) {
        if (IDS_ATTRIB (tmp) == ST_was_reference) {
            IDS_ATTRIB (tmp) = ST_unique;
            IDS_STATUS (tmp) = ST_artificial;

            DBUG_PRINT ("LAC2FUN", ("ATTRIB/STATUS[ %s = %s( .. ) ] "
                                    " .. -> ST_unique/ST_artificial",
                                    IDS_NAME (tmp), funname));
        }
        tmp = IDS_NEXT (tmp);
    }

    DBUG_RETURN (let);
}

/******************************************************************************
 *
 * function:
 *   node *DoLifting( char *prefix, statustype status,
 *                    DFMmask_t in, DFMmask_t out, DFMmask_t local,
 *                    node *arg_node, node *arg_info)
 *
 * description:
 *   This function carries out the lifting of a conditional or loop.
 *
 ******************************************************************************/

static node *
DoLifting (char *prefix, statustype status, DFMmask_t in, DFMmask_t out, DFMmask_t local,
           node *arg_node, node *arg_info)
{
    char *funname, *modname;
    node *fundef, *let;

    DBUG_ENTER ("DoLifting");

    /*
     * build call of the new dummy function
     */
    funname = GetDummyFunName (prefix);
    modname = FUNDEF_MOD (INFO_L2F_FUNDEF (arg_info));
#ifndef MAIN_HAS_MODNAME
    if (modname == NULL) {
        /* okay, we are in the main() function ... */
        modname = MAIN_MOD_NAME;
    }
#endif
    DBUG_ASSERT ((modname != NULL), "modul name for LAC function is NULL!");
    let = MakeL2fFunLet (funname, modname, in, out);

    /*
     * build new dummy function
     */
    fundef = MakeL2fFundef (funname, modname, status, arg_node, let, in, out, local);

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
    INFO_L2F_ISTRANS (arg_info) = 1;

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
        INFO_L2F_ISTRANS (arg_info) = 0;

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
 *   INFO_L2F_ISTRANS indicates whether an assignment has been transformed
 *   into a function call or not. If (INFO_L2F_ISTRANS > 0) is hold, the
 *   assign-node has been modificated and must be correctly inserted into the
 *   AST.
 *
 ******************************************************************************/

node *
L2Fassign (node *arg_node, node *arg_info)
{
    node *assign_next;

    DBUG_ENTER ("L2Fassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        assign_next = Trav (ASSIGN_NEXT (arg_node), arg_info);
        if (INFO_L2F_ISTRANS (arg_info)) {
            ASSIGN_NEXT (assign_next) = ASSIGN_NEXT (ASSIGN_NEXT (arg_node));
            ASSIGN_NEXT (arg_node) = assign_next;
            INFO_L2F_ISTRANS (arg_info) = 0;
        }
    }

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

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

    arg_node
      = DoLifting ("Cond", ST_condfun, COND_IN_MASK (arg_node), COND_OUT_MASK (arg_node),
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

    arg_node = DoLifting ("While", ST_whilefun, WHILE_IN_MASK (arg_node),
                          WHILE_OUT_MASK (arg_node), WHILE_LOCAL_MASK (arg_node),
                          arg_node, arg_info);

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

    arg_node = DoLifting ("Do", ST_dofun, DO_IN_MASK (arg_node), DO_OUT_MASK (arg_node),
                          DO_LOCAL_MASK (arg_node), arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *Lac2Fun( node *syntax_tree)
 *
 * description:
 *   Converts all loops and conditions into (annotated) functions.
 *
 ******************************************************************************/

node *
Lac2Fun (node *syntax_tree)
{
    node *info_node;

    DBUG_ENTER ("Lac2Fun");

    /*
     * infer the in-, out- and local-masks of each conditional or loop
     * (via fixpoint iteration)
     */
    syntax_tree = InferDFMs (syntax_tree);

    info_node = MakeInfo ();
    act_tab = l2f_tab;
    syntax_tree = Trav (syntax_tree, info_node);
    info_node = FreeNode (info_node);

    /*
     * cleanup declarations (remove unused vardecs, ...)
     */
    syntax_tree = CleanupDecls (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
