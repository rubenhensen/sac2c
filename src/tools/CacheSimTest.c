/*
 * $Log$
 * Revision 3.1  2000/11/20 18:02:49  sacbase
 * new release made
 *
 * Revision 1.4  2000/01/27 11:11:27  her
 * removed the suscicious pointer arithmetic on a void*.
 *
 * Revision 1.3  1999/07/08 12:01:31  cg
 * File moved to new directory src/tools
 *
 * Revision 1.2  1999/04/26 09:32:28  her
 * fixed some things
 *
 * Revision 1.1  1999/03/19 11:02:43  her
 * Initial revision
 *
 */

#include <stdio.h>
#include "sac_cachesim.h"

int
main ()
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
