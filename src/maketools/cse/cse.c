#include <sys/time.h>
#include <utime.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "getoptions.h"

/*****************************************************************************
 *
 *  function:
 *    void Usage()
 *
 *  description:
 *    prints usage info to stdout.
 */

#define HEADER "   "
void
Usage (void)
{
    printf ("cse file [file]*\n");
    printf (HEADER "-h              print this help mesage.\n");
    printf (HEADER "-d delay        set the time of the file <delay> secs earlier!\n");
}

/*****************************************************************************
 *
 *  function:
 *    void fool( char* file)
 *
 *  description:
 *    updates the file's access time to the actual time minus 2secs.
 */

void
fool (char *file, int delay)
{
    time_t sti;
    struct utimbuf uti;

    sti = time (NULL);
    uti.actime = sti - delay;
    uti.modtime = sti - delay;
    utime (file, &uti);
}

int
main (int argc, char *argv[])
{
    int delay = 0;

    ARGS_BEGIN (argc, argv);
    ARGS_FLAG ("h", Usage (); exit (0));
    ARGS_OPTION ("d", ARG_NUM (delay));

    ARGS_ARGUMENT ({ fool (ARG, delay); });
    ARGS_END ();

    return (0);
}
