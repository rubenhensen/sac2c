/*
 *
 * $Log$
 * Revision 3.7  2004/09/28 13:22:48  ktr
 * Removed generatemasks.
 *
 * Revision 3.6  2004/01/16 12:55:02  skt
 * added some DBUG-Output
 *
 * Revision 3.5  2000/12/15 18:24:44  dkr
 * infer_dfms.h renamed into InferDFMs.h
 *
 * Revision 3.4  2000/12/15 10:42:46  dkr
 * signature of InferDFMs() modified
 *
 * Revision 3.3  2000/12/12 12:12:09  dkr
 * NWITH_INOUT removed
 * interpretation of NWITH_IN changed:
 * the LHS of a with-loop assignment is now longer included in
 * NWITH_IN!!!
 *
 * Revision 3.2  2000/12/07 12:56:42  dkr
 * nothings changed
 *
 * Revision 3.1  2000/11/20 18:03:08  sacbase
 * new release made
 *
 * Revision 1.10  2000/07/13 09:35:27  jhs
 * Added some comments.
 * Unexported functions are static now.
 *
 * Revision 1.9  2000/07/11 15:42:10  jhs
 * Added creation of ST_ALLOC and ST_SYNC.
 *
 * Revision 1.8  2000/07/04 14:39:57  jhs
 * Fixed bug, while generating MT_ALLOC.
 *
 * Revision 1.7  2000/06/21 13:32:23  jhs
 * Added creation of MT_ALLOC-mask.
 *
 * Revision 1.6  2000/06/08 12:16:03  jhs
 * Fixed some problems caused by non-flatend code of IVE.
 *
 * Revision 1.5  2000/03/31 12:28:11  jhs
 * fixed bug
 *
 * Revision 1.4  2000/03/29 16:09:16  jhs
 * DFMPrintMask's outcommented.
 *
 * Revision 1.3  2000/03/21 16:11:34  jhs
 * NEEDCHAIN and NEEDBLOCK are pushed and poped at N_mt and N_st now.
 *
 * Revision 1.2  2000/03/21 13:07:54  jhs
 * Implemented extended version.
 *
 * Revision 1.1  2000/03/09 19:49:54  jhs
 * Initial revision
 *
 */

/******************************************************************************
 *
 * file:   dataflow_analysis.c
 *
 * prefix: DFA
 *
 * description:
 *   Each N_let get attached 2 dataflow-masks:
 *   - the USEmask containing all variables that are used by the N_let,
 *     this corresponds to the left-hand-side.
 *   - the DEFmask containing  all variables that are defined (calculated) by
 *     the N_let, this corresponds to the right-hand-side.
 *
 *   Not traversed are:
 *   - functions f with no body (FUNDEF_BODY( f) == NULL)
 *   - functions f with FUNDEF_ATTRIB( f) = ST_call_rep
 *   - functions f with FUNDEF_STATUS( f) = ST_foldfun
 *   - with-loops with schedulings (but we annotate at the first level!!)
 *
 * attention:
 *   ASSIGN_INSTRs will be traversed twice first while traversing downwards
 *   second while traversing upwards. Further informations see DFAtrav.
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "globals.h"
#include "free.h"
#include "DataFlowMask.h"
#include "InferDFMs.h"

#include "internal_lib.h"
#include "multithread_lib.h"

#include "mtfuns_init.h"

/******************************************************************************
 *
 * function:
 *   node *DataflowAnalysis( node *arg_node, node *arg_info)
 *
 * description:
 *   Initiate the dataflow-analysis as described above.
 *
 *   Traverses *only* the function handed over via arg_node with dfa_tab,
 *   will not traverse FUNDEF_NEXT( arg_node).
 *
 *   This routine ignores (returns without changes):
 *   - functions f with no body (FUNDEF_BODY( f) == NULL)
 *   - functions f with FUNDEF_STATUS( f) = ST_foldfun
 *   - repfuns
 *     functions f with FUNDEF_ATTRIB( f) = ST_call_rep
 *
 ******************************************************************************/
