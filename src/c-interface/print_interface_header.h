/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 11:00:05  ktr
 * Ismop 2004 SacDevCamp 04
 *
 * Revision 3.2  2001/03/22 18:55:32  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:03:44  sacbase
 * new release made
 *
 * Revision 1.1  2000/08/02 14:22:47  nmw
 * Initial revision
 *
 */

#ifndef _SAC_PRINT_INTERFACEHEADER_H_
#define _SAC_PRINT_INTERFACEHEADER_H_

#include "types.h"

/******************************************************************************
 *
 * Print interface header traversal ( pih_tab)
 *
 * Prefix: PIH
 *
 *****************************************************************************/
extern node *PIHmodule (node *arg_node, info *arg_info);
extern node *PIHcwrapper (node *arg_node, info *arg_info);
extern node *PIHfundef (node *arg_node, info *arg_info);
extern node *PIHarg (node *arg_node, info *arg_info);

extern node *PIHcwrapperPrototype (node *wrapper, info *arg_info);

#endif /* _SAC_PRINT_INTERFACEHEADER_H_ */
