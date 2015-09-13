/** <!--********************************************************************-->
 *
 * @file uuid_reqqueue.c
 *
 * @brief Implementation of the request queue data structure.
 *
 * @author tvd
 *
 *****************************************************************************/

#include "config.h"

#if ENABLE_RTSPEC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "registry.h"
#include "uuid_reqqueue.h"

#define SAC_DO_TRACE 1
#include "sac.h"

uuid_reqqueue_t *uuid_request_queue = NULL;
pthread_mutex_t uuid_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t uuid_empty_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t uuid_processed_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t uuid_empty_queue_cond = PTHREAD_COND_INITIALIZER;

static int do_trace;
uuid_reqqueue_t *uuid_processed = NULL;

/** <!--*******************************************************************-->
 *
 * @fn  SAC_UUID_wasProcessed( uuid_queue_node_t *node)
 *
 * @brief Iterates over all the nodes in the list of processed requests and
 * returns TRUE if a node contains the same information as the current request
 * and FALSE otherwise.
 *
 * @param  node  The queue node holding information about the function
 *
 * @return  TRUE if the request was allready handled, FALSE otherwise.
 *
 ****************************************************************************/
int
SAC_UUID_wasProcessed (uuid_queue_node_t *node)
{
    uuid_queue_node_t *current;

    pthread_mutex_lock (&uuid_processed_mutex);

    if (uuid_processed->first == NULL) {
        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Nothing processed yet.");
        }
        pthread_mutex_unlock (&uuid_processed_mutex);
        return 0;
    }

    current = uuid_processed->first;
    while (current != NULL) {
        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Checking queue.");
        }
        if (current->shape_info_size == node->shape_info_size
            && (memcmp (current->shape_info, node->shape_info,
                        sizeof (int) * node->shape_info_size)
                == 0)
            && (strcmp (current->func_name, node->func_name) == 0)) {
            if (do_trace == 1) {
                SAC_TR_Print ("Runtime specialization: Already processed.");
            }
            pthread_mutex_unlock (&uuid_processed_mutex);
            return 1;
        }

        current = current->next;
    }

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Found no match.");
    }

    pthread_mutex_unlock (&uuid_processed_mutex);

    return 0;
}

/** <!--*******************************************************************-->
 *
 * @fn  SAC_UUID_addProcessed( uuid_queue_node_t *node)
 *
 * @brief Adds a new node to the list of processed requests.
 *
 * @param  node  The queue node holding information about the function
 *
 ****************************************************************************/
