/*
 * $Id$
 */

#include "types.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "check.h"
#include "check_mem.h"
#include "phase_drivers.h"
#include "phase.h"

/*
 * Extern declarations of all functions used to implement a phase, subphase,
 * cycle or cyclephase.
 */

#define SUBPHASEfun(fun) extern node *fun (node *syntax_tree);
#define CYCLEPHASEfun(fun) extern node *fun (node *syntax_tree);

#include "phase_sac2c.mac"

#undef SUBPHASEfun
#undef CYCLEPHASEfun

/*
 * Function table of phase, subphase, cycle and cyclephase implementing
 * functions
 */

typedef node *(*phase_fun_p) (node *);

#define PHASEname(name) PHDdrivePhase_##name,
#define SUBPHASEfun(fun) fun,
#define CYCLEname(name) PHDdriveCycle_##name,
#define CYCLEPHASEfun(fun) fun,

static const phase_fun_p phase_fun[] = {PHdummy,
#include "phase_sac2c.mac"
                                        PHdummy};

#undef PHASEfun
#undef SUBPHASEfun
#undef CYCLEfun
#undef CYCLEPHASEfun

/*
 * Description table of phase, subphase, cycle and cyclephase implementing
 * functions
 */

#define PHASEtext(text) text,
#define SUBPHASEtext(text) text,
#define CYCLEtext(text) text,
#define CYCLEPHASEtext(text) text,

static const char *phase_text[] = {"initial",
#include "phase_sac2c.mac"
                                   ""};

#undef PHASEtext
#undef SUBPHASEtext
#undef CYCLEtext
#undef CYCLEPHASEtext

/*
 * Phase type table of phase, subphase, cycle and cyclephase implementing
 * functions
 */

typedef enum {
    PHT_dummy,
    PHT_phase,
    PHT_subphase,
    PHT_cycle,
    PHT_cyclephase
} phase_type_t;

#define PHASEname(name) PHT_phase,
#define SUBPHASEname(name) PHT_subphase,
#define CYCLEname(name) PHT_cycle,
#define CYCLEPHASEname(name) PHT_cyclephase,

static phase_type_t phase_type[] = {PHT_dummy,
#include "phase_sac2c.mac"
                                    PHT_dummy};

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname

/*
 * Phase name table of phase, subphase, cycle and cyclephase implementing
 * functions
 */

#define PHASEname(name) #name,
#define SUBPHASEname(name) #name,
#define CYCLEname(name) #name,
#define CYCLEPHASEname(name) #name,

static const char *phase_name[] = {"",
#include "phase_sac2c.mac"
                                   ""};

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname

/*
 * Phase parent phase table of phase, subphase, cycle and cyclephase implementing
 * functions
 */

#define PHASE(name, text, cond) PH_initial,

#define SUBPHASE(name, text, fun, cond, phase) PH_##phase,

#define CYCLE(name, text, cond, phase) PH_##phase,

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) PH_##phase##_##cycle,

static compiler_phase_t phase_parent[] = {PH_initial,
#include "phase_sac2c.mac"
                                          PH_initial};

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE

const char *
PHphaseText (compiler_phase_t phase)
{
    DBUG_ENTER ("PHphaseText");

    DBUG_RETURN (phase_text[phase]);
}

static compiler_phase_t
SearchPhaseByName (char *name)
{
    compiler_phase_t phase;

    DBUG_ENTER ("SearchPhaseByName");

    phase = PH_initial;

    do {
        phase++;
    } while ((phase < PH_final) && !STReq (phase_name[phase], name));

    DBUG_RETURN (phase);
}

static compiler_phase_t
SearchPhaseByNumber (int num)
{
    compiler_phase_t phase;
    int cnt;

    DBUG_ENTER ("SearchPhaseByNumber");

    phase = PH_initial;
    cnt = 0;

    do {
        phase++;
        if (phase_type[phase] == PHT_phase)
            cnt++;
    } while ((phase < PH_final) && (cnt < num));

    DBUG_RETURN (phase);
}

static compiler_phase_t
SearchSubPhase (compiler_phase_t phase, char *name)
{
    compiler_phase_t subphase;

    DBUG_ENTER ("SearchSubPhase");

    subphase = phase;

    do {
        do {
            subphase++;
        } while (phase_type[subphase] == PHT_cyclephase);
    } while ((phase_parent[subphase] == phase) && !STReq (phase_name[subphase], name));

    if (phase_parent[subphase] != phase) {
        subphase = PH_final;
    }

    DBUG_RETURN (subphase);
}

