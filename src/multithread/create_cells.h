/*
 * $Log$
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

#ifndef CREATE_CELLS_H

#define CREATE_CELLS_H

extern node *CreateCells (node *arg_node);

extern node *CRECEfundef (node *arg_node, info *arg_info);

extern node *CRECEassign (node *arg_node, info *arg_info);

void CRECEHandleIv (node *withloop, info *arg_info);

#endif /* CREATE_CELLS_H */
