/*
 * $Log$
 * Revision 1.3  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.2  2004/11/22 14:37:39  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.1  2004/09/02 16:02:25  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   consolidate_cells.h
 *
 * description:
 *   header file for consolidate_cells.c
 *
 *****************************************************************************/

#ifndef _SAC_CONSOLIDATE_CELLS_H_
#define _SAC_CONSOLIDATE_CELLS_H_

#include "types.h"

extern node *CONCELdoConsolidateCells (node *arg_node);

extern node *CONCELfundef (node *arg_node, info *arg_info);

extern node *CONCELex (node *arg_node, info *arg_info);

extern node *CONCELst (node *arg_node, info *arg_info);

extern node *CONCELmt (node *arg_node, info *arg_info);

#endif /* _SAC_CONSOLIDATE_CELLS_H_ */
