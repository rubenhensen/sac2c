/*
 * $Log$
 * Revision 2.1  1999/07/08 12:00:25  cg
 * initial revision
 *
 * Revision 1.7  1999/07/02 10:05:51  cg
 * Result presentation beautified.
 *
 * Revision 1.6  1999/06/11 12:56:58  cg
 * Analyser made compatible to substantial improvements in
 * cache simulation library.
 *
 * Revision 1.5  1999/06/10 09:51:20  cg
 * Added piped cache simulation on remote host machine.
 *
 * Revision 1.4  1999/05/20 14:14:37  cg
 * Input scanning facilities optimized.
 * All simulation parameters may now be set dynamically, including
 * global/blocked simulation.
 *
 *
 */

/*****************************************************************************
 *
 * file:   CacheSimAnalyser.c
 *
 * prefix: SAC_CS
 *
 * description:
 *
 *
 *
 *
 *
 *
 *****************************************************************************/

#include <stdio.h>
#include "sac_cachesim.h"
#include "sac_message.h"
#include "cachesim.h"

static void
AnalyserSetup (int argc, char *argv[])
{
    tProfilingLevel profilinglevel = SAC_CS_none;
    int cs_global = 1;
    char *cshost = "";
    char *csfile = "";
    char *csdir = "";

    unsigned long int cachesize1 = 0;
    int cachelinesize1 = 1;
    int associativity1 = 1;
    tWritePolicy writepolicy1 = SAC_CS_default;

    unsigned long int cachesize2 = 0;
    int cachelinesize2 = 1;
    int associativity2 = 1;
    tWritePolicy writepolicy2 = SAC_CS_default;

    unsigned long int cachesize3 = 0;
    int cachelinesize3 = 1;
    int associativity3 = 1;
    tWritePolicy writepolicy3 = SAC_CS_default;

    SAC_CS_CheckArguments (argc, argv, &profilinglevel, &cs_global, &cshost, &csfile,
                           &csdir, &cachesize1, &cachelinesize1, &associativity1,
                           &writepolicy1, &cachesize2, &cachelinesize2, &associativity2,
                           &writepolicy2, &cachesize3, &cachelinesize3, &associativity3,
                           &writepolicy3);

    /*
     * The CacheSimAnalyser must not get a profiling level
     * like SAC_CS_piped_X!!!
     */
    switch (profilinglevel) {
    case SAC_CS_piped_simple:
        profilinglevel = SAC_CS_simple;
        break;
    case SAC_CS_piped_advanced:
        profilinglevel = SAC_CS_advanced;
        break;
    case SAC_CS_file:
        SAC_RuntimeError ("Command line argument -cs f is illegal for external "
                          "cache simulator");
        break;
    default:
        break;
    }

    fprintf (stderr,
             "%s"
             "# Running external SAC cache simulation analyser:\n"
             "#   %s\n",
             SAC_CS_separator, argv[0]);

    SAC_CS_Initialize (1, profilinglevel, cs_global, cshost, csfile, csdir, cachesize1,
                       cachelinesize1, associativity1, writepolicy1, cachesize2,
                       cachelinesize2, associativity2, writepolicy2, cachesize3,
                       cachelinesize3, associativity3, writepolicy3);

} /* AnalyserSetup */

int
main (int argc, char *argv[])
{
    ULINT baseaddress, elemaddress;
    unsigned size;
    char op;
    FILE *in_stream = stdin;
    char tagbuffer[MAX_TAG_LENGTH];

    AnalyserSetup (argc, argv);

    op = getc (in_stream);

    while (!feof (stdin)) {
        switch (op) {
        case 'R':
            fscanf (in_stream, "%lx %lx\n", &baseaddress, &elemaddress);
            SAC_CS_ReadAccess ((void *)baseaddress, (void *)elemaddress);
            break;

        case 'W':
            fscanf (in_stream, "%lx %lx\n", &baseaddress, &elemaddress);
            SAC_CS_WriteAccess ((void *)baseaddress, (void *)elemaddress);
            break;

        case 'G':
            fscanf (in_stream, "%lx %u\n", &baseaddress, &size);
            SAC_CS_RegisterArray ((void *)baseaddress, size);
            break;

        case 'U':
            fscanf (in_stream, "%lx\n", &baseaddress);
            SAC_CS_UnregisterArray ((void *)baseaddress);
            break;

        case 'B':
            fscanf (in_stream, " %s\n", tagbuffer);
            SAC_CS_Start (tagbuffer);
            break;

        case 'E':
            fscanf (in_stream, " \n");
            SAC_CS_Stop ();
            break;

        case 'F':
            fscanf (in_stream, " \n");
            SAC_CS_Finalize ();
            break;

        default:
            fscanf (in_stream, "%s\n", tagbuffer);
            SAC_RuntimeError ("CacheSimAnalyser: unknown input: %c %s", op, tagbuffer);
        } /*switch */

        op = getc (in_stream);
    }
    return (0);
} /* main */
