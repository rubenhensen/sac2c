/** <!--********************************************************************-->
 * @file  simple_controller.c
 *
 * @brief  This file contains the implementation of the central dynamic
 * optimization controller.
 *
 *****************************************************************************/

#include "config.h"

#if SAC_DO_RTSPEC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <string.h>

#include "simple_controller.h"
#include "simple_reqqueue.h"
#include "registry.h"
#include "persistence.h"
#include "trace.h"

#define SAC_DO_TRACE 1
#include "sac.h"

#define TMP_DIR_NAME_PREFIX "SACrt_"
#define RTSPEC_MODULE_PREFIX "RTSpec_"
#define RTSPEC_MODULE_PREFIX_LENGTH 7
#define MAX_INT_DIGITS 21
#define CALL_FORMAT_STRLEN 141

#define SAC_RTC_ENV_VAR_NAME "SAC_RTSPEC_CONTROLLER"

static int running = 1;
static int do_trace;
static char *tmpdir_name = NULL;
static char *rtspec_syscall = NULL;
static int tmpdir_strlen = 0;
static char *cli_arguments;
static char *executable_name;
static int sbi_strlen = 0;
static int target_env_strlen = 0;
static int modext_strlen = 0;
static int cli_arguments_strlen = 0;
static int executable_name_strlen = 0;

/* TLS key to retrieve the Thread Self ID Ptr */
pthread_key_t SAC_RTSPEC_self_id_key;

/* The number of controller threads used for runtime specialization. */
unsigned int SAC_RTSPEC_controller_threads;

pthread_t *controller_threads;

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
    tmpdir_strlen = strlen (dir) + 16;
    tmpdir_name = (char *)malloc (sizeof (char) * tmpdir_strlen);

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

/** <!--*******************************************************************-->
 *
 * @fn SAC_Simple_setupController( char *dir, int trace, char *command_line, char
 **binary_name )
 *
 * @brief Initializes the request queue and starts the optimization controller
 *        in a separate thread.
 *
 ****************************************************************************/

