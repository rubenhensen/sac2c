/*
 * $Log$
 * Revision 1.1  2001/05/15 15:41:17  nmw
 * Initial revision
 *
 *
 * created from WLF.h, Revision 3.1 on 2001/05/15 by nmw
 *
 */

#ifndef _SSAWLF_h
#define _SSAWLF_h

extern node *SSAWLFfundef (node *arg_node, node *arg_info);
extern node *SSAWLFassign (node *arg_node, node *arg_info);
extern node *SSAWLFid (node *arg_node, node *arg_info);
extern node *SSAWLFNwith (node *arg_node, node *arg_info);
extern node *SSAWLFlet (node *arg_node, node *arg_info);
extern node *SSAWLFNcode (node *arg_node, node *arg_info);

#endif
