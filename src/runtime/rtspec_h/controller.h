/** <!--*******************************************************************-->
 *
 * @file  controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 ****************************************************************************/

#ifndef _SAC_CONTROLLER_H_
#define _SAC_CONTROLLER_H_

/**
 * @brief Structure for holding processed requests.
 */
typedef struct list {
    char request[256];
    struct list *next;
} list_t;

extern void SAC_setupController (char *dir, int trace);

extern void *SAC_runController (void);

extern void SAC_handleRequest (queue_node_t *);

extern void SAC_finalizeController (void);

#endif /* _SAC_CONTROLLER_H_ */
