/*
 *
 * $Log$
 * Revision 1.1  2000/03/09 19:49:54  jhs
 * Initial revision
 *
 *
 *
 * constructed by jhs@dArtagnan at home
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
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"
#include "free.h"
#include "DataFlowMask.h"

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

    if ((FUNDEF_BODY (arg_node) != NULL) && (FUNDEF_STATUS (arg_node) != ST_foldfun)
        && (FUNDEF_ATTRIB (arg_node) != ST_call_rep)) {
        old_tab = act_tab;
        act_tab = dfa_tab;

        /* push info */

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
    DBUG_ENTER ("DFAfundef");
    DBUG_PRINT ("DFA", ("begin"));

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "arg_node of wrong NODE_TYPE");

    DBUG_ASSERT ((INFO_MUTH_FUNDEF (arg_info) == arg_node),
                 "INFO_MUTH_FUNDEF does not point to this function");

    /*
      FUNDEF_DFM_BASE( arg_node) = DFMGenMaskBase( FUNDEF_ARGS( arg_node),
                                                   FUNDEF_VARDEC( arg_node));
    */

    DBUG_PRINT ("DFA", ("into body"));
    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    /* DO NOT TRAVERSE FUNDEF_NEXT */
    DBUG_PRINT ("DFA", ("from body"));

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAreturn( node *arg_node, node *arg_info)
 *
 * description:
 *   Annotates RETURN_USEMAKS and RETURN_DEFMAKS.
 *   - for return (r1, ..., rN) where N > 0: RETURN_USEMAKS = {r1, ..., rN}.
 *   - RETURN_DEFMAKS = {}
 *
 ******************************************************************************/
