/** <!--*******************************************************************-->
 *
 * @file supervisor.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 * @author  hmw
 *
 ****************************************************************************/

#ifndef _SAC_SUPERVISOR_H_
#define _SAC_SUPERVISOR_H_

#include <pthread.h>


extern void SAC_RTSPEC_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                     int trace, int mode, char *command_line,
                                     char *binary_name);

extern unsigned int SAC_RTSPEC_CurrentThreadId (void);

extern void SAC_setupController (char *dir);

extern void SAC_finalizeController (void);

/* number of runtime specialization controller threads in the environment */
extern unsigned int SAC_RTSPEC_controller_threads;

/*
 * shared within rtspec:
 */
extern pthread_key_t SAC_RTSPEC_self_id_key;


#endif /* _SAC_SUPERVISOR_H_ */
