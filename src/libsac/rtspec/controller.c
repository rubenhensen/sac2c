/** <!--********************************************************************-->
 * @file  controller.c
 *
 * @brief  This file contains the implementation of the central dynamic
 * optimization controller.
 *
 *****************************************************************************/

#include "config.h"

#if ENABLE_RTSPEC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <string.h>

#include "controller.h"
#include "reqqueue.h"
#include "registry.h"

#define SAC_DO_TRACE 1
#include "sac.h"

#define TMP_DIR_NAME_PREFIX "SACrt_"
#define MAX_SYS_CALL 512
#define MAX_STRING_LENGTH 256

#define SAC_RTC_ENV_VAR_NAME "SAC_RTSPEC_CONTROLLER"

static int running = 1;
static int do_trace;
static char *tmpdir_name = NULL;
static char *rtspec_syscall = NULL;
static list_t *processed;

/* TLS key to retrieve the Thread Self ID Ptr */
pthread_key_t SAC_RTSPEC_self_id_key;

/* The number of controller threads used for runtime specialization. */
unsigned int SAC_RTSPEC_controller_threads;

/** <!--*******************************************************************-->
 *
 * @fn CreateTmpDir( char *dir)
 *
 * @brief creates a uniquely named tmp directory in the given directory
 *
 ****************************************************************************/

#ifdef HAVE_MKDTEMP
/* mkdtemp is safer than tempnam and recommended */
/* on linux/bsd platforms.                       */

static char *
CreateTmpDir (char *dir)
{
    tmpdir_name = (char *)malloc (sizeof (char) * (strlen (dir) + 16));

    strcpy (tmpdir_name, dir);
    strcat (tmpdir_name, "/" TMP_DIR_NAME_PREFIX "XXXXXX");

    tmpdir_name = mkdtemp (tmpdir_name);

    return (tmpdir_name);
}

#else /* HAVE_MKDTEMP */

/* the old way for platforms not */
/* supporting mkdtemp            */

static char *
CreateTmpDir (char *dir)
{
    tmpdir_name = tempnam (dir, TMP_DIR_NAME_PREFIX);

    if (tmpdir_name != NULL) {
        rtspec_syscall = (char *)malloc (sizeof (char) * (strlen (tmpdir_name) + 16));
        strcpy (rtspec_syscall, "mkdir ");
        strcat (rtspec_syscall, tmpdir_name);

        system (rtspec_syscall);
    }

    return (tmpdir_name);
}

#endif /* HAVE_MKDTEMP */

/******************************************************************************
 *
 * function:
 *   void SAC_RTSPEC_SetupInitial( int argc, char *argv[],
 *                                    unsigned int num_threads)
 *
 * description:
 *  Parse the command line and determine the number of rtspec controller threads.
 *  Looks for the -rtc cmdline option, sets SAC_RTSPEC_controller_threads.
 *
 ******************************************************************************/
void
SAC_RTSPEC_SetupInitial (int argc, char *argv[], unsigned int num_threads, int trace)
{
    do_trace = trace;

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
        SAC_TR_PRINT (
          ("Number of threads determined as %u.", SAC_RTSPEC_controller_threads));
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

    pthread_t controller_thread;
    int result;
    char *tmpdir_name;

    result = 0;
    unsigned int rtspeac_thread_ids[SAC_HM_RTSPEC_THREADS ()];

    for (unsigned int i = 0; i < SAC_HM_RTSPEC_THREADS (); i++) {
        rtspeac_thread_ids[i] = SAC_MT_GLOBAL_THREADS () + i;
    }

    if (0 != pthread_key_create (&SAC_RTSPEC_self_id_key, NULL)) {
        SAC_RuntimeError (
          "Unable to create thread specific data key (SAC_RTSPEC_self_id_key).");
    }

    SAC_initializeQueue (do_trace);

    tmpdir_name = CreateTmpDir (dir);

    if (tmpdir_name == NULL) {
        SAC_RuntimeError ("Unable to create tmp directory for specialization controller");
    } else if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Setup specialization repository in: ");
        SAC_TR_Print (tmpdir_name);
    }

    for (unsigned int i = 0; i < SAC_HM_RTSPEC_THREADS (); i++) {
        result = pthread_create (&controller_thread, NULL, SAC_runController,
                                 &rtspeac_thread_ids[i]);

        if (result != 0) {
            SAC_RuntimeError ("Runtime specialization controller could not be launched");
        }
    }

    processed = (list_t *)malloc (sizeof (list_t));
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_runController (void)
 *
 * @brief  Controls all the aspects of dynamically optimizing functions.
 *
 * This function runs as long as the variable 'running' is set to TRUE (1). It
 * dequeues requests from the global request queue and handles them. If the
 * queue is empty this function goes to sleep and waits for a signal from the
 * request queue.
 *
 * This function may be stopped by calling 'SAC_finalizeController()'.
 *
 ****************************************************************************/
