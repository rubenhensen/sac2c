#include <sys/time.h>
#include <utime.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "main_args.h"

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
Usage ()
{
    printf ("cse file [file]*\n");
    printf (HEADER "-h              print this help mesage.\n");
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
fool (char *file)
{
    time_t sti;
    struct utimbuf uti;

    sti = time (NULL);
    uti.actime = sti - 2;
    uti.modtime = sti - 2;
    utime (file, &uti);
}

int
main (int argc, char *argv[])
{
    ARGS_BEGIN (argc, argv);
    ARGS_FLAG ("h", Usage (); exit (0));
    ARGS_ARGUMENT ({ fool (ARG); });
    ARGS_END ();

    return (0);
}
