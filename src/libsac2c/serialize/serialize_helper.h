#ifndef _SAC_SERIALIZE_HELPER_H_
#define _SAC_SERIALIZE_HELPER_H_

#include "config.h"

#if IS_CYGWIN
extern node *SHLPmakeNodeVa (int _node_type, int lineno, char *sfile, va_list args);
#endif
extern void SHLPfixLink (serstack_t *stack, int from, int no, int to);

#endif /* _SAC_SERIALIZE_HELPER_H_ */
