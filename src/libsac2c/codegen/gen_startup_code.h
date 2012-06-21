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
extern void GSCprintDefines (void);
extern void GSCprintMain (void);
extern void GSCprintMainBegin (void);
extern void GSCprintMainEnd (void);
extern void GSCprintSACargCopyFreeStubs (void);

#endif /* _SAC_GEN_STARTUP_CODE_H_ */
