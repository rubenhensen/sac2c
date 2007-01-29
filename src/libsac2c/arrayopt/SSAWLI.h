/*
 * $Log$
 * Revision 1.6  2005/10/05 13:27:11  ktr
 * removed WLIap
 *
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
 * Revision 1.2  2004/03/26 13:02:12  khf
 * SSAWLINgenerator added
 *
 * Revision 1.1  2001/05/15 15:41:11  nmw
 * Initial revision
 *
 *
 * created from WLI.h, Revision 3.2  on 2001/05/15 by nmw
 */

#ifndef _SAC_WLI_H_
#define _SAC_WLI_H_

#include "types.h"

extern node *WLIfundef (node *, info *);
extern node *WLIid (node *, info *);
extern node *WLIassign (node *, info *);
extern node *WLIcond (node *, info *);
extern node *WLIwith (node *, info *);
extern node *WLIlet (node *, info *);

extern node *WLImodarray (node *, info *);
extern node *WLIpart (node *, info *);
extern node *WLIgenerator (node *, info *);
extern node *WLIcode (node *, info *);

extern node *WLIdoWLI (node *arg_node);

#endif /* _SAC_SSAWLI_H_ */