node *
DFAreturn (node *arg_node, node *arg_info)
{
    node *exprs;
    node *arg;

    DBUG_ENTER ("DFAreturn");
    DBUG_PRINT ("DFA", ("begin"));

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

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *DFAlet( node *arg_node, node *arg_info)
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
node *
DFAlet (node *arg_node, node *arg_info)
{
    node *expr;
    node *args;
    node *arg;
    node *vardec;
    /*
     *  *done_lhs* annotates whether the lhs as done (e.g. if we passed a
     *  with-loop) or not and we have to check the rhs explicitly.
     *  the variable is used as boolean!!
     */
    int done_lhs;
    ids *let_ids;

    DBUG_ENTER ("DFAlet");
    DBUG_PRINT ("DFA", ("begin"));

    LET_USEMASK (arg_node)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));
    LET_DEFMASK (arg_node)
      = DFMGenMaskClear (FUNDEF_DFM_BASE (INFO_MUTH_FUNDEF (arg_info)));

    expr = LET_EXPR (arg_node);
    done_lhs = FALSE;
    if ((NODE_TYPE (expr) == N_num) || (NODE_TYPE (expr) == N_array)) {
        DBUG_PRINT ("DFA", ("reached N_num or N_array"));
        /* see comment above */
    } else if ((NODE_TYPE (expr) == N_id)) {
        DBUG_PRINT ("DFA", ("reached N_id"));
        /* see comment above */
        DFMSetMaskEntrySet (LET_USEMASK (arg_node), ID_NAME (expr), NULL);
    } else if ((NODE_TYPE (expr) == N_ap) || (NODE_TYPE (expr) == N_prf)) {
        DBUG_PRINT ("DFA", ("reached N_ap or N_prf"));
        /* see comment above */
        args = AP_OR_PRF_ARGS (expr);
        DBUG_PRINT ("DFA", ("hit %s", NODE_TEXT (args)));
        DBUG_ASSERT ((NODE_TYPE (args) == N_exprs), ("args not N_exprs"));
        while (args != NULL) {
            arg = EXPRS_EXPR (args);
            DBUG_PRINT ("DFA", ("type of arg is %s", NODE_TEXT (arg)));
            if (NODE_TYPE (arg) == N_id) {
                DBUG_PRINT ("DFA", ("add %s", ID_NAME (arg)));
                DFMSetMaskEntrySet (LET_USEMASK (arg_node), ID_NAME (arg), NULL);
            } else if ((NODE_TYPE (arg) == N_num) || (NODE_TYPE (arg) == N_array)) {
                /* ignore */
            } else {
                DBUG_ASSERT (0, "unknown kind of arg found (watch DFA)");
            }
            args = EXPRS_NEXT (args);
        }
    } else if ((NODE_TYPE (expr) == N_Nwith2)) {
        /*
         *  If there is no scheduling, we traverse the with-loop.
         */
        if (!NWITH2_ISSCHEDULED (expr)) {
            DBUG_PRINT ("DFA", ("into with-loop"));
            LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
            DBUG_PRINT ("DFA", ("from with-loop"));
            expr = LET_EXPR (arg_node);
        }

        /*
         *  the with-loop (N_Nwith2) already has DFMs attached:
         *  NWITH2_IN, NWITH2_INOUT, NWITH2_OUT and NWITH2_LOCAL.
         *  Here we use the information to build the LET_USEMASK and LET_DEFMASK.
         *  LET_USEMASK = NWITH2_IN
         *  LET_DEFMASK = NWITH2_INOUT + NWITH2_OUT
         */

        /*
            fixmaskbase( NWITH2_IN( expr),
                         FUNDEF_DFM_BASE( INFO_MUTH_FUNDEF( arg_info)));
            fixmaskbase( NWITH2_INOUT( expr),
                         FUNDEF_DFM_BASE( INFO_MUTH_FUNDEF( arg_info)));
            fixmaskbase( NWITH2_OUT( expr),
                         FUNDEF_DFM_BASE( INFO_MUTH_FUNDEF( arg_info)));
        */

        /* LET_USEMASK = LET_USEMASK + NWITH2_IN */
        vardec = DFMGetMaskEntryDeclSet (NWITH2_IN (expr));
        while (vardec != NULL) {
            DBUG_PRINT ("DFA", ("from in add use %s", VARDEC_NAME (vardec)));
            DFMSetMaskEntrySet (LET_USEMASK (arg_node), NULL, vardec);
            vardec = DFMGetMaskEntryDeclSet (NULL);
        }

        /* LET_DEFMASK = LET_DEFMASK + NWITH2_INOUT */
        vardec = DFMGetMaskEntryDeclSet (NWITH2_INOUT (expr));
        while (vardec != NULL) {
            DBUG_PRINT ("DFA", ("from inout add def %s", VARDEC_NAME (vardec)));
            DFMSetMaskEntrySet (LET_DEFMASK (arg_node), NULL, vardec);
            vardec = DFMGetMaskEntryDeclSet (NULL);
        }

        /* LET_DEFMASK = LET_DEFMASK + NWITH2_OUT */
        vardec = DFMGetMaskEntryDeclSet (NWITH2_OUT (expr));
        while (vardec != NULL) {
            DBUG_PRINT ("DFA", ("from out add def %s", VARDEC_NAME (vardec)));
            DFMSetMaskEntrySet (LET_DEFMASK (arg_node), NULL, vardec);
            vardec = DFMGetMaskEntryDeclSet (NULL);
        }
        done_lhs = TRUE;
    } else {
        DBUG_ASSERT (0, "unhandled kind of let");
    }

    /*
     *  if the lhs has not been done already we infer it here.
     *  we simply traverse" the ids of lhs.
     */
    if (!done_lhs) {
        let_ids = LET_IDS (arg_node);
        while (let_ids != NULL) {
            vardec = IDS_VARDEC (let_ids);
            DFMSetMaskEntrySet (LET_DEFMASK (arg_node), NULL, vardec);
            let_ids = IDS_NEXT (let_ids);
        }
    }

    DBUG_PRINT ("DFA", ("end"));
    DBUG_RETURN (arg_node);
}
