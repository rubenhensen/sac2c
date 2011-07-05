/*****************************************************************************
 *
 * file:   rc_methods.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides type and macro definitions for the supported rc methods
 *
 *****************************************************************************/

#ifndef _SAC_RC_METHODS_H
#define _SAC_RC_METHODS_H

#define SAC_RCM_local 0
#define SAC_RCM_norc 1
#define SAC_RCM_async 2
#define SAC_RCM_local_norc_desc 3
#define SAC_RCM_local_norc_ptr 4
#define SAC_RCM_async_norc_copy_desc 5
#define SAC_RCM_async_norc_two_descs 6
#define SAC_RCM_async_norc_ptr 7
#define SAC_RCM_local_pasync_norc_desc 8
#define SAC_RCM_local_async_norc_ptr 9

#endif /* _SAC_RC_METHODS_H */
