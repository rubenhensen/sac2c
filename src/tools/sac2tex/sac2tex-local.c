#include "sac.h"
#include "sactools.h"

#define LIBRARY "libsac2c-local" SHARED_LIB_EXT
#define MAINFUN "SACrunSac2texLocal"

int
main (int argc, char *argv[])
{
    return launch_function_from_library (LIBRARY, MAINFUN, TRUE, argc, argv);
}
