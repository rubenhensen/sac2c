/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:43:37  sacbase
 * new release made
 *
 * Revision 1.3  1998/06/29 08:50:14  cg
 * added '#define _POSIX_C_SOURCE 199506L' for multi-threaded execution.
 *
 * Revision 1.2  1998/05/07 08:13:24  cg
 * SAC runtime library implementation converted to new naming conventions.
 *
 * Revision 1.1  1998/03/19 16:33:23  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   libsac_malloc.c
 *
 * prefix: SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *
 *   It contains facilities for storage allocation and de-allocation.
 *   Currently, only a wrapper for malloc is defined which checks for
 *   having been successfull.
 *
 *****************************************************************************/

#define _POSIX_C_SOURCE 199506L

#include <malloc.h>
#include <stdio.h>

#include "sac_message.h"

void *
SAC_MallocCheck (unsigned int size)
{
    void *tmp;

    tmp = malloc (size);

    if (tmp == NULL) {
        SAC_RuntimeError ("Unable to allocate %d bytes of memory", size);
    }

    return (tmp);
}
