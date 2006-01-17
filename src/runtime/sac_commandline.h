/* $Id$ */

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

#ifndef _SAC_COMMANDLINE_H_
#define _SAC_COMMANDLINE_H_

extern char **SAC_commandline_argv;
extern int SAC_commandline_argc;

#define SAC_COMMANDLINE_GET(argc, argv)                                                  \
    argc = SAC_commandline_argc;                                                         \
    argv = SAC_commandline_argv;

#define SAC_COMMANDLINE_SET(argc, argv)                                                  \
    SAC_commandline_argc = argc;                                                         \
    SAC_commandline_argv = argv;

#endif /* _SAC_COMMANDLINE_H_ */