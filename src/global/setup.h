/*
 *
 * $Log$
 * Revision 1.1  2005/03/07 13:41:16  cg
 * Initial revision
 *
 *
 */

#ifndef _SAC_SETUP_H_
#define _SAC_SETUP_H_

#include "types.h"

extern void SETUPdoSetupCompiler (int argc, char *argv[]);
extern node *SETUPdoInitializations (node *syntax_tree);
extern node *SETUPdriveSetup (node *syntax_tree);

#endif /* _SAC_SETUP_H_ */