node *
DataflowAnalysis (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("DataFlowAnalysis");
    DBUG_PRINT ("DFA", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "DataFlowAnalysis expects a N_fundef as arg_node");

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        arg_node = InferDFMs (arg_node, HIDE_LOCALS_NEVER);

        old_tab = act_tab;
        act_tab = dfa_tab;

        /* push info */

        INFO_DFA_CONT (arg_info) = NULL;

        arg_node = Trav (arg_node, arg_info);

        /* pop info */

        act_tab = old_tab;
    }

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAfundef( node *arg_node, node *arg_info)
 *
 * description:
 *   Traversal of N_fundef in DFA.
 *   Traverses the body of the function.
 *
 * attention:
 *   Do not traverse the FUNDEF_NEXT!!!
 *   This traversal shall traverse only this function!!!
 *
 ******************************************************************************/
node *
DFAfundef (node *arg_node, node *arg_info)
{
    DFMmask_t old_use;
    DFMmask_t old_def;
    DFMmask_t old_needchain;
    DFMmask_t old_needblock;

    DBUG_ENTER ("DFAfundef");
    DBUG_PRINT ("DFA", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "arg_node of wrong NODE_TYPE");

    DBUG_ASSERT ((INFO_MUTH_FUNDEF (arg_info) == arg_node),
                 "INFO_MUTH_FUNDEF does not point to this function");

    old_use = INFO_DFA_USEMASK (arg_info);
    old_def = INFO_DFA_DEFMASK (arg_info);
    old_needchain = INFO_DFA_NEEDCHAIN (arg_info);
    old_needblock = INFO_DFA_NEEDBLOCK (arg_info);

    if (FUNDEF_DFM_BASE (arg_node) == NULL) {
        FUNDEF_DFM_BASE (arg_node)
          = DFMGenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
    }

    INFO_DFA_USEMASK (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
    INFO_DFA_DEFMASK (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
    INFO_DFA_NEEDCHAIN (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));
    INFO_DFA_NEEDBLOCK (arg_info) = DFMGenMaskClear (FUNDEF_DFM_BASE (arg_node));

    DBUG_PRINT ("DFA", ("into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    /* DO NOT TRAVERSE FUNDEF_NEXT */
    DBUG_PRINT ("DFA", ("from body"));

    INFO_DFA_USEMASK (arg_info) = DFMRemoveMask (INFO_DFA_USEMASK (arg_info));
    INFO_DFA_DEFMASK (arg_info) = DFMRemoveMask (INFO_DFA_DEFMASK (arg_info));
    INFO_DFA_NEEDCHAIN (arg_info) = DFMRemoveMask (INFO_DFA_NEEDCHAIN (arg_info));
    INFO_DFA_NEEDBLOCK (arg_info) = DFMRemoveMask (INFO_DFA_NEEDBLOCK (arg_info));
    INFO_DFA_USEMASK (arg_info) = old_use;
    INFO_DFA_DEFMASK (arg_info) = old_def;
    INFO_DFA_NEEDCHAIN (arg_info) = old_needchain;
    INFO_DFA_NEEDBLOCK (arg_info) = old_needblock;

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

#define HD_DOWN 1
#define HD_UP 2

/******************************************************************************
 *
 * function:
 *   static
 *   node *DFAtrav(node *arg_node, node *arg_info, funptr down, funptr up)
 *
 * description:
 *   The traversal-technique used in this is tricky.
 *   We need to traverse the "N_instr", i.e. N_let, N_mt, N_st, N_cond and
 *   N_return during top-down *and* bottom-up with *differnt* actions.
 *   The chains of N_instr is builded by N_assigns, so we traverse
 *   ASSIGN_INSTR first, then ASSIGN_NEXT and afterwards ASSIGN_INSTR
 *   again.
 *   Because of the differnt actions there are two routines for each N_instr
 *   DFAinstr_dn and DFAinstr_up, to be used within the corresponding
 *   traversals.
 *   From the dfa_tab we call the ordinary DFAinstr, this calls DFAtrav
 *   with DFAinstr_dn and DFAinstr_up as arguments, DFAtrav then decides
 *   by checking INFO_DFA_HEADING which of the two to call.
 *   INFO_DFA_HEADING is set in DFAassign.
 *
 ******************************************************************************/
static node *
DFAtrav (node *arg_node, node *arg_info, funptr down, funptr up)
{
    DBUG_ENTER ("DFAtrav");
    DBUG_PRINT ("DFA",
                ("begin %i %s", INFO_DFA_HEADING (arg_info), NODE_TEXT (arg_node)));

    if (INFO_DFA_HEADING (arg_info) == HD_DOWN) {
        if (down != NULL) {
            arg_node = down (arg_node, arg_info);
        }
    } else if (INFO_DFA_HEADING (arg_info) == HD_UP) {
        if (up != NULL) {
            arg_node = up (arg_node, arg_info);
        }
    } else {
        DBUG_ASSERT (0, ("Unknown Heading!"));
    }

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Compare description of DFAtrav for the double traversal of ASSIGN_INSTR!
 *
 ******************************************************************************/
node *
DFAassign (node *arg_node, node *arg_info)
{
    int old_heading;
    node *old_thisassign;
    node *old_cont;

    DBUG_ENTER ("DFAassign");
    DBUG_PRINT ("DFA", ("begin"));

    old_thisassign = INFO_DFA_THISASSIGN (arg_info);
    old_heading = INFO_DFA_HEADING (arg_info);
    INFO_DFA_THISASSIGN (arg_info) = arg_node;
    INFO_DFA_HEADING (arg_info) = HD_DOWN;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        /*
         *  If we reach the end of a chain, we check if we have to continue
         *  with INFO_DFA_CONT or not.
         */
        if (INFO_DFA_CONT (arg_info) != NULL) {
            old_cont = INFO_DFA_CONT (arg_info);
            INFO_DFA_CONT (arg_info) = NULL;
            /* ... = */ Trav (old_cont, arg_info);
            INFO_DFA_CONT (arg_info) = old_cont;
        }
    }

    INFO_DFA_HEADING (arg_info) = HD_UP;
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    INFO_DFA_THISASSIGN (arg_info) = old_thisassign;
    INFO_DFA_HEADING (arg_info) = old_heading;

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static
 *   node *DFAreturn_dn( node *arg_node, node *arg_info)
 *
 * description:
 *   Annotates RETURN_USEMAKS and RETURN_DEFMAKS.
 *   - for return (r1, ..., rN) where N > 0: RETURN_USEMAKS = {r1, ..., rN}.
 *   - RETURN_DEFMAKS = {}
 *
 ******************************************************************************/
static node *
DFAreturn_dn (node *arg_node, node *arg_info)
{
    node *exprs;
    node *arg;

    DBUG_ENTER ("DFAreturn_dn");
    DBUG_PRINT ("DFA", ("begin %i", INFO_DFA_HEADING (arg_info)));
    /*
      DFMPrintMask( stderr, "chain: %s\n", INFO_DFA_NEEDCHAIN( arg_info));
      DFMPrintMask( stderr, "block: %s\n", INFO_DFA_NEEDBLOCK( arg_info));
    */
    RETURN_USEMASK (arg_node)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    RETURN_DEFMASK (arg_node)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

    exprs = RETURN_EXPRS (arg_node);
    while (exprs != NULL) {
        arg = EXPRS_EXPR (exprs);
        DFMSetMaskEntrySet (LET_USEMASK (arg_node), ID_NAME (arg), NULL);
        exprs = EXPRS_NEXT (exprs);
    }

    /* RETURN_DEFMAKS is by definition empty */

    DBUG_PRINT ("DFA", ("end %i", INFO_DFA_HEADING (arg_info)));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static
 *   node *DFAreturn_up( node *arg_node, node *arg_info)
 *
 * description:
 *   updates needchain by return variables and prepares INFO_DFA_NEEDBLOCK
 *   for upwards traversal by setting it empty
 *
 ******************************************************************************/
static node *
DFAreturn_up (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAreturn_up");
    DBUG_PRINT ("DFA", ("begin"));

    DFMSetMaskOr (INFO_DFA_NEEDCHAIN (arg_info), RETURN_USEMASK (arg_node));
    DFMSetMaskClear (INFO_DFA_NEEDBLOCK (arg_info));
    /*
      DFMPrintMask( stderr, "chain: %s\n", INFO_DFA_NEEDCHAIN( arg_info));
      DFMPrintMask( stderr, "block: %s\n", INFO_DFA_NEEDBLOCK( arg_info));
    */
    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAreturn( node *arg_node, node *arg_info)
 *
 * description:
 *   the real traversal function for N_return, steering via DFAtrav
 *   the upward and downward actions.
 *
 ******************************************************************************/
node *
DFAreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAreturn");
    DBUG_PRINT ("DFA", ("begin"));

    arg_node = DFAtrav (arg_node, arg_info, DFAreturn_dn, DFAreturn_up);

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *DFAlet_dn( node *arg_node, node *arg_info)
 *
 * description:
 *   Annotates LET_USEMASK and LET_DEFMASK. How? See below ...
 *
 * the following types of lhs can be handled:
 * - a1, ..., aN = ..., where N > 0
 *   => LET_DEFMASK = { a1, ..., aN }
 *
 * the following types of rhs can be handled:
 * - ... = c
 *   where c is a constant
 *   LET_EXPR( arg_node) == (N_num || N_array)
 *   => LET_USEMASK = {}
 * - ... = v
 *   where v is a variable
 *   LET_EXPR( arg_node) == N_id
 *   => LET_USEMASK = { v }
 * - ... = f(p1, ..., pM)
 *   where f is a function and M>=0 and pi is (N_id || N_Num || N_array)
 *   LET_EXPR( arg_node) == (N_ap || N_prf)
 *   => LET_USEMASK = { pi | if pi is N_id }
 * - ... = with(...)
 *   LET_EXPR( arg_node) == (N_with2)
 *   => LET_USEMASK = { ... }, fetched from with-loop
 * - anything else should have be deleted
 *   (eg. by FLATTEN, WLT and LAC2FUN)
 *   => fail
 *
 ******************************************************************************/
static node *
DFAlet_dn (node *arg_node, node *arg_info)
{
#if 0
  node *expr;
  node *args;
  node *arg;
#endif
    node *vardec;
    ids *let_ids;
    DFMmask_t helper;
    DFMmask_t old_infer_let_usemask;
    DFMmask_t old_infer_let_defmask;

    DBUG_ENTER ("DFAlet_dn");
    DBUG_PRINT ("DFA", ("begin %i", INFO_DFA_HEADING (arg_info)));
    /*
      DFMPrintMask( stderr, "chain: %s\n", INFO_DFA_NEEDCHAIN( arg_info));
      DFMPrintMask( stderr, "block: %s\n", INFO_DFA_NEEDBLOCK( arg_info));
    */
    LET_USEMASK (arg_node)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    LET_DEFMASK (arg_node)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

    /* push info */
    old_infer_let_usemask = INFO_DFA_INFER_LET_USEMASK (arg_info);
    old_infer_let_defmask = INFO_DFA_INFER_LET_DEFMASK (arg_info);
    INFO_DFA_INFER_LET_USEMASK (arg_info) = LET_USEMASK (arg_node);
    INFO_DFA_INFER_LET_DEFMASK (arg_info) = LET_DEFMASK (arg_node);

    LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);

    /* pop info */
    LET_USEMASK (arg_node) = INFO_DFA_INFER_LET_USEMASK (arg_info);
    LET_DEFMASK (arg_node) = INFO_DFA_INFER_LET_DEFMASK (arg_info);
    INFO_DFA_INFER_LET_USEMASK (arg_info) = old_infer_let_usemask;
    INFO_DFA_INFER_LET_DEFMASK (arg_info) = old_infer_let_defmask;

#if 0
  expr = LET_EXPR( arg_node);
  if ((NODE_TYPE( expr) == N_num) || (NODE_TYPE( expr) == N_array)) {
    DBUG_PRINT( "DFA", ("reached N_num or N_array"));
    /* see comment above */
  } else if ((NODE_TYPE( expr) == N_id)) {
    DBUG_PRINT( "DFA", ("reached N_id"));
    /* see comment above */
    DFMSetMaskEntrySet( LET_USEMASK( arg_node), ID_NAME( expr), NULL);
  } else if ((NODE_TYPE( expr) == N_ap) || (NODE_TYPE( expr) == N_prf)) {
    DBUG_PRINT( "DFA", ("reached N_ap or N_prf"));
    DBUG_PRINT( "DFA", ("it is:  %s", NODE_TEXT( expr)));
    /* see comment above */
    args = AP_OR_PRF_ARGS( expr);
    if (args != NULL) {
      DBUG_PRINT( "DFA", ("found %s", NODE_TEXT( args)));
      DBUG_ASSERT( (NODE_TYPE( args) == N_exprs), ("args not N_exprs"));
      while (args != NULL) {
        arg = EXPRS_EXPR( args);
        DBUG_PRINT( "DFA", ("type of arg is %s", NODE_TEXT( arg)));
        if (NODE_TYPE( arg) == N_id) {
          DBUG_PRINT( "DFA", ("add %s", ID_NAME( arg)));
          DFMSetMaskEntrySet( LET_USEMASK( arg_node), ID_NAME( arg), NULL); 
        } else if ((NODE_TYPE( arg) == N_num) ||
                   (NODE_TYPE( arg) == N_array)) {
          /* ignore */
        } else {
          DBUG_ASSERT(0, "unknown kind of arg found (watch DFA)");
        }
        args = EXPRS_NEXT( args);
      } /* while (args != NULL) */
    }
  } else if ((NODE_TYPE( expr) == N_Nwith2)) {
    /*
     *  We don not traverse the with-loop, because all information
     *  is annotated at the with-loop (see below)!!!
     *  If you want to traverse the with-loop you have to push and pop
     *  INFO_DFA_NEEDCHAIN and INFO_DFA_NEEDBLOCK!!!
     *
     *  the with-loop (N_Nwith2) already has DFMs attached:
     *  NWITH2_IN_MASK, NWITH2_OUT_MASK and NWITH2_LOCAL_MASK.
     *  Here we use the information to build the LET_USEMASK and LET_DEFMASK.
     *  LET_USEMASK = NWITH2_IN_MASK
     */

    /* LET_USEMASK = LET_USEMASK + NWITH2_IN_MASK */
    vardec = DFMGetMaskEntryDeclSet( NWITH2_IN_MASK( expr));
    while (vardec != NULL) {
      DBUG_PRINT( "DFA", ("from in add use %s", VARDEC_NAME( vardec)));
      DFMSetMaskEntrySet( LET_USEMASK( arg_node), NULL, vardec);
      vardec = DFMGetMaskEntryDeclSet( NULL);
    }
  }
  else {
    DBUG_ASSERT(0, "unhandled kind of let"); 
  }
#endif

    /*
     *  we simply traverse the ids of lhs.
     */
    let_ids = LET_IDS (arg_node);
    while (let_ids != NULL) {
        vardec = IDS_VARDEC (let_ids);
        DFMSetMaskEntrySet (LET_DEFMASK (arg_node), NULL, vardec);
        let_ids = IDS_NEXT (let_ids);
    }

    /* INFO_DFA_USEMASK += LET_USEMASK - INFO_DFA_DEFMASK */
    helper = DFMGenMaskMinus (LET_USEMASK (arg_node), INFO_DFA_DEFMASK (arg_info));
    DFMSetMaskOr (INFO_DFA_USEMASK (arg_info), helper);
    helper = DFMRemoveMask (helper);

    /* INFO_DFA_DEFMASK += LET_DEFMASK */
    DFMSetMaskOr (INFO_DFA_DEFMASK (arg_info), LET_DEFMASK (arg_node));
    /*
      DFMPrintMask( stderr, "chain: %s\n", INFO_DFA_NEEDCHAIN( arg_info));
      DFMPrintMask( stderr, "block: %s\n", INFO_DFA_NEEDBLOCK( arg_info));
    */
    DBUG_PRINT ("DFA", ("end %i", INFO_DFA_HEADING (arg_info)));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *DFAlet_up( node *arg_node, node *arg_info)
 *
 * description:
 *   upward part of let traversal
 *
 ******************************************************************************/
static node *
DFAlet_up (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAlet_up");
    DBUG_PRINT ("DFA", ("begin %i", INFO_DFA_HEADING (arg_info)));
    /*
      DFMPrintMask( stderr, "chain: %s\n", INFO_DFA_NEEDCHAIN( arg_info));
      DFMPrintMask( stderr, "block: %s\n", INFO_DFA_NEEDBLOCK( arg_info));
    */
    /* tag if ... #### */

    DFMSetMaskMinus (INFO_DFA_NEEDCHAIN (arg_info), LET_DEFMASK (arg_node));
    DFMSetMaskOr (INFO_DFA_NEEDCHAIN (arg_info), LET_USEMASK (arg_node));

    DFMSetMaskMinus (INFO_DFA_NEEDBLOCK (arg_info), LET_DEFMASK (arg_node));
    /*
      DFMPrintMask( stderr, "chain: %s\n", INFO_DFA_NEEDCHAIN( arg_info));
      DFMPrintMask( stderr, "block: %s\n", INFO_DFA_NEEDBLOCK( arg_info));
    */
    DBUG_PRINT ("DFA", ("end %i", INFO_DFA_HEADING (arg_info)));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAlet( node *arg_node, node *arg_info)
 *
 * description:
 *   the real traversal function for N_let, steering via DFAtrav
 *   the upward and downward actions.
 *
 ******************************************************************************/
node *
DFAlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAlet");
    DBUG_PRINT ("DFA", ("begin"));

    arg_node = DFAtrav (arg_node, arg_info, DFAlet_dn, DFAlet_up);

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static
 *   node *DFAxt_dn( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
static node *
DFAxt_dn (node *arg_node, node *arg_info)
{
    node *old_cont;
    DFMmask_t old_usemask;
    DFMmask_t old_defmask;
    DFMmask_t old_needchain;
    DFMmask_t old_needblock;
    DFMmask_t tmp_needchain;
    DFMmask_t tmp_needblock;

    DBUG_ENTER ("DFAxt_dn");
    DBUG_PRINT ("DFA", ("begin"));

    /* USEMASK = {} and DEFMASK = {} */
    L_MT_OR_ST_USEMASK (arg_node,
                        DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info))));
    L_MT_OR_ST_DEFMASK (arg_node,
                        DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info))));

    tmp_needchain = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    tmp_needblock = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

    old_usemask = INFO_DFA_USEMASK (arg_info);
    old_defmask = INFO_DFA_DEFMASK (arg_info);
    old_cont = INFO_DFA_CONT (arg_info);
    old_needchain = INFO_DFA_NEEDCHAIN (arg_info);
    old_needblock = INFO_DFA_NEEDBLOCK (arg_info);
    INFO_DFA_USEMASK (arg_info) = MT_OR_ST_USEMASK (arg_node);
    INFO_DFA_DEFMASK (arg_info) = MT_OR_ST_DEFMASK (arg_node);
    INFO_DFA_CONT (arg_info) = NULL;
    INFO_DFA_NEEDCHAIN (arg_info) = tmp_needchain;
    INFO_DFA_NEEDBLOCK (arg_info) = tmp_needblock;

    L_MT_OR_ST_REGION (arg_node, Trav (MT_OR_ST_REGION (arg_node), arg_info));

    INFO_DFA_USEMASK (arg_info) = old_usemask;
    INFO_DFA_DEFMASK (arg_info) = old_defmask;
    INFO_DFA_CONT (arg_info) = old_cont;
    INFO_DFA_NEEDCHAIN (arg_info) = old_needchain;
    INFO_DFA_NEEDBLOCK (arg_info) = old_needblock;

    tmp_needchain = DFMRemoveMask (tmp_needchain);
    tmp_needblock = DFMRemoveMask (tmp_needblock);

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static
 *   node *DFAxt_up( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
static node *
DFAxt_up (node *arg_node, node *arg_info)
{
    DFMmask_t helper;

    DBUG_ENTER ("DFAxt_up");

    /* DEF_B = DEF_B * (NEEDCHAIN + NEEDBLOCK) */
    helper = DFMGenMaskOr (INFO_DFA_NEEDCHAIN (arg_info), INFO_DFA_NEEDBLOCK (arg_info));
    DFMSetMaskAnd (MT_OR_ST_DEFMASK (arg_node), helper);
    helper = DFMRemoveMask (helper);

    if (NODE_TYPE (arg_node) == N_mt) {
        MT_NEEDLATER (arg_node)
          = DFMGenMaskOr (INFO_DFA_NEEDCHAIN (arg_info), INFO_DFA_NEEDBLOCK (arg_info));
        MT_ALLOC (arg_node)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    } else if (NODE_TYPE (arg_node) == N_st) {
        ST_NEEDLATER_ST (arg_node) = DFMGenMaskMinus (INFO_DFA_NEEDCHAIN (arg_info),
                                                      INFO_DFA_NEEDBLOCK (arg_info));

        ST_NEEDLATER_MT (arg_node) = DFMGenMaskCopy (INFO_DFA_NEEDCHAIN (arg_info));
        ST_ALLOC (arg_node)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
        ST_SYNC (arg_node)
          = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    } else {
        DBUG_ASSERT (0, ("wrong node type"));
    }

    DFMSetMaskMinus (INFO_DFA_NEEDCHAIN (arg_info), MT_OR_ST_DEFMASK (arg_node));

    DFMSetMaskMinus (INFO_DFA_NEEDBLOCK (arg_info), MT_OR_ST_DEFMASK (arg_node));
    DFMSetMaskOr (INFO_DFA_NEEDBLOCK (arg_info), MT_OR_ST_USEMASK (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAlet( node *arg_node, node *arg_info)
 *
 * description:
 *   the real traversal function for N_xt, steering via DFAtrav
 *   the upward and downward actions.
 *
 ******************************************************************************/
node *
DFAxt (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAxt");

    arg_node = DFAtrav (arg_node, arg_info, DFAxt_dn, DFAxt_up);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *DFAcond_dn( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
static node *
DFAcond_dn (node *arg_node, node *arg_info)
{
    node *old_cont;
    DFMmask_t tmp_needchain;
    DFMmask_t tmp_needblock;

    DBUG_ENTER ("DFAcond_dn");

    old_cont = INFO_DFA_CONT (arg_info);
    INFO_DFA_CONT (arg_info) = ASSIGN_NEXT (INFO_DFA_THISASSIGN (arg_info));

    COND_THEN (arg_node) = Trav (COND_THEN (arg_node), arg_info);

    tmp_needchain = INFO_DFA_NEEDCHAIN (arg_info);
    tmp_needblock = INFO_DFA_NEEDBLOCK (arg_info);
    INFO_DFA_NEEDCHAIN (arg_info)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    INFO_DFA_NEEDBLOCK (arg_info)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

    COND_ELSE (arg_node) = Trav (COND_ELSE (arg_node), arg_info);

    DFMSetMaskOr (INFO_DFA_NEEDCHAIN (arg_info), tmp_needchain);
    DFMSetMaskOr (INFO_DFA_NEEDBLOCK (arg_info), tmp_needblock);
    tmp_needchain = DFMRemoveMask (tmp_needchain);
    tmp_needblock = DFMRemoveMask (tmp_needblock);

    INFO_DFA_CONT (arg_info) = old_cont;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static node *DFAcond( node *arg_node, node *arg_info)
 *
 * description:
 *   the real traversal function for N_xt, steering via DFAtrav
 *   the upward and downward actions.
 *
 * attention:
 *   there is no function for the upward-part, but DFAtrav can handle
 *   NULL-functions.
 *
 ******************************************************************************/
node *
DFAcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAcond");

    arg_node = DFAtrav (arg_node, arg_info, DFAcond_dn, NULL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   DFAnum( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
DFAnum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAnum");

    DBUG_PRINT ("DFA", ("reached N_num"));
    /* nothing to be done */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   DFAarray( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
DFAarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAarray");

    DBUG_PRINT ("DFA", ("reached N_array"));
    /* nothing to be done */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   DFAid( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
DFAid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAid");

    DBUG_PRINT ("DFA", ("reached N_id"));
    DFMSetMaskEntrySet (INFO_DFA_INFER_LET_USEMASK (arg_info), ID_NAME (arg_node), NULL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   DFAap( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
DFAap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAap");

    DBUG_PRINT ("DFA", ("reached N_ap"));

    AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   DFAprf( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
DFAprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAap");

    DBUG_PRINT ("DFA", ("reached N_prf"));

    PRF_ARGS (arg_node) = Trav (PRF_ARGS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAexprs( node *arg_node, node *arg_info)
 *
 * desription:
 *   ####
 *
 ******************************************************************************/
node *
DFAexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("DFAexprs");

    DBUG_PRINT ("DFA", ("reached N_exprs"));

    EXPRS_EXPR (arg_node) = Trav (EXPRS_EXPR (arg_node), arg_info);

    if (EXPRS_NEXT (arg_node) != NULL) {
        EXPRS_NEXT (arg_node) = Trav (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAnwith2( node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
DFAnwith2 (node *arg_node, node *arg_info)
{
    node *vardec;

    DBUG_ENTER ("DFAnwith2");

    /* LET_USEMASK = LET_USEMASK + NWITH2_IN_MASK */
    vardec = DFMGetMaskEntryDeclSet (NWITH2_IN_MASK (arg_node));
    while (vardec != NULL) {
        DBUG_PRINT ("DFA", ("from in add use %s", VARDEC_NAME (vardec)));
        DFMSetMaskEntrySet (INFO_DFA_INFER_LET_USEMASK (arg_info), NULL, vardec);
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (arg_node);
}
