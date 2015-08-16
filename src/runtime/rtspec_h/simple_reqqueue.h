/** <!--********************************************************************-->
 *
 * @file simple_reqqueue.h
 *
 * @brief Header file for the request queue data structure.
 *
 * @author tvd
 *
 *****************************************************************************/

#ifndef _SAC_SIMPLE_REQQUEUE_H_
#define _SAC_SIMPLE_REQQUEUE_H_

/**
 * @brief A node of the request queue.
 */
typedef struct simple_queue_node {
    /** @brief The name of the function being optimized. */
    char *func_name;

    /** @brief The types of the arguments of the function being optimized. */
    char *type_info;

    /** @brief The shapes of the arguments. */
    int *shape_info;

    /** @brief The size of the shape info array */
    int shape_info_size;

    /** @brief A registry object, needed for updating the function pointer. */
    reg_obj_t *reg_obj;

    /** @brief The next request in the queue. */
    struct simple_queue_node *next;

} simple_queue_node_t;

/**
 * @brief Structure to administer a single queue.
 */
typedef struct simple_reqqueue {
    /** @brief The amount of objects in the queue. */
    int size;

    /** @brief The first element in the queue. */
    simple_queue_node_t *first;

    /** @brief The last element in the queue. */
    simple_queue_node_t *last;

} simple_reqqueue_t;

void SAC_Simple_initializeQueue (int trace);

void SAC_Simple_deinitializeQueue (void);

simple_queue_node_t *SAC_Simple_createNode (char *, char *, int *, int, reg_obj_t *);

simple_queue_node_t *SAC_Simple_dequeueRequest (void);

void SAC_Simple_enqueueRequest (char *, char *, int *, int, reg_obj_t *);

void SAC_Simple_freeReqqueue (simple_queue_node_t *);

#endif /* _SAC_SIMPLE_REQQUEUE_H_ */
