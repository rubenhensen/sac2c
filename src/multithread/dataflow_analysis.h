/*
 *
 * $Log$
 * Revision 1.1  2000/03/09 19:49:54  jhs
 * Initial revision
 *
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/*****************************************************************************
 *
 * file:   dataflow_analysis.h
 *
 * description:
 *   header file for dataflow_analysis.c
 *
 *****************************************************************************/

#ifndef DATAFLOW_ANALYSIS_H

#define DATAFLOW_ANALYSIS_H

extern node *DataflowAnalysis (node *arg_node, node *arg_info);

extern node *DFAfundef (node *arg_node, node *arg_info);
extern node *DFAlet (node *arg_node, node *arg_info);
extern node *DFAreturn (node *arg_node, node *arg_info);

#endif /* DATAFLOW_ANALYSIS_H */
