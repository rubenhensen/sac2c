/*
 *
 * $Log$
 * Revision 1.3  1998/05/11 09:51:22  cg
 * added definition of SPMD frame
 *
 * Revision 1.2  1998/05/08 09:04:34  cg
 * The syntax tree is now given as an argument to function GSCPrintFileHeader()
 *
 * Revision 1.1  1998/03/24 14:33:35  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   gen_startup_code.h
 *
 * prefix: GSC
 *
 * description:
 *
 *   Header file providing prototypes of public functions defined in
 *   gen_startup_code.c
 *
 *****************************************************************************/

#ifndef GEN_STARTUP_CODE_H

#define GEN_STARTUP_CODE_H

#include "types.h"

extern void GSCPrintFileHeader (node *);
extern void GSCPrintMainBegin ();
extern void GSCPrintMainEnd ();

extern node *GSCfundef (node *arg_node, node *arg_info);
extern node *GSCspmd (node *arg_node, node *arg_info);
extern node *GSCicm (node *arg_node, node *arg_info);

#endif /* GEN_STARTUP_CODE_H */
