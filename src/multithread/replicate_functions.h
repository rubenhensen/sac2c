/*
 * $Log$
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

#ifndef REPLICATE_FUNCTIONS_H

#define REPLICATE_FUNCTIONS_H

extern node *ReplicateFunctions (node *arg_node);

extern node *REPFUNfundef (node *arg_node, info *arg_info);

extern node *REPFUNassign (node *arg_node, info *arg_info);

extern node *REPFUNap (node *arg_node, info *arg_info);

extern node *REPFUNex (node *arg_node, info *arg_info);

extern node *REPFUNst (node *arg_node, info *arg_info);

extern node *REPFUNmt (node *arg_node, info *arg_info);
#endif /* REPLICATE_FUNCTIONS_H */