void
SAC_Simple_setupController (char *dir, int trace, char *command_line, char *binary_name)
{
    do_trace = trace;
    cli_arguments = command_line;
    executable_name = binary_name;

    cli_arguments_strlen = strlen (cli_arguments);
    executable_name_strlen = strlen (executable_name);
    sbi_strlen = strlen (SAC_SBI_STRING);
    target_env_strlen = strlen (SAC_TARGET_ENV_STRING);
    modext_strlen = strlen (SAC_MODEXT_STRING);

    SAC_RTSPEC_TR_Print ("Runtime specialization: Setup simple controller.");

    char *tmpdir_name;

    SAC_Simple_initializeQueue (do_trace);

    tmpdir_name = CreateTmpDir (dir);

    if (tmpdir_name == NULL) {
        SAC_RuntimeError ("Unable to create tmp directory for specialization controller");
    } else {
        SAC_RTSPEC_TR_Print (
          "Runtime specialization: Setup specialization repository in:\n%s", tmpdir_name);
    }
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_Simple_runController (void)
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
SAC_Simple_runController (void *param)
{
    pthread_setspecific (SAC_RTSPEC_self_id_key, param);

    SAC_RTSPEC_TR_Print ("Runtime specialization: Starting controller.");

    simple_queue_node_t *current;

    while (running) {
        pthread_mutex_lock (&simple_empty_queue_mutex);

        /* If the queue is empty and we're still running the controller, wait
         * for requests.
         */
        while (simple_request_queue->size == 0 && running) {
            pthread_cond_wait (&simple_empty_queue_cond, &simple_empty_queue_mutex);
        }

        pthread_mutex_unlock (&simple_empty_queue_mutex);

        if (running) {
            current = SAC_Simple_dequeueRequest ();

            SAC_Simple_handleRequest (current);
        }
    }

    SAC_RTSPEC_TR_Print ("Runtime specialization: Exiting controller.");

    pthread_exit (NULL);
    return NULL;
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_Simple_handle_request (queue_node *request)
 *
 * @brief  Handles a single request.
 *
 * This function starts the compiler and passes it the AST stored in the request
 * object. This function returns when the compiler is finished.
 *
 * @param  request  The request being handled.
 ****************************************************************************/
void
SAC_Simple_handleRequest (simple_queue_node_t *request)
{
    SAC_RTSPEC_TR_Print ("Runtime specialization: Handling new specialization request.");

    static char call_format[CALL_FORMAT_STRLEN] = "%s -v%i -runtime "
                                                  "-rt_old_mod %s -rt_new_mod %s "
                                                  "-rtfunname %s -rtnewname %s "
                                                  "-rttypeinfo %s -rtshapeinfo %s "
                                                  "-L ./%s/%s/ "
                                                  "-T ./tree/%s/ -o %s %s";

    static int counter = 0;

    if (request->shape_info == NULL) {
        free (request);
        // fprintf(stderr, "Could not optimize as shape information is missing for "
        //        "function %s!", request->func_name);
        return;
    }

    /*
     * Only process requests that haven't been processed yet.
     */
    if (SAC_Simple_wasProcessed (request)) {
        free (request);
        return;
    }

    /*
     * Encode the shapes of the arguments.
     */
    char *shape_info = encodeShapes (request->shape_info);

    /*
     * Get a new module name so we don't have name clashes of the generated library files.
     */
    int old_module_strlen = strlen (request->reg_obj->module);
    int new_module_strlen
      = old_module_strlen + (RTSPEC_MODULE_PREFIX_LENGTH + MAX_INT_DIGITS + 1);
    char *new_module = (char *)malloc (sizeof (char) * new_module_strlen);

    sprintf (new_module, "%s%s_%d", RTSPEC_MODULE_PREFIX, request->reg_obj->module,
             counter++);

    char *syscall = (char *)malloc (
      sizeof (char)
      * (strlen (request->func_name) * 2 + strlen (request->type_info)
         + strlen (shape_info) + target_env_strlen * 2 + sbi_strlen
         + executable_name_strlen + cli_arguments_strlen + CALL_FORMAT_STRLEN
         + old_module_strlen + new_module_strlen + tmpdir_strlen * 2 + 1));

    /* Build the system call. */
    sprintf (syscall, call_format, executable_name, (do_trace == 1) ? 3 : 0,
             request->reg_obj->module, new_module, request->func_name, request->func_name,
             request->type_info, shape_info, SAC_TARGET_ENV_STRING, SAC_SBI_STRING,
             SAC_TARGET_ENV_STRING, tmpdir_name, cli_arguments);

    SAC_RTSPEC_TR_Print ("Runtime specialization: Calling runtime compiler with:\n%s",
                         syscall);

    char *filename
      = (char *)malloc (sizeof (char)
                        * (tmpdir_strlen + new_module_strlen + target_env_strlen
                           + sbi_strlen + modext_strlen + 4));

    /* The path to the new library. */
    sprintf (filename,
             "%s/" SAC_TARGET_ENV_STRING "/" SAC_SBI_STRING "/lib%sMod" SAC_MODEXT_STRING,
             tmpdir_name, new_module);

    SAC_TR_Print ("Runtime specialization: Generating specialized library at:\n%s",
                  filename);

    // Mark specialization as processed early to avoid concurrently processing
    // it twice by two individual controllers
    SAC_Simple_addProcessed (request);

    /* Execute the system call and act according to the return value. */
    switch (system (syscall)) {
    default:
        fprintf (stderr, "ERROR -- \t [RTSpec Controller: "
                         "SAC_Simple_handle_request()] Compilation failed!\n");

        exit (EXIT_FAILURE);
        break;

    case -1:
        fprintf (stderr, "ERROR -- \t [RTSpec Controller: "
                         "SAC_Simple_handle_request()] System call failed!\n");

        exit (EXIT_FAILURE);

    case 0:
        SAC_RTSPEC_TR_Print ("Runtime specialization: Linking with generated library.");

        /* Dynamically link with the new libary. */
        request->reg_obj->dl_handle = dlopen (filename, RTLD_NOW | RTLD_GLOBAL);

        SAC_RTSPEC_TR_Print ("Runtime specialization: Check handle not being NULL.");

        /* Exit on failure. */
        if (request->reg_obj->dl_handle == NULL) {
            fprintf (stderr, "ERROR -- \t %s\n", dlerror ());

            exit (EXIT_FAILURE);
        }

        SAC_RTSPEC_TR_Print ("Runtime specialization: Check linking error.");

        dlerror ();

        SAC_RTSPEC_TR_Print ("Runtime specialization: Load symbols for new wrapper.");

        /* Load the symbol for the new wrapper. */
        request->reg_obj->func_ptr
          = dlsym (request->reg_obj->dl_handle, request->func_name);

        request->reg_obj->module = new_module;

        /* Exit on failure. */
        if (request->reg_obj->func_ptr == NULL) {
            fprintf (stderr, "ERROR -- \t Could not load symbol!\n");

            exit (EXIT_FAILURE);
        }
    }

    free (filename);
    free (syscall);
    free (shape_info);
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_Simple_stopController (void)
 *
 * @brief  Stop the optimization controller.
 *
 ****************************************************************************/

void
SAC_Simple_stopController (void)
{
    SAC_RTSPEC_TR_Print ("Runtime specialization: Stopping simple controllers!");

    running = 0;
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_Simple_finalizeController (void)
 *
 * @brief  Kills the optimization controller.
 *
 ****************************************************************************/

void
SAC_Simple_finalizeController (void)
{
    SAC_RTSPEC_TR_Print ("Runtime specialization: Finalize simple controller!");

    SAC_Simple_deinitializeQueue ();

    int success;

    if (tmpdir_name != NULL) {
        if (rtspec_syscall == NULL) {
            rtspec_syscall = (char *)malloc (sizeof (char) * (strlen (tmpdir_name) + 16));
        }

        strcpy (rtspec_syscall, "rm -rf  ");
        strcat (rtspec_syscall, tmpdir_name);

        success = system (rtspec_syscall);

        free (rtspec_syscall);
        free (tmpdir_name);
    }
}

#else
static int this_translation_unit = 0xdead;
#endif /* SAC_DO_RTSPEC  */
