/*
 *
 * $Log$
 * Revision 1.1  2004/02/25 13:17:02  khf
 * Initial revision
 *
 *
 */

#ifndef _WLPartitionGeneration_h
#define _WLPartitionGeneration_h

extern node *WLPartitionGeneration (node *arg_node);

extern node *WLPGfundef (node *, node *);
extern node *WLPGassign (node *, node *);

extern node *WLPGNwith (node *, node *);
extern node *WLPGNwithop (node *, node *);
extern node *WLPGNpart (node *, node *);
extern node *WLPGNgenerator (node *, node *);
extern node *WLPGNcode (node *, node *);

#endif
