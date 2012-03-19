/*
 * $Id$
 */

/*
 * File : globals.h
 *
 * Description:
 *
 * We alow exactly one global variable named 'global', which is a huge
 * structure containing all globally available information.
 *
 * Most of the work is done in types.h where the complicated type
 * global_t is generated from globals.mac and in globals.c where the
 * initialization code again is generated from globals.mac.
 *
 * We define a macro GLOBALS_MODFLAGS here that evaluates to an integer
 * describing the state of some flags in the globals structure. It should
 * contain all those flags that might lead to incompatible code generation
 * when toggled. Make sure that each incompatible configuration leads to a
 * distinct value.
 *
 * To be able to give some account on the used settings, we define
 * GLOBALS_MODFLAGS_TEXT( flags) that evaluates to a string depending
 * on the settings used.
 */

#ifndef _SAC_GLOBALS_H_
#define _SAC_GLOBALS_H_

#include "types.h"
#include <stdio.h>

#define GLOBALS_MODFLAGS (global.optimize.dophm)
#define GLOBALS_MODFLAGS_TEXT(n) ((n == 1) ? "PHM enabled" : "PHM disabled")

extern FILE *yyin;
extern global_t global;

extern void GLOBinitializeGlobal (int argc, char *argv[], tool_t tool, char *toolname);
extern void GLOBsetupBackend (void);

#define PARSE_RC 10
#define PARSE_PRG 20

#endif /* _SAC_GLOBALS_H_ */
