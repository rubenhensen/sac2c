/** <!--*******************************************************************-->
 *
 * @file abstract_controller.h
 *
 * @brief  Contains macro's and function prototypes for the optimization
 * controller.
 *
 * @author  hmw
 *
 ****************************************************************************/

#ifndef _SAC_ABSTRACT_CONTROLLER_H_
#define _SAC_ABSTRACT_CONTROLLER_H_

extern void SAC_RTSPEC_SetupInitial (int argc, char *argv[], unsigned int num_threads,
                                     int trace, int mode);

extern unsigned int SAC_RTSPEC_CurrentThreadId (void);

extern void SAC_setupController (char *dir);

extern void SAC_finalizeController (void);

#endif /* _SAC_ABSTRACT_CONTROLLER_H_ */