static compiler_phase_t
SearchCyclePhase (compiler_phase_t cycle, char *name)
{
    compiler_phase_t cyclephase;

    DBUG_ENTER ("SearchCyclePhase");

    cyclephase = cycle;

    do {
        cyclephase++;
    } while ((phase_parent[cyclephase] == cycle)
             && !STReq (phase_name[cyclephase], name));

    if (phase_parent[cyclephase] != cycle) {
        cyclephase = PH_final;
    }

    DBUG_RETURN (cyclephase);
}

#if 1

void
PHinterpretBreakOption (char *option)
{
    compiler_phase_t phase;
    compiler_phase_t subphase;
    compiler_phase_t cyclephase;
    char *break_phase;
    char *break_subphase;
    char *break_cyclephase;
    char *break_cyclepass;
    char *rest;
    int num;

    DBUG_ENTER ("PHinterpretBreakOption");

    DBUG_PRINT ("PH", ("Interpreting break option: %s", option));

    break_phase = STRtok (option, ":");

    num = strtol (break_phase, &rest, 10);

    if (rest == break_phase) {
        /*
         * The phase spec is not a number.
         */
        phase = SearchPhaseByName (break_phase);
    } else if (rest[0] == '\0') {
        /*
         * The phase spec is a number.
         */
        phase = SearchPhaseByNumber (num);
    } else {
        phase = PH_final;
    }

    if (phase == PH_final) {
        CTIerror ("Illegal compiler phase specification in break option: \n"
                  "  -b %s\n"
                  "See sac2c -h for a list of legal break options.",
                  option);
    } else {
        global.break_after_phase = phase;
    }

    break_phase = MEMfree (break_phase);

    break_subphase = STRtok (NULL, ":");

    if (break_subphase != NULL) {
        subphase = SearchSubPhase (phase, break_subphase);

        if (subphase == PH_final) {
            CTIerror ("Illegal compiler subphase specification in break option:\n"
                      "  -b %s\n"
                      "See sac2c -h for a list of legal break options.",
                      option);
        } else {
            global.break_after_subphase = subphase;
        }

        break_subphase = MEMfree (break_subphase);

        break_cyclephase = STRtok (NULL, ":");

        if (break_cyclephase != NULL) {
            cyclephase = SearchCyclePhase (subphase, break_cyclephase);

            if (cyclephase == PH_final) {
                CTIerror ("Illegal compiler cycle phase specification in break option: \n"
                          "  -b %s\n"
                          "See sac2c -h for a list of legal break options.",
                          option);
            } else {
                global.break_after_cyclephase = cyclephase;
            }

            break_cyclephase = MEMfree (break_cyclephase);

            break_cyclepass = STRtok (NULL, ":");

            if (break_cyclepass != NULL) {
                num = strtol (break_cyclepass, &rest, 10);

                if ((rest[0] == '\0') && (num >= 1)) {
                    global.break_cycle_specifier = num;
                } else {
                    CTIerror (
                      "Illegal compiler cycle pass specification in break option: \n"
                      "  -b %s\n"
                      "See sac2c -h for a list of legal break options.",
                      option);
                }

                break_cyclepass = MEMfree (break_cyclepass);
            }
        }
    }

    DBUG_PRINT ("PH", ("break phase: %s", PHphaseText (global.break_after_phase)));
    DBUG_PRINT ("PH", ("break subphase: %s", PHphaseText (global.break_after_subphase)));
    DBUG_PRINT ("PH",
                ("break cycle phase: %s", PHphaseText (global.break_after_cyclephase)));
    DBUG_PRINT ("PH", ("break cycle pass: %d", global.break_cycle_specifier));

    DBUG_VOID_RETURN;
}

#else

