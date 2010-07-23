/** <!--*******************************************************************-->
 *
 * @file  meldqueue.c
 *
 * @brief  Contains the implementation of the request queue.
 *
 * This file contains an implementation of a Randomized Meldable Priority Queue
 * as described by Anna Gambin and Adam Malinowski (rand_meld_pq.pdf).
 *
 * @NOTE:  THIS FILE IS OUTDATED AND CURRENTLY NOT USED BY THE RUNTIME
 * SPECIALIZATION FRAMEWORK! SEE reqqueue.[ch] FOR THE REQUEST QUEUE
 * IMPLEMENTATION!
 *
 * @author  tvd
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "registry.h"
#include "meldqueue.h"

meldqueue_t *request_queue = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

/** <!--*******************************************************************-->
 *
 * @fn  initialize_queue(void)
 *
 * @brief Initialize the central meld queue by allocating memory for the cental
 *        adminstration object.
 *
 * Calling this function twice will destroy the initial tree and free its
 * memory.
 *
 ****************************************************************************/
void
initialize_queue (void)
{
    pthread_mutex_lock (&queue_mutex);

    if (request_queue != NULL) {
        free_meldqueue ();
    }

    request_queue = malloc (sizeof (meldqueue_t));

    if (request_queue == NULL) {
        fprintf (stderr, "ERROR -- [meldqueue.c: initialize_queue()] malloc().");

        exit (0);
    }

    request_queue->size = 0;
    request_queue->root = NULL;

    pthread_mutex_unlock (&queue_mutex);
}

/** <!--********************************************************************-->
 *
 * @fn  queue_node_t *create_node (int priority, char *func_name)
 *
 * @brief  Create a new empty queue node.
 *
 * @param  priority  The priority for this node.
 * @param  func_name  The name of the function being optimized.
 *
 * @return  A pointer to a new node or NULL if malloc failed.
 *
 *****************************************************************************/
queue_node_t *
create_node (int priority, char *symbol_name, reg_obj_t *reg_obj)
{
    queue_node_t *new = malloc (sizeof (queue_node_t));

    if (new == NULL) {
        fprintf (stderr, "ERROR -- [meldqueue.c: create_node()] malloc().");

        return NULL;
    }

    new->priority = priority;
    strcpy (new->symbol_name, symbol_name);
    new->reg_obj = reg_obj;
    new->left = NULL;
    new->right = NULL;

    return new;
}

/** <!--********************************************************************-->
 *
 * @fn  queue_node_t *find_min (void)
 *
 * @brief  Peek at the minimum element in the queue.
 * @return  Returns the root of the tree, the head of the queue.
 *
 *****************************************************************************/
queue_node_t *
find_min (void)
{
    return request_queue->root;
}

/** <!--********************************************************************-->
 *
 * @fn  queue_node_t *dequeue_request (void)
 *
 * @brief  Find and remove the minimum element in the queue.
 *
 * @return  Returns the root of the tree, the head of the queue.
 *
 *****************************************************************************/
queue_node_t *
dequeue_request (void)
{
    pthread_mutex_lock (&queue_mutex);

    queue_node_t *min = request_queue->root;

    if (request_queue->root != NULL) {
        request_queue->root
          = meld (request_queue->root->left, request_queue->root->right);
    }

    request_queue->size--;

    pthread_mutex_unlock (&queue_mutex);

    return min;
}

/** <!--********************************************************************-->
 *
 * @fn  enqueue_request (int priority, char *name)
 *
 * @brief  Insert a new element into the queue.
 *
 * @param  priority  The priority of the node.
 * @param  name  The name of the function being optimized.
 *
 *****************************************************************************/
void
enqueue_request (int priority, char *symbol_name, reg_obj_t *reg_obj)
{
    pthread_mutex_lock (&queue_mutex);

    request_queue->root
      = meld (create_node (priority, symbol_name, reg_obj), request_queue->root);

    request_queue->size++;

    pthread_mutex_unlock (&queue_mutex);
}

/** <!--********************************************************************-->
 *
 * @fn  queue_node_t *meld(queue_node_t *q1, queue_node_t *q2)
 *
 * @brief  Recursively meld to priority queues.
 *
 * @param  q1  A binary tree with n >= 1 elements.
 * @param  q2  A binary tree with n >= 1 elements.
 *
 *****************************************************************************/
queue_node_t *
meld (queue_node_t *q1, queue_node_t *q2)
{
    queue_node_t *tmp = NULL;
    float coin_toss = 0.0;

    if (q1 == NULL) {
        return q2;
    }

    if (q2 == NULL) {
        return q1;
    }

    /* Switch the queues if q1 is the larger one.
     * NOTE: this should be the other way around if the maximum element is
     * always in the root!
     */
    if (q1->priority > q2->priority) {
        tmp = q1;
        q1 = q2;
        q2 = tmp;
    }

    /* This is not pretty, but I haven't found another way to get a random
     * 'boolean' value.
     */
    coin_toss = 2 * ((float)(1 + rand ()) / RAND_MAX);

    if (coin_toss > 1.0) {
        q1->left = meld (q1->left, q2);
    } else {
        q1->right = meld (q1->right, q2);
    }

    return q1;
}

/** <!--********************************************************************-->
 *
 * @fn  free_tree (queue_node_t *root)
 *
 * @brief  Recursively free all the nodes of a binary tree.
 *
 * @param  root  The root of the tree being freed.
 *
 * @return  1 if the tree was succesfully freed, 0 otherwise.
 *
 *****************************************************************************/
static int
free_tree (queue_node_t *root)
{
    if (root == NULL) {
        return 0;
    }

    if (root->left != NULL) {
        free_tree (root->left);
    }

    if (root->right != NULL) {
        free_tree (root->right);
    }

    free (root);

    return 1;
}

/** <!--********************************************************************-->
 *
 * @fn  free_meldqueue (void)
 *
 * @brief  Free all the nodes of the internal binary tree representing the queue.
 *
 *****************************************************************************/
void
free_meldqueue (void)
{
    if (request_queue == NULL || request_queue->root == NULL) {
        return;
    }

    /* Recursively free all the nodes of the internal tree. */
    free_tree (request_queue->root);
}
