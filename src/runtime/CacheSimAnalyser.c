#include <stdio.h>
#include "sac_cachesim.h"
#include "libsac_cachesim.h"

char buffer[255];

int
main ()
{
    int nr_of_cpu, profilinglevel;
    ULINT cachesize1, cachesize2, cachesize3;
    int cachelinesize1, cachelinesize2, cachelinesize3, associativity1, associativity2,
      associativity3;
    int writepolicy1, writepolicy2, writepolicy3;
    ULINT baseaddress, elemaddress;
    unsigned size;
    char tag[255];

    while (fgets (buffer, 255, stdin) != 0) {
        /*printf("r: %s", buffer);*/
        switch (buffer[0]) {
        case 'I':
            sscanf (buffer,
                    "I %d %d "
                    "%lu %d %d %d "
                    "%lu %d %d %d "
                    "%lu %d %d %d\n",
                    &nr_of_cpu, &profilinglevel, &cachesize1, &cachelinesize1,
                    &associativity1, &writepolicy1, &cachesize2, &cachelinesize2,
                    &associativity2, &writepolicy2, &cachesize3, &cachelinesize3,
                    &associativity3, &writepolicy3);

            SAC_CS_Initialize (nr_of_cpu, profilinglevel, cachesize1, cachelinesize1,
                               associativity1, writepolicy1, cachesize2, cachelinesize2,
                               associativity2, writepolicy2, cachesize3, cachelinesize3,
                               associativity3, writepolicy3);
            break;

        case 'F':
            SAC_CS_Finalize ();
            break;

        case 'G':
            sscanf (buffer, "G %lx %u\n", &baseaddress, &size);
            SAC_CS_RegisterArray ((void *)baseaddress, size);
            break;

        case 'U':
            sscanf (buffer, "U %lx\n", &baseaddress);
            SAC_CS_UnregisterArray ((void *)baseaddress);
            break;

        case 'R':
            sscanf (buffer, "R %lx %lx\n", &baseaddress, &elemaddress);
            SAC_CS_ReadAccess ((void *)baseaddress, (void *)elemaddress);
            break;

        case 'W':
            sscanf (buffer, "W %lx %lx\n", &baseaddress, &elemaddress);
            SAC_CS_WriteAccess ((void *)baseaddress, (void *)elemaddress);
            break;

        case 'B':
            strcpy (tag, &(buffer[2]));
            SAC_CS_Start (tag);
            break;

        case 'E':
            SAC_CS_Stop ();
            break;

        default:
            fprintf (stderr, "CacheSimAnalyser: unknown input - '%s'\n", buffer);
            exit (1);
            break;
        } /*switch */
    }
    return (0);
}
