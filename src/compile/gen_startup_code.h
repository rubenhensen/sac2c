/*
 *
 * $Log$
 * Revision 3.5  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.4  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.3  2002/04/16 21:07:12  dkr
 * GSCPrintMain() added
 *
 * Revision 3.2  2001/04/03 12:06:53  dkr
 * GSCPrintDefines added
 *
 * Revision 3.1  2000/11/20 18:01:12  sacbase
 * new release made
 *
 * Revision 2.2  2000/08/01 13:28:17  nmw
 * header generation for c library internal PHM init function added
 *
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

#ifndef _SAC_GEN_STARTUP_CODE_H_
#define _SAC_GEN_STARTUP_CODE_H_

#include "types.h"

extern void GSCPrintFileHeader (node *syntax_tree);
extern void GSCPrintInternalInitFileHeader (node *syntax_tree);
extern void GSCPrintDefines ();
extern void GSCPrintMain ();
extern void GSCPrintMainBegin ();
extern void GSCPrintMainEnd ();

extern int GSCCalcMasterclass (int num_threads);

/******************************************************************************
 *
 * Generate startup code traversal ( gsc_tab)
 *
 * Prefix: GSC
 *
 *****************************************************************************/
extern node *GSCfundef (node *arg_node, info *arg_info);
extern node *GSCspmd (node *arg_node, info *arg_info);
extern node *GSCicm (node *arg_node, info *arg_info);

#endif /* _SAC_GEN_STARTUP_CODE_H_ */
