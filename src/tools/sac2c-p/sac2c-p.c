#include "sac.h"
#include "sactools.h"

#define LIBRARY "libsac2c.p" SHARED_LIB_EXT
#define MAINFUN "SACrunSac2c"

int
main (int argc, char *argv[])
{
    return launch_function_from_library (LIBRARY, MAINFUN, argc, argv);
}
