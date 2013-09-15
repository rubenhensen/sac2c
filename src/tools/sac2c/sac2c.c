#include "sac.h"
#include "sactools.h"

#define LIBRARY "/lib/libsac2c" SHARED_LIB_EXT
#define MAINFUN "SACrunSac2c"

int
main (int argc, char *argv[])
{
    int result = 0;

    LAUNCHFUNCTIONFROMLIB (LIBRARY, MAINFUN, argc, argv, result);

    return (result);
}
