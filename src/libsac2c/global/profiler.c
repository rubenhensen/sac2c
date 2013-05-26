
#include "config.h"
#ifdef HAVE_GETTIME

#define DBUG_PREFIX "TIME"
#include "tree_basic.h"
#include "types.h"
#include "ctinfo.h"
#include "debug.h"
#include "LookUpTable.h"
#include <string.h>
#include "globals.h"
#include "phase_info.h"
#include "memory.h"
#include "profiler.h"
#include <time.h>

static void CreateReport (timeinfo_t *);
static void *WrapCreateReport (void *, void *);
static void *OrderedReport (void *, void *);

static lut_t *timetable = 0;
static FILE *reportfile = 0;
static const char *phasename[]
  = {"Dummy", "Phase", "Subphase", "Cycle", "Cycle_fun", "Cyclephase", "Cyclephase_fun"};

bool
TIMEshouldTime (void)
{
    phase_type_t phasetype = PHIphaseType (global.compiler_anyphase);

    if (!global.timefreq) {
        return FALSE;
    }
    if (global.timefreq == 1 && phasetype == PHT_phase) {
        return TRUE;
    }
    if (global.timefreq == 2
        && (phasetype == PHT_subphase || phasetype == PHT_cycle
            || phasetype == PHT_phase)) {
        return TRUE;
    }
    if (global.timefreq == 3 && (phasetype == PHT_cycle)) {
        return TRUE;
    }
    if (global.timefreq == 4
        && (phasetype == PHT_cyclephase || phasetype == PHT_cyclephase_fun
            || phasetype == PHT_cycle)) {
        return TRUE;
    }
    if (global.timefreq == 5
        && (phasetype == PHT_cyclephase || phasetype == PHT_cyclephase_fun
            || phasetype == PHT_cycle)) {
        return TRUE;
    }
    if (global.timefreq == 6
        && (phasetype == PHT_cyclephase || phasetype == PHT_cyclephase_fun
            || phasetype == PHT_cycle || phasetype == PHT_phase
            || phasetype == PHT_subphase)) {
        return TRUE;
    }
    return FALSE;
}

void
TIMEbegin (compiler_phase_t phase)
{
    timeinfo_t *phasetime = 0;

    if (!TIMEshouldTime ()) {
        return;
    }

    if (timetable == 0) {
        timetable = LUTgenerateLut ();
    }

    if (!(phasetime = (void *)LUTsearchInLutP (timetable, (void *)phase))) {
        phasetime = MEMmalloc (sizeof (timeinfo_t));
        memset (phasetime, 0, sizeof (timeinfo_t));

        phasetime->phase = phase;
        timetable = LUTinsertIntoLutP (timetable, (void *)phasetime->phase, phasetime);
    } else {
        phasetime = *((void **)phasetime);
    }

    if (global.timefreq == 4 && PHIphaseType (phase) == PHT_cycle) {
        CreateReport (phasetime);
    }
    phasetime->timestraversed++;
    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &phasetime->timer);
}

void
TIMEend (compiler_phase_t phase)
{
    timeinfo_t *phasetime = 0;
    struct timespec preusage;

    if (!(phasetime = (void *)LUTsearchInLutP (timetable, (void *)phase))) {
        return;
    } else {
        phasetime = *((void **)phasetime);
    }

    preusage.tv_sec = phasetime->timer.tv_sec;
    preusage.tv_nsec = phasetime->timer.tv_nsec;
    clock_gettime (CLOCK_PROCESS_CPUTIME_ID, &phasetime->timer);
    if ((phasetime->timer.tv_nsec - preusage.tv_nsec) < 0) {
        phasetime->timer.tv_sec -= (preusage.tv_sec + 1);
        phasetime->timer.tv_nsec += 1000000000 - preusage.tv_nsec;
    } else {
        phasetime->timer.tv_sec -= preusage.tv_sec;
        phasetime->timer.tv_nsec -= preusage.tv_nsec;
    }

    phasetime->duration.tv_sec += phasetime->timer.tv_sec;
    phasetime->duration.tv_nsec += phasetime->timer.tv_nsec;
    if (phasetime->duration.tv_nsec >= 1000000000) {
        phasetime->duration.tv_nsec -= 1000000000;
        phasetime->duration.tv_sec++;
    }

    if (global.timefreq == 4 && PHIphaseType (phase) != PHT_cycle) {
        CreateReport (phasetime);
    }
    if (global.timefreq == 3) {
        CreateReport (phasetime);
        memset (&phasetime->duration, 0, sizeof (struct timespec));
    }
}

