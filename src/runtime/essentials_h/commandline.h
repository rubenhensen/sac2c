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

#ifndef SAC_C_EXTERN_VAR
#define SAC_C_EXTERN_VAR extern
#endif /* SAC_C_EXTERN_VAR */

SAC_C_EXTERN_VAR char **SAC_commandline_argv;
SAC_C_EXTERN_VAR int SAC_commandline_argc;

#define SAC_COMMANDLINE_GET(argc, argv)                                                  \
    argc = SAC_commandline_argc;                                                         \
    argv = SAC_commandline_argv;

#define SAC_COMMANDLINE_SET(argc, argv)                                                  \
    SAC_commandline_argc = argc;                                                         \
    SAC_commandline_argv = argv;

#endif /* _SAC_COMMANDLINE_H_ */
