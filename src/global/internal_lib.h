/*
 *
 * $Log$
 * Revision 3.40  2005/09/09 17:50:28  sah
 * added ILIBhexStringToByteArray and ILIBbyteArrayToHexString
 *
 * Revision 3.39  2005/07/26 12:41:40  sah
 * ILIBreplaceSpecialCharacters now has a const char argument
 *
 * Revision 3.38  2005/04/12 15:13:27  sah
 * updated signature of ILIBcreateCppCallString.
 *
 * Revision 3.37  2005/03/10 09:41:09  cg
 * Eliminated deprecated macro definitions.
 *
 * Revision 3.36  2005/02/14 14:15:04  cg
 * ABORT_ON_ERROR macro replaced by new ctinfo function.
 *
 * Revision 3.35  2005/02/11 14:40:30  jhb
 * added tree_check to PHASE_DONE_EPILOG
 *
 * Revision 3.34  2004/11/27 01:43:41  ktr
 * removed ILIBrenamelocalidentifier
 *
 * Revision 3.33  2004/11/26 23:01:36  ktr
 * From denmark with love.
 *
 * Revision 3.32  2004/11/24 22:43:37  cg
 * Moved NumberOfDigits() from Error.c.
 *
 * Revision 3.31  2004/11/24 22:30:02  ktr
 * replaceSpecialCharacters moved from precompile.
 *
 * Revision 3.30  2004/11/24 22:19:44  cg
 * SacDevCamp approved.
 *
 * Revision 3.29  2004/11/22 21:16:44  ktr
 * globals.h
 *
 * Revision 3.28  2004/11/22 15:42:55  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 3.27  2004/11/07 14:31:34  ktr
 * CreateCppCallString now needs a third parameter for the temporary file
 * created in /tmp that is used for CPP's output.
 *
 * Revision 3.26  2004/11/02 14:58:36  sah
 * added MemCopy
 *
 * Revision 3.25  2004/09/22 13:19:11  sah
 * changed argument to StringCopy to const char*
 * as the functions does not modify it
 *
 * Revision 3.24  2003/09/09 14:57:23  sbs
 * PtrBuf support added.
 *
 * Revision 3.23  2003/03/24 16:36:16  sbs
 * CreateCppCallString added.
 *
 * Revision 3.22  2003/03/20 23:24:50  sah
 * NEED_0X_PREFIX renamed to NEED_PTR_PREFIX
 *
 * Revision 3.21  2003/03/20 14:01:58  sbs
 * config.h included; NEED_0x_PREFIX used.
 *
 * Revision 3.20  2002/09/05 20:29:19  dkr
 * PrefixForTmpVar() added
 *
 * Revision 3.19  2002/09/03 13:18:06  sbs
 * StrBuf support added
 *
 * Revision 3.18  2002/03/01 13:37:31  dkr
 * minor changes done
 *
 * Revision 3.17  2001/07/13 13:23:41  cg
 * DBUG tag MEM_LEAK_CHECK renamed to MEM_LEAK
 *
 * Revision 3.16  2001/06/20 11:34:19  ben
 * StrTok implemented
 *
 * Revision 3.15  2001/05/18 11:51:01  dkr
 * macro FREE_VECT added
 *
 * Revision 3.14  2001/05/18 10:38:58  cg
 * Old memory management macros MALLOC and FREE removed.
 *
 * Revision 3.13  2001/05/17 13:29:29  cg
 * Moved de-allocation function Free() from free.c to internal_lib.c
 *
 * Revision 3.12  2001/05/17 08:36:02  sbs
 * PHASE_DONE_EPILOG added.
 *
 * Revision 3.11  2001/04/24 13:19:49  dkr
 * FALSE, TRUE moved to types.h
 *
 * Revision 3.10  2001/04/24 09:59:30  dkr
 * macro STR_OR_UNKNOWN added
 *
 * Revision 3.9  2001/04/24 09:34:42  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 * STR_OR_NULL moved from my_debug.h to internal_lib.h
 *
 * Revision 3.8  2001/04/10 09:36:46  dkr
 * macro F_PTR added
 *
 * Revision 3.7  2001/03/28 14:53:26  dkr
 * CHECK_NULL moved from tree_compound.h to internal_lib.h
 *
 * Revision 3.6  2001/03/28 09:12:29  dkr
 * bug in MALLOC_INIT_VECT fixed
 *
 * Revision 3.5  2001/03/27 21:54:24  dkr
 * comment added
 *
 * Revision 3.4  2001/03/27 21:39:09  dkr
 * macros ..._VECT added
 *
 * Revision 3.3  2001/03/27 11:29:43  dkr
 * OptCmp() removed
 *
 * Revision 3.2  2000/12/15 10:43:20  dkr
 * macros for bit fields added
 *
 * Revision 3.1  2000/11/20 17:59:31  sacbase
 * new release made
 *
 * Revision 2.15  2000/04/27 13:50:19  cg
 * Bug fixed in DBUG_OFF versions of CHECK_DBUG_START and CHECK_DBUG_STOP.
 *
 * Revision 2.14  2000/03/17 20:43:26  dkr
 * fixed a bug in CHECK_DBUG_STOP
 *
 * Revision 2.13  2000/03/17 11:12:51  dkr
 * definition of macro PHASE_EPILOG modified:
 * ABORT_ON_ERROR embedded, lac<->fun rearranged
 *
 * Revision 2.12  2000/03/16 16:38:10  dkr
 * fixed a bug in PHASE_EPILOG
 *
 * Revision 2.11  2000/03/16 14:30:53  dkr
 * macros PHASE_PROLOG and PHASE_EPILOG added
 *
 * Revision 2.10  2000/02/11 16:26:43  dkr
 * function StringConcat added
 *
 * Revision 2.9  2000/01/25 13:39:26  dkr
 * all the constvec stuff moved to tree_compound.h
 *
 * Revision 2.8  1999/07/20 11:48:29  cg
 * Definition (!) of global variable malloc_align_step removed;
 * malloc_align_step is now defined in globals.c.
 *
 * Revision 2.7  1999/07/20 07:53:42  cg
 * Definition (!) of global variable malloc_align_step removed;
 * malloc_align_step is now defined in globals.c.
 *
 * Revision 2.6  1999/05/14 09:25:13  jhs
 * Dbugged constvec annotations and their housekeeping in various
 * compilation stages.
 *
 * Revision 2.5  1999/05/12 08:41:11  sbs
 * CopyIntVector and friends eliminated ; instead,
 * CopyConstVec, AllocConstVec, and ModConstVec have been added.
 *
 * Revision 2.4  1999/04/19 09:56:42  jhs
 * TRUE and FALSE added
 *
 * Revision 2.3  1999/03/15 13:53:25  bs
 * CopyIntArray renamed into CopyIntVector, CopyFloatVector and
 * CopyDoubleVector added.
 *
 * Revision 2.2  1999/02/24 20:23:04  bs
 * New function added: CopyIntArray
 *
 * Revision 2.1  1999/02/23 12:39:24  sacbase
 * new release made
 *
 * [...]
 *
 */

