/*
 *
 * $Log$
 * Revision 3.3  2004/11/22 11:00:05  ktr
 * Ismop 2004 SacDevCamp 04
 *
 * Revision 3.2  2001/03/22 18:55:08  dkr
 * include of tree.h eliminated
 *
 * Revision 3.1  2000/11/20 18:03:45  sacbase
 * new release made
 *
 * Revision 1.1  2000/08/02 14:25:07  nmw
 * Initial revision
 *
 */

#ifndef _SAC_PRINT_INTERFACE_WRAPPER_H_
#define _SAC_PRINT_INTERFACE_WRAPPER_H_

#include "types.h"

/******************************************************************************
 *
 * Print interface wrapper traversal ( piw_tab)
 *
 * Prefix: PIW
 *
 *****************************************************************************/
extern node *PIWarg (node *arg_node, info *arg_info);
extern node *PIWcwrapper (node *arg_node, info *arg_info);
extern node *PIWfundef (node *arg_node, info *arg_info);
extern node *PIWmodule (node *arg_node, info *arg_info);
extern node *PIWobjdef (node *arg_node, info *arg_info);

#endif /* _SAC_PRINT_INTERFACE_WRAPPER_H_ */
