/* $Id$
 * $Log$
 * Revision 1.4  1998/04/08 20:34:06  srs
 * new WLF functions
 *
 * Revision 1.3  1998/04/07 16:50:17  srs
 * new WLF functions
 *
 * Revision 1.2  1998/03/22 18:19:30  srs
 * *** empty log message ***
 *
 * Revision 1.1  1998/03/22 18:17:26  srs
 * Initial revision
 *
 */

#ifndef _WLF_h
#define _WLF_h

extern node *WLFfundef (node *arg_node, node *arg_info);
extern node *WLFassign (node *arg_node, node *arg_info);
extern node *WLFid (node *arg_node, node *arg_info);
extern node *WLFNwith (node *arg_node, node *arg_info);
extern node *WLFlet (node *arg_node, node *arg_info);
extern node *WLFNcode (node *arg_node, node *arg_info);

#endif
