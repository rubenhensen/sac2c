/* 	$Id$
 *
 * $Log$
 * Revision 1.5  1998/02/09 15:58:47  srs
 * *** empty log message ***
 *
 * Revision 1.4  1998/02/06 14:33:29  srs
 * RCS-Test
 *
 * Revision 1.3  1998/02/06 14:32:49  srs
 * *** empty log message ***
 *
 */

#ifndef _WithloopFolding_h
#define _WithloopFolding_h

extern node *WLFNwith (node *arg_node, node *arg_info);
extern node *WLFlet (node *, node *);
extern node *WLFfundef (node *, node *);
extern node *WLFWithloopFolding (node *, node *);

#endif
