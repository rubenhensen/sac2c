/*
 * $Log$
 * Revision 1.2  2004/11/22 13:48:10  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.1  2004/08/17 09:06:54  skt
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   cell_growth.h
 *
 * description:
 *   header file for cell_growth.c
 *
 *****************************************************************************/

#ifndef _SAC_CELL_GROWTH_H
#define _SAC_CELL_GROWTH_H

#include "types.h"

extern node *CEGROdoCellGrowth (node *arg_node);

extern node *CEGROblock (node *arg_node, info *arg_info);

extern node *CEGROassign (node *arg_node, info *arg_info);

#endif /* _SAC_CELL_GROWTH_H */
