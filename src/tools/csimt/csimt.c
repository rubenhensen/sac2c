#include <stdio.h>

#include "sac.h"

int
main (void)
{
    int i;
    long base;

    SAC_CS_Initialize (1, SAC_CS_advanced, 1, "dali", "cstest.cs", ".", 16, 16, 4,
                       SAC_CS_fetch_on_write, 256, 16, 1, SAC_CS_fetch_on_write, 1024, 32,
                       1, SAC_CS_fetch_on_write);
    /* aligned(12345)=12336 */
    SAC_CS_RegisterArray ((void *)12345, 1024 * 1024);
    SAC_CS_RegisterArray ((void *)12345678, 1024 * 1024);
    /*
    SAC_CS_RegisterArray((void*)23456789, 1024*1024);
    SAC_CS_RegisterArray((void*)34567890, 1024*1024);
    */

    SAC_CS_Start ("linear Read");

    base = 12345;
    for (i = 0; i < 1024 * 512; i++) {
        SAC_CS_ReadAccess ((void *)base, (void *)(base + i));
    }

    base = 12345;
    for (i = 0; i < 1024 * 512; i++) {
        SAC_CS_ReadAccess ((void *)base, (void *)(base + i));
    }
    /*
    base = 12345678;
    for(i=0; i<1024*1024; i++) {
      SAC_CS_ReadAccess((void*)base, (void*)(base+i));
    }

    base = 12345;
    for(i=0; i<1024*1024; i++) {
      SAC_CS_ReadAccess((void*)base, (void*)(base+i));
    }
    */

    SAC_CS_Stop ();

    SAC_CS_UnregisterArray ((void *)12345);
    SAC_CS_UnregisterArray ((void *)12345678);
    SAC_CS_UnregisterArray ((void *)23456789);
    SAC_CS_UnregisterArray ((void *)34567890);

    SAC_CS_Finalize ();

    return (0);
}
