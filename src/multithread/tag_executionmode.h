/*
 * $Log$
 * Revision 1.1  2004/06/08 14:16:34  skt
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tag_executionmode.h
 *
 * description:
 *   header file for tag_executionmode.c
 *
 *****************************************************************************/

#ifndef TAG_EXECUTIONMODE_H

#define TAG_EXECUTIONMODE_H

/* access macros for arg_info
 *
 *   int        EXECMODE  (the current execution mode)
 *                              the current assignment)
 */
#define INFO_TEM_EXECMODE(n) (n->flag)

node *TagExecutionmode (node *arg_node, node *arg_info);

node *TEMassign (node *arg_node, node *arg_info);

node *TEMwith2 (node *arg_node, node *arg_info);

#endif /* TAG_EXECUTIONMODE_H */
