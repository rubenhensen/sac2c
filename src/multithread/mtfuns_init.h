/*
 *
 * $Log$
 * Revision 1.1  2000/03/02 12:54:17  jhs
 * Initial revision
 *
 *
 * constructed by jhs@dArtagnan at home
 */

/*****************************************************************************
 *
 * file:   mtfuns_init.h
 *
 * description:
 *   header file for mtfuns_init.c
 *
 *****************************************************************************/

#ifndef MTFUNS_INIT_H

#define MTFUNS_INIT_H

extern node *MtfunsInit (node *arg_node, node *arg_info);

extern node *MTFINxt (node *arg_node, node *arg_info);
extern node *MTFINlet (node *arg_node, node *arg_info);
extern node *MTFINfundef (node *arg_node, node *arg_info);

#endif /* MTFUNS_INIT_H */