void *
SAC_runController (void *param)
{
    pthread_setspecific (SAC_RTSPEC_self_id_key, param);

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Starting controller.");
    }

    queue_node_t *current;

    while (running) {
        pthread_mutex_lock (&empty_queue_mutex);

        /* If the queue is empty and we're still running the controller, wait
         * for requests.
         */
        while (request_queue->size == 0 && running) {
            pthread_cond_wait (&empty_queue_cond, &empty_queue_mutex);
        }

        pthread_mutex_unlock (&empty_queue_mutex);

        current = SAC_dequeueRequest ();

        SAC_handleRequest (current);
    }

    pthread_exit (NULL);
    return NULL;
}

/** <!--*******************************************************************-->
 *
 * @fn getNewFunctionName( char *func, char *result)
 *
 * @brief Creates a new random function name used for the newly created wrapper
 * instance.
 *
 * @param func  The orignal function name.
 * @param result  The resulting function name.
 *
 ****************************************************************************/
static void
getNewFunctionName (char *func, char *result)
{
    static char *format = "RTSPECf_%s_%d";
    static int counter = 0;

    sprintf (result, format, func, counter++);
}

/** <!--*******************************************************************-->
 *
 * @fn getNewModuleName( char *mod, char *result)
 *
 * @brief Creates a new random module name used by the compiler for the new
 * module.
 *
 * @param mod  The original module name.
 * @param result  The resulting module name.
 *
 ****************************************************************************/
static void
getNewModuleName (char *result)
{
    static char *format = "RTSPECmod_%d";
    static int counter = 0;

    sprintf (result, format, counter++);
}

/** <!--*******************************************************************-->
 *
 * @fn encodeShapes( int *shapes, char *result)
 *
 * @brief  Creates a string representation of the shape information stored in
 * the integer array 'shapes'.
 *
 * The format of both the shape array and the string representation is as
 * follows:
 *
 * {
 * number_of_arguments
 *  dimension_arg_1
 *    extent_dim_1
 *    ...
 *    extent_dim_n
 *  dimension_arg_2
 *   ...
 *  dimension_arg_n
 * }
 *
 * @param shapes  The array of shape information.
 * @param result  The resulting string representation.
 ****************************************************************************/
static void
encodeShapes (int *shapes, char *result)
{
    int num_args, i, j, k;

    char current[15];

    if (shapes == NULL) {
        fprintf (stderr, "ERROR -- \t Missing shape information!");
        return;
    }

    num_args = shapes[0];
    sprintf (result, "%d-", num_args);

    i = 1;
    k = 1;
    for (; i <= num_args; i++) {
        if (shapes[k] > 0) {
            j = 0;
            for (; j <= shapes[k]; j++) {
                sprintf (current, "%d-", shapes[k]);
                strcat (result, current);
                k++;
            }
        } else {
            sprintf (current, "%d-", shapes[k]);
            strcat (result, current);
            k++;
        }
    }
}

/** <!--*******************************************************************-->
 *
 * @fn  wasProcessed( char *module, char *function, char *shape_info)
 *
 * @brief Iterates over all the nodes in the list of processed requests and
 * returns TRUE if a node contains the same information as the current request
 * and FALSE otherwise.
 *
 * @param  module  The module for which the request was made.
 * @param  function  The function for which the request was made.
 * @param  shape_info  The encoded shapes that were part of the request.
 *
 * @return  TRUE if the request was allready handled, FALSE otherwise.
 *
 ****************************************************************************/
static int
wasProcessed (char *function, char *shape_info)
{
    char compare[256];
    list_t *current;

    if (strlen (processed->request) == 0) {
        return 0;
    }

    sprintf (compare, "%s_%s", function, shape_info);

    current = processed;
    while (current != NULL) {
        if (strlen (current->request) != 0) {
            if (strcmp (current->request, compare) == 0) {
                return 1;
            }
        } else {
            return 0;
        }

        current = current->next;
    }

    return 0;
}

/** <!--*******************************************************************-->
 *
 * @fn  addProcessed( char *module, char *function, char *shape_info)
 *
 * @brief Adds a new node to the list of processed requests.
 *
 * @param  module  The module for which the request was made.
 * @param  function  The function for which the request was made.
 * @param  shape_info  The encoded shapes that were part of the request.
 *
 ****************************************************************************/
