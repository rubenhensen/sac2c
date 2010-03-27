/*
 *
 * $Id$
 *
 */

#include "sactools.h"

#define LIBRARY "/lib/libsac2c.so"
#define MAINFUN "SACrunSac2tex"

int
main (int argc, char *argv[])
{
    int result;

    LAUNCHFUNCTIONFROMLIB (LIBRARY, MAINFUN, argc, argv, result);

    return (result);
}
