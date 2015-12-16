/** <!--*******************************************************************-->
 *
 * @file  simple_controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 ****************************************************************************/

#ifndef _SAC_SIMPLE_CONTROLLER_H_
#define _SAC_SIMPLE_CONTROLLER_H_

extern void SAC_Simple_setupController (char *dir, int trace, char *command_line);

extern void *SAC_Simple_runController (void);

extern void SAC_Simple_handleRequest (simple_queue_node_t *);

extern void SAC_Simple_stopController (void);

extern void SAC_Simple_finalizeController (void);

#endif /* _SAC_SIMPLE_CONTROLLER_H_ */
