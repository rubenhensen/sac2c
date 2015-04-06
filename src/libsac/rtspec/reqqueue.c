/** <!--********************************************************************-->
 *
 * @file reqqueue.c
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
#include "reqqueue.h"

#define SAC_DO_TRACE 1
#include "sac.h"

reqqueue_t *request_queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t empty_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t processed_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty_queue_cond = PTHREAD_COND_INITIALIZER;

static int do_trace;
static list_t *processed;

/** <!--*******************************************************************-->
 *
 * @fn  wasProcessed( queue_node_t *node)
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
wasProcessed (queue_node_t *node)
{
    list_t *current;

    pthread_mutex_lock (&processed_mutex);

    if (processed->node == NULL) {
        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Nothing processed yet.");
        }
        pthread_mutex_unlock (&processed_mutex);
        return 0;
    }

    current = processed;
    while (current != NULL) {
        if (do_trace == 1) {
            SAC_TR_Print ("Runtime specialization: Checking queue.");
        }
        if (current->node != NULL) {
            if (current->node->shape_info_size == node->shape_info_size
                && (memcmp (current->node->shape_info, node->shape_info,
                            sizeof (int) * node->shape_info_size)
                    == 0)
                && (strcmp (current->node->func_name, node->func_name) == 0)) {
                if (do_trace == 1) {
                    SAC_TR_Print ("Runtime specialization: Already processed.");
                }
                pthread_mutex_unlock (&processed_mutex);
                return 1;
            }
        } else {
            if (do_trace == 1) {
                SAC_TR_Print ("Runtime specialization: invalid queue node");
            }
            pthread_mutex_unlock (&processed_mutex);
            return 0;
        }

        current = current->next;
    }

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Found no match.");
    }

    pthread_mutex_unlock (&processed_mutex);

    return 0;
}

/** <!--*******************************************************************-->
 *
 * @fn  addProcessed( queue_node_t *node)
 *
 * @brief Adds a new node to the list of processed requests.
 *
 * @param  node  The queue node holding information about the function
 *
 ****************************************************************************/
void
addProcessed (queue_node_t *node)
{
    list_t *xnew = (list_t *)malloc (sizeof (list_t));

    if (xnew == NULL) {
        fprintf (stderr, "Could not allocate new processed request node!");

        exit (EXIT_FAILURE);
    }

    pthread_mutex_lock (&processed_mutex);

    /* Add the new processed request at the beginning of the list, this is
     * cheaper than at the end.
     */
    if (processed->node != NULL) {
        xnew->next = processed;
    }

    xnew->node = node;

    processed = xnew;

    pthread_mutex_unlock (&processed_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_initializeQueue(int trace)
 *
 * @brief Allocate memory for the central request queue.
 *
 ****************************************************************************/
void
SAC_initializeQueue (int trace)
{
    do_trace = trace;

    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Initialize request queue.");
    }

    pthread_mutex_lock (&queue_mutex);

    /*
     * Reinitialization, so we clean up first.
     */
    if (request_queue != NULL) {
        SAC_freeReqqueue (request_queue->first);
    }

    request_queue = (reqqueue_t *)malloc (sizeof (reqqueue_t));

    if (request_queue == NULL) {
        fprintf (stderr, "ERROR -- \t "
                         "[reqqueue.c: SAC_initializeQueue()] malloc().");

        exit (EXIT_FAILURE);
    }

    request_queue->size = 0;
    request_queue->first = NULL;
    request_queue->last = NULL;

    pthread_mutex_unlock (&queue_mutex);

    pthread_mutex_lock (&processed_mutex);

    processed = (list_t *)malloc (sizeof (list_t));

    processed->node = NULL;
    processed->next = NULL;

    pthread_mutex_unlock (&processed_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_createNode (char *func_name, char *types, int *shapes,
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
queue_node_t *
SAC_createNode (char *func_name, char *types, int *shapes, int shapes_size,
                reg_obj_t *registry)
{
    queue_node_t *xnew = (queue_node_t *)malloc (sizeof (queue_node_t));

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
 * @fn SAC_dequeueRequest(void)
 *
 * @brief Retrieve the first element in the queue and move the pointer to the
 *        first element to the second element.
 *
 * @return The first node in the queue or NULL if the queue is empty.
 *
 ****************************************************************************/
queue_node_t *
SAC_dequeueRequest (void)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Dequeue specialization request.");
    }

    pthread_mutex_lock (&queue_mutex);

    if (request_queue->first == NULL) {
        return NULL;
    }

    queue_node_t *result = request_queue->first;

    /* Move the pointer to the first object to the next object. */
    request_queue->first = request_queue->first->next;

    /* The result is no longer part of the queue. */
    result->next = NULL;

    request_queue->size--;

    pthread_mutex_unlock (&queue_mutex);

    return result;
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_enqueueRequest (char *func_name, char *types, int *shapes,
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
SAC_enqueueRequest (char *func_name, char *types, int *shapes, int shapes_size,
                    reg_obj_t *registry)
{
    if (do_trace == 1) {
        SAC_TR_Print ("Runtime specialization: Enqueue specialization request.");
    }

    if (request_queue == NULL) {
        // using rtspec enabled library in rtspec-disabled application
        return;
    }

    pthread_mutex_lock (&queue_mutex);

    queue_node_t *xnew = SAC_createNode (func_name, types, shapes, shapes_size, registry);

    /* Exit on error. */
    if (xnew == NULL) {
        fprintf (stderr, "ERROR -- \t [reqqueue.c: enqueue_request()] "
                         "Couldn't create node, exiting.");

        pthread_mutex_unlock (&queue_mutex);
        exit (EXIT_FAILURE);
    }

    if (request_queue->first == NULL) {
        /* Insert the first element. */
        request_queue->first = xnew;

        /* The only element in the queue is both first and last. */
        request_queue->last = request_queue->first;

    } else {
        /* Insert the new node after the last node in the queue. */
        request_queue->last->next = xnew;

        /* Move the pointer to the last object to the newly inserted node. */
        request_queue->last = request_queue->last->next;
    }

    request_queue->size++;

    /* Signal the controller if it was waiting for requests. */
    if (request_queue->size > 0) {
        pthread_cond_signal (&empty_queue_cond);
    }

    pthread_mutex_unlock (&queue_mutex);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_freeReqqueue( queue_node_t *node)
 *
 * @param node  the first node in the queue.
 *
 * @brief Free the memory used by the queue.
 *
 ****************************************************************************/
void
SAC_freeReqqueue (queue_node_t *node)
{
    queue_node_t *current;

    /* Walk through the queue and free each node. */
    if (node != NULL) {
        current = node->next;

        free (node);
        node = NULL;

        SAC_freeReqqueue (current);
    }
}

#endif /* ENABLE_RTSPEC */
