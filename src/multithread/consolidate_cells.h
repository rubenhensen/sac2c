/*
 * $Log$
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

#ifndef CONSOLIDATE_CELLS_H

#define CONSOLIDATE_CELLS_H

extern node *ConsolidateCells (node *arg_node);

extern node *CONCELfundef (node *arg_node, info *arg_info);

extern node *CONCELex (node *arg_node, info *arg_info);

extern node *CONCELst (node *arg_node, info *arg_info);

extern node *CONCELmt (node *arg_node, info *arg_info);

#endif /* CONSOLIDATE_CELLS_H */
