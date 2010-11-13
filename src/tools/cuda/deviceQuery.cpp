
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
    printf("There is no device supporting CUDA\n");
  }

  cudaDeviceProp deviceProp;
  cudaGetDeviceProperties(&deviceProp, 0);

  if (deviceProp.major == 9999 && deviceProp.minor == 9999) {
    printf("There is no device supporting CUDA.\n");
    return(1);
  } 

//  printf("  CUDA Capability Major revision number:         %d\n", deviceProp.major);
//  printf("  CUDA Capability Minor revision number:         %d\n", deviceProp.minor);

  printf("%d\n",deviceProp.major*100+deviceProp.minor*10);

  return( 0);
}

