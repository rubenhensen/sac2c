/*
 *
 * $Log$
 * Revision 3.3  2003/09/16 13:23:12  dkr
 * sac_misc.h included
 *
 * Revision 3.2  2003/04/29 11:55:05  cg
 * Added function SAC_RuntimeWarning in analogy to SAC_RuntimeError.
 *
 * Revision 3.1  2000/11/20 18:02:44  sacbase
 * new release made
 *
 * Revision 2.4  2000/05/24 09:32:35  cg
 * Heap manager diagnostics are now printed after termination through
 * runtime error.
 *
 * Revision 2.3  2000/01/17 16:25:58  cg
 * The implementation of the SAC runtime message system is now
 * thread-safe !!
 *
 * Revision 2.2  1999/07/08 12:29:20  cg
 * File moved to new directory src/libsac.
 *
 */

/*
 *
 * Revision 2.1  1999/02/23 12:43:38  sacbase
 * new release made
 *
 * Revision 1.4  1998/06/29 08:50:14  cg
 * added '#define _POSIX_C_SOURCE 199506L' for multi-threaded execution.
 *
 * Revision 1.3  1998/05/07 08:13:24  cg
 * SAC runtime library implementation converted to new naming conventions.
 *
 * Revision 1.2  1998/03/24 13:51:45  cg
 * First working revision
 *
 * Revision 1.1  1998/03/19 16:34:14  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   message.c
 *
 * prefix: SAC_
 *
 * description:
 *
 *   This file is part of the implementation of the SAC runtime library.
 *   It contains two functions for producing arbitrary output during runtime
 *   or for terminating program execution with a runtime error message,
 *   respectively. The direct usage of respective C functions is not possible
 *   since name/type clashes with functions whose prototypes are derived from
 *   imported external modules may occur.
 *
 *   During multi-threaded execution, the integrity of messages is guaranteed
 *   through a locking mechanism.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "sac_misc.h"
#include "sac_heapmgr.h"

#ifdef MT
#define SAC_DO_MULTITHREAD 1
#define SAC_DO_THREADS_STATIC 1
#else
#define SAC_DO_MULTITHREAD 0
#endif /*  MT  */

#include "sac_mt.h"

#undef SAC_DO_MULTITHREAD
#undef SAC_DO_THREADS_STATIC

/*
 * Function definitions
 */

void
SAC_RuntimeError (char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    SAC_HM_ShowDiagnostics ();
    /*
     * If the program is not linked with the diagnostic version of the private
     * heap manager, a dummy function will be called here defined either in
     * heapmgr/setup.c or in libsac/nophm.c.
     */

    fprintf (stderr, "\n\n*** SAC runtime error\n");
    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);

    exit (1);
}

void
SAC_RuntimeWarning (char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    fprintf (stderr, "\n\n*** SAC runtime warning\n");
    fprintf (stderr, "*** ");

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    fprintf (stderr, "\n\n");

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}

void
SAC_Print (char *format, ...)
{
    va_list arg_p;

    SAC_MT_ACQUIRE_LOCK (SAC_MT_output_lock);

    va_start (arg_p, format);
    vfprintf (stderr, format, arg_p);
    va_end (arg_p);

    SAC_MT_RELEASE_LOCK (SAC_MT_output_lock);
}
