/*
 *
 * $Log$
 * Revision 1.1  2004/04/08 08:15:52  khf
 * Initial revision
 *
 *
 *
 */

#ifndef _WithloopFusion_h
#define _WithloopFusion_h

extern node *WithloopFusion (node *arg_node);

extern node *WLFSfundef (node *, node *);
extern node *WLFSassign (node *, node *);
extern node *WLFSid (node *, node *);

extern node *WLFSNwith (node *, node *);
extern node *WLFSNwithop (node *, node *);
extern node *WLFSNpart (node *, node *);
extern node *WLFSNgenerator (node *, node *);

#endif
