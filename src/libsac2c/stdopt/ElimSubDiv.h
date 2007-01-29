/* *
 * $Log$
 * Revision 1.5  2004/11/22 17:58:37  khf
 * more codebrushing
 *
 * Revision 1.4  2004/11/21 20:34:10  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.3  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.2  2004/07/07 15:57:05  mwe
 * former log-messages added
 *
 *
 *
 * revision 1.1    locked by: mwe;
 * date: 2003/04/26 20:54:49;  author: mwe;  state: Exp;
 * Initial revision
 */

#ifndef _SAC_ELIMSUBDIV_H_
#define _SAC_ELIMSUBDIV_H_

#include "types.h"

/******************************************************************************
 *
 * Eliminate subtraction and division traversal ( esd_tab)
 *
 * Prefix: ESD
 *
 *****************************************************************************/
extern node *ESDdoElimSubDiv (node *);

extern node *ESDblock (node *, info *);
extern node *ESDassign (node *, info *);
extern node *ESDlet (node *, info *);
extern node *ESDprf (node *, info *);

#endif /* _SAC_ELIMSUBDIV_H_ */
