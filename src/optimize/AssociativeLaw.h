/*
 *
 * $Log$
 * Revision 1.9  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.8  2003/02/24 17:40:36  mwe
 * removed some functions
 *
 * Revision 1.7  2002/11/02 21:01:25  mwe
 * removed unused functions
 *
 * Revision 1.6  2002/10/10 09:28:16  mwe
 * new functions added
 * seems to be the first correct working version
 *
 *
 *
 *----------------------------
 *revision 1.5
 *date: 2002/08/25 14:25:30;  author: mwe;  state: Exp;  lines: +3 -1
 *rename AssociativeLawOptimize in ALprf
 *----------------------------
 *revision 1.4
 *date: 2002/07/21 21:45:57;  author: mwe;  state: Exp;  lines: +2 -2
 *editing ALassign
 *----------------------------
 *revision 1.3
 *date: 2002/07/21 15:55:17;  author: mwe;  state: Exp;  lines: +1 -1
 *editing CreateNAssignNodes and CommitNAssignNodes
 *----------------------------
 *revision 1.2
 *date: 2002/07/20 14:25:13;  author: mwe;  state: Exp;  lines: +18 -1
 *adding first functions
 *----------------------------
 *revision 1.1
 *date: 2002/06/07 17:38:02;  author: mwe;  state: Exp;
 *Initial revision
 *=============================================================================
 *
 */

#ifndef _AssociativeLaw_h_
#define _AssociativeLaw_h_

extern node *AssociativeLaw (node *arg_node);
extern node *ALblock (node *, info *);
extern node *ALassign (node *, info *);
extern node *ALlet (node *, info *);
extern node *ALprf (node *, info *);

#endif
