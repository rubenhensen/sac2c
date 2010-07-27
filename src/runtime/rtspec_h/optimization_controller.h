/** <!--*******************************************************************-->
 *
 * @file  optimization_controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 * @author  tvd
 *
 ****************************************************************************/

#ifndef _SAC_OPTIMIZATION_CONTROLLER_H_
#define _SAC_OPTIMIZATION_CONTROLLER_H_

/**
 * @brief Structure for holding processed requests.
 */
typedef struct list {
    char request[256];
    struct list *next;
} list_t;

void SAC_setupController ();

void *SAC_runController ();

void SAC_handleRequest (queue_node_t *);

void SAC_finalizeController (void);

#endif /* _SAC_OPTIMIZATION_CONTROLLER_H_ */
