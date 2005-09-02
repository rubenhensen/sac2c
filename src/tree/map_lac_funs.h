/*
 * $Log$
 * Revision 1.1  2005/09/02 17:44:46  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_MAP_LAC_FUNS_H_
#define _SAC_MAP_LAC_FUNS_H_

#include "types.h"

extern node *MLFfundef (node *arg_node, info *arg_info);
extern node *MLFap (node *arg_node, info *arg_info);

extern info *MLFdoMapLacFuns (node *fundef, travfun_p mapfundown, travfun_p mapfunup,
                              info *info);

#endif /* _SAC_MAP_LAC_FUNS_H_ */
