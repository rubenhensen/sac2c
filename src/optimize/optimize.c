/*
 *
 * $Log$
 * Revision 2.5  1999/04/11 10:35:16  bs
 * TSI call moved to the 'right' place.
 *
 * Revision 2.4  1999/03/15 15:42:52  bs
 * temporary DBUG-Flag inserted
 *
 * Revision 2.3  1999/03/15 14:06:10  bs
 * Access macros renamed (take a look at tree_basic.h).
 *
 * Revision 2.2  1999/02/28 21:06:21  srs
 * removed DBUG output for WLF
 *
 * Revision 2.1  1999/02/23 12:41:53  sacbase
 * new release made
 *
 * Revision 1.108  1999/02/12 16:30:25  sbs
 * In the end of OPTfundef, the masks are only freed iff
 * MASK is not used as a DBUG_flag!
 *
 * Revision 1.107  1999/01/18 15:47:07  sbs
 * more dedicated breaks (wlt, wlf, cf2) inserted
 * generatemasks call after WLF / WLT made explicit here rather than implicit
 * calls in WithloopFolding / WithloopFoldingWLT respectively.
 *
 * Revision 1.106  1999/01/11 16:52:33  sbs
 * typos in arfg[3~[3~[3~_info nodes for ConstantFolding and
 * WithloopFolding.
 *
 * Revision 1.105  1999/01/08 11:29:46  sbs
 * fun-args for NOTE in OPTfundef added!
 *
 * Revision 1.104  1999/01/07 13:56:58  sbs
 * optimization process restructured for a function-wise optimization!
 *
 * Revision 1.103  1998/12/10 17:28:27  sbs
 * *** empty log message ***
 *
 * Revision 1.102  1998/11/08 15:05:56  dkr
 * OptTrav:
 *   NCODE_CBLOCK should never be NULL
 *   (if so, we will get an assert now :-)
 *
 * Revision 1.101  1998/08/20 12:11:17  srs
 * added GenerateMasks after DCR in Optimize()
 *
 * Revision 1.100  1998/08/06 17:27:43  dkr
 * removed unused vars
 *
 * Revision 1.99  1998/08/06 17:20:46  dkr
 * added OPTicm
 *
 * Revision 1.98  1998/08/04 13:23:48  srs
 * removed assertion in switch construct in OPTTrav
 *
 * Revision 1.97  1998/07/20 12:27:23  srs
 * inserted a second CF-phase after successful WLF. This may speed up the
 * optimizations a lot if much code has been generated by WLF.
 *
 * Revision 1.96  1998/07/16 11:32:43  sbs
 * inserted some comments
 *
 * Revision 1.95  1998/05/15 14:44:19  srs
 * added break specifier bo:wli
 *
 * Revision 1.94  1998/05/13 13:45:14  srs
 * renamed unr_expr to lunr_expr and inserted wlunr_expr
 * renamed unr_opt to lunr_opt and inserted wlunr_optr
 *
 * Revision 1.93  1998/05/12 15:12:39  srs
 * AE now uses MRDs so GenerateMasks has to be called before AE
 *
 * Revision 1.92  1998/05/05 13:02:59  srs
 * Splitted WLT phase from WLI/WLF.
 * WLT now stays active if WLI/WLF are deactivated by -noWLF.
 * WLT can be deactivated with the new switch -noWLT.
 *
 * Revision 1.91  1998/05/05 11:17:52  srs
 * changed MRD behaviour of old WLs.
 * MRD of index variables are now defined (inside WL body) and point
 * to N_with node
 *
 * Revision 1.90  1998/04/29 17:15:51  dkr
 * removed OPTSpmd
 *
 * ... [elminated] ...
 *
 * Revision 1.1  1994/12/09  10:47:40  sbs
 * Initial revision
 *
 *
 */

/*
 * This file contains functions needed for code optimization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "print.h"
#include "convert.h"

#include "optimize.h"
#include "generatemasks.h"
#include "freemasks.h"
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "DeadFunctionRemoval.h"
#include "LoopInvariantRemoval.h"
#include "Inline.h"
#include "Unroll.h"
#include "Unswitch.h"
#include "ArrayElimination.h"
#include "CSE.h"
#include "WithloopFolding.h"
#include "tile_size_inference.h"

/*
 * global variables to keep track of optimization's success
 */

