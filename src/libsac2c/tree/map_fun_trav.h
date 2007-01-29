/*
 * $Log$
 * Revision 1.1  2005/09/04 12:10:57  ktr
 * Initial revision
 *
 */

#ifndef _SAC_MAP_FUN_TRAV_H_
#define _SAC_MAP_FUN_TRAV_H_

#include "types.h"

extern node *MFTfundef (node *arg_node, info *arg_info);

extern node *MFTdoMapFunTrav (node *module, node *(*trav) (node *));

#endif /* _SAC_MAP_LAC_FUNS_H_ */
