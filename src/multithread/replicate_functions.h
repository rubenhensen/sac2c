/*
 * $Log$
 * Revision 1.2  2004/11/22 17:59:48  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.1  2004/08/31 16:56:20  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   replicate_functions.h
 *
 * description:
 *   header file for replicate_functions.c
 *
 *****************************************************************************/

#ifndef _SAC_REPLICATE_FUNCTIONS_H
#define _SAC_REPLICATE_FUNCTIONS_H

#include "types.h"

extern node *REPFUNdoReplicateFunctions (node *arg_node);

extern node *REPFUNfundef (node *arg_node, info *arg_info);

extern node *REPFUNassign (node *arg_node, info *arg_info);

extern node *REPFUNap (node *arg_node, info *arg_info);

extern node *REPFUNex (node *arg_node, info *arg_info);

extern node *REPFUNst (node *arg_node, info *arg_info);

extern node *REPFUNmt (node *arg_node, info *arg_info);
#endif /* _SAC_REPLICATE_FUNCTIONS_H */
