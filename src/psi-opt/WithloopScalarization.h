/*
 *
 * $Log$
 * Revision 1.5  2002/06/18 10:23:15  ktr
 * Support for N_id nodes in generator's N_array nodes added.
 *
 * Revision 1.4  2002/06/09 19:59:49  ktr
 * works even better, still some known bugs
 *
 * Revision 1.3  2002/05/16 09:40:07  ktr
 * an early version of a working WithLoop-Scalarization
 *
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
extern node *WLSblock (node *arg_node, node *arg_info);

#endif
