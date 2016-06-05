/** <!--*******************************************************************-->
 *
 * @file simple_controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 * @author  tvd
 *
 ****************************************************************************/

#ifndef _SAC_SIMPLE_CONTROLLER_H_
#define _SAC_SIMPLE_CONTROLLER_H_

#include "simple_reqqueue.h"
#include "persistence.h"

extern void SAC_Simple_setupController (char *dir, int trace, char *command_line,
                                        char *binary_name);

extern void *SAC_Simple_runController (void *);

extern void SAC_Simple_handleRequest (simple_queue_node_t *);

extern void SAC_Simple_stopController (void);

extern void SAC_Simple_finalizeController (void);

#endif /* _SAC_SIMPLE_CONTROLLER_H_ */
