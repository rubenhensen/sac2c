/*****************************************************************************
 *
 * $Id$
 *
 * file:   gen_startup_code.h
 *
 * prefix: GSC
 *
 * description:
 *
 *   Header file providing prototypes of public functions defined in
 *   gen_startup_code.c
 *
 *****************************************************************************/

#ifndef _SAC_GEN_STARTUP_CODE_H_
#define _SAC_GEN_STARTUP_CODE_H_

#include "types.h"

extern void GSCprintFileHeader (node *syntax_tree);
extern void GSCprintInternalInitFileHeader (node *syntax_tree);
extern void GSCprintDefines ();
extern void GSCprintMain ();
extern void GSCprintMainBegin ();
extern void GSCprintMainEnd ();

#endif /* _SAC_GEN_STARTUP_CODE_H_ */
