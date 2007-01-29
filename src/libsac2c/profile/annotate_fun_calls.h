/*
 * $Log$
 * Revision 1.4  2004/11/25 01:01:07  skt
 * code brushing during SACDevCampDK 2k4
 *
 * Revision 1.3  2004/11/22 16:13:54  sbs
 * SACDecCamp04
 *
 * Revision 1.2  2004/07/17 14:52:03  sah
 * switch to INFO structure
 * PHASE I
 *
 * Revision 1.1  2001/03/09 11:08:08  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_ANNOTATE_FUN_CALLS_H_
#define _SAC_ANNOTATE_FUN_CALLS_H_

#include "types.h"

extern node *PFdoProfileFunCalls (node *fundef);

extern node *PFfundef (node *arg_node, info *arg_info);
extern node *PFassign (node *arg_node, info *arg_info);
extern node *PFap (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATE_FUN_CALLS_H_ */
