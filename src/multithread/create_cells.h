/*
 * $Log$
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

extern node *CreateCells (node *arg_node, node *arg_info);

extern node *CRECEfundef (node *arg_node, node *arg_info);

extern node *CRECEassign (node *arg_node, node *arg_info);

node *CRECEAddIv (node *arg_node, node *arg_info);

#endif /* CREATE_CELLS_H */
