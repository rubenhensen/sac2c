/*
 * $Log$
 * Revision 1.6  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.5  2004/11/22 17:59:48  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.4  2004/08/18 13:24:31  skt
 * switch to mtexecmode_t done
 *
 * Revision 1.3  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.2  2004/07/23 10:05:46  skt
 * complete redesign
 *
 * Revision 1.1  2004/07/06 12:31:20  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   propagate_executionmode.h
 *
 * description:
 *   header file for propagate_executionmode.c
 *
 *****************************************************************************/

#ifndef _SAC_PROPAGATE_EXECUTIONMODE_H_
#define _SAC_PROPAGATE_EXECUTIONMODE_H_

#include "types.h"

extern node *PEMdoPropagateExecutionmode (node *arg_node);

extern node *PEMfundef (node *arg_node, info *arg_info);

extern node *PEMassign (node *arg_node, info *arg_info);

extern node *PEMap (node *arg_node, info *arg_info);

extern node *PEMcond (node *arg_node, info *arg_info);

extern node *PEMwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_PROPAGATE_EXECUTIONMODE_H_ */
