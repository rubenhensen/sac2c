/*
 *
 * $Log$
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

    DBUG_ENTER ("APprintDiag");

    if (apdiag && (cnt <= 3 * apdiag_limit)) {
        va_start (arg_p, format);
        vfprintf (apdiag_file, format, arg_p);
        va_end (arg_p);
        cnt++;
        if (cnt > apdiag_limit) {
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
                     apdiag_limit);
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

    NOTE ((""));
    NOTE (("padding array types:"));

    /* init pad_info structure */
    PIinit ();

    DBUG_PRINT ("AP", ("Entering Array Padding"));

    /* open apdiag_file for output */
    if (apdiag) {
        apdiag_file = WriteOpen ("%s.ap", outfilename);

        fprintf (apdiag_file,
                 "     **************************************************\n"
                 "     *                                                *\n"
                 "     *        Array Padding Inference Report          *\n"
                 "     *                                                *\n"
                 "     **************************************************\n\n\n");
    }

    /* collect information for inference phase */
    APcollect (arg_node);

    /* apply array padding */
    APinfer ();

    /* apply array padding */
    APtransform (arg_node);

    /* close apdiag_file */
    if (apdiag) {
        fclose (apdiag_file);
    }

    PInoteResults ();

    /* free pad_info structure */
    PIfree ();

    DBUG_RETURN (arg_node);
}
