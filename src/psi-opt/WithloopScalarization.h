/*
 *
 * $Log$
 * Revision 1.2  2002/04/09 08:13:02  ktr
 * Some functionality added, but still bugs
 *
 * Revision 1.1  2002/03/13 15:57:11  ktr
 * Initial revision
 *
 *
 */

#ifndef _WithloopScalarization_h
#define _WithloopScalarization_h

extern node *WithloopScalarization (node *fundef, node *modul);

extern node *WLSfundef (node *arg_node, node *arg_info);
extern node *WLSNwith (node *arg_node, node *arg_info);
extern node *WLSNpart (node *arg_node, node *arg_info);
extern node *WLSNcode (node *arg_node, node *arg_info);
extern node *WLSNwithid (node *arg_node, node *arg_info);
extern node *WLSNgenerator (node *arg_node, node *arg_info);
extern node *WLSNwithop (node *arg_node, node *arg_info);

#endif
