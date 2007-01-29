/*
 *
 * $Log$
 * Revision 3.4  2005/01/11 13:32:21  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.3  2004/11/27 02:51:24  mwe
 * APTdoTransform deactivated
 *
 * Revision 3.2  2004/11/26 20:36:07  jhb
 * compile
 *
 * Revision 3.1  2000/11/20 18:01:47  sacbase
 * new release made
 *
 * Revision 1.14  2000/10/31 18:13:27  cg
 * If no cache specification is available, array padding is now
 * skipped at a reasonably high level.
 *
 * Revision 1.13  2000/10/27 13:39:54  cg
 * Added correction factor for -apdiaglimit option.
 *
 * Revision 1.12  2000/10/27 13:24:56  cg
 * Modified function APprintDiag() in order to support new command
 * line option -apdiaglimit.
 *
 * Revision 1.11  2000/08/08 11:49:33  dkr
 * DBUG_PRINT added
 *
 * Revision 1.10  2000/08/03 15:29:49  mab
 * added apdiag_file, APprintDiag
 * removed all dummies
 *
 * Revision 1.8  2000/07/19 12:39:28  mab
 * added dummy values for testing purposes
 *
 * Revision 1.6  2000/07/05 09:12:34  mab
 * fixed memory problem
 *
 * Revision 1.5  2000/06/14 10:41:31  mab
 * comments added
 *
 * Revision 1.4  2000/06/08 11:14:49  mab
 * pad_info added
 *
 * Revision 1.3  2000/05/31 16:16:58  mab
 * initial version
 *
 * Revision 1.2  2000/05/26 14:24:29  sbs
 * dummy function ArrayPadding added.
 *
 * Revision 1.1  2000/05/26 13:41:35  sbs
 * Initial revision
 *
 */

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

#include "dbug.h"

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
  
  DBUG_ENTER("APprintDiag");

  if (apdiag) {
    va_start( arg_p, format);
    vsprintf( buffer, format, arg_p);
    va_end( arg_p);

    fprintf( apdiag_file, buffer);
  }

  DBUG_VOID_RETURN;
}

#else

void
APprintDiag (char *format, ...)
{
    va_list arg_p;
    static int cnt = 1;
    int correction = 1;

    DBUG_ENTER ("APprintDiag");

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

    DBUG_VOID_RETURN;
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
    DBUG_ENTER ("ArrayPadding");

    DBUG_PRINT ("OPT", ("ARRAY PADDING"));
    DBUG_PRINT ("AP", ("Entering Array Padding"));

    CTInote ("Optimizing array types:");

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