int dead_expr;
int dead_var;
int dead_fun;
int cf_expr;
int lir_expr;
int lunr_expr;
int wlunr_expr;
int uns_expr;
int elim_arrays;
int inl_fun;
int optvar_counter;
int cse_expr;
int wlf_expr;
int wlt_expr;

int old_wlf_expr, old_wlt_expr;

/******************************************************************************
 *
 * function:
 *   node *GetExpr(node *arg_node)
 *
 * description:
 *   If arg_node is a let expression the RHS of it is returned.
 *   If such a RHS is of the kind "reshape(e1,e2)" only e2 is returned.
 *   Otherwise, arg_node itself is returned.
 *
 ******************************************************************************/

node *
GetExpr (node *arg_node)
{
    DBUG_ENTER ("GetExpr");
    if (arg_node && (NODE_TYPE (arg_node) == N_assign)) {
        if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let) {
            arg_node = LET_EXPR (ASSIGN_INSTR (arg_node));
            if (N_prf == NODE_TYPE (arg_node) && (F_reshape == PRF_PRF (arg_node)))
                arg_node = PRF_ARG2 (arg_node);
        }
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GenOptVar
 *  arguments     : 1) counter number
 *  description   : allocate string for variable needed for optimization
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : ---
 *  macros        : ---
 *
 *  remarks       :
 *
 */

char *
GenOptVar (int count, char *var_name)
{
    char *string;

    DBUG_ENTER ("GenOptVar");

    string = (char *)Malloc (sizeof (char) * VAR_LENGTH);
    sprintf (string, "%s%d", var_name, count);

    DBUG_PRINT ("TMP", ("new variable: %s", string));

    DBUG_RETURN (string);
}

/******************************************************************************
 *
 * function:
 *  void ResetCounters()
 *
 * description:
 *   sets all global optimization counters to zero.
 *
 ******************************************************************************/

