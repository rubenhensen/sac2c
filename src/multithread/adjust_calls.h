/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:00  sacbase
 * new release made
 *
 * Revision 1.2  2000/03/30 15:08:30  jhs
 *  Tried to build in removal of st-interior ...
 *
 * Revision 1.1  2000/03/30 13:24:53  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   adjust_calls.h
 *
 * description:
 *   header file for adjust_calls.c
 *
 *****************************************************************************/

#ifndef ADJUST_CALLS_H

#define ADJUST_CALLS_H

extern node *AdjustCalls1 (node *arg_node, node *arg_info);
extern node *AdjustCalls2 (node *arg_node, node *arg_info);

extern node *ADJCA1fundef (node *arg_node, node *arg_info);

extern node *ADJCA2fundef (node *arg_node, node *arg_info);
extern node *ADJCA2st (node *arg_node, node *arg_info);
extern node *ADJCA2let (node *arg_node, node *arg_info);

#endif /* ADJUST_CALLS_H */
