/*****************************************************************************
 *
 * file:   commandline.h
 *
 * description:
 *
 *   This file is part of the SAC standard header file sac.h
 *
 *   It provides the interface to commandline.c
 *
 *****************************************************************************/

#ifndef _SAC_COMMANDLINE_H_
#define _SAC_COMMANDLINE_H_

#ifndef SAC_C_EXTERN_VAR
#define SAC_C_EXTERN_VAR extern
#endif /* SAC_C_EXTERN_VAR */

SAC_C_EXTERN_VAR char **SAC_commandline_argv;
SAC_C_EXTERN_VAR int SAC_commandline_argc;

#endif /* _SAC_COMMANDLINE_H_ */

