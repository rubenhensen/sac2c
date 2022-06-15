#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

/*******************************************************************************
 *
 *  program for detecting clock differences between actual machine and the file
 *  server. If compiled with VERBOSE being defined, the data read will be
 *  printed to stdout verbosely, otherwise, a non-negative integer number
 *  bigger than the actual clock-screw will be printed only.
 *
 */

#ifdef VERBOSE
void
print_time (char *msg, time_t tp)
{
    printf ("%s: %ld\n", msg, (long)tp);
}
#endif

int
main (void)
{
    struct stat *buf;
    long int clock_screw;
    time_t sti;
    FILE *f;

    buf = (struct stat *)malloc (sizeof (struct stat));

    f = fopen (".clock_screw_detection", "w");
    fprintf (f, "x");

    stat (".clock_screw_detection", buf);
    sti = time (NULL);

    fclose (f);
    unlink (".clock_screw_detection");

#ifdef VERBOSE
    print_time ("last access", buf->st_atime);
    print_time ("last data modification", buf->st_mtime);
    print_time ("last status change", buf->st_ctime);
    print_time ("actual system time", sti);
#endif

    clock_screw = buf->st_mtime - sti;

#ifdef VERBOSE
    if (clock_screw > 0) {
        printf ("clock screw by %d secs detected!\n", clock_screw);
    } else {
        printf ("no clock screw detected; file server is %d secs behind!\n",
                -clock_screw);
    }
#else
    if (clock_screw > 0) {
        printf ("%ld\n", clock_screw + 1);
    } else {
        printf ("0\n");
    }
#endif

    return (0);
}
