/*
 * $Id$
 */

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

#if ENABLE_RTSPEC

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
 * @return A registry objec.
 *
 ****************************************************************************/
reg_obj_t *
SAC_registrate (char *module, void *func_ptr)
{
    reg_obj_t *entry = malloc (sizeof (reg_obj_t));

    if (entry == NULL) {
        fprintf (stderr, "ERROR -- \t [registry.c: create_registry()] Can't "
                         " allocate registry object!");

        exit (EXIT_FAILURE);
    }

    strcpy (entry->module, module);
    entry->func_ptr = func_ptr;
    entry->dl_handle = NULL;

    return entry;
}

#endif /* ENABLE_RTSPEC */
