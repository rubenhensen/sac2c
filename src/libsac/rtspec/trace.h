/** <!--*******************************************************************-->
 *
 * @file trace.h
 *
 * @brief  Contains tracing helper code
 *
 * @author  hmw
 *
 ****************************************************************************/

#ifndef _SAC_RTSPEC_TRACE_H_
#define _SAC_RTSPEC_TRACE_H_

#include "libsac/essentials/trace.h" // SAC_TR_Print

#define SAC_RTSPEC_TR_Print(...)                                                         \
    {                                                                                    \
        if (do_trace == 1) {                                                             \
            SAC_TR_Print (__VA_ARGS__);                                                  \
        }                                                                                \
    }

#endif /* _SAC_RTSPEC_TRACE_H_ */
