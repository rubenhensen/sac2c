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

/** <!--*******************************************************************-->
 *
 * @fn SAC_registrate(void *func_ptr)
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

#else
static int this_translation_unit = 0xdead;
#endif /* SAC_DO_RTPSPEC  */
