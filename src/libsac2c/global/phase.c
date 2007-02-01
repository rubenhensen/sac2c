/*
 * $Id$
 */

#include "types.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "check.h"
#include "check_mem.h"

#include "phase.h"

#define PHASEfun(it_fun) extern node *it_fun (node *syntax_tree);
#define SUBPHASEfun(it_fun) extern node *it_fun (node *syntax_tree);
#define OPTCYCLEfun(it_fun) extern node *it_fun (node *syntax_tree);
#define OPTINCYCfun(it_fun) extern node *it_fun (node *syntax_tree);
#define OPTINCYCFUNfun(it_fun) extern node *it_fun (node *syntax_tree);

#include "phase_info.mac"

#undef PHASEfun
#undef SUBPHASEfun
#undef OPTCYCLEfun
#undef OPTINCYCfun
#undef OPTINCYCFUNfun

typedef node *(*phase_fun_p) (node *);

static const char *phase_name[] = {"initial",
#define PHASEtext(it_text) it_text,
#include "phase_info.mac"
#undef PHASEtext
                                   ""};

static const phase_fun_p phase_fun[] = {PHdummy,
#define PHASEfun(it_fun) it_fun,
#include "phase_info.mac"
#undef PHASEfun
                                        PHdummy};

static const char *subphase_name[] = {"initial",
#define SUBPHASEtext(it_text) it_text,
#define OPTCYCLEtext(it_text) it_text,
#include "phase_info.mac"
#undef SUBPHASEtext
#undef OPTCYCLEtext
                                      ""};

static const phase_fun_p subphase_fun[] = {PHdummy,
#define SUBPHASEfun(it_fun) it_fun,
#define OPTCYCLEfun(it_fun) it_fun,
#include "phase_info.mac"
#undef SUBPHASEfun
#undef OPTCYCLEfun
                                           PHdummy};

static const char *optincyc_name[] = {"initial",
#define OPTINCYCtext(it_text) it_text,
#define OPTINCYCFUNtext(it_text) it_text,
#include "phase_info.mac"
#undef OPTINCYCtext
#undef OPTINCYCFUNtext
                                      ""};

static const phase_fun_p optincyc_fun[] = {PHdummy,
#define OPTINCYCfun(it_fun) it_fun,
#define OPTINCYCFUNfun(it_fun) it_fun,
#include "phase_info.mac"
#undef OPTINCYCfun
#undef OPTINCYCFUNfun
                                           PHdummy};

static const compiler_allphase_t phase2allphase[] = {PHALL_initial,
#define PHASEelement(it_element) PHALL_##it_element,
#include "phase_info.mac"
                                                     PHALL_final
#undef PHASEelement
};

static const compiler_allphase_t subphase2allphase[] = {PHALL_initial,
#define SUBPHASEelement(it_element) PHALL_##it_element,
#define OPTCYCLEelement(it_element) PHALL_##it_element,
#include "phase_info.mac"
                                                        PHALL_final
#undef SUBPHASEelement
#undef OPTCYCLEelement
};

static const compiler_allphase_t optincyc2allphase[] = {PHALL_initial,
#define OPTINCYCelement(it_element) PHALL_##it_element,
#define OPTINCYCFUNelement(it_element) PHALL_##it_element,
#include "phase_info.mac"
                                                        PHALL_final
#undef OPTINCYCelement
#undef OPTINCYCFUNelement
};

const char *
PHphaseName (compiler_phase_t phase)
{
    DBUG_ENTER ("PHphaseName");

    DBUG_RETURN (phase_name[phase]);
}

const char *
PHsubPhaseName (compiler_subphase_t subphase)
{
    DBUG_ENTER ("PHsubPhaseName");

    DBUG_RETURN (subphase_name[subphase]);
}

const char *
PHoptInCycName (compiler_optincyc_t optincyc)
{
    DBUG_ENTER ("PHoptInCycName");

    DBUG_RETURN (optincyc_name[optincyc]);
}

