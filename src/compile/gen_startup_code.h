/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:42:33  sacbase
 * new release made
 *
 * Revision 1.4  1998/07/07 13:43:12  cg
 * Global flags SAC_DO_MULTITHREAD, SAC_DO_THREADS_STATC and
 * settings for multithreaded execution may now be set by the C
 * compiler instead of being fixed in the C source.
 * This was necessary to implement the -mt-all command line option.
 *
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

extern int GSCCalcMasterclass (int num_threads);

extern node *GSCfundef (node *arg_node, node *arg_info);
extern node *GSCspmd (node *arg_node, node *arg_info);
extern node *GSCicm (node *arg_node, node *arg_info);

#endif /* GEN_STARTUP_CODE_H */
