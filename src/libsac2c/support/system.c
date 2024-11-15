/**
 * @file
 *
 * @brief Call system command conviently
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
#include "str_buffer.h"
#include "filemgr.h"

/**
 * buffer size for construction of system call strings
 */

#define MAX_SYSCALL 10000

/**
 * global file handle for syscall tracking
 */
static FILE *syscalltrack = NULL;
static bool syscalltrack_active = FALSE;

/**
 * @brief Initiates tracking of system calls for -d gencccall option
 */
void
SYSstartTracking (void)
{
    DBUG_ENTER ();

    DBUG_ASSERT (syscalltrack == NULL, "tracking has already been enabled!");

    if (syscalltrack_active) {
        syscalltrack = FMGRappendOpen ("%s.sac2c", global.outfilename);
    } else {
        CTInote (EMPTY_LOC, "Creating cc call shell script `%s.sac2c'", global.outfilename);
        syscalltrack = FMGRwriteOpenExecutable ("%s.sac2c", global.outfilename);
        fprintf (syscalltrack, "#! /bin/sh\n\n");
    }

    syscalltrack_active = TRUE;

    DBUG_RETURN ();
}

 /**
  * @briefStops tracking of system calls for -d gencccall option
 */
void
SYSstopTracking (void)
{
    DBUG_ENTER ();

    DBUG_ASSERT (syscalltrack != NULL, "no tracking log open!");

    fclose (syscalltrack);

    syscalltrack = NULL;

    DBUG_RETURN ();
}

/**
 * @brief Santize path, removing blanks characters front and back, and
 *        escaping spaces.
 *
 * @param p path
 * @return new string
 */
char *
SYSsanitizePath (const char *p)
{
    char *tmp, *res = NULL;
    DBUG_ENTER ();

    if (p != NULL)
    {
        tmp = STRcpy (p);
        tmp = STRstrip (tmp);

        // escape all spaces
        res = STRsubstToken (tmp, " ", "\\ ");
        MEMfree (tmp);
    }

    DBUG_RETURN (res);
}

 /**
  * @brief Reports system call to file
  *
  * @param call call to print
  */
static void
TrackSystemCall (const char *call)
{
    DBUG_ENTER ();

    if (syscalltrack != NULL) {
        fprintf (syscalltrack, "%s\n\n", call);
        fflush (syscalltrack);
    }

    DBUG_RETURN ();
}

/**
 * @brief Evaluates the given string and executes the respective system call.
 *        If the system call fails, an error message occurs and compilation is
 *        aborted.
 *
 * @param format string format
 * @param ... format inputs
 */
void
SYScall (char *format, ...)
{
    va_list arg_p;
    str_buf *syscall;
    int exit_code;

    DBUG_ENTER ();

    syscall = SBUFcreate (MAX_SYSCALL);

    va_start (arg_p, format);
    syscall = SBUFvprintf (syscall, format, arg_p);
    va_end (arg_p);

    /* if -d syscall flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        CTItell (0, "System call:\n %s", SBUF2str (syscall));
    }

    char *syscall_str = SBUF2str (syscall);
    TrackSystemCall (syscall_str);
    exit_code = system (syscall_str);
    MEMfree (syscall_str);

    if (exit_code == -1) {
        CTIabort (EMPTY_LOC, 
                  "System failure while trying to execute shell command.\n"
                  "(e.g. out of memory).");
    } else if (WEXITSTATUS (exit_code) > 0) {
        CTIabort (EMPTY_LOC, 
                  "System failed to execute shell command\n%s\n"
                  "with exit code %d",
                  SBUF2strAndFree (&syscall), WEXITSTATUS (exit_code));
    } else if (WIFSIGNALED (exit_code)) {
        if (WTERMSIG (exit_code) == SIGINT) {
            CTIabort (EMPTY_LOC, 
                      "Child recieved SIGINT when executing shell command \n%s\n",
                      SBUF2strAndFree (&syscall));
        } else if (WTERMSIG (exit_code) == SIGQUIT) {
            CTIabort (EMPTY_LOC, 
                      "Child recieved SIGQUIT when executing shell command \n%s\n",
                      SBUF2strAndFree (&syscall));
        }
    } else if (exit_code != 0) {
        CTIabort (EMPTY_LOC, 
                  "Unknown failure while executing shell command \n%s\n"
                  "Return value was %d",
                  SBUF2strAndFree (&syscall), exit_code);
    } else {
        SBUFfree (syscall);
    }

    DBUG_RETURN ();
}

/**
 * @brief Evaluates the given string and executes the respective system call.
 *        In contrast to SYScall() no error message is printed upon failure but
 *        the exit code is returned.
 *
 * @param format string format
 * @param ... format inputs
 */
int
SYScallNoErr (char *format, ...)
{
    va_list arg_p;
    str_buf *syscall;

    DBUG_ENTER ();

    syscall = SBUFcreate (MAX_SYSCALL);

    va_start (arg_p, format);
    syscall = SBUFvprintf (syscall, format, arg_p);
    va_end (arg_p);

    /* if -dnocleanup flag is set print all syscalls !
     * This allows for easy C-code patches.
     */
    if (global.show_syscall) {
        CTItell (0, "System call:\n%s", SBUF2str (syscall));
    }

    TrackSystemCall (SBUF2str (syscall));

    DBUG_RETURN (system (SBUF2strAndFree (&syscall)));
}

/**
 * @brief Evaluates the given string and executes the respective system call.
 *        If the system call fails, an error message occurs and compilation is
 *        aborted.
 *
 * @param format string format
 * @param ... format inputs
 * @return status code of syscall
 */
int
SYStest (char *format, ...)
{
    va_list arg_p;
    str_buf *buffer;
    int exit_code;
    char *extended_format;

    DBUG_ENTER ();

    extended_format = STRcat ("test ", format);
    buffer = SBUFcreate (MAX_SYSCALL);

    va_start (arg_p, format);
    buffer = SBUFvprintf (buffer, extended_format, arg_p);
    va_end (arg_p);

    exit_code = system (SBUF2str (buffer));

    if (exit_code == 0) {
        exit_code = 1;
    } else {
        exit_code = 0;
    }

    extended_format = MEMfree (extended_format);
    buffer = SBUFfree (buffer);

    DBUG_PRINT ("test returns %d", exit_code);

    DBUG_RETURN (exit_code);
}

/**
 * @brief evaluate command and capture the output
 *
 * @param cmd command to be run
 * @param out command output
 * @return exit status of command
 */
int
SYSexec_and_read_output (char *cmd, char **out)
{
    int ret;
    str_buf *outp;
    char buffer[1024] = "";

    FILE *f = popen (cmd, "r");
    if (!f) {
        perror ("popen");
        CTIabort (EMPTY_LOC, "system call '%s' failed", cmd);
    }

    outp = SBUFcreate (1024);

    while (fread (buffer, sizeof (char), 1024, f) > 0) {
       SBUFprint (outp, buffer);
    }

    *out = SBUF2strAndFree (&outp);
    ret = pclose (f);
    return ret;
}

#undef DBUG_PREFIX
