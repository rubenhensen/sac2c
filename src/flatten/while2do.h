/*
 * $Log$
 * Revision 1.1  2001/04/18 15:38:34  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   while2do.h
 *
 * prefix: W2D
 *
 * description:
 *   this module transforms all while in do loops (in an conditional)
 *
 *****************************************************************************/

#ifndef SAC_WHILE2DO_H

#define SAC_WHILE2DO_H

extern node *TransformWhile2Do (node *ast);

extern node *W2Dwhile (node *arg_node, node *arg_info);
#endif /* SAC_WHILE2DO_H */