#ifndef _SAC_INTERNAL_LIB_H_
#define _SAC_INTERNAL_LIB_H_

#include <stdio.h>
#include "config.h"
#include "types.h"
#include "dbug.h"
#include "globals.h"
#include "check_lib.h"
#include "ctinfo.h"

/*********************************
 *
 * Internal lib
 *
 * Prefix: ILIB
 *
 *********************************/

extern void *ILIBmalloc (int size);
extern void *ILIBfree (void *address);

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

extern char *ILIBstringCopy (const char *source);
extern char *ILIBstringConcat (const char *first, const char *second);
extern char *ILIBstringConcat3 (const char *first, const char *second, const char *third);
extern char *ILIBstrTok (char *first, char *sep);
extern bool ILIBstringCompare (const char *first, const char *second);
extern int ILIBnumberOfDigits (int number);

extern void *ILIBmemCopy (int size, void *mem);

extern int ILIBlcm (int x, int y);
extern char *ILIBitoa (long number);

extern char *ILIBhexStringToByteArray (char *array, const char *string);
extern char *ILIBbyteArrayToHexString (int len, char *array);

extern void ILIBsystemCall (char *format, ...);
extern int ILIBsystemCall2 (char *format, ...);
extern int ILIBsystemTest (char *format, ...);

extern void ILIBcreateCppCallString (const char *file, char *cccallstr,
                                     const char *cppfile);

extern char *ILIBtmpVar (void);
extern char *ILIBtmpVarName (char *postfix);

extern char *ILIBreplaceSpecialCharacters (const char *name);

#ifdef SHOW_MALLOC
extern void ILIBcomputeMallocAlignStep (void);
#endif

#ifndef DBUG_OFF
extern void ILIBdbugMemoryLeakCheck (void);
#endif

/*********************************
 * macro definitions
 *********************************/

/* format string for pointers */
#ifdef NEED_PTR_PREFIX
#define F_PTR "0x%p"
#else
#define F_PTR "%p"
#endif

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