static void
addProcessed (char *function, char *shape_info)
{
    list_t *xnew;
    char request[256];

    sprintf (request, "%s_%s", function, shape_info);

    xnew = (list_t *)malloc (sizeof (list_t));

    if (xnew == NULL) {
        fprintf (stderr, "Could not allocate new processed request node!");

        exit (EXIT_FAILURE);
    }

    strcpy (xnew->request, request);

    /* Add the new processed request at the beginning of the list, this is
     * cheaper than at the end.
     */
    if (strlen (processed->request) != 0) {
        xnew->next = processed;
    }

    processed = xnew;
}

/** <!--*******************************************************************-->
 *
 * @fn  handle_request (queue_node *request)
 *
 * @brief  Handles a single request.
 *
 * This function starts the compiler and passes it the AST stored in the request
 * object. This function returns when the compiler is finished.
 *
 * @param  request  The request being handled.
 ****************************************************************************/
void
SAC_handleRequest (queue_node_t *request)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Handling new specialization request.");
    }

    static char *call_format = "sac2c -v%i -runtime "
                               "-rt_old_mod %s -rt_new_mod %s "
                               "-rtfunname %s -rtnewname %s "
                               "-rttypeinfo %s -rtshapeinfo %s "
                               "-L %s -o %s";

    char syscall[MAX_SYS_CALL] = "";
    char filename[MAX_STRING_LENGTH] = "";
    char new_func_name[MAX_STRING_LENGTH] = "";
    char new_module[MAX_STRING_LENGTH] = "";
    char shape_info[MAX_STRING_LENGTH] = "";

    if (request->shape_info == NULL) {
        // fprintf(stderr, "Could not optimize as shape information is missing for "
        //        "function %s!", request->func_name);
        return;
    }

    /*
     * Get a new random module and function name and encode the shapes of the
     * arguments.
     */
    getNewFunctionName (request->func_name, new_func_name);
    getNewModuleName (new_module);
    encodeShapes (request->shape_info, shape_info);

    /*
     * Only process requests that haven't been processed yet.
     */
    if (wasProcessed (request->func_name, shape_info)) {
        return;
    }

    /* Build the system call. */
    sprintf (syscall, call_format, (do_trace == 1) ? 3 : 0, request->module_name,
             new_module, request->func_name, new_func_name, request->type_info,
             shape_info, tmpdir_name, tmpdir_name);

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Calling runtime compiler with:");
        SAC_TR_Print (syscall);
    }

    /* The path to the new library. */
    sprintf (filename, "%s/lib%sMod.so", tmpdir_name, new_module);

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Generating specialized library at:");
        SAC_TR_Print (filename);
    }

    /* Execute the system call and act according to the return value. */
    switch (system (syscall)) {
    default:
        fprintf (stderr, "ERROR -- \t [RTSpec Controller: "
                         "handle_request()] Compilation failed!\n");

        exit (EXIT_FAILURE);
        break;

    case -1:
        fprintf (stderr, "ERROR -- \t [RTSpec Controller: "
                         "handle_request()] System call failed!\n");

        exit (EXIT_FAILURE);

    case 0:
        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Linking with generated library.");
        }
        /* Dynamically link with the new libary. */
        request->reg_obj->dl_handle = dlopen (filename, RTLD_NOW | RTLD_GLOBAL);

        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Check handle not being NULL.");
        }

        /* Exit on failure. */
        if (request->reg_obj->dl_handle == NULL) {
            fprintf (stderr, "ERROR -- \t %s\n", dlerror ());

            exit (EXIT_FAILURE);
        }

        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Check linking error.");
        }

        dlerror ();

        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Load symbols for new wrapper.");
        }
        /* Load the symbol for the new wrapper. */
        request->reg_obj->func_ptr = dlsym (request->reg_obj->dl_handle, new_func_name);

        strcpy (request->reg_obj->module, new_module);

        /* Exit on failure. */
        if (request->reg_obj->func_ptr == NULL) {
            fprintf (stderr, "ERROR -- \t Could not load symbol!\n");

            exit (EXIT_FAILURE);
        }
    }

    /*
     * Add this request to the list of processed requests.
     */
    addProcessed (request->func_name, shape_info);
}

/** <!--*******************************************************************-->
 *
 * @fn  exit_controller (void)
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

    int success;

    if (tmpdir_name != NULL) {
        if (rtspec_syscall == NULL) {
            rtspec_syscall = (char *)malloc (sizeof (char) * (strlen (tmpdir_name) + 16));
        }

        strcpy (rtspec_syscall, "rm -rf  ");
        strcat (rtspec_syscall, tmpdir_name);

        success = system (rtspec_syscall);

        free (rtspec_syscall);
    }

    running = 0;
}

#endif /* ENABLE_RTSPEC */