void
PHinterpretBreakOption (char *option)
{
    int num;
    char *rest;

    DBUG_ENTER ("PHinterpreteBreakOption");

    num = strtol (option, &rest, 10);

    if ((rest != option) && (rest[0] != '\0')) {
        CTIerror ("Illegal argument for break option: -b %s", option);
    } else {

        /*
         * First, we check for compiler phase specifications.
         */

#define PHASEelement(it_element)                                                         \
    if (STReq (option, #it_element) || (num == (int)PH_##it_element)) {                  \
        global.break_after = PH_##it_element;                                            \
    } else

#include "phase_sac2c.mac"

#undef PHASEelement

        /*
         * Next, we check for compiler subphase specifications.
         */

#define SUBPHASEelement(it_element)                                                      \
    if (STReq (option, #it_element)) {                                                   \
        global.break_after_subphase = SUBPH_##it_element;                                \
    } else

#define CYCLEelement(it_element)                                                         \
    if (STReq (option, #it_element)) {                                                   \
        global.break_after_subphase = SUBPH_##it_element;                                \
    } else

#include "phase_sac2c.mac"

#undef SUBPHASEelement
#undef CYCLEelement

        /*
         * At last, we check for cycle optimisation specifications.
         */

#define CYCLEPHASEelement(it_element)                                                    \
    if (STReq (option, #it_element)) {                                                   \
        global.break_after_optincyc = OIC_##it_element;                                  \
    } else

#define CYCLEPHASEFUNelement(it_element)                                                 \
    if (STReq (option, #it_element)) {                                                   \
        global.break_after_optincyc = OIC_##it_element;                                  \
    } else

#include "phase_sac2c.mac"

#undef CYCLEPHASEelement
#undef CYCLEPHASEFUNelement

        {
            CTIerror ("Illegal argument for break option: -b %s", option);
        }
    }

    DBUG_VOID_RETURN;
}
#endif

node *
PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree, bool cond)
{
    static int phase_num = 0;

    DBUG_ENTER ("PHrunCompilerPhase");

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerPhase called with non N_module node");

    global.compiler_phase = phase;
    global.compiler_anyphase = phase;
    phase_num += 1;

#ifndef DBUG_OFF
    if ((global.my_dbug) && !global.my_dbug_active && (phase >= global.my_dbug_from)
        && (phase <= global.my_dbug_to)) {
        DBUG_PUSH (global.my_dbug_str);
        global.my_dbug_active = 1;
    }
#endif

    CTInote (" ");

    if (cond) {
        CTIstate ("** %2d: %s ...", phase_num, PHphaseText (phase));
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
        CTIstate ("** %2d: %s skipped.", phase_num, PHphaseText (phase));
    }

    if ((global.my_dbug) && (global.my_dbug_active) && (phase >= global.my_dbug_to)) {
        DBUG_POP ();
        global.my_dbug_active = 0;
    }

    if (global.break_after_phase == phase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCompilerSubPhase (compiler_phase_t subphase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ("PHrunCompilerSubPhase");

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerSubPhase called with non N_module node");

    global.compiler_subphase = subphase;
    global.compiler_anyphase = subphase;

    if (cond) {
        if (phase_type[subphase] != PHT_cycle) {
            CTInote ("**** %s ...", PHphaseText (subphase));
        }
        syntax_tree = phase_fun[subphase](syntax_tree);
        CTIabortOnError ();

#ifdef SHOW_MALLOC
        if (global.treecheck && (syntax_tree != NULL)) {
            syntax_tree = CHKdoTreeCheck (syntax_tree);
        }

        if (global.memcheck && (syntax_tree != NULL)) {
            syntax_tree = CHKMdoMemCheck (syntax_tree);
        }
#endif
    }

    if (global.break_after_subphase == subphase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCompilerCyclePhase (compiler_phase_t cyclephase, int pass, node *arg_node, bool cond,
                         bool funbased)
{
    DBUG_ENTER ("PHrunCompilerCyclePhase");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

    DBUG_ASSERT (((arg_node != NULL)
                  && ((funbased && (NODE_TYPE (arg_node) == N_fundef))
                      || (!funbased && (NODE_TYPE (arg_node) == N_module)))),
                 "PHrunCompilerCyclePhase called with wrong node type.");

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (pass < global.break_cycle_specifier))) {
        if (funbased) {
            CTItell (4, "         %s ...", PHphaseText (cyclephase));
        } else {
            CTInote ("****** %s ...", PHphaseText (cyclephase));
        }

        arg_node = phase_fun[cyclephase](arg_node);
        CTIabortOnError ();
    }

    DBUG_RETURN (arg_node);
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
