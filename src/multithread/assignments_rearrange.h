/*
 * $Log$
 * Revision 1.9  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.8  2004/11/22 12:49:31  skt
 * code brushing in SACDevCampDK 2005
 *
 * Revision 1.7  2004/08/19 15:01:03  skt
 * rearranging algorithm improved
 *
 * Revision 1.6  2004/08/12 12:52:19  skt
 * some debugging...
 *
 * Revision 1.5  2004/08/11 09:31:54  skt
 * ASMRAPrintCluster bug fixed
 *
 * Revision 1.4  2004/08/11 08:38:44  skt
 * full redesigned, still under construction but looks well
 *
 * Revision 1.3  2004/07/29 00:41:50  skt
 * build compilable intermediate version
 * work in progress
 *
 * Revision 1.2  2004/04/30 14:10:05  skt
 * some debugging
 *
 * Revision 1.1  2004/04/27 09:59:21  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   assignments_rearrange.h
 *
 * description:
 *   header file for assignments_rearrange.c
 *
 *****************************************************************************/

#ifndef _SAC_ASSIGNMENTS_REARRANGE_H_

#define _SAC_ASSIGNMENTS_REARRANGE_H_

#include "types.h"

extern node *ASMRAdoAssignmentsRearrange (node *arg_node);

extern node *ASMRAblock (node *arg_node, info *arg_info);

#endif /* ASSIGNMENTS_REARRANGE_H_ */
