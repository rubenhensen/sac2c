/*
 *
 * $Log$
 * Revision 3.3  2003/07/28 15:35:06  cg
 * Changed quoted mail addresses to sac-home.org.
 *
 * Revision 3.2  2001/11/13 19:35:36  dkr
 * *** empty log message ***
 *
 * Revision 3.1  2000/11/20 17:59:31  sacbase
 * new release made
 *
 * Revision 2.5  2000/03/23 14:00:28  jhs
 * Added build_os.
 *
 * Revision 2.4  1999/11/16 11:55:31  sbs
 * translated UUUPS into proper English 8-)
 *
 * Revision 2.3  1999/10/22 11:09:13  dkr
 * CompilerErrorBreak() finished by an explizit exit() now
 *
 * Revision 2.2  1999/05/12 14:30:40  cg
 * converted to new build variables, included from build.h
 *
 * Revision 2.1  1999/02/23 12:39:25  sacbase
 * new release made
 *
 * Revision 1.2  1999/02/15 13:34:51  sbs
 * added generation of SACbugreport
 *
 * Revision 1.1  1999/01/25 16:24:30  sbs
 * Initial revision
 *
 */

#include <signal.h>
#include <stdio.h>

#include "Error.h"
#include "interrupt.h"
#include "internal_lib.h"
#include "build.h"
#include "globals.h"

/******************************************************************************
 *
 * function: void CompilerErrorBreak( int sig)
 *
 * description: Interrupt handler for breaks that "should not occur" 8-))
 *   Here, we do not only clean up all temporarily created files/ directories,
 *   but we generate an "error message file" accompanied by a message that
 *   asks the user to send that file to sacbase....
 *
 ******************************************************************************/

void
CompilerErrorBreak (int sig)
{
    FILE *error_file;

    /* should we use SYSERROR from Error.h here?? */
    fprintf (stderr, "\n\nOOOPS your program crashed the compiler 8-((\n");
    fprintf (stderr, "Please send a bug report to bug@sac-home.org.\n\n");
    fprintf (stderr,
             "For your convenience, the compiler pre-fabricated a bug report in\n"
             "the file \"SACbugreport\" which was created in the current directory!\n"
             "Besides some infos concerning the compiler version and its\n"
             "usage it contains the specified source file.\n"
             "If you want to send that bug report to us you may simply use\n\n"
             "  mail bug@sac-home.org < SACbugreport\n\n");

    error_file = fopen ("SACbugreport", "w");
    if (error_file != NULL) {
        fprintf (error_file, "/*\n"
                             " * SAC - bug report\n"
                             " * ================\n"
                             " *\n"
                             " * automatically generated on ");
        fclose (error_file);
        SystemCall2 ("date >> SACbugreport");
        error_file = fopen ("SACbugreport", "a");

        fprintf (error_file, " *\n");
        fprintf (error_file, " * using sac2c %s for %s\n", version_id, target_platform);
        fprintf (error_file, " * built %s.\n", build_date);
        fprintf (error_file, " * by user %s on host %s for %s.\n", build_user, build_host,
                 build_os);
        fprintf (error_file, " *\n");
        if (commandline[0] != '\0') {
            fprintf (error_file, " * The compiler was called by\n");
            fprintf (error_file, " * %s\n", commandline);
        } else {
            fprintf (error_file,
                     " * Compiler crashed before the commandline has been examined!\n");
        }
        fprintf (error_file, " *\n");
        if (sacfilename[0] != '\0') {
            fprintf (error_file, " * The contents of %s is:\n", sacfilename);
            fprintf (error_file, " */\n\n");
            fclose (error_file);
            SystemCall2 ("cat %s >> SACbugreport", sacfilename);
        } else {
            fprintf (error_file,
                     " * Compiler crashed before sacfilename could be determined!\n");
            fprintf (error_file, " */\n\n");
            fclose (error_file);
        }
    }
    EXIT (0);
}

/******************************************************************************
 *
 * function: void UserForcedBreak( int sig)
 *
 * description: Interrupt handler for breaks that are due to user interaction.
 *   Basically, we only clean up any files/directories which have been
 *   temporarily created.
 *
 ******************************************************************************/

void
UserForcedBreak (int sig)
{
    EXIT (0);
}

/******************************************************************************
 *
 * function: void SetupInterruptHandlers()
 *
 * description: installs new signal handlers for some interrupts.
 *
 ******************************************************************************/

void
SetupInterruptHandlers ()
{
    signal (SIGSEGV, CompilerErrorBreak); /* Segmentation Fault */
    signal (SIGBUS, CompilerErrorBreak);  /* Bus Error */
    signal (SIGINT, UserForcedBreak);     /* Interrupt (Control-C) */
}
