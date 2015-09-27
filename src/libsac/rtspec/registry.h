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

#include "uthash.h"

/**
 * @brief  Struct representing a specialized function.
 */
typedef struct {
    char *module;
    void *func_ptr;
    void *dl_handle;
} reg_obj_t;

typedef struct {
    char *key;
    void *func_ptr;
    void *dl_handle;

    UT_hash_handle hh;
} reg_item_t;

reg_obj_t *SAC_registrate (char *, void *);

void SAC_register_specialization (char *key, void *dl_handle, void *func_ptr);

void *SAC_lookup_function (char *func_name, char *uuid, char *types, int *shapes,
                           int shape_size, char *mod_name, void *func_ptr);

#endif /* _SAC_REGISTRY_H_ */
