/*
 * $Log$
 * Revision 1.3  2004/11/22 18:10:19  sbs
 * SacDevCamp04
 *
 * Revision 1.2  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.1  2001/05/22 09:09:45  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   rmcasts.h
 *
 * prefix: RC
 *
 * description:
 *   this module removes all casts from AST
 *
 *****************************************************************************/

#ifndef SAC_RMCASTS_H_
#define SAC_RMCASTS_H_

#include "types.h"

extern node *RCdoRemoveCasts (node *ast);
extern node *RCcast (node *arg_node, info *arg_info);

#endif /* SAC_RMCASTS_H_ */
