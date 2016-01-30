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

char *SAC_persistence_add (char *filename, char *func_name, char *uuid, char *type_info,
                           char *shape, char *mod_name);

void *SAC_persistence_load (char *filename, char *symbol_name, char *key);

void *SAC_persistence_get (char *key, char *func_name, char *uuid, char *type_info,
                           char *shape, char *mod_name);

#endif /* _SAC_PERSISTENCE_H_ */
