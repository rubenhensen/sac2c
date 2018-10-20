#include "phase.h"

#define DBUG_PREFIX "OPT"
#include "debug.h"

#include "print.h"

#include "filemgr.h"
#include "ctinfo.h"
#include "globals.h"
#include "profiler.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "specialize.h"
#include "DupTree.h"
#include "check.h"
#include "check_mem.h"
#include "check_reset.h"
#include "phase_drivers.h"
#include "phase_info.h"
#include "statistics.h"
#include "setfundefwasoptimized.h"
#include "check_lacfuns.h"

/*
 * static global variables
 */

static optimize_counter_t oc_global;
static optimize_counter_t oc_pass;

/** <!--********************************************************************-->
 *
 * @fn bool PHisSAAMode( node *arg_node)
 *
 * @brief Predicates for those compiler phases in which AVIS_DIM and AVIS_SHAPE
 *        should be generated and propagated.
 *
 *****************************************************************************/

bool
PHisSAAMode (void)
{
    bool z;

    DBUG_ENTER ();

    z = global.optimize.dosaa
        && ((global.compiler_anyphase > PH_opt_saacyc_isaa)
            && (global.compiler_anyphase < PH_opt_esaa));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool PHisSSAMode( node *arg_node)
 *
 * @brief Predicates for those compiler phases that are running in SSA mode.
 *
 *****************************************************************************/

bool
PHisSSAMode (void)
{
    bool z;

    DBUG_ENTER ();

    z = ((global.compiler_anyphase >= PH_tc) && (global.compiler_anyphase < PH_ussa));

    DBUG_RETURN (z);
}

#ifndef DBUG_OFF

static void
CheckEnableDbug (compiler_phase_t phase)
{
    DBUG_ENTER ();

    if (global.my_dbug && (phase >= global.my_dbug_from) && !global.my_dbug_active) {
        DBUG_PUSH (global.my_dbug_str);
        global.my_dbug_active = TRUE;
    }

    DBUG_RETURN ();
}

static void
CheckDisableDbug (compiler_phase_t phase)
{
    DBUG_ENTER ();

    if (global.my_dbug && global.my_dbug_active && (phase >= global.my_dbug_to)) {
        DBUG_POP ();
        global.my_dbug_active = FALSE;
    }

    DBUG_RETURN ();
}

node *
PHrunConsistencyChecks (node *arg_node)
{
    DBUG_ENTER ();

    if (arg_node != NULL) {
        if (global.treecheck) {
            CTItell (4, "         -> Running syntax tree checks");
            arg_node = CHKRSTdoTreeCheckReset (arg_node);
            arg_node = CHKdoTreeCheck (arg_node);
        }

        if (global.lacfuncheck) {
            CTItell (4, "         -> Running LaC fun checks");
            arg_node = CHKLACFdoCheckLacFuns (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

#else /* DBUG_OFF */
/* production version */

node *
PHrunConsistencyChecks (node *arg_node)
{
    DBUG_ENTER ();
    /* nothing in production version */
    DBUG_RETURN (arg_node);
}

#endif /* DBUG_OFF */

node *
PHrunPhase (compiler_phase_t phase, node *syntax_tree, bool cond)
{

    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (phase) == PHT_phase,
                 "PHrunPhase called with incompatible phase: %s", PHIphaseIdent (phase));

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunPhase called with non N_module node");

    global.compiler_phase = phase;
    global.compiler_anyphase = phase;
    global.phase_num = global.phase_num + 1;

#ifndef DBUG_OFF
    CheckEnableDbug (phase);
#endif

    CTInote (" ");

    if (cond) {
        CTIstate ("** %2d: %s ...", global.phase_num, PHIphaseText (phase));

        TIMEbegin (phase);
        syntax_tree = PHIphaseFun (phase) (syntax_tree);
        TIMEend (phase);

        CTIabortOnError ();

#ifndef DBUG_OFF
        if (global.check_frequency == 1) {
            syntax_tree = PHrunConsistencyChecks (syntax_tree);
        }

        if (global.memcheck && (syntax_tree != NULL)) {
            syntax_tree = CHKMdoMemCheck (syntax_tree);
        }
#endif

    } else {
        CTIstate ("** %2d: %s skipped.", global.phase_num, PHIphaseText (phase));
    }

#ifndef DBUG_OFF
    CheckDisableDbug (phase);
#endif

    CTIabortOnError ();

    /*
     *phase printing
     */
    if ((global.prtphafun_start_phase != PH_undefined && global.prt_cycle_range == TRUE)
        || (global.prtphafun_start_phase == phase
            && global.prtphafun_start_subphase == PH_undefined)) {
        if (global.prtphafun_stop_phase == PH_undefined) {
            CTIerror (
              "Please use both -printstart <phase_id> and -printstop <phase_id>\n"
              "If it is only one phase/subphase/cyclephase you want reported\n"
              "\nthen the -printstart and -printstop options will be identical.\n");
        } else {
            global.prt_cycle_range = TRUE;
            PRTdoPrintFile (FMGRwriteOpen ("%s.%d", global.outfilename, global.phase_num),
                            syntax_tree);
            if (global.prtphafun_stop_phase == phase) {
                global.prt_cycle_range = FALSE;
            }
        }
    }

    CTIabortOnError ();

    if (global.break_after_phase == phase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunSubPhase (compiler_phase_t subphase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (subphase) == PHT_subphase,
                 "PHrunSubPhase called with incompatible phase: %s",
                 PHIphaseIdent (subphase));

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunSubPhase called with non N_module node");

    global.compiler_subphase = subphase;
    global.compiler_anyphase = subphase;

#ifndef DBUG_OFF
    CheckEnableDbug (subphase);
#endif

    if (cond) {
        if (PHIphaseType (subphase) != PHT_cycle) {
            CTInote ("**** %s ...", PHIphaseText (subphase));
        }

        TIMEbegin (subphase);
        syntax_tree = PHIphaseFun (subphase) (syntax_tree);
        TIMEend (subphase);

        CTIabortOnError ();

#ifndef DBUG_OFF
        if (global.check_frequency >= 2) {
            syntax_tree = PHrunConsistencyChecks (syntax_tree);
        }

        if (global.memcheck && (syntax_tree != NULL)) {
            syntax_tree = CHKMdoMemCheck (syntax_tree);
        }
#endif
    }

#ifndef DBUG_OFF
    CheckDisableDbug (subphase);
#endif

    CTIabortOnError ();

    /*
     *subphase printing
     */
    if ((global.prtphafun_start_subphase != PH_undefined
         && global.prt_cycle_range == TRUE)
        || global.prtphafun_start_subphase == subphase) {
        if (global.prtphafun_stop_phase == PH_undefined) {
            CTIerror (
              "Please use both -printstart <phase_id> and -printstop <phase_id>\n"
              "If it is only one phase/subphase/cyclephase you want reported\n"
              "\nthen the -printstart and -printstop options should be identical.\n");
        } else {
            global.prt_cycle_range = TRUE;
            PRTdoPrintFile (FMGRwriteOpen ("%s.%d.%s", global.outfilename,
                                           global.phase_num, PHIphaseIdent (subphase)),
                            syntax_tree);
            if (global.prtphafun_stop_subphase == subphase) {
                global.prt_cycle_range = FALSE;
            }
        }
    }

    if (global.break_after_subphase == subphase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCycle (compiler_phase_t cycle, node *syntax_tree, bool cond, bool reset)
{
    bool go_on;

    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (cycle) == PHT_cycle,
                 "PHrunCycle called with incompatible phase: %s", PHIphaseIdent (cycle));

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCycle called with wrong node type.");

    global.compiler_subphase = cycle;
    global.compiler_anyphase = cycle;

    if (cond) {

#ifndef DBUG_OFF
        CheckEnableDbug (cycle);
#endif

        if (reset) {
            syntax_tree = SFWOdoSetFundefWasOptimized (syntax_tree);
        }

        STATcopyCounters (&oc_global, &global.optcounters);
        STATclearCounters (&global.optcounters);

        global.cycle_counter = 1;
        do {
            CTInote (" ");
            CTInote ("**** %s pass: %i", PHIphaseText (cycle), global.cycle_counter);
            STATclearCounters (&oc_pass);

            TIMEbegin (cycle);
            syntax_tree = PHIphaseFun (cycle) (syntax_tree);
            TIMEend (cycle);

            CTIabortOnError ();

#ifndef DBUG_OFF
            if (global.check_frequency >= 2) {
                syntax_tree = PHrunConsistencyChecks (syntax_tree);
            }

            if (global.memcheck) {
                syntax_tree = CHKMdoMemCheck (syntax_tree);
            }
#endif

            STATaddCounters (&oc_pass, &global.optcounters);
            STATclearCounters (&global.optcounters);

            if (STATdidSomething (&oc_pass)) {
                go_on = TRUE;
                STATaddCounters (&oc_global, &oc_pass);
            } else {
                go_on = FALSE;
                CTInote (" ");
            }

            /*
             *cycle printing
             */
            if ((global.prtphafun_start_cycle != PH_undefined
                 && global.prt_cycle_range == TRUE)
                || (global.prtphafun_start_subphase == cycle
                    && global.prtphafun_start_cycle == PH_undefined)) {
                if (global.prtphafun_stop_phase == PH_undefined) {
                    CTIerror (
                      "Please use both -printstart <phase_id> and -printstop <phase_id>\n"
                      "If it is only one phase/subphase/cyclephase you want reported\n"
                      "\nthen the -printstart and -printstop options will be "
                      "identical.\n");
                } else {
                    global.prt_cycle_range = TRUE;
                    PRTdoPrintFile (FMGRwriteOpen ("%s.%d.%s.%d", global.outfilename,
                                                   global.phase_num,
                                                   PHIphaseIdent (cycle),
                                                   global.cycle_counter),
                                    syntax_tree);
                    if (global.prtphafun_stop_subphase == cycle) {
                        global.prt_cycle_range = FALSE;
                    }
                }
            }

            global.cycle_counter += 1;

        } while (go_on && (global.cycle_counter <= global.max_optcycles)
                 && ((global.cycle_counter <= global.break_cycle_specifier)
                     || (global.break_after_cyclephase > global.compiler_cyclephase)));

        STATcopyCounters (&global.optcounters, &oc_global);

        if (go_on && (global.cycle_counter == global.max_optcycles)) {
            CTIwarn ("Maximum number of optimization cycles reached");
            global.run_stabilization_cycle = TRUE;
        }

#ifndef DBUG_OFF
        CheckDisableDbug (cycle);
#endif
    }

    CTIabortOnError ();

    if (global.break_after_subphase == cycle) {
        CTIterminateCompilation (syntax_tree);
    }

    global.cycle_counter = 0; /* Give each cycle an equal chance */
    DBUG_RETURN (syntax_tree);
}

node *
PHrunCyclePhase (compiler_phase_t cyclephase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (cyclephase) == PHT_cyclephase,
                 "PHrunPhase called with incompatible phase: %s",
                 PHIphaseIdent (cyclephase));

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCyclePhase called with wrong node type.");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (global.cycle_counter < global.break_cycle_specifier))) {
        CTInote ("****** %s ...", PHIphaseText (cyclephase));

        TIMEbegin (cyclephase);
        syntax_tree = PHIphaseFun (cyclephase) (syntax_tree);
        TIMEend (cyclephase);

        CTIabortOnError ();

#ifndef DBUG_OFF
        if (global.check_frequency == 3) {
            syntax_tree = PHrunConsistencyChecks (syntax_tree);
        }
#endif
    }

#ifndef DBUG_OFF
    if (global.memcheck) {
        syntax_tree = CHKMdoMemCheck (syntax_tree);
    }
    CheckDisableDbug (cyclephase);
#endif

    CTIabortOnError ();

    /*
     *cyclephase printing
     */
    if ((global.prtphafun_start_cycle == cyclephase
         && global.prtphafun_start_cycle_specifier <= global.cycle_counter
         && global.prtphafun_stop_cycle_specifier >= global.cycle_counter)
        || (global.prtphafun_start_cycle == cyclephase
            && global.prtphafun_stop_cycle_specifier == 0)) {
        if (global.prtphafun_stop_phase == PH_undefined) {
            CTIerror (
              "Please use both -printstart <phase_id> and -printstop <phase_id>\n"
              "If it is only one phase/subphase/cyclephase you want reported\n"
              "\nthen the -printstart and -printstop options should be identical.\n");
        } else {
            PRTdoPrintFile (FMGRwriteOpen ("%s.%d.%s.%d", global.outfilename,
                                           global.phase_num, PHIphaseIdent (cyclephase),
                                           global.cycle_counter),
                            syntax_tree);
        }
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCycleFun (compiler_phase_t cycle, node *syntax_tree)
{
    node *fundef;
    node *specialized_fundefs;
    node *copied_special_fundefs;

    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (cycle) == PHT_cycle_fun,
                 "PHrunPhase called with incompatible phase: %s", PHIphaseIdent (cycle));

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCycleFun called with wrong node type.");

    STATaddCounters (&oc_pass, &global.optcounters);
    STATclearCounters (&global.optcounters);

    fundef = MODULE_FUNS (syntax_tree);

    while (fundef != NULL) {
        if (!FUNDEF_ISZOMBIE (fundef) && !FUNDEF_ISTYPEERROR (fundef)
            && !FUNDEF_ISWRAPPERFUN (fundef) && FUNDEF_WASOPTIMIZED (fundef)) {
            CTItell (4, " ");

            if (FUNDEF_ISLOOPFUN (fundef)) {
                CTInote ("****** Optimizing loop function:");
            } else if (FUNDEF_ISCONDFUN (fundef)) {
                CTInote ("****** Optimizing conditional function:");
            } else {
                CTInote ("****** Optimizing regular function:");
            }

            CTInote ("******  %s( %s): ...", CTIitemName (fundef), CTIfunParams (fundef));

            FUNDEF_ISINLINECOMPLETED (fundef) = FALSE;

            fundef = PHIphaseFun (cycle) (fundef);

            CTIabortOnError ();

            FUNDEF_WASOPTIMIZED (fundef) = STATdidSomething (&global.optcounters);

            if (FUNDEF_WASOPTIMIZED (fundef)) {
                STATaddCounters (&oc_pass, &global.optcounters);
                STATclearCounters (&global.optcounters);
                DBUG_PRINT ("Function %s was optimized", FUNDEF_NAME (fundef));
            } else {
                DBUG_PRINT ("Function %s was not optimized", FUNDEF_NAME (fundef));
            }

            DBUG_EXECUTE (STATprint (&global.optcounters));
        }

        if (FUNDEF_NEXT (fundef) == NULL) {
            specialized_fundefs = SPECresetSpecChain ();
            copied_special_fundefs = DUPgetCopiedSpecialFundefs ();
            FUNDEF_NEXT (fundef)
              = TCappendFundef (specialized_fundefs, copied_special_fundefs);
        }

        fundef = FUNDEF_NEXT (fundef);
    }

#ifndef DBUG_OFF
    if (global.memcheck) {
        syntax_tree = CHKMdoMemCheck (syntax_tree);
    }
#endif

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCyclePhaseFun (compiler_phase_t cyclephase, node *fundef, bool cond)
{
    node *fundef_next;

    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (cyclephase) == PHT_cyclephase_fun,
                 "PHrunCyclePhaseFun called with incompatible phase: %s",
                 PHIphaseIdent (cyclephase));

    DBUG_ASSERT ((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef),
                 "PHrunCyclePhaseFun called with wrong node type.");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (global.cycle_counter < global.break_cycle_specifier))) {

        CTItell (4, "         %s ...", PHIphaseText (cyclephase));

        fundef_next = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = NULL;

        DBUG_PRINT ("Calling phase for function: %s", FUNDEF_NAME (fundef));

        TIMEbegin (cyclephase);
        fundef = PHIphaseFun (cyclephase) (fundef);
        TIMEend (cyclephase);

        //DBUG_ASSERT (FUNDEF_NEXT (fundef) == NULL,
        //             "Fun-based cycle phase returned more than one fundef.");
        while (FUNDEF_NEXT (fundef))
          fundef = FUNDEF_NEXT (fundef);
        FUNDEF_NEXT (fundef) = fundef_next;

        CTIabortOnError ();

        /*
         *fundef cyclephase printing
         */
        if ((global.prtphafun_start_cycle == cyclephase
             && global.prtphafun_start_cycle_specifier <= global.cycle_counter
             && global.prtphafun_stop_cycle_specifier >= global.cycle_counter)
            || (global.prtphafun_start_cycle == cyclephase
                && global.prtphafun_stop_cycle_specifier == 0)) {
            if (global.prtphafun_stop_phase == PH_undefined) {
                CTIerror (
                  "Please use both -printstart <phase_id> and -printstop <phase_id>\n"
                  "If it is only one phase/subphase/cyclephase you want reported\n"
                  "\nthen the -printstart and -printstop options will be identical.\n");
            } else {
                if (global.break_fun_name == NULL
                    || STReq (FUNDEF_NAME (fundef), global.break_fun_name)) {
                    PRTdoPrintNodeFile (FMGRwriteOpen ("%s.%d.%s.%d.%s",
                                                       global.outfilename,
                                                       global.phase_num,
                                                       PHIphaseIdent (cyclephase),
                                                       global.cycle_counter,
                                                       FUNDEF_NAME (fundef)),
                                        fundef);
                }
            }
        }

#ifndef DBUG_OFF
        if (global.check_frequency >= 4) {
            fundef = PHrunConsistencyChecks (fundef);
        }
        CTIabortOnError ();

#endif
    }

#ifndef DBUG_OFF
    CheckDisableDbug (cyclephase);
#endif

    DBUG_RETURN (fundef);
}

node *
PHrunCyclePhaseFunOld (compiler_phase_t cyclephase, node *fundef, bool cond)
{
    DBUG_ENTER ();

    DBUG_ASSERT (PHIphaseType (cyclephase) == PHT_cyclephase_fun,
                 "PHrunCyclePhaseFun called with incompatible phase: %s",
                 PHIphaseIdent (cyclephase));

    DBUG_ASSERT ((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef),
                 "PHrunCyclePhaseFun called with wrong node type.");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (global.cycle_counter < global.break_cycle_specifier))) {

        CTItell (4, "         %s ...", PHIphaseText (cyclephase));

        TIMEbegin (cyclephase);
        fundef = PHIphaseFun (cyclephase) (fundef);
        TIMEend (cyclephase);

        CTIabortOnError ();

#ifndef DBUG_OFF
        if (global.check_frequency >= 4) {
            fundef = PHrunConsistencyChecks (fundef);
        }
        CTIabortOnError ();
#endif
    }

#ifndef DBUG_OFF
    CheckDisableDbug (cyclephase);
#endif

    DBUG_RETURN (fundef);
}

#undef DBUG_PREFIX
