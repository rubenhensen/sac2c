/*
 * $Log$
 * Revision 1.5  2004/11/22 17:29:51  sbs
 * SacDevCamp04
 *
 * Revision 1.4  2004/10/05 13:50:58  sah
 * lifted start of WLI/WLT traversal to the
 * defining source files to allow for local
 * info structures
 *
 * Revision 1.3  2004/07/19 14:19:38  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 1.2  2003/03/12 23:40:25  dkr
 * SSAWLTNwithop added
 *
 * Revision 1.1  2001/05/14 15:55:08  nmw
 * Initial revision
 *
 * created from WLT.h, Revision 3.2 on 2001/05/14 by nmw
 */

#ifndef _SAC_WLT_H_
#define _SAC_WLT_H_

include "types.h"

  extern node *
  WLTfundef (node *, info *);
extern node *WLTassign (node *, info *);
extern node *WLTcond (node *, info *);
extern node *WLTlet (node *, info *);
extern node *WLTap (node *, info *);

extern node *WLTwith (node *, info *);
extern node *WLTgenarray (node *, info *);
extern node *WLTmodarray (node *, info *);
extern node *WLTfold (node *, info *);
extern node *WLTpart (node *, info *);
extern node *WLTgenerator (node *, info *);
extern node *WLTcode (node *, info *);

extern node *WLTdoWLT (node *arg_node);

#endif /* _SAC_WLT_H_ */
