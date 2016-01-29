/** <!--*******************************************************************-->
 *
 * @file persistence.h
 *
 * @brief  Contains macro's and function prototypes for the rtspec persistence
 * handling.
 *
 * @author  hmw
 *
 ****************************************************************************/

#ifndef _SAC_PERSISTENCE_H_
#define _SAC_PERSISTENCE_H_

extern char *encodeShapes (int *shapes);

void SAC_persistence_init (int argc, char *argv[], int trace);

#endif /* _SAC_PERSISTENCE_H_ */
