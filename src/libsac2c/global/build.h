/*****************************************************************************
 *
 * file:   build.h
 *
 * prefix:
 *
 * description:
 *
 *   This header file contains declarations of variables used for the -V
 *   command line option. The corresponding build.c file is created by
 *   make upon each compilation attempt.
 *
 *
 *****************************************************************************/

#ifndef _SAC_BUILD_H_
#define _SAC_BUILD_H_

extern char build_date[];
extern char build_user[];
extern char build_host[];
extern char build_os[];
extern char build_lic[];
extern char build_author[];
extern char build_style[];
extern char build_rev[];
extern char build_srev[];
extern char build_ast[];

#endif /* _SAC_BUILD_H_ */
