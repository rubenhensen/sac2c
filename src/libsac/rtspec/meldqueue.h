/*
 * $Id$
 */

/** <!--*******************************************************************-->
 *
 * @file  meldqueue.h
 *
 * @brief  Contains the stuct definitions and prototypes for the request queue.
 *
 * @NOTE:  THIS FILE IS OUTDATED AND CURRENTLY NOT USED BY THE RUNTIME
 * SPECIALIZATION FRAMEWORK! SEE reqqueue.[ch] FOR THE REQUEST QUEUE
 * IMPLEMENTATION!
 *
 * @author  tvd
 *
 ****************************************************************************/

#ifndef _MELDQUEUE_H
#define _MELDQUEUE_H

#include "registry.h"

/**
 * @brief The nodes of the binary tree used to build up the Priority Queue.
 */
typedef struct queue_node {
    /** @brief The priority of a node. */
    int priority;

    /** @brief The name of the function being optimized. */
    char symbol_name[256];

    /** @brief A registry object, needed for updating the function pointer. */
    reg_obj_t *reg_obj;

    /** @brief The left child of the current node. */
    struct queue_node *left;

    /** @brief The right child of the current node. */
    struct queue_node *right;
} queue_node_t;

/**
 * @brief Structure to administer a single meldqueue.
 */
typedef struct meldqueue {
    /** @brief The amount of objects in the queue. */
    int size;

    /** @brief The root of the internal binary tree. */
    queue_node_t *root;
} meldqueue_t;

/**
 * @var  request_queue
 *
 * @brief Central administrative object.
 */
extern meldqueue_t *request_queue;

void initialize_queue (void);

queue_node_t *create_node (int, char *, reg_obj_t *);

queue_node_t *find_min (void);

queue_node_t *dequeue_request (void);

void enqueue_request (int, char *, reg_obj_t *);

queue_node_t *meld (queue_node_t *, queue_node_t *);

void free_meldqueue (void);

#endif /* _MELDQUEUE_H */