void
SAC_UUID_addProcessed (uuid_queue_node_t *node)
{
    pthread_mutex_lock (&uuid_processed_mutex);

    /* Add the new processed request at the beginning of the list, this is
     * cheaper than at the end.
     */
    node->next = uuid_processed->first;

    if (uuid_processed->first == NULL) {
        uuid_processed->last = node;
    }

    uuid_processed->first = node;

    uuid_processed->size++;

    pthread_mutex_unlock (&uuid_processed_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_UUID_initializeQueue(int trace)
 *
 * @brief Allocate memory for the central request queue.
 *
 ****************************************************************************/
void
SAC_UUID_initializeQueue (int trace)
{
    do_trace = trace;

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Initialize request queue.");
    }

    pthread_mutex_lock (&uuid_queue_mutex);

    /*
     * Reinitialization, so we clean up first.
     */
    if (uuid_request_queue != NULL) {
        SAC_UUID_freeReqqueue (uuid_request_queue->first);
    }

    uuid_request_queue = (uuid_reqqueue_t *)malloc (sizeof (uuid_reqqueue_t));

    if (uuid_request_queue == NULL) {
        fprintf (stderr, "ERROR -- \t "
                         "[reqqueue.c: SAC_initializeQueue()] malloc().");

        exit (EXIT_FAILURE);
    }

    uuid_request_queue->size = 0;
    uuid_request_queue->first = NULL;
    uuid_request_queue->last = NULL;

    pthread_mutex_unlock (&uuid_queue_mutex);

    pthread_mutex_lock (&uuid_processed_mutex);

    /*
     * Reinitialization, so we clean up first.
     */
    if (uuid_processed != NULL) {
        SAC_UUID_freeReqqueue (uuid_processed->first);
    }

    uuid_processed = (uuid_reqqueue_t *)malloc (sizeof (uuid_reqqueue_t));

    if (uuid_processed == NULL) {
        fprintf (stderr, "ERROR -- \t "
                         "[reqqueue.c: SAC_initializeQueue()] malloc().");

        exit (EXIT_FAILURE);
    }

    uuid_processed->size = 0;
    uuid_processed->first = NULL;
    uuid_processed->last = NULL;

    pthread_mutex_unlock (&uuid_processed_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_UUID_deinitializeQueue(void)
 *
 * @brief Deallocate memory for the central request queue.
 *
 ****************************************************************************/
void
SAC_UUID_deinitializeQueue (void)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Deinitialize request queue.");
    }

    pthread_mutex_lock (&uuid_queue_mutex);

    if (uuid_request_queue != NULL) {
        SAC_UUID_freeReqqueue (uuid_request_queue->first);
    }

    free (uuid_request_queue);

    pthread_mutex_unlock (&uuid_queue_mutex);
    pthread_mutex_lock (&uuid_processed_mutex);

    if (uuid_request_queue != NULL) {
        SAC_UUID_freeReqqueue (uuid_processed->first);
    }

    free (uuid_processed);

    pthread_mutex_unlock (&uuid_processed_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_UUID_createNode (char *func_name, char *types, int *shapes,
 *                     reg_obj_t *registry)
 *
 * @brief Allocate a new queue node and initialize the fields of the node.
 *
 * @param func_name  The name of the function being optimized.
 * @param types    The types of the arguments of the function being optimized.
 * @param shapes   The shapes of the arguments.
 * @param shapes_size The size of the shape arguments array
 * @param registry The registry object used for dynamically loading and
 *                 storing the optimized function.
 *
 * @return A new queue node or NULL on failure.
 *
 ****************************************************************************/
uuid_queue_node_t *
SAC_UUID_createNode (char *func_name, char *types, int *shapes, int shapes_size,
                     reg_obj_t *registry)
{
    uuid_queue_node_t *xnew = (uuid_queue_node_t *)malloc (sizeof (uuid_queue_node_t));

    if (xnew == NULL) {
        fprintf (stderr, "ERROR -- \t [reqqueue.c: SAC_createNode()] malloc().");

        return NULL;
    }

    xnew->func_name = func_name;
    xnew->type_info = types;
    xnew->shape_info = shapes;
    xnew->shape_info_size = shapes_size;
    xnew->reg_obj = registry;
    xnew->next = NULL;

    return xnew;
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_UUID_dequeueRequest(void)
 *
 * @brief Retrieve the first element in the queue and move the pointer to the
 *        first element to the second element.
 *
 * @return The first node in the queue or NULL if the queue is empty.
 *
 ****************************************************************************/
uuid_queue_node_t *
SAC_UUID_dequeueRequest (void)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Dequeue specialization request.");
    }

    pthread_mutex_lock (&uuid_queue_mutex);

    if (uuid_request_queue->first == NULL) {
        return NULL;
    }

    uuid_queue_node_t *result = uuid_request_queue->first;

    /* Move the pointer to the first object to the next object. */
    uuid_request_queue->first = uuid_request_queue->first->next;

    /* The result is no longer part of the queue. */
    result->next = NULL;

    uuid_request_queue->size--;

    pthread_mutex_unlock (&uuid_queue_mutex);

    return result;
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_UUID_enqueueRequest (char *func_name, char *types, int *shapes,
 *                         reg_obj_t *registry)
 *
 * @param func_name  The name of the function being optimized.
 * @param types    The types of the arguments of the function being optimized.
 * @param shapes   The shapes of the arguments.
 * @param shapes_size The size of the shape arguments array
 * @param registry The registry object used for dynamically loading and
 *                 storing the optimized function.
 *
 * @brief Add a new request to the request queue.
 *
 ****************************************************************************/
void
SAC_UUID_enqueueRequest (char *func_name, char *types, int *shapes, int shapes_size,
                         reg_obj_t *registry)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Enqueue specialization request.");
    }

    if (uuid_request_queue == NULL) {
        // using rtspec enabled library in rtspec-disabled application
        return;
    }

    if (strcmp (registry->module, "_MAIN") == 0) {
        // we do not support specializing functions within main yet
        return;
    }

    pthread_mutex_lock (&uuid_queue_mutex);

    uuid_queue_node_t *xnew
      = SAC_UUID_createNode (func_name, types, shapes, shapes_size, registry);

    /* Exit on error. */
    if (xnew == NULL) {
        fprintf (stderr, "ERROR -- \t [reqqueue.c: enqueue_request()] "
                         "Couldn't create node, exiting.");

        pthread_mutex_unlock (&uuid_queue_mutex);
        exit (EXIT_FAILURE);
    }

    if (uuid_request_queue->first == NULL) {
        /* Insert the first element. */
        uuid_request_queue->first = xnew;

        /* The only element in the queue is both first and last. */
        uuid_request_queue->last = uuid_request_queue->first;

    } else {
        /* Insert the new node after the last node in the queue. */
        uuid_request_queue->last->next = xnew;

        /* Move the pointer to the last object to the newly inserted node. */
        uuid_request_queue->last = uuid_request_queue->last->next;
    }

    uuid_request_queue->size++;

    /* Signal the controller if it was waiting for requests. */
    if (uuid_request_queue->size > 0) {
        pthread_cond_signal (&uuid_empty_queue_cond);
    }

    pthread_mutex_unlock (&uuid_queue_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_UUID_freeReqqueue( uuid_queue_node_t *node)
 *
 * @param node  the first node in the queue.
 *
 * @brief Free the memory used by the queue.
 *
 ****************************************************************************/
void
SAC_UUID_freeReqqueue (uuid_queue_node_t *node)
{
    uuid_queue_node_t *current;

    /* Walk through the queue and free each node. */
    if (node != NULL) {
        current = node->next;

        free (node);
        node = NULL;

        SAC_UUID_freeReqqueue (current);
    }
}

#endif /* ENABLE_RTSPEC */
