/*
 *
 * $Log$
 * Revision 1.4  2004/11/21 20:18:08  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.3  2004/10/20 08:10:29  khf
 * added DDEPENDprf, DDEPENDcode and changed signature of startfunction
 *
 * Revision 1.2  2004/09/02 15:27:01  khf
 * DDEPENDwithop removed
 *
 * Revision 1.1  2004/08/26 15:06:28  khf
 * Initial revision
 *
 *
 *
 */

#include "types.h"

#ifndef _SAC_DETECTDEPENDENCIES_H_
#define _SAC_DETECTDEPENDENCIES_H_

extern node *DDEPENDdoDetectDependencies (node *with, node *fusionable_wl,
                                          nodelist *references_fwl);

extern node *DDEPENDassign (node *arg_node, info *arg_info);
extern node *DDEPENDprf (node *arg_node, info *arg_info);
extern node *DDEPENDid (node *arg_node, info *arg_info);

extern node *DDEPENDwith (node *arg_node, info *arg_info);
extern node *DDEPENDcode (node *arg_node, info *arg_info);

#endif /* _SAC_DETECTDEPENDENCIES_H_ */
