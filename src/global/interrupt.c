/*
 *
 * $Log$
 * Revision 1.1  1999/01/25 16:24:30  sbs
 * Initial revision
 *
 *
 *
 */

#include <signal.h>
#include <stdio.h>

#include "Error.h"
#include "interrupt.h"

/******************************************************************************
 *
 * function: void void CompilerErrorBreak( int sig)
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
    /* should we use SYSERROR from Error.h here?? */
    fprintf (stderr, "\n\nUUUPS your program crashed the compiler 8-((\n");
    fprintf (stderr, "Please send a bug report to sacbase@informatik.uni-kiel.de.\n\n");
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
