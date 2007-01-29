/*
 * $Log$
 * Revision 1.9  2004/11/23 20:52:11  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 1.8  2004/11/22 14:59:51  skt
 * code brushing in SACDevCampDK 2004
 *
 * Revision 1.7  2004/08/16 18:15:26  skt
 * implementation finished
 *
 * Revision 1.6  2004/08/16 16:52:35  skt
 * implementation expanded
 *
 * Revision 1.5  2004/08/05 17:42:19  skt
 * moved handling of the allocation around the withloop into propagate_executionmode
 *
 * Revision 1.4  2004/08/05 13:50:18  skt
 * welcome to the new INFO structure
 *
 * Revision 1.3  2004/07/28 22:45:22  skt
 * changed CRECEAddIv into CRECEHandleIv,
 * implementation changed & tested
 *
 * Revision 1.2  2004/07/28 17:46:14  skt
 * CRECEfundef added
 *
 * Revision 1.1  2004/07/26 16:11:29  skt
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   create_cells.h
 *
 * description:
 *   header file for create_cells.c
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_CELLS_H_
#define _SAC_CREATE_CELLS_H_

#include "types.h"

extern node *CRECEdoCreateCells (node *arg_node);

extern node *CRECEblock (node *arg_node, info *arg_info);

extern node *CRECEassign (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_CELLS_H_ */
