/**
 * @file
 * @brief Specifies enum-equvilant values for which CUDA synchronisation method
 *        to use when using any asynchronise transfer mechanism.
 *
 * @see types.h for reference details on enum values
 */
#ifndef _SAC_RUNTIME_CUDA_SYNC_H_
#define _SAC_RUNTIME_CUDA_SYNC_H_

#define SAC_CS_none 0
#define SAC_CS_device 1
#define SAC_CS_stream 2
#define SAC_CS_callback 3

#endif /* _SAC_RUNTIME_CUDA_SYNC_H_ */
