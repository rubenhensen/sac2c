
// utilities and system includes
//#include <shrUtils.h>

#include <stdio.h>

// CUDA-C includes
#include <cuda_runtime_api.h>

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int main( int argc, const char** argv) 
{
  int deviceCount = 0;
  if (cudaGetDeviceCount(&deviceCount) != cudaSuccess) {
    printf("cudaGetDeviceCount FAILED CUDA Driver and Runtime version may be mismatched.\n");
    return(1);
  }

  // This function call returns 0 if there are no CUDA capable devices.
  if (deviceCount == 0) {
    // There is no device supporting CUDA
    printf("");
    return(1);
  }

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, 0);

  if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
    // There is no device supporting CUDA
    printf("");
    return(1);
  } 

  printf("-arch=sm_%d\n",deviceProp.major*10+deviceProp.minor);

  return( 0);
}

