/*
 *
 * $Log$
 * Revision 1.4  2004/11/21 20:18:08  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.3  2004/10/20 08:10:29  khf
 * changed signature of startfunction
 *
 * Revision 1.2  2004/09/02 15:27:01  khf
 * TDEPENDwithop removed
 *
 * Revision 1.1  2004/08/26 15:06:32  khf
 * Initial revision
 *
 *
 *
 */

#include "types.h"

#ifndef _SAC_TAGDEPENDENCIES_H_
#define _SAC_TAGDEPENDENCIES_H_

extern node *TDEPENDdoTagDependencies (node *with, node *fusionable_wl);

extern node *TDEPENDassign (node *arg_node, info *arg_info);
extern node *TDEPENDid (node *arg_node, info *arg_info);

extern node *TDEPENDwith (node *arg_node, info *arg_info);

#endif /* _SAC_TAGDEPENDENCIES_H_ */