void
PHinterpretBreakOption (char *option)
{
    int num;
    char *rest;

    DBUG_ENTER ("PHinterpreteBreakOption");

    num = strtol (option, &rest, 10);

    if (rest[0] != '\0') {
        CTIerror ("Illegal argument for break option: -b %s", option);
    } else {

        /*
         * First, we check for compiler phase specifications.
         */

#define PHASEelement(it_element)                                                         \
    if (ILIBstringCompare (option, #it_element) || (num == (int)PH_##it_element)) {      \
        global.break_after = PH_##it_element;                                            \
    } else

#include "phase_info.mac"

#undef PHASEelement

        /*
         * Next, we check for compiler subphase specifications.
         */

#define SUBPHASEelement(it_element)                                                      \
    if (ILIBstringCompare (option, #it_element)) {                                       \
        global.break_after_subphase = SUBPH_##it_element;                                \
    } else

#define OPTCYCLEelement(it_element)                                                      \
    if (ILIBstringCompare (option, #it_element)) {                                       \
        global.break_after_subphase = SUBPH_##it_element;                                \
    } else

#include "phase_info.mac"

#undef SUBPHASEelement
#undef OPTCYCLEelement

        /*
         * At last, we check for cycle optimisation specifications.
         */

#define OPTINCYCelement(it_element)                                                      \
    if (ILIBstringCompare (option, #it_element)) {                                       \
        global.break_after_optincyc = OIC_##it_element;                                  \
    } else

#define OPTINCYCFUNelement(it_element)                                                   \
    if (ILIBstringCompare (option, #it_element)) {                                       \
        global.break_after_optincyc = OIC_##it_element;                                  \
    } else

#include "phase_info.mac"

#undef OPTINCYCelement
#undef OPTINCYCFUNelement

        {
            CTIerror ("Illegal argument for break option: -b %s", option);
        }
    }

    DBUG_VOID_RETURN;
}

node *
PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ("PHrunCompilerPhase");

    global.compiler_phase = phase;

    if ((global.compiler_phase <= phase) && (phase != PH_initial)
        && (phase != PH_final)) {
        global.compiler_phase = phase;
        global.compiler_allphase = phase2allphase[phase];

#ifndef DBUG_OFF
        if ((global.my_dbug) && (!global.my_dbug_active)
            && (global.compiler_phase >= global.my_dbug_from)
            && (global.compiler_phase <= global.my_dbug_to)) {
            DBUG_PUSH (global.my_dbug_str);
            global.my_dbug_active = 1;
        }
#endif

        CTInote (" ");

        if (cond) {
            CTIstate ("** %2d: %s ...", (int)phase, PHphaseName (phase));
            syntax_tree = phase_fun[phase](syntax_tree);

            CTIabortOnError ();

#ifdef SHOW_MALLOC
            DBUG_EXECUTE ("MEM_LEAK", MEMdbugMemoryLeakCheck (););
#endif

#ifdef SHOW_MALLOC
            if (global.treecheck && (syntax_tree != NULL)) {
                syntax_tree = CHKdoTreeCheck (syntax_tree);
            }

            if (global.memcheck && (syntax_tree != NULL)) {
                syntax_tree = CHKMdoMemCheck (syntax_tree);
            }
#endif
        } else {
            CTIstate ("** %2d: %s skipped.", (int)phase, PHphaseName (phase));
        }

        if ((global.my_dbug) && (global.my_dbug_active)
            && (global.compiler_phase >= global.my_dbug_to)) {
            DBUG_POP ();
            global.my_dbug_active = 0;
        }

        if (global.break_after == phase) {
            CTIterminateCompilation (syntax_tree);
        }
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCompilerSubPhase (compiler_subphase_t subphase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ("PHrunCompilerSubPhase");

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerSubPhase called with non N_module node");

    global.compiler_subphase = subphase;
    global.compiler_allphase = subphase2allphase[subphase];

    if (cond) {
        CTInote ("**** %s ...", PHsubPhaseName (subphase));
        syntax_tree = subphase_fun[subphase](syntax_tree);
        CTIabortOnError ();

#ifdef SHOW_MALLOC
        if ((global.treecheck) && (syntax_tree != NULL)) {
            syntax_tree = CHKdoTreeCheck (syntax_tree);
        }

        if (global.memcheck && (syntax_tree != NULL)) {
            syntax_tree = CHKMdoMemCheck (syntax_tree);
        }
#endif
    } else {
        /* CTIstate("**** Skipped: %s ", PHsubPhaseName( subphase)); */
    }

    if (global.break_after_subphase == subphase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunOptimizationInCycleFun (compiler_optincyc_t optincyc, int pass, node *fundef,
                             bool cond)
{
    DBUG_ENTER ("PHrunOptimizationInCycleFun");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_fundef)),
                 "PHrunOptimizationInCycleFun called with non N_fundef node");

    global.compiler_optincyc = optincyc;
    global.compiler_allphase = optincyc2allphase[optincyc];

    if (cond
        && ((optincyc <= global.break_after_optincyc)
            || (pass < global.break_cycle_specifier))) {

        DBUG_EXECUTE ("OPT", CTIstate ("****** %s ...", PHoptInCycName (optincyc)););

        fundef = optincyc_fun[optincyc](fundef);

        CTIabortOnError ();

        fundef = TCappendFundef (fundef, DUPgetCopiedSpecialFundefs ());
    }

    if (optincyc == global.break_after_optincyc) {
        global.break_after_subphase = global.compiler_subphase;
    }

    DBUG_RETURN (fundef);
}

node *
PHrunOptimizationInCycle (compiler_optincyc_t optincyc, int pass, node *fundef, bool cond)
{
    DBUG_ENTER ("PHrunOptimizationInCycle");

    DBUG_ASSERT (((fundef != NULL) && (NODE_TYPE (fundef) == N_module)),
                 "PHrunOptimizationInCycle called with non N_module node");

    global.compiler_optincyc = optincyc;
    global.compiler_allphase = optincyc2allphase[optincyc];

    if (cond
        && ((optincyc <= global.break_after_optincyc)
            || (pass < global.break_cycle_specifier))) {

        DBUG_EXECUTE ("OPT", CTIstate ("****** %s ...", PHoptInCycName (optincyc)););

        fundef = optincyc_fun[optincyc](fundef);

        CTIabortOnError ();
    }

    if (optincyc == global.break_after_optincyc) {
        global.break_after_subphase = global.compiler_subphase;
    }

    DBUG_RETURN (fundef);
}

node *
PHdummy (node *syntax_tree)
{
    DBUG_ENTER ("PHdummy");

    DBUG_ASSERT (FALSE, "This function should never be called.");

    DBUG_RETURN (syntax_tree);
}

node *
PHidentity (node *syntax_tree)
{
    DBUG_ENTER ("PHidentity");

    DBUG_RETURN (syntax_tree);
}
