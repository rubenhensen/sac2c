/*
 *
 * $Log$
 * Revision 3.1  2000/11/20 18:03:08  sacbase
 * new release made
 *
 * Revision 1.3  2000/06/08 12:16:03  jhs
 * Fixed some problems caused by non-flatend code of IVE.
 *
 * Revision 1.2  2000/03/21 13:08:07  jhs
 * Implemented extended version.
 *
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
extern node *DFAassign (node *arg_node, node *arg_info);
extern node *DFAxt (node *arg_node, node *arg_info);
extern node *DFAcond (node *arg_node, node *arg_info);
extern node *DFAnum (node *arg_node, node *arg_info);
extern node *DFAarray (node *arg_node, node *arg_info);
extern node *DFAid (node *arg_node, node *arg_info);
extern node *DFAap (node *arg_node, node *arg_info);
extern node *DFAprf (node *arg_node, node *arg_info);
extern node *DFAexprs (node *arg_node, node *arg_info);
extern node *DFAnwith2 (node *arg_node, node *arg_info);

#endif /* DATAFLOW_ANALYSIS_H */
