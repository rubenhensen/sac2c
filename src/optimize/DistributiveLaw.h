/* *
 * $Log$
 * Revision 1.7  2004/11/22 17:58:37  khf
 * more codebrushing
 *
 * Revision 1.6  2004/11/21 20:34:10  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.5  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.4  2003/02/24 17:40:36  mwe
 * removed some functions
 *
 * Revision 1.3  2003/02/10 18:01:30  mwe
 * removed needles functions
 *
 * Revision 1.2  2003/02/09 22:32:19  mwe
 * removed bugs
 *
 * Revision 1.1  2003/02/08 16:08:16  mwe
 * Initial revision
 *
 *
 */

#ifndef _SAC_DISTRIBUTIVELAW_H_
#define _SAC_DISTRIBUTIVELAW_H_

#include "types.h"

/******************************************************************************
 *
 * Distributive law traversal ( dl_tab)
 *
 * Prefix: DL
 *
 *****************************************************************************/
extern node *DLdoDistributiveLaw (node *);
extern node *DLblock (node *, info *);
extern node *DLassign (node *, info *);
extern node *DLlet (node *, info *);
extern node *DLap (node *, info *);
extern node *DLprf (node *, info *);

#endif /* _SAC_DISTRIBUTIVELAW_H_ */
