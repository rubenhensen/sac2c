/*
 * $Log$
 * Revision 1.2  2004/08/26 17:05:04  skt
 * implementation finished
 *
 * Revision 1.1  2004/08/24 16:49:09  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   create_withinwith.h
 *
 * description:
 *   header file for create_withinwith.c
 *
 *****************************************************************************/

#ifndef CREATE_WITHINWITH_H

#define CREATE_WITHINWITH_H

extern node *CreateWithinwith (node *arg_node);

extern node *CRWIWfundef (node *arg_node, info *arg_info);

extern node *CRWIWassign (node *arg_node, info *arg_info);

extern node *CRWIWap (node *arg_node, info *arg_info);

node *CRWIWBuildReplication (node *fundef, info *arg_info);
#endif /* CREATE_WITHINWITH_H */
