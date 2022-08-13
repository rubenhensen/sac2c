/*****************************************************************************
 *
 * file:   pad.c
 *
 * prefix: AP
 *
 * description:
 *
 *   This compiler module infers new array shapes and applies array padding
 *   to improve cache performance.
 *
 *****************************************************************************/

#include <stdarg.h>

#define DBUG_PREFIX "OPT"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "filemgr.h"
#include "resource.h"
#include "ctinfo.h"

#include "pad.h"
#include "pad_info.h"
#include "pad_collect.h"
#include "pad_infer.h"
#include "pad_transform.h"

static FILE *apdiag_file;

/*****************************************************************************
 *
 * function:
 *   void APprintDiag( char *format, ...)
 *
 * description:
 *
 *   print diagnostic info from array padding to apdiag_file,
 *   if compiler flag -apdiag is enabled.
 *
 *   A counter prevents diagnostic output files to grow beyond given size.
 *
 *****************************************************************************/

#if 0

/*
 * Why the dangerous detour via a limited-size static buffer ??
 */

void APprintDiag( char *format, ...)
{
  va_list arg_p;
  static char buffer[1024];
  
  DBUG_ENTER ();

  if (apdiag) {
    va_start( arg_p, format);
    vsprintf( buffer, format, arg_p);
    va_end( arg_p);

    fprintf( apdiag_file, buffer);
  }

  DBUG_RETURN ();
}

#else

void
APprintDiag (char *format, ...)
{
    va_list arg_p;
    static int cnt = 1;
    int correction = 1;

    DBUG_ENTER ();

    if (global.apdiag && (cnt <= correction * global.apdiag_limit)) {
        va_start (arg_p, format);
        vfprintf (apdiag_file, format, arg_p);
        va_end (arg_p);
        cnt++;
        if (cnt > global.apdiag_limit) {
            fprintf (apdiag_file,
                     "\n\n************************************************************\n"
                     "*\n"
                     "*  Diagnostic output interupted !\n"
                     "*\n"
                     "*    Limit of approximately %d lines reached.\n"
                     "*\n"
                     "*      Use option -apdiaglimit\n"
                     "*      to increase / decrease this limit.\n"
                     "*\n"
                     "************************************************************\n\n",
                     global.apdiag_limit);
        }
    }

    DBUG_RETURN ();
}

#endif

/*****************************************************************************
 *
 * function:
 *   node *ArrayPadding( node *arg_node)
 *
 * description:
 *   main function for array padding,
 *   calls APcollect, APinfer and APtransform
 *
 *****************************************************************************/

node *
ArrayPadding (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_PRINT ("ARRAY PADDING");
    DBUG_PRINT_TAG ("AP", "Entering Array Padding");

    CTInote (EMPTY_LOC, "Optimizing array types:");

    if (global.config.cache1_size > 0) {

        /* init pad_info structure */
        PIinit ();

        /* open apdiag_file for output */
        if (global.apdiag) {
            apdiag_file = FMGRwriteOpen ("%s.ap", global.outfilename);

            fprintf (apdiag_file,
                     "     **************************************************\n"
                     "     *                                                *\n"
                     "     *        Array Padding Inference Report          *\n"
                     "     *                                                *\n"
                     "     **************************************************\n\n\n");
        }

        /* collect information for inference phase */
        APCdoCollect (arg_node);

        /* apply array padding */
        APinfer ();

        /* apply array padding */
#if 0
    APTdoTransform( arg_node);
#endif
        /* close apdiag_file */
        if (global.apdiag) {
            fclose (apdiag_file);
        }

        PInoteResults ();

        /* free pad_info structure */
        PIfree ();
    } else {
        /*
         * Padding is enabled but no cache specification is given.
         */
        if (global.apdiag) {
            apdiag_file = FMGRwriteOpen ("%s.ap", global.outfilename);

            fprintf (apdiag_file,
                     "     **************************************************\n"
                     "     *                                                *\n"
                     "     *        Array Padding Inference Report          *\n"
                     "     *                                                *\n"
                     "     **************************************************\n\n\n");
            fprintf (apdiag_file,
                     "     **************************************************\n"
                     "     *                                                *\n"
                     "     *  No cache specification  =>  No array padding  *\n"
                     "     *                                                *\n"
                     "     **************************************************\n\n\n");

            fclose (apdiag_file);
        }
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
