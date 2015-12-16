/** <!--********************************************************************-->
 * @file  supervisor.c
 *
 * @brief  This file contains the shared functionality between all different
 *         implementations of a dynamic specialization controller.
 *
 * @author tvd, hmw
 *
 *****************************************************************************/

#include "config.h"

#if ENABLE_RTSPEC

#include <stdlib.h>
#include <pthread.h>

#include "rtspec_modes.h"
#include "simple_controller.h"
#include "uuid_controller.h"

#define SAC_DO_TRACE 1
#include "sac.h"

#define SAC_RTC_ENV_VAR_NAME "SAC_RTSPEC_CONTROLLER"

/* TLS key to retrieve the Thread Self ID Ptr */
pthread_key_t SAC_RTSPEC_self_id_key;

/* The number of controller threads used for runtime specialization. */
unsigned int SAC_RTSPEC_controller_threads;

pthread_t *controller_threads;

static int do_trace;

static int rtspec_mode;

static char *cli_arguments;

/******************************************************************************
 *
 * function:
 *   void SAC_RTSPEC_SetupInitial( int argc, char *argv[],
 *                                 unsigned int num_threads)
 *
 * description:
 *  Parse the command line and determine the number of rtspec controller threads.
 *  Looks for the -rtc cmdline option, sets SAC_RTSPEC_controller_threads.
 *
 ******************************************************************************/
void
SAC_RTSPEC_SetupInitial (int argc, char *argv[], unsigned int num_threads, int trace,
                         int mode, char *command_line)
{
    do_trace = trace;
    rtspec_mode = mode;
    cli_arguments = command_line;

    int i;
    bool rtc_option_exists = FALSE;
    char *rtc_parallel = NULL;

    if (argv) {
        for (i = 1; i < argc - 1; i++) {
            if ((argv[i][0] == '-') && (argv[i][1] == 'r') && (argv[i][2] == 't')
                && (argv[i][3] == 'c') && (argv[i][4] == '\0')) {
                SAC_RTSPEC_controller_threads = atoi (argv[i + 1]);
                rtc_option_exists = TRUE;
                break;
            }
        }
    }
    if (!rtc_option_exists) {
        rtc_parallel = getenv (SAC_RTC_ENV_VAR_NAME);
        SAC_RTSPEC_controller_threads = (rtc_parallel != NULL) ? atoi (rtc_parallel) : 0;
    }

    if (SAC_RTSPEC_controller_threads == 0) {
        SAC_RTSPEC_controller_threads = num_threads;
    }

    if (SAC_RTSPEC_controller_threads <= 0) {
        SAC_RuntimeError (
          "Number of rtspec controller threads is unspecified or exceeds legal"
          " range (>0).\n"
          "    Use the '%s' environment variable or the option"
          " -rtc <num>' (which override the environment variable).",
          SAC_RTC_ENV_VAR_NAME);
    }

    if (do_trace == 1) {
        SAC_TR_Print ("Number of threads determined as %u.",
                      SAC_RTSPEC_controller_threads);
    }
}

/******************************************************************************
 *
 * function:
 *   unsigned int SAC_RTSPEC_CurrentThreadId(void)
 *
 * description:
 *
 *  Return the Thread ID of the current rtspec controller thread.
 *
 ******************************************************************************/
unsigned int
SAC_RTSPEC_CurrentThreadId (void)
{
    void *thread_id = pthread_getspecific (SAC_RTSPEC_self_id_key);

    if (thread_id == NULL) {
        return 0;
    } else {
        return *(unsigned int *)thread_id;
    }
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_setupController(void)
 *
 * @brief Initializes the request queue and starts the optimization controller
 *        in a separate thread.
 *
 ****************************************************************************/

void
SAC_setupController (char *dir)
{

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Setup controller.");
    }

    if (rtspec_mode == RTSPEC_MODE_SIMPLE) {
        SAC_Simple_setupController (dir, do_trace, cli_arguments);
    } else {
        SAC_UUID_setupController (dir, do_trace, cli_arguments);
    }

    int result;
    pthread_t controller_thread;
    result = 0;
    unsigned int rtspec_thread_ids[SAC_HM_RTSPEC_THREADS ()];

    for (unsigned int i = 0; i < SAC_HM_RTSPEC_THREADS (); i++) {
        rtspec_thread_ids[i] = SAC_MT_GLOBAL_THREADS () + i;
    }

    if (0 != pthread_key_create (&SAC_RTSPEC_self_id_key, NULL)) {
        SAC_RuntimeError (
          "Unable to create thread specific data key (SAC_RTSPEC_self_id_key).");
    }

    controller_threads = malloc (sizeof (pthread_t) * SAC_HM_RTSPEC_THREADS ());

    for (unsigned int i = 0; i < SAC_HM_RTSPEC_THREADS (); i++) {
        if (rtspec_mode == RTSPEC_MODE_SIMPLE) {
            result = pthread_create (&controller_thread, NULL, SAC_Simple_runController,
                                     &rtspec_thread_ids[i]);
        } else {
            result = pthread_create (&controller_thread, NULL, SAC_UUID_runController,
                                     &rtspec_thread_ids[i]);
        }

        if (result != 0) {
            SAC_RuntimeError ("Runtime specialization controller could not be launched");
        }

        controller_threads[i] = controller_thread;
    }
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_finalizeController (void)
 *
 * @brief  Kills the optimization controller.
 *
 ****************************************************************************/

void
SAC_finalizeController (void)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Finalize controller!");
    }

    if (rtspec_mode == RTSPEC_MODE_SIMPLE) {
        SAC_Simple_stopController ();
    } else {
        SAC_UUID_stopController ();
    }

    for (unsigned int i = 0; i < SAC_HM_RTSPEC_THREADS (); i++) {
        if (rtspec_mode == RTSPEC_MODE_SIMPLE) {
            pthread_cond_broadcast (&simple_empty_queue_cond);
        } else {
            pthread_cond_broadcast (&uuid_empty_queue_cond);
        }
        pthread_join (controller_threads[i], NULL);
    }

    free (controller_threads);

    if (rtspec_mode == RTSPEC_MODE_SIMPLE) {
        SAC_Simple_finalizeController ();
    } else {
        SAC_UUID_finalizeController ();
    }
}

#endif /* ENABLE_RTSPEC */
