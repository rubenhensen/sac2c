/*
 *
 * $Log$
 * Revision 3.5  2001/03/21 18:09:33  dkr
 * bug fixed
 *
 * Revision 3.4  2001/03/21 17:50:17  dkr
 * Inlining recoded
 *
 * Revision 3.3  2001/02/13 17:16:19  dkr
 * MakeNode() eliminated
 *
 * Revision 3.2  2000/11/23 16:23:39  sbs
 * mem_inl_fun in Inline enclosed by ifndef DBUG_OFF to avoid compiler warning
 * in product version & node_behind initialized by NULL in INLassign to avoid
 * superfluous warning "might be used uninitialized in this function".
 *
 * Revision 3.1  2000/11/20 18:00:31  sacbase
 * new release made
 *
 * Revision 2.6  2000/07/14 11:34:47  dkr
 * FUNDEF_INLINE==0 replaced by !FUNDEF_INLINE
 *
 * Revision 2.5  2000/07/12 15:14:31  dkr
 * function DuplicateTypes renamed into DupTypes
 * function SearchDecl moved to tree_compound.c
 *
 * Revision 2.4  2000/06/23 15:19:11  dkr
 * function DupTree() with argument (arg_info != NULL) is replaced by
 * function DupTreeInfo()
 *
 * Revision 2.3  2000/01/26 17:29:49  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.2  1999/09/01 17:11:01  jhs
 * Fixed Duplicating of masks in DupAssign.
 *
 * Revision 2.1  1999/02/23 12:41:19  sacbase
 * new release made
 *
 * Revision 1.24  1999/01/18 15:46:02  sbs
 * DBUG_PRINT( "OPTMEM",...) inserted for mem-info during optimization
 *
 * Revision 1.23  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.22  1998/08/07 21:46:22  dkr
 * comment added
 *
 * Revision 1.20  1998/07/16 17:20:58  sbs
 * InlineSingleApplication generated
 *
 * Revision 1.19  1998/05/08 15:46:03  srs
 * no semantic changes
 *
 * Revision 1.18  1998/04/16 16:07:34  srs
 * renamed macros which access arg_info
 *
 * Revision 1.17  1998/04/16 14:26:50  srs
 * removed NEWTREE
 *
 * Revision 1.16  1998/04/09 21:24:31  dkr
 * renamed macros:
 *   INLINE -> DUP_INLINE
 *   NORMAL -> DUP_NORMAL
 *   INVARIANT -> DUP_INVARIANT
 *
 * [...]
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "free.h"
#include "string.h"
#include "typecheck.h"
#include "internal_lib.h"

#include "optimize.h"
#include "LoopInvariantRemoval.h"
#include "DupTree.h"
#include "Inline.h"

#define INLINE_PREFIX "__inl"
#define INLINE_PREFIX_LENGTH 5

static int inline_nr = 0;

/******************************************************************************
 *
 * Function:
 *   void ResetInlineNo( node *module)
 *
 * Description:
 *
 *
 ******************************************************************************/

