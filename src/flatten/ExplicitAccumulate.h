/*
 *
 * $Log$
 * Revision 1.1  2004/07/21 12:35:36  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _ExplicitAccumulate_h
#define _ExplicitAccumulate_h

extern node *ExplicitAccumulate (node *arg_node);

extern node *EAmodul (node *arg_node, info *arg_info);
extern node *EAfundef (node *arg_node, info *arg_info);
extern node *EAlet (node *arg_node, info *arg_info);

extern node *EANwith (node *arg_node, info *arg_info);
extern node *EANcode (node *arg_node, info *arg_info);

#endif
