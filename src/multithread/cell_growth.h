/*
 * $Log$
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

#ifndef CELL_GROWTH_H

#define CELL_GROWTH_H

extern node *CellGrowth (node *arg_node);

extern node *CEGROblock (node *arg_node, info *arg_info);

extern node *CEGROassign (node *arg_node, info *arg_info);

#endif /* CELL_GROWTH_H */
