/*
 *
 * $Log$
 * Revision 3.2  2004/07/16 17:36:23  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.1  2000/11/20 17:59:22  sacbase
 * new release made
 *
 * Revision 1.1  2000/02/17 16:15:25  cg
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   fun2lac.h
 *
 * prefix: FUN2LAC
 *
 * description:
 *
 *   header file for fun2lac.c.
 *
 *
 *****************************************************************************/

#ifndef FUN2LAC_H
#define FUN2LAC_H

extern node *Fun2Lac (node *syntax_tree);

extern node *FUN2LACmodul (node *arg_node, info *arg_info);
extern node *FUN2LACfundef (node *arg_node, info *arg_info);
extern node *FUN2LACblock (node *arg_node, info *arg_info);
extern node *FUN2LACassign (node *arg_node, info *arg_info);
extern node *FUN2LAClet (node *arg_node, info *arg_info);
extern node *FUN2LACap (node *arg_node, info *arg_info);

#endif /* FUN2LAC_H */