void
ResetCounters ()
{
    DBUG_ENTER ("ResetCounters");

    inl_fun = 0;
    dead_expr = 0;
    dead_var = 0;
    dead_fun = 0;
    lir_expr = 0;
    cf_expr = 0;
    lunr_expr = 0;
    wlunr_expr = 0;
    uns_expr = 0;
    inl_fun = 0;
    elim_arrays = 0;
    wlf_expr = 0;
    wlt_expr = 0;
    cse_expr = 0;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *  void PrintStatistics(int off_inl_fun, int off_dead_expr, int off_dead_var,
 *                       int off_dead_fun, int off_lir_expr, int off_cf_expr,
 *                       int off_lunr_expr, int off_wlunr_expr, int off_uns_expr,
 *                       int off_elim_arrays, int off_wlf_expr, int off_wlt_expr,
 *                       int off_cse_expr, int flag)
 *
 * description:
 *   prints all counters - specified offset provided that the respective
 *   optimization is turned on!
 *
 ******************************************************************************/

#define NON_ZERO_ONLY 0
#define ALL 1

void
PrintStatistics (int off_inl_fun, int off_dead_expr, int off_dead_var, int off_dead_fun,
                 int off_lir_expr, int off_cf_expr, int off_lunr_expr, int off_wlunr_expr,
                 int off_uns_expr, int off_elim_arrays, int off_wlf_expr,
                 int off_wlt_expr, int off_cse_expr, int flag)
{
    int diff;
    DBUG_ENTER ("PrintStatistics");

    diff = inl_fun - off_inl_fun;
    if (opt_inl && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d function(s) inlined", diff));

    diff = elim_arrays - off_elim_arrays;
    if (opt_ae && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d array(s) eliminated", diff));

    diff = cf_expr - off_cf_expr;
    if (opt_cf && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d primfun application(s) eliminated (constant folding)", diff));

    diff = (dead_expr - off_dead_expr) + (dead_var - off_dead_var);
    if (opt_dcr && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d dead assignment(s) and %d unused variable declaration(s) removed",
               dead_expr - off_dead_expr, dead_var - off_dead_var));

    diff = dead_fun - off_dead_fun;
    if (opt_dfr && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d dead functions(s) removed", diff));

    diff = cse_expr - off_cse_expr;
    if (opt_cse && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d common subexpression(s) eliminated", diff));

    diff = wlf_expr - off_wlf_expr;
    if (opt_wlf && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d withloop(s) folded", diff));

    diff = lir_expr - off_lir_expr;
    if (opt_lir && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d loop invariant expression(s) moved", diff));

    diff = uns_expr - off_uns_expr;
    if (opt_uns && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d loop(s) unswitched", diff));

    diff = lunr_expr - off_lunr_expr;
    if (opt_lunr && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d loop(s) unrolled", diff));

    diff = wlunr_expr - off_wlunr_expr;
    if (opt_wlunr && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d withloop(s) unrolled", diff));

    DBUG_VOID_RETURN;
}

/******************************************************************************/
/*
 * Here, the functions needed for opt_tab follow.
 * Basically, these are
 *   Optimize      which governs the whole optimization phase and installs opt_tab
 *   OPTmodul      which runs all inter-procedural optimizations, i.e.:
 *                 - Inline
 *   OPTfundef     which runs all intra-procedural optimizations.
 */
/******************************************************************************
 *
 * function:
 *  node *Optimize( node *arg_node)
 *
 * description:
 *   steers the whole optimization process.
 *
 ******************************************************************************/

node *
Optimize (node *arg_node)
{
    funptr *tmp_tab;

    DBUG_ENTER ("Optimize");

    optvar_counter = optvar;

    ResetCounters ();

    tmp_tab = act_tab;
    act_tab = opt_tab;

    Trav (arg_node, NULL);

    act_tab = tmp_tab;

    NOTE ((""));
    NOTE (("overall optimization statistics:"));
    PrintStatistics (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ALL);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *OPTmodul( node *arg_node, node *arg_info)
 *
 * description:
 *   this functions applies all those optimizations that are inter-procedural,
 *   i.e., Inlining (INL) and DeadFunctionRemoval (DFR).
 *   Although INL could be done function-wise, i.e. in OPTfundef, it seems
 *   to be advantageous to do it for all functions BEFORE applying the other
 *   optimizations function-wise. The reason being, that it allows to prevent
 *   the optimization of functions that will be inlined and thus eliminated
 *   anyway in a later stage of optimization!
 *   Furthermore, DFR uses the inline-flag to indicate functions that are to
 *   be removed and thus requires all inlining to be finished before DFR is
 *   called.
 *   So the overall course of action during optimization is:
 *
 *               INL
 *               DFR
 *                |
 *   optimize function-wise by calling Trav (and thus OPTfundef)
 *                |
 *               DFR
 *
 ******************************************************************************/

node *
OPTmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OPTmodul");

    if (opt_inl) {
        arg_node = Inline (arg_node, arg_info); /* inline_tab */
    }

    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "inl")))
        goto DONE;

    if (opt_dfr) {
        arg_node = DeadFunctionRemoval (arg_node, arg_info);
    }

    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "dfr")))
        goto DONE;

    if (MODUL_FUNS (arg_node)) {
        /*
         * Now, we apply the intra-procedural optimizations function-wise!
         */
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);

        /*
         * After doing so, we apply DFR once again!
         */
        if (opt_dfr) {
            arg_node = DeadFunctionRemoval (arg_node, arg_info);
        }
    }

DONE:

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *OPTfundef( node *arg_node, node *arg_info)
 *
 * description:
 *    this function steers the intr-procedural optimization process. It
 *    successively applies most of the optimizations, i.e. a single function
 *    is optimized "completely" before the next one is processed.
 *    The order in which the optimizations are applied is critical to the
 *    overall effect; so changes made here should be done very CAREFULLY!
 *    The actual course of action is:
 *
 *        AE
 *        DCR
 *         |<-------\
 * loop1: CSE        |
 *        CF         |
 *        WLT        |
 *        WLF        |
 *        (CF)       |   (applied only if WLF succeeded!)
 *        DCR        |
 *        LUNR/WLUNR |
 *        UNS        |
 *        LIR        |
 *         |--------/
 *         |<-------\
 * loop2: CF         |
 *        WLT        |
 *         |--------/
 *        DCR
 *
 ******************************************************************************/

node *
OPTfundef (node *arg_node, node *arg_info)
{
    int mem_inl_fun = inl_fun;
    int mem_dead_expr = dead_expr;
    int mem_dead_var = dead_var;
    int mem_dead_fun = dead_fun;
    int mem_lir_expr = lir_expr;
    int mem_cf_expr = cf_expr;
    int mem_lunr_expr = lunr_expr;
    int mem_wlunr_expr = wlunr_expr;
    int mem_uns_expr = uns_expr;
    int mem_elim_arrays = elim_arrays;
    int mem_wlf_expr = wlf_expr;
    int mem_wlt_expr = wlt_expr;
    int mem_cse_expr = cse_expr;

    int old_lir_expr, old_lunr_expr, old_wlunr_expr, old_uns_expr;
    int old_cse_expr, old_wlf_expr, old_wlt_expr, old_dcr_expr, old_cf_expr;

    int loop1 = 0;
    int loop2 = 0;

    node *arg;
    static char argtype_buffer[80];
    static int buffer_space;
    char *tmp_str;
    int tmp_str_size;

    DBUG_ENTER ("OPTfundef");

    strcpy (argtype_buffer, "( ");
    buffer_space = 77;

    arg = FUNDEF_ARGS (arg_node);
    while ((arg != NULL) && (buffer_space > 5)) {

        tmp_str = Type2String (ARG_TYPE (arg), 1);
        tmp_str_size = strlen (tmp_str);

        if ((tmp_str_size + 3) <= buffer_space) {
            strcat (argtype_buffer, tmp_str);
            buffer_space -= tmp_str_size;
            if (ARG_NEXT (arg) != NULL) {
                strcat (argtype_buffer, ", ");
                buffer_space -= 2;
            }
        } else {
            strcat (argtype_buffer, "...");
            buffer_space = 0;
        }
        arg = ARG_NEXT (arg);
    }
    strcat (argtype_buffer, ")");

    NOTE (("optimizing %s %s: ...", FUNDEF_NAME (arg_node), argtype_buffer));

    if (opt_ae) {
        /*
         * AE needs mask for MRD generation now.
         */
        arg_node = GenerateMasks (arg_node, NULL);

        arg_node = ArrayElimination (arg_node, arg_node); /* ae_tab */
    }

    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "ae")))
        goto INFO;

    /*
     * necessary after AE (which does not care about masks while introducing
     * new variables:
     */
    arg_node = GenerateMasks (arg_node, NULL);

    if (opt_dcr) {
        arg_node = DeadCodeRemoval (arg_node, arg_info);
    }

    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "dcr")))
        goto INFO;

    /*
     * Now, we enter the first loop. It consists of:
     *   CSE, CF, WLT, WLF, (CF), DCR, LUNR/WLUNR, UNS, and LIR.
     */
    do {
        loop1++;
        DBUG_PRINT ("OPT", ("---------------------------------------- loop ONE, pass %d",
                            loop1));

        old_lir_expr = lir_expr;
        old_lunr_expr = lunr_expr;
        old_wlunr_expr = wlunr_expr;
        old_uns_expr = uns_expr;
        old_cse_expr = cse_expr;
        old_wlf_expr = wlf_expr;
        old_wlt_expr = wlt_expr;
        old_dcr_expr = dead_fun + dead_var + dead_expr;
        old_cf_expr = cf_expr;

        if (opt_cse) {
            arg_node = CSE (arg_node, arg_info); /* cse_tab */
            arg_node = GenerateMasks (arg_node, NULL);
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "cse")))
            goto INFO;

        if (opt_cf) {
            arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
            /* srs: CF does not handle the USE mask correctly. For example
               a = f(3);
               b = a;
               c = b;
               will be transformed to
               a = f(3);
               b = a;
               c = a;
               after CF. But the USE mask of b is not reduced.
               This leads to a DCR problem (b = a is removed but variable declaration
               for b not. */

            /* quick fix: always rebuild masks after CF */
            arg_node = GenerateMasks (arg_node, NULL);
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "cf")))
            goto INFO;

        if (opt_wlt) {
            arg_node = WithloopFoldingWLT (arg_node, arg_info); /* wlt */
            arg_node = GenerateMasks (arg_node, NULL);
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "wlt")))
            goto INFO;

        if (opt_wlf) {
            arg_node = WithloopFolding (arg_node, arg_info); /* wli, wlf */
            /*
             * rebuild mask which is necessary because of WL-body-substitutions
             * and nserted new variables to prevent wrong variable bindings.
             */
            arg_node = GenerateMasks (arg_node, NULL);
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && ((0 == strcmp (break_specifier, "wli"))
                || (0 == strcmp (break_specifier, "wlf"))))
            goto INFO;

        if (wlf_expr != old_wlf_expr) {
            /*
             * this may speed up the optimization phase a lot if a lot of code
             * has been inserted by WLF.
             */
            if (opt_cf) {
                arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
                arg_node = GenerateMasks (arg_node, NULL);
            }
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "cf2")))
            goto INFO;

        if (opt_dcr) {
            arg_node = DeadCodeRemoval (arg_node, arg_info); /* s.o. */
            arg_node = GenerateMasks (arg_node, NULL);
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "dcr")))
            goto INFO;

        if (opt_lunr || opt_wlunr) {
            arg_node = Unroll (arg_node, arg_info); /* unroll_tab */
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "unr")))
            goto INFO;

        if (opt_uns) {
            arg_node = Unswitch (arg_node, arg_info); /* unswitch_tab */
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "uns")))
            goto INFO;

        if (opt_lir) {
            arg_node
              = LoopInvariantRemoval (arg_node, arg_info); /* lir_tab and lir_mov_tab */
        }

        if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
            && (0 == strcmp (break_specifier, "lir")))
            goto INFO;

    } while (((lir_expr != old_lir_expr) || (lunr_expr != old_lunr_expr)
              || (uns_expr != old_uns_expr) || (wlunr_expr != old_wlunr_expr)
              || (wlf_expr != old_wlf_expr) || (cse_expr != old_cse_expr))
             && (loop1 < max_optcycles));

    /*
     * Now, we enter the second loop consisting of
     *   WLT and CF only.
     */
    while (wlt_expr != old_wlt_expr && opt_cf && (loop1 + loop2) < max_optcycles) {
        old_wlt_expr = wlt_expr;
        old_cf_expr = cf_expr;

        loop2++;

        DBUG_PRINT ("OPT", ("---------------------------------------- loop TWO, pass %d",
                            loop2));

        arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
        /* srs: CF does not handle the USE mask correctly. */
        /* quick fix: always rebuild masks after CF */
        arg_node = GenerateMasks (arg_node, NULL);

        if ((break_after == PH_sacopt) && (break_cycle_specifier == (loop1 + loop2))
            && (0 == strcmp (break_specifier, "cf")))
            goto INFO;

        /*
         *  This is needed to transform more index vectors in skalars
         * or vice versa.
         */
        DBUG_PRINT ("WLF", ("WITHLOOP TRANSFORMATIONS"));
        arg_node = WithloopFoldingWLT (arg_node, arg_info); /* wlt */
        arg_node = GenerateMasks (arg_node, NULL);

        if ((break_after == PH_sacopt) && (break_cycle_specifier == (loop1 + loop2))
            && (0 == strcmp (break_specifier, "wli")))
            goto INFO;
    }

    /*
     * Finally, we apply DCR once again:
     */
    if (opt_dcr) {
        arg_node = DeadCodeRemoval (arg_node, arg_info);
    }

    DBUG_EXECUTE ("DO_THE_TSI", (arg_node = TileSizeInference (arg_node)););
    /* This DBUG-Flag is a temporary one! It's for testing the tsi. */

INFO:
    if (loop1 + loop2 == max_optcycles
        && ((lir_expr != old_lir_expr) || (lunr_expr != old_lunr_expr)
            || (uns_expr != old_uns_expr) || (wlunr_expr != old_wlunr_expr)
            || (wlf_expr != old_wlf_expr) || (cse_expr != old_cse_expr))) {
        SYSWARN (("max_optcycles reached !!"));
    }
    PrintStatistics (mem_inl_fun, mem_dead_expr, mem_dead_var, mem_dead_fun, mem_lir_expr,
                     mem_cf_expr, mem_lunr_expr, mem_wlunr_expr, mem_uns_expr,
                     mem_elim_arrays, mem_wlf_expr, mem_wlt_expr, mem_cse_expr,
                     NON_ZERO_ONLY);

    DBUG_DO_NOT_EXECUTE ("MASK", arg_node = FreeMasks (arg_node););
    if (FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
