/*
 * $Log$
 * Revision 1.9  2004/11/27 03:14:17  cg
 * removed CDFGcond
 *
 * Revision 1.8  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.7  2004/11/22 16:27:16  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.6  2004/08/13 10:27:48  skt
 * comments, comments, comments...
 *
 * Revision 1.5  2004/08/12 12:39:28  skt
 * killed a bug in CDFGFirstIsWithinSecond
 * moved PrintDataflowgraph and PrintDataflownode to print
 *
 * Revision 1.4  2004/08/09 03:47:34  skt
 * some very painful bugfixing
 * added support for dataflowgraphs within with-loops
 * (I hope someone'll use it in future)
 *
 * Revision 1.3  2004/08/06 17:20:24  skt
 * some adaptions for creating the dataflowgraph
 *
 * Revision 1.2  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.1  2004/07/29 08:38:53  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   create_dataflowgraph.h
 *
 * description:
 *   header file for create_dataflowgraph.c
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_DATAFLOWGRAPH_H_
#define _SAC_CREATE_DATAFLOWGRAPH_H_

#include "types.h"

extern node *CDFGdoCreateDataflowgraph (node *arg_node);

extern node *CDFGblock (node *arg_node, info *arg_info);

extern node *CDFGassign (node *arg_node, info *arg_info);

/*
extern node *CDFGcond(node *arg_node, info *arg_info);
*/

extern node *CDFGid (node *arg_node, info *arg_info);

extern node *CDFGwithid (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_DATAFLOWGRAPH_H_ */