static void
CreateReport (timeinfo_t *phasetime)
{
    char *name = 0;

    if (reportfile == NULL) {
        name = MEMmalloc ((strlen (global.outfilename) + 8) * sizeof (char));
        sprintf (name, "%s.treport", global.outfilename);
        reportfile = fopen (name, "w");
    }

    if (global.timefreq == 3 && PHIphaseType (phasetime->phase) == PHT_cycle) {
        fprintf (reportfile, "\n******** Cycle: %s\n", PHIphaseText (phasetime->phase));
        fprintf (reportfile, "******** Pass: %d\n", global.cycle_counter);
    }
    if (global.timefreq == 2 && PHIphaseType (phasetime->phase) == PHT_phase) {
        fprintf (reportfile, "\n******** Phase: %s\n", PHIphaseText (phasetime->phase));
    } else if (global.timefreq == 4 && PHIphaseType (phasetime->phase) == PHT_cycle) {
        fprintf (reportfile, "\n******** Cycle: %s\n", PHIphaseText (phasetime->phase));
        fprintf (reportfile, "******** Pass: %d\n", global.cycle_counter);
    } else if (global.timefreq == 5 && PHIphaseType (phasetime->phase) == PHT_cycle) {
        fprintf (reportfile, "\n******** Cycle: %s\n", PHIphaseText (phasetime->phase));
    } else {
        fprintf (reportfile, "\n");
        fprintf (reportfile, "  Name: %s\n", PHIphaseText (phasetime->phase));
        fprintf (reportfile, "  Time: %d.%.3d seconds\n", (int)phasetime->duration.tv_sec,
                 ((int)phasetime->duration.tv_nsec) / 1000000);
        fprintf (reportfile, "  Phasetype: %s\n",
                 phasename[PHIphaseType (phasetime->phase)]);
        fprintf (reportfile, "  Ident: %s\n", PHIphaseIdent (phasetime->phase));
        if (global.timefreq == 5) {
            fprintf (reportfile, "  Times traversed: %d\n",
                     PHIphaseType (phasetime->timestraversed));
        }
    }
}

static void *
WrapCreateReport (void *head, void *node)
{
    if (node != NULL) {
        CreateReport ((timeinfo_t *)node);
    }
    return NULL;
}

static void *
OrderedReport (void *head, void *phasetime)
{
    timeinfo_t *iterator = (timeinfo_t *)head;
    timeinfo_t *prev = 0;

    if (head == 0) {
        return phasetime;
    }

    while (iterator) {
        if (iterator->duration.tv_sec < ((timeinfo_t *)phasetime)->duration.tv_sec
            || (iterator->duration.tv_sec == ((timeinfo_t *)phasetime)->duration.tv_sec
                && iterator->duration.tv_nsec
                     < ((timeinfo_t *)phasetime)->duration.tv_nsec)) {
            break;
        }
        prev = iterator;
        iterator = iterator->next;
    }
    if (!prev) {
        head = phasetime;
    } else {
        prev->next = (timeinfo_t *)phasetime;
    }
    ((timeinfo_t *)phasetime)->next = iterator;

    return head;
}

node *
TIMEtimeReport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    timeinfo_t *head = 0;

    if (global.timefreq == 6) {
        head = (timeinfo_t *)LUTfoldLutP (timetable, head, OrderedReport);
        while (head) {
            CreateReport (head);
            head = head->next;
        }
    } else if (global.timefreq != 4 && global.timefreq != 3) {
        LUTfoldLutP (timetable, 0, WrapCreateReport);
    }

    DBUG_RETURN (arg_node);
}

#endif /* HAVE_GETTIME */
