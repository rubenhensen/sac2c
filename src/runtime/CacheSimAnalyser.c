#include <stdio.h>
#include "sac_cachesim.h"
#include "libsac_cachesim.h"

char buffer[255];

static void
AnalyserSetup (int argc, char *argv[])
{
    tProfilingLevel profilinglevel = SAC_CS_LEVEL;
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

    SAC_CS_CheckArguments (argc, argv, &profilinglevel, &cachesize1, &cachelinesize1,
                           &associativity1, &writepolicy1, &cachesize2, &cachelinesize2,
                           &associativity2, &writepolicy2, &cachesize3, &cachelinesize3,
                           &associativity3, &writepolicy3);

    /* The CacheSimAnalyser must not get an profilinglevel
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
        profilinglevel = SAC_CS_default;
        break;
    default:
        break;
    }

    SAC_CS_Initialize (1, profilinglevel, cachesize1, cachelinesize1, associativity1,
                       writepolicy1, cachesize2, cachelinesize2, associativity2,
                       writepolicy2, cachesize3, cachelinesize3, associativity3,
                       writepolicy3);

    SAC_CS_START_GLOBAL ();
} /* AnalyserSetup */

int
main (int argc, char *argv[])
{
    ULINT baseaddress, elemaddress;
    unsigned size;
    char tag[255];

    AnalyserSetup (argc, argv);

    while (fgets (buffer, 255, stdin) != 0) {
        /*printf("r: %s", buffer);*/
        switch (buffer[0]) {
        case 'R':
            sscanf (buffer, "R %lx %lx\n", &baseaddress, &elemaddress);
            SAC_CS_ReadAccess ((void *)baseaddress, (void *)elemaddress);
            break;

        case 'W':
            sscanf (buffer, "W %lx %lx\n", &baseaddress, &elemaddress);
            SAC_CS_WriteAccess ((void *)baseaddress, (void *)elemaddress);
            break;

        case 'G':
            sscanf (buffer, "G %lx %u\n", &baseaddress, &size);
            SAC_CS_RegisterArray ((void *)baseaddress, size);
            break;

        case 'U':
            sscanf (buffer, "U %lx\n", &baseaddress);
            SAC_CS_UnregisterArray ((void *)baseaddress);
            break;

        case 'B':
            strcpy (tag, &(buffer[2]));
            SAC_CS_Start (tag);
            break;

        case 'E':
            SAC_CS_Stop ();
            break;

        case 'F':
            SAC_CS_Finalize ();
            break;

        default:
            fprintf (stderr, "CacheSimAnalyser: unknown input - %s", buffer);
            exit (1);
            break;
        } /*switch */
    }
    return (0);
} /* main */
