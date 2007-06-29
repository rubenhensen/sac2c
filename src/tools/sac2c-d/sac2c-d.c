/*
 *
 * $Id$
 *
 */

#include "sactools.h"

#define LIBRARY "/lib/libsac2c.d.so"
#define MAINFUN "SACrunSac2c"

int
main (int argc, char *argv[])
{
    int result;

    LAUNCHFUNCTIONFROMLIB (LIBRARY, MAINFUN, argc, argv, result);

    return (result);
}