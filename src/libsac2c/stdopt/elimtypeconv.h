/*
 * $Log$
 * Revision 1.1  2005/08/20 12:11:41  ktr
 * Initial revision
 *
 */
#ifndef _SAC_ELIMTYPECONV_H_
#define _SAC_ELIMTYPECONV_H_

#include "types.h"

/******************************************************************************
 *
 * Eliminate Type Conversions traversal
 *
 * prefix: ETC
 *
 *****************************************************************************/
extern node *ETCdoEliminateTypeConversions (node *arg_node);
extern node *ETCdoEliminateTypeConversionsModule (node *arg_node);

extern node *ETCfundef (node *arg_node, info *arg_info);
extern node *ETCprf (node *arg_node, info *arg_info);

#endif /* _SAC_ELIMTYPECONV_H_ */
