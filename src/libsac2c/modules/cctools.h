#ifndef _SAC_CCTOOLS_H_
#define _SAC_CCTOOLS_H_

#include "types.h"

/******************************************************************************
 *
 * CCtools
 *
 * Prefix: CCT
 *
 *****************************************************************************/

extern node *CCTpreprocessCompileAndLink (node *syntax_tree);

extern void CCTcompileOnly (void);
extern void CCTlinkOnly (void);
extern char *CCTreturnCompileFlags (void);
extern char *CCTreturnLinkFlags (void);

#endif /* _SAC_CCTOOLS_H_ */
