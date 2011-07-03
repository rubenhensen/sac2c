/*
 *
 * $Id$
 *
 */

#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

#include "system.h"

#include "types.h" /* for bool */

#define DBUG_PREFIX "SYSCALL"
#include "debug.h"

#include "ctinfo.h"
#include "globals.h"
#include "memory.h"
#include "str.h"
#include "filemgr.h"

/**
 * buffer size for construction of system call strings
 *
 * STATIC BUFFERS SHOULD BE REPLACED BY USING str_buffer
 */

#define MAX_SYSCALL 10000

/**
 * global file handle for syscall tracking
 */
static FILE *syscalltrack = NULL;
static bool syscalltrack_active = FALSE;

/******************************************************************************
 *
 * Function:
 *   void SYSstartTracking( void)
 *
 * Description:
 *   Initiates tracking of system calls for -d gencccall option
 *
 ******************************************************************************/

void
SYSstartTracking (void)
{
    DBUG_ENTER ();

    DBUG_ASSERT (syscalltrack == NULL, "tracking has already been enabled!");

    if (syscalltrack_active) {
        syscalltrack = FMGRappendOpen ("%s.sac2c", global.outfilename);
    } else {
        CTInote ("Creating cc call shell script `%s.sac2c'", global.outfilename);
        syscalltrack = FMGRwriteOpenExecutable ("%s.sac2c", global.outfilename);
        fprintf (syscalltrack, "#! /bin/sh\n\n");
    }

    syscalltrack_active = TRUE;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void SYSstopTracking( void)
 *
 * Description:
 *   Stops tracking of system calls for -d gencccall option
 *
 ******************************************************************************/

void
SYSstopTracking (void)
{
    DBUG_ENTER ();

    DBUG_ASSERT (syscalltrack != NULL, "no tracking log open!");

    fclose (syscalltrack);

    syscalltrack = NULL;

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void TrackSystemCall( const char *call)
 *
 * Description:
 *   Reports system call to file
 *
 ******************************************************************************/

static void
TrackSystemCall (const char *call)
{
    DBUG_ENTER ();

    if (syscalltrack != NULL) {
        fprintf (syscalltrack, "%s\n\n", call);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   void SYScall( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
 *
 ******************************************************************************/

void
SYScall (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];
    int exit_code;

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

    /* if -d syscall flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        CTInote ("System call:\n %s", syscall);
    }

    TrackSystemCall (syscall);
    exit_code = system (syscall);

    if (exit_code == -1) {
        CTIabort ("System failure while trying to execute shell command.\n"
                  "(e.g. out of memory).");
    } else if (WEXITSTATUS (exit_code) > 0) {
        CTIabort ("System failed to execute shell command\n%s\n"
                  "with exit code %d",
                  syscall, WEXITSTATUS (exit_code));
    } else if (WIFSIGNALED (exit_code)) {
        if (WTERMSIG (exit_code) == SIGINT) {
            CTIabort ("Child recieved SIGINT when executing shell command \n%s\n",
                      syscall);
        } else if (WTERMSIG (exit_code) == SIGQUIT) {
            CTIabort ("Child recieved SIGQUIT when executing shell command \n%s\n",
                      syscall);
        }
    } else if (exit_code != 0) {
        CTIabort ("Unknown failure while executing shell command \n%s\n"
                  "Return value was %d",
                  syscall, exit_code);
    }

    DBUG_RETURN ();
}

/******************************************************************************
 *
 * Function:
 *   int SYScallNoErr( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   In contrast to SYScall() no error message is printed upon failure but
 *   the exit code is returned.
 *
 ******************************************************************************/

int
SYScallNoErr (char *format, ...)
{
    va_list arg_p;
    static char syscall[MAX_SYSCALL];

    DBUG_ENTER ();

    va_start (arg_p, format);
    vsprintf (syscall, format, arg_p);
    va_end (arg_p);

    /* if -dnocleanup flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        CTInote ("System call:\n%s", syscall);
    }

    TrackSystemCall (syscall);

    DBUG_RETURN (system (syscall));
}

/******************************************************************************
 *
 * Function:
 *   int SYStest( char *format, ...)
 *
 * Description:
 *   Evaluates the given string and executes the respective system call.
 *   If the system call fails, an error message occurs and compilation is
 *   aborted.
 *
 ******************************************************************************/

int
SYStest (char *format, ...)
{
    va_list arg_p;
    static char buffer[MAX_SYSCALL];
    int exit_code;
    char *extended_format;

    DBUG_ENTER ();

    extended_format = STRcat ("test ", format);

    va_start (arg_p, format);
    vsprintf (buffer, extended_format, arg_p);
    va_end (arg_p);

    exit_code = system (buffer);

    if (exit_code == 0) {
        exit_code = 1;
    } else {
        exit_code = 0;
    }

    extended_format = MEMfree (extended_format);

    DBUG_PRINT ("test returns %d", exit_code);

    DBUG_RETURN (exit_code);
}

#undef DBUG_PREFIX
