
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
    fprintf(stderr, "cudaGetDeviceCount FAILED CUDA Driver and Runtime version may be mismatched.\n");
    return(1);
  }

  if (deviceCount == 0) {
    // There is no device supporting CUDA
    fprintf(stderr, "");
    return(2);
  }

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, 0);

  if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
    // There is no device supporting CUDA
    fprintf(stderr, "");
    return(3);
  } 

  printf("-arch=sm_%d\n",deviceProp.major*10+deviceProp.minor);

  return( 0);
}

