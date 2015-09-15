/** <!--*******************************************************************-->
 *
 * @file  registry.h
 *
 * @brief  Contains the global variables and prototypes for the registry.
 *
 * @author  tvd
 *
 ****************************************************************************/

#ifndef _SAC_REGISTRY_H_
#define _SAC_REGISTRY_H_

/**
 * @brief  Struct representing a specialized function.
 */
typedef struct {
    char *module;
    void *func_ptr;
    void *dl_handle;
} reg_obj_t;

reg_obj_t *SAC_registrate (char *, void *);

#endif /* _SAC_REGISTRY_H_ */
