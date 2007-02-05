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
#ifdef SHOW_MALLOC
#define ILIBmallocAt(size, file, line) MEMmallocAt (size, file, line)
#endif
#define ILIBmalloc(size) MEMmalloc (size)

#define ILIBfree(n) MEMfree (n)

extern ptr_buf *ILIBptrBufCreate (int size);
extern ptr_buf *ILIBptrBufAdd (ptr_buf *s, void *ptr);
extern int ILIBptrBufGetSize (ptr_buf *s);
extern void *ILIBptrBufGetPtr (ptr_buf *s, int pos);
extern void ILIBptrBufFlush (ptr_buf *s);
extern void *ILIBptrBufFree (ptr_buf *s);

extern str_buf *ILIBstrBufCreate (int size);
extern str_buf *ILIBstrBufPrint (str_buf *s, const char *string);
extern str_buf *ILIBstrBufPrintf (str_buf *s, const char *format, ...);
extern char *ILIBstrBuf2String (str_buf *s);
extern void ILIBstrBufFlush (str_buf *s);
extern bool ILIBstrBufIsEmpty (str_buf *s);
extern void *ILIBstrBufFree (str_buf *s);

extern int ILIBlcm (int x, int y);

extern void ILIBsystemCall (char *format, ...);
extern int ILIBsystemCall2 (char *format, ...);
extern int ILIBsystemTest (char *format, ...);
extern void ILIBsystemCallStartTracking ();
extern void ILIBsystemCallStopTracking ();

extern char *ILIBtmpVar (void);
extern char *ILIBtmpVarName (char *postfix);

/*********************************
 * macro definitions
 *********************************/

/* handling of strings */
#define STR_OR_NULL(str, null_str) (((str) != NULL) ? (str) : (null_str))
#define STR_OR_EMPTY(str) STR_OR_NULL ((str), "")
#define STR_OR_UNKNOWN(str) STR_OR_NULL ((str), "?")

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
        (vect) = (type *)ILIBmalloc ((dims) * sizeof (type));                            \
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
