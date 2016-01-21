/** <!--*******************************************************************-->
 *
 * @file  registry.c
 *
 * @brief  This file contains the implementation of the registry.
 *
 * @author  tvd
 *
 ****************************************************************************/

#include "config.h"

#if SAC_DO_RTSPEC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "registry.h"

#define SAC_DO_TRACE 1
#include "sac.h"

#include "uthash.h"

static int do_trace;

reg_item_t *registry = NULL;

/* Lock for accessing the registry */
pthread_rwlock_t SAC_RTSPEC_registry_lock;

/** <!--*******************************************************************-->
 *
 * @fn SAC_registrate(char *module, void *func_ptr)
 *
 * @brief Register a single function.
 *
 * Each function has a local registry object, which is allocated and filled in
 * using this function.
 *
 * @return A registry object.
 *
 ****************************************************************************/
reg_obj_t *
SAC_registrate (char *module, void *func_ptr)
{
    reg_obj_t *entry = (reg_obj_t *)malloc (sizeof (reg_obj_t));

    if (entry == NULL) {
        fprintf (stderr, "ERROR -- \t [registry.c: create_registry()] Can't "
                         " allocate registry object!");

        exit (EXIT_FAILURE);
    }

    entry->module = module;
    entry->func_ptr = func_ptr;
    entry->dl_handle = NULL;

    return entry;
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_registry_init(int trace)
 *
 * @brief Setup the registry.
 *
 ****************************************************************************/
void
SAC_registry_init (int trace)
{
    do_trace = trace;

    // set up registry access lock
    if (pthread_rwlock_init (&SAC_RTSPEC_registry_lock, NULL) != 0) {
        fprintf (stderr, "ERROR -- \t [registry.c: SAC_registry_init()] Can't "
                         " initialize registry lock!");

        exit (EXIT_FAILURE);
    }
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_register_specialization( char *key, void *dl_handle, void *func_ptr)
 *
 * @brief Register a specialized function.
 *
 ****************************************************************************/
void
SAC_register_specialization (char *key, void *dl_handle, void *func_ptr)
{
    reg_item_t *item = (reg_item_t *)calloc (1, sizeof (reg_item_t));

    item->key = key;
    item->dl_handle = dl_handle;
    item->func_ptr = func_ptr;

    if (pthread_rwlock_wrlock (&SAC_RTSPEC_registry_lock) != 0) {
        fprintf (stderr, "ERROR -- \t [registry.c: SAC_register_specialization()] Can't "
                         " get write lock!");

        exit (EXIT_FAILURE);
    }

    HASH_ADD_KEYPTR (hh, registry, key, strlen (key), item);

    pthread_rwlock_unlock (&SAC_RTSPEC_registry_lock);

    SAC_TR_Print ("Registered UUID: %s", key);
}

/** <!--*******************************************************************-->
 *
 * @fn SAC_lookup_function( char *func_name, char *uuid, char *types, int *shapes, int
 *shape_size, char *shape, void *func_ptr)
 *
 * @brief Look up the best matching function pointer.
 *
 * @return A function pointer.
 *
 ****************************************************************************/
void *
SAC_lookup_function (char *func_name, char *uuid, char *types, int *shapes,
                     int shape_size, char *mod_name, void *func_ptr)
{
    SAC_TR_Print ("Look up function: %s::%s", mod_name, func_name);

    if (strcmp (mod_name, "_MAIN") == 0) {
        // we do not support specializing functions within main yet
        return func_ptr;
    }

    char *shape = encodeShapes (shapes);

    SAC_TR_Print ("Look up UUID: %s", uuid);
    SAC_TR_Print ("Look up types: %s", types);
    SAC_TR_Print ("Look up shape: %s", shape);
    reg_item_t *item = NULL;
    char *key
      = (char *)calloc (1, sizeof (char)
                             * (strlen (uuid) + strlen (types) + strlen (shape) + 1));

    key[0] = '\0';

    strcat (key, uuid);
    strcat (key, types);
    strcat (key, shape);

    SAC_TR_Print ("Look up key: %s", key);

    if (pthread_rwlock_rdlock (&SAC_RTSPEC_registry_lock) != 0) {
        fprintf (stderr, "ERROR -- \t [registry.c: SAC_register_specialization()] Can't "
                         " get write lock!");

        exit (EXIT_FAILURE);
    }

    HASH_FIND_STR (registry, key, item);

    pthread_rwlock_unlock (&SAC_RTSPEC_registry_lock);

    if (item) {
        SAC_TR_Print ("Found specialization");

        return item->func_ptr;
    } else {
        SAC_TR_Print ("No specialization found");

        SAC_UUID_enqueueRequest (func_name, uuid, types, shapes, shape_size, shape,
                                 mod_name, key);

        return func_ptr;
    }
}

#else
static int this_translation_unit = 0xdead;
#endif /* SAC_DO_RTPSPEC  */
