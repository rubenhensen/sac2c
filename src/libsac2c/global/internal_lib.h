/*
 * $Id$
 */

#ifndef _SAC_INTERNAL_LIB_H_
#define _SAC_INTERNAL_LIB_H_

#include <stdio.h>

#include "config.h"
#include "types.h"
#include "dbug.h"
#include "globals.h"
#include "ctinfo.h"
#include "memory.h"

/*********************************
 *
 * Internal lib
 *
 * Prefix: ILIB
 *
 *********************************/

extern char *TRAVtmpVar (void);
extern char *TRAVtmpVarName (char *postfix);

/*********************************
 * macro definitions
 *********************************/

/* min, max */
#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)

/*
 * THE FOLLOWING MACROS ARE DEPRECATED!!  DO NOT USE!!!
 */

/*
 * macros for handling vectors
 */

#define MALLOC_VECT(vect, dims, type)                                                    \
    if (vect == NULL) {                                                                  \
        (vect) = (type *)MEMmalloc ((dims) * sizeof (type));                             \
    }

/* caution: 'val' should occur in the macro implementation only once! */
#define MALLOC_INIT_VECT(vect, dims, type, val)                                          \
    MALLOC_VECT (vect, dims, type);                                                      \
    INIT_VECT (vect, dims, type, val)

/* caution: 'val' should occur in the macro implementation only once! */
#define INIT_VECT(vect, dims, type, val)                                                 \
    {                                                                                    \
        int d;                                                                           \
        for (d = 0; d < (dims); d++) {                                                   \
            (vect)[d] = val;                                                             \
        }                                                                                \
    }

#define DUP_VECT(new_vect, old_vect, dims, type)                                         \
    {                                                                                    \
        int d;                                                                           \
        if ((old_vect) != NULL) {                                                        \
            MALLOC_VECT (new_vect, dims, type);                                          \
            for (d = 0; d < (dims); d++) {                                               \
                (new_vect)[d] = (old_vect)[d];                                           \
            }                                                                            \
        }                                                                                \
    }

#define PRINT_VECT(handle, vect, dims, format)                                           \
    {                                                                                    \
        int d;                                                                           \
        if ((vect) != NULL) {                                                            \
            fprintf (handle, "[ ");                                                      \
            for (d = 0; d < (dims); d++) {                                               \
                fprintf (handle, format, (vect)[d]);                                     \
                fprintf (handle, " ");                                                   \
            }                                                                            \
            fprintf (handle, "]");                                                       \
        } else {                                                                         \
            fprintf (handle, "NULL");                                                    \
        }                                                                                \
    }

#endif /* _SAC_INTERNAL_LIB_H_ */
