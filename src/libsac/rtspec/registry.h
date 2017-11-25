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

struct reg_obj;
struct reg_item;

typedef struct reg_obj reg_obj_t;
typedef struct reg_item reg_item_t;

/*
 * We want to hide the implementation of the data types from sac.h
 */
#ifdef INCLUDED_FROM_LIBSAC

#include "uthash.h"

/**
 * @brief  Struct representing a specialized function.
 */
struct reg_obj {
    char *module;
    void *func_ptr;
    void *dl_handle;
};

struct reg_item {
    char *key;
    void *func_ptr;
    void *dl_handle;

    UT_hash_handle hh;
};

#endif

reg_obj_t *SAC_registrate (char *, void *);

void SAC_registry_init (int trace);

void SAC_register_specialization (char *key, void *dl_handle, void *func_ptr);

void *SAC_lookup_function (char *func_name, char *uuid, char *types, int *shapes,
                           int shape_size, char *mod_name, void *func_ptr);

#endif /* _SAC_REGISTRY_H_ */
