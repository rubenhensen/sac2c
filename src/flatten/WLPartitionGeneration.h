/*
 *
 * $Log$
 * Revision 1.4  2004/04/08 08:13:25  khf
 * some corrections and new startfunction WLPartitionGenerationOPT
 * added
 *
 * Revision 1.3  2004/03/02 09:21:43  khf
 * WLPGlet added
 *
 * Revision 1.2  2004/02/26 13:11:01  khf
 * WLPartitionGeneration implemented in parts (but not tested)
 *
 * Revision 1.1  2004/02/25 13:17:02  khf
 * Initial revision
 *
 *
 */

#ifndef _WLPartitionGeneration_h
#define _WLPartitionGeneration_h

extern node *WLPartitionGeneration (node *arg_node);
extern node *WLPartitionGenerationOPT (node *arg_node);

extern node *WLPGmodul (node *, node *);
extern node *WLPGfundef (node *, node *);
extern node *WLPGassign (node *, node *);
extern node *WLPGlet (node *, node *);

extern node *WLPGNwith (node *, node *);
extern node *WLPGNwithop (node *, node *);
extern node *WLPGNpart (node *, node *);
extern node *WLPGNgenerator (node *, node *);

#endif
