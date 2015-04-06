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
#include "persistence.h"

extern void SAC_RTSPEC_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                     int trace);

extern unsigned int SAC_RTSPEC_CurrentThreadId (void);

extern void SAC_setupController (char *dir);

extern void *SAC_runController (void *);

extern void SAC_handleRequest (queue_node_t *);

extern void SAC_finalizeController (void);

#endif /* _SAC_CONTROLLER_H_ */
