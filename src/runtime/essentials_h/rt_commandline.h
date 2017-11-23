/*****************************************************************************
 *
 * file:   sac_commandline.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides definitions for ICMs to access the commandline.
 *
 *****************************************************************************/

#ifndef _SAC_RT_COMMANDLINE_H_
#define _SAC_RT_COMMANDLINE_H_

#include "libsac/essentials/commandline.h"

#define SAC_COMMANDLINE_GET(argc, argv)                                                  \
    argc = SAC_commandline_argc;                                                         \
    argv = SAC_commandline_argv;

#define SAC_COMMANDLINE_SET(argc, argv)                                                  \
    SAC_commandline_argc = argc;                                                         \
    SAC_commandline_argv = argv;

#endif /* _SAC_RT_COMMANDLINE_H_ */