static void
ResetInlineNo (node *module)
{
    node *fun_node;

    DBUG_ENTER ("ResetInlineNo");

    fun_node = MODUL_FUNS (module);
    while (fun_node != NULL) {
        if (FUNDEF_INLINE (fun_node)) {
            FUNDEF_INLREC (fun_node) = inlnum;
        }
        fun_node = FUNDEF_NEXT (fun_node);
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * Function:
 *   node *InlineArg( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InlineArg (node *arg_node, node *arg_info)
{
    node *new_vardec;
    node *new_ass;
    char *new_name;

    DBUG_ENTER ("InlineArg");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_arg), "no N_arg node found!");

    DBUG_ASSERT ((INFO_INL_ARG (arg_info) != NULL), "INFO_INL_ARG not found!");
    DBUG_ASSERT ((NODE_TYPE (INFO_INL_ARG (arg_info)) == N_exprs),
                 "illegal INFO_INL_ARG found!");

    /*
     * build a new vardec based on 'arg_node' and rename it
     */
    new_vardec = MakeVardecFromArg (arg_node);
    new_name = CreateInlineName (ARG_NAME (arg_node));
    FREE (VARDEC_NAME (new_vardec));
    VARDEC_NAME (new_vardec) = new_name;

    /*
     * insert new vardec into INFO_INL_VARDECS chain
     */
    VARDEC_NEXT (new_vardec) = INFO_INL_VARDECS (arg_info);
    INFO_INL_VARDECS (arg_info) = new_vardec;

    /*
     * insert ['arg_node', 'new_vardec'] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT (INFO_INL_LUT (arg_info), arg_node, new_vardec);

    /*
     * insert assignment
     *   VARDEC_NAME( new_vardec) = EXPRS_EXPR( INFO_INL_ARG( arg_info));
     * into INFO_INL_PROLOG
     */
    new_ass = MakeAssignLet (StringCopy (new_name), new_vardec,
                             DupTree (EXPRS_EXPR (INFO_INL_ARG (arg_info))));
    ASSIGN_NEXT (new_ass) = INFO_INL_PROLOG (arg_info);
    INFO_INL_PROLOG (arg_info) = new_ass;

    if (ARG_NEXT (arg_node)) {
        INFO_INL_ARG (arg_info) = EXPRS_NEXT (INFO_INL_ARG (arg_info));
        ARG_NEXT (arg_node) = InlineArg (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InlineVardec( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InlineVardec (node *arg_node, node *arg_info)
{
    node *new_vardec;
    char *new_name;

    DBUG_ENTER ("InlineVardec");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_vardec), "no N_vardec node found!");

    /*
     * build a new vardec based on 'arg_node' and rename it
     */
    new_vardec = DupTree (arg_node);
    new_name = CreateInlineName (VARDEC_NAME (arg_node));
    FREE (VARDEC_NAME (new_vardec));
    VARDEC_NAME (new_vardec) = new_name;

    /*
     * insert new vardec into INFO_INL_VARDECS chain
     */
    VARDEC_NEXT (new_vardec) = INFO_INL_VARDECS (arg_info);
    INFO_INL_VARDECS (arg_info) = new_vardec;

    /*
     * insert ['arg_node', 'new_vardec'] into INFO_INL_LUT
     */
    INFO_INL_LUT (arg_info)
      = InsertIntoLUT (INFO_INL_LUT (arg_info), arg_node, new_vardec);

    if (VARDEC_NEXT (arg_node)) {
        VARDEC_NEXT (arg_node) = InlineVardec (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *InlineReturn( node *arg_node, node *arg_info)
 *
 * Description:
 *
 *
 ******************************************************************************/

static node *
InlineReturn (node *arg_node, node *arg_info)
{
    node *exprs;

    DBUG_ENTER ("InlineReturn");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_return), "no N_return node found!");

    exprs = RETURN_EXPRS (arg_node);
    while (exprs != NULL) {
        DBUG_ASSERT ((INFO_INL_IDS (arg_info) != NULL), "INFO_INL_IDS not found!");

        /*
         * insert assignment
         *   INFO_INL_IDS( arg_info) = DupTree( EXPRS_EXPR( exprs), INFO_INL_LUT);
         * into INFO_INL_EPILOG
         */
        INFO_INL_EPILOG (arg_info)
          = MakeAssign (MakeLet (DupTreeLUT_Type (EXPRS_EXPR (exprs),
                                                  INFO_INL_LUT (arg_info), DUP_INLINE),
                                 DupOneIds (INFO_INL_IDS (arg_info))),
                        INFO_INL_EPILOG (arg_info));

        INFO_INL_IDS (arg_info) = IDS_NEXT (INFO_INL_IDS (arg_info));
        exprs = EXPRS_NEXT (exprs);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *DoInline( node *let_node, node *arg_info)
 *
 * Description:
 *   Do function inlining. Returns a N_assign chain.
 *
 ******************************************************************************/

static node *
DoInline (node *let_node, node *arg_info)
{
    node *ap_node;
    node *inl_fundef;
    node *inl_nodes;

    DBUG_ENTER ("DoInline");

    DBUG_ASSERT ((arg_info != NULL), "DoInline called with (arg_info == NULL)!");
    DBUG_ASSERT ((NODE_TYPE (let_node) == N_let), "DoInline called with no N_let node!");

    ap_node = LET_EXPR (let_node);
    inl_fundef = AP_FUNDEF (ap_node);

    DBUG_PRINT ("INL", ("Inlining function %s", AP_NAME (ap_node)));

    INFO_INL_LUT (arg_info) = GenerateLUT ();
    INFO_INL_PROLOG (arg_info) = NULL;
    INFO_INL_EPILOG (arg_info) = NULL;
    inl_fun++;

    /*
     * generate new vardecs and fill INFO_INL_LUT
     */
    if (FUNDEF_VARDEC (inl_fundef) != NULL) {
        FUNDEF_VARDEC (inl_fundef) = InlineVardec (FUNDEF_VARDEC (inl_fundef), arg_info);
    }

    /*
     * generate new vardecs, fill INFO_INL_LUT and generate prolog assignments
     */
    if (FUNDEF_ARGS (inl_fundef) != NULL) {
        INFO_INL_ARG (arg_info) = AP_ARGS (ap_node);
        FUNDEF_ARGS (inl_fundef) = InlineArg (FUNDEF_ARGS (inl_fundef), arg_info);
    }

    /*
     * generate epilog assignments
     *
     * *** CAUTION ***
     * FUNDEF_RETURN must be traversed after FUNDEF_VARDEC and FUNDEF_ARGS !!
     */
    if (FUNDEF_RETURN (inl_fundef) != NULL) {
        INFO_INL_IDS (arg_info) = LET_IDS (let_node);
        FUNDEF_RETURN (inl_fundef) = InlineReturn (FUNDEF_RETURN (inl_fundef), arg_info);
    }

    /*
     * duplicate function body (with LUT to get the right back-references!)
     */
    inl_nodes = DupTreeLUT_Type (BLOCK_INSTR (FUNDEF_BODY (inl_fundef)),
                                 INFO_INL_LUT (arg_info), DUP_INLINE);

    /*
     * insert INFO_INL_PROLOG, INFO_INL_EPILOG
     */
    inl_nodes = AppendAssign (INFO_INL_PROLOG (arg_info), inl_nodes);
    inl_nodes = AppendAssign (inl_nodes, INFO_INL_EPILOG (arg_info));

    inline_nr++;
    INFO_INL_LUT (arg_info) = RemoveLUT (INFO_INL_LUT (arg_info));

    DBUG_RETURN (inl_nodes);
}

/******************************************************************************
 *
 * Function:
 *   node *INLmodul( node *arg_node, node *arg_info)
 *
 * Description:
 *   Stores pointer to module in info-node and traverses function-chain.
 *
 ******************************************************************************/

node *
INLmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLmodul");

    if (MODUL_FUNS (arg_node)) {
        INFO_INL_MODUL (arg_info) = arg_node;
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLfundef( node *arg_node, node *arg_info)
 *
 * Description:
 *   Traverses instructons if function not inlined marked
 *
 ******************************************************************************/

node *
INLfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLfundef");

    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_INLINE (arg_node))) {
        DBUG_PRINT ("INL", ("*** Trav function %s", FUNDEF_NAME (arg_node)));

        ResetInlineNo (INFO_INL_MODUL (arg_info));
        INFO_INL_VARDECS (arg_info) = NULL;

        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);

        FUNDEF_VARDEC (arg_node)
          = AppendVardec (FUNDEF_VARDEC (arg_node), INFO_INL_VARDECS (arg_info));
    }

    if (FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *INLassign( node *arg_node, node *arg_info)
 *
 * Description:
 *   Initiates function inlining if substitution-counter not 0
 *
 ******************************************************************************/

node *
INLassign (node *arg_node, node *arg_info)
{
    node *instr;
    node *inl_fundef = NULL;
    node *inlined_nodes = NULL;

    DBUG_ENTER ("INLassign");

    instr = ASSIGN_INSTR (arg_node);
    if ((NODE_TYPE (instr) == N_let) && (NODE_TYPE (LET_EXPR (instr)) == N_ap)) {
        /*
         * application -> try to inline
         */
        inl_fundef = AP_FUNDEF (LET_EXPR (instr));

        DBUG_PRINT ("INL", ("Function call %s found in line %d with"
                            " inline %d and to do %d",
                            AP_NAME (LET_EXPR (instr)), NODE_LINE (arg_node),
                            FUNDEF_INLINE (inl_fundef), FUNDEF_INLREC (inl_fundef)));

        if (FUNDEF_INLREC (inl_fundef) > 0) {
            inlined_nodes = DoInline (instr, arg_info);
        }
    }

    if (inlined_nodes != NULL) {
        /*
         * inlining done
         *  -> traverse inline code
         *  -> insert inline code into assignment chain
         */

        /* 'inl_fundef' definitly is set, since (inlined_nodes != NULL) */
        FUNDEF_INLREC (inl_fundef)--;
        inlined_nodes = Trav (inlined_nodes, arg_info);
        FUNDEF_INLREC (inl_fundef)++;

        if (ASSIGN_NEXT (arg_node)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }

        arg_node = AppendAssign (inlined_nodes, FreeNode (arg_node));
    } else {
        /*
         * no inlining done -> traverse sons
         */

        if (ASSIGN_INSTR (arg_node)) {
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        }

        if (ASSIGN_NEXT (arg_node)) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   char *CreateInlineName( char *old_name)
 *
 * Description:
 *   Renames the given variable.
 *   Example:
 *     a  ->  _inl100_a     if (inline_nr == 100) and (INLINE_PREFIX == "_inl")
 *
 ******************************************************************************/

char *
CreateInlineName (char *old_name)
{
    char *new_name;

    DBUG_ENTER ("CreateInlineName");

    new_name = (char *)MALLOC (sizeof (char)
                               * (strlen (old_name) + INLINE_PREFIX_LENGTH + 1 + /* _ */
                                  NumberOfDigits (inline_nr) + 1)); /* '\0' */

    sprintf (new_name, INLINE_PREFIX "%d_%s", inline_nr, old_name);

    DBUG_RETURN (new_name);
}

/******************************************************************************
 *
 * function:
 *  node *InlineSingleApplication( node *let_node, node *fundef_node)
 *
 * description:
 *   this function allows a single function application to be inlined.
 *   It is ment for external calls only and preserves the actual fun_tab!
 *   <let_node> should point to the N_let node which RHS is to be unrolled,
 *   <fundef_node> should point to the N_fundef node of the function in which
 *      body the <let_node> is situated!! (needed for new vardecs only!)
 *   It returns an assignment-chain to be inserted for the N_assign node
 *   which has <let_node> as its body!
 *
 ******************************************************************************/

node *
InlineSingleApplication (node *let_node, node *fundef_node)
{
    node *arg_info;
    node *assigns;
    funtab *mem_tab;

    DBUG_ENTER ("InlineSingleApplication");

    mem_tab = act_tab;
    act_tab = inline_tab;
    arg_info = MakeInfo ();
    INFO_INL_VARDECS (arg_info) = FUNDEF_VARDEC (fundef_node);

    assigns = DoInline (let_node, arg_info);

    FUNDEF_VARDEC (fundef_node) = INFO_INL_VARDECS (arg_info);
    FreeTree (arg_info);
    act_tab = mem_tab;

    DBUG_RETURN (assigns);
}

/******************************************************************************
 *
 * Function:
 *   node *Inline( node *arg_node, node *arg_info)
 *
 * Description:
 *   Starts function inlining.
 *
 ******************************************************************************/

node *
Inline (node *arg_node, node *arg_info)
{
    funtab *mem_tab;
#ifndef DBUG_OFF
    int mem_inl_fun = inl_fun;
#endif

    DBUG_ENTER ("Inline");

    DBUG_PRINT ("OPT", ("FUNCTION INLINING"));
    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    mem_tab = act_tab;
    act_tab = inline_tab;
    arg_info = MakeInfo ();

    arg_node = Trav (arg_node, arg_info);

    DBUG_PRINT ("OPT", ("                        result: %d", inl_fun - mem_inl_fun));

    DBUG_PRINT ("OPTMEM", ("mem currently allocated: %d bytes", current_allocated_mem));

    FreeTree (arg_info);
    act_tab = mem_tab;

    DBUG_RETURN (arg_node);
}
