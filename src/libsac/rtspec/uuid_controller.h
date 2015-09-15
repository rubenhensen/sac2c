/** <!--*******************************************************************-->
 *
 * @file uuid_controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 * @author  tvd
 *
 ****************************************************************************/

#ifndef _SAC_UUID_CONTROLLER_H_
#define _SAC_UUID_CONTROLLER_H_

#include "uuid_reqqueue.h"
#include "persistence.h"

extern void SAC_UUID_setupController (char *dir, int trace);

extern void *SAC_UUID_runController (void *);

extern void SAC_UUID_handleRequest (uuid_queue_node_t *);

extern void SAC_UUID_stopController (void);

extern void SAC_UUID_finalizeController (void);

#endif /* _SAC_UUID_CONTROLLER_H_ */
