/*
 * $Id: omp.c 16606 2009-11-19 18:33:30Z cg $
 */

/** <!--*******************************************************************-->
 *
 * @file controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 * @author  tvd
 *
 ****************************************************************************/

#ifndef _SAC_CONTROLLER_H_
#define _SAC_CONTROLLER_H_

#include "reqqueue.h"

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

#endif /* _SAC_CONTROLLER_H_ */
