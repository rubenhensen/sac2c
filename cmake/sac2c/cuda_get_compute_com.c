/**
 *  The purpose here is to determine what the compute capabilities are
 *  of a CUDA device for use by the CUDA-backend in SAC.
 *
 *  If we are unable to connect to the device (assuming one is there),
 *  we should error out with a clear message.
 *
 *  Newlines are omitted as this interferes with CMakes message printing
 *  facility.
 */
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
static inline void
__cuda_error (cudaError_t error)
{
    if (error != cudaSuccess) {
        fprintf (stderr, "CUDA_ERROR: %s `%s'", cudaGetErrorName (error),
                 cudaGetErrorString (error));
        exit (error);
    }
}
int
main ()
{
    int cuda_device_id;
    struct cudaDeviceProp cuda_device_props;
    __cuda_error (cudaGetDevice (&cuda_device_id));
    __cuda_error (cudaGetDeviceProperties (&cuda_device_props, cuda_device_id));
    printf ("sm_%d%d", cuda_device_props.major, cuda_device_props.minor);
    return EXIT_SUCCESS;
}
