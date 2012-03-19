/*
 * $Id$
 */

/*****************************************************************************
 *
 * This file contains functions to interpret command line options related
 * to the phase mechanism, in particular the break option -b and the dbug
 * option -#.
 *
 *****************************************************************************/

#include "phase_options.h"

#include "types.h"

#define DBUG_PREFIX "PHO"
#include "debug.h"

#include "str.h"
#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "phase_info.h"

static compiler_phase_t
SearchPhaseByName (char *name)
{
    compiler_phase_t phase;

    DBUG_ENTER ();

    phase = PHIfirstPhase ();

    do {
        phase++;
    } while (
      (phase < PHIlastPhase ())
      && ((PHIphaseType (phase) != PHT_phase) || !STReq (PHIphaseName (phase), name)));

    DBUG_RETURN (phase);
}

static compiler_phase_t
SearchPhaseByNumber (int num)
{
    compiler_phase_t phase;
    int cnt;

    DBUG_ENTER ();

    phase = PHIfirstPhase ();
    cnt = 0;

    do {
        phase++;
        if (PHIphaseType (phase) == PHT_phase)
            cnt++;
    } while ((phase < PHIlastPhase ()) && (cnt < num));

    DBUG_RETURN (phase);
}

static compiler_phase_t
SearchSubPhase (compiler_phase_t phase, char *name)
{
    compiler_phase_t subphase;

    DBUG_ENTER ();

    subphase = phase;

    do {
        do {
            subphase++;
        } while (PHIphaseType (subphase) > PHT_cycle);
    } while ((PHIphaseParent (subphase) == phase)
             && !STReq (PHIphaseName (subphase), name));

    if (PHIphaseParent (subphase) != phase) {
        subphase = PHIlastPhase ();
    }

    DBUG_RETURN (subphase);
}

static compiler_phase_t
SearchCyclePhase (compiler_phase_t cycle, char *name)
{
    compiler_phase_t cyclephase;

    DBUG_ENTER ();

    cyclephase = cycle;

    do {
        cyclephase++;
    } while ((PHIphaseParent (cyclephase) == cycle)
             && !STReq (PHIphaseName (cyclephase), name));

    if (PHIphaseParent (cyclephase) != cycle) {
        cyclephase = PHIlastPhase ();
    }

    DBUG_RETURN (cyclephase);
}

void
PHOinterpretBreakOption (char *option)
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

    DBUG_ENTER ();

    DBUG_PRINT ("Interpreting break option: %s", option);

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
        phase = PHIlastPhase ();
    }

    if (phase == PHIlastPhase ()) {
        CTIerror ("Illegal compiler phase specification in break option: \n"
                  "  -b %s\n"
                  "See %s -h for a list of legal break options.",
                  option, global.toolname);
    } else {
        global.break_after_phase = phase;
    }

    break_phase = MEMfree (break_phase);

    break_subphase = STRtok (NULL, ":");

    if (break_subphase != NULL) {
        subphase = SearchSubPhase (phase, break_subphase);

        if (subphase == PHIlastPhase ()) {
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

            if (cyclephase == PHIlastPhase ()) {
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

    if (break_subphase)
        break_subphase = MEMfree (break_subphase);

    DBUG_RETURN ();
}

#ifndef DBUG_OFF

static compiler_phase_t
SearchPhaseIdent (char *ident)
{
    compiler_phase_t phase;

    DBUG_ENTER ();

    phase = PHIfirstPhase ();

    do {
        phase++;
    } while ((phase < PHIlastPhase ()) && !STReq (PHIphaseIdent (phase), ident));

    DBUG_RETURN (phase);
}

void
PHOinterpretDbugOption (char *option)
{
    char *tok;
    compiler_phase_t phase;

    DBUG_ENTER ();

    tok = STRtok (option, "/");

    DBUG_ASSERT (tok != NULL, "Corruption in dbug option");

    if (tok[0] != '\0') {
        phase = SearchPhaseIdent (tok);

        if (phase == PHIlastPhase ()) {
            CTIerror ("Illegal start compiler phase specification in dbug option: \n"
                      "  -# %s\n"
                      "See %s -h for a list of legal break options.",
                      option, global.toolname);
        } else {
            global.my_dbug_from = phase;
        }
    }

    tok = MEMfree (tok);

    tok = STRtok (NULL, "/");

    if (tok == NULL) {
        CTIerror ("Missing stop compiler phase specification in dbug option: \n"
                  "  -# %s\n"
                  "See %s -h for a list of legal break options.",
                  option, global.toolname);
    } else {
        if (tok[0] != '\0') {
            phase = SearchPhaseIdent (tok);

            if (phase == PHIlastPhase ()) {
                CTIerror ("Illegal start compiler phase specification in dbug option: \n"
                          "  -# %s\n"
                          "See %s -h for a list of legal break options.",
                          option, global.toolname);
            } else if (phase < global.my_dbug_from) {
                CTIerror ("Stop phase is before start phase in dbug option: \n"
                          "  -# %s\n"
                          "See %s -h for sequence of phases.",
                          option, global.toolname);
            } else {
                global.my_dbug_to = phase;
            }
        }

        tok = MEMfree (tok);

        tok = STRtok (NULL, "/");

        if (tok == NULL) {
            CTIerror ("Missing dbug string in dbug option: \n"
                      "  -# %s\n"
                      "See %s -h for syntac of dbug option.",
                      option, global.toolname);
        } else {
            global.my_dbug_str = tok;
            global.my_dbug = 1;
        }
    }

    DBUG_RETURN ();
}

#endif /* DBUG_OFF */

#define PHASEname(name)                                                                  \
    cnt += 1;                                                                            \
    printf ("\n    %-3s | %-2d", #name, cnt);

#define PHASEtext(text) printf (" : " text "\n");

#define SUBPHASEname(name) printf ("      %-8s", #name);

#define SUBPHASEtext(text) printf (" : " text "\n");

#define CYCLEname(name) printf ("      %-8s", #name);

#define CYCLEtext(text) printf (" : " text "\n");

#define CYCLEPHASEname(name) printf ("        %-8s", #name);

#define CYCLEPHASEtext(text) printf (" : " text "\n");

#define CYCLEPHASEFUNname(name) printf ("        %-8s", #name);

#define CYCLEPHASEFUNtext(text) printf (" : " text " (fun based)\n");

void
PHOprintPhasesSac2c (void)
{
    int cnt = 0;

    DBUG_ENTER ();

#include "phase_sac2c.mac"

    DBUG_RETURN ();
}

void
PHOprintPhasesSac4c (void)
{
    int cnt = 0;

    DBUG_ENTER ();

#include "phase_sac4c.mac"

    DBUG_RETURN ();
}

#undef PHASname
#undef PHASEtext
#undef SUBPHASEname
#undef SUBPHASEtext
#undef CYCLEname
#undef CYCLEtext
#undef CYCLEPHASEname
#undef CYCLEPHASEtext
#undef CYCLEPHASEFUNname
#undef CYCLEPHASEFUNtext

#undef DBUG_PREFIX
