/*
 *
 * $Log$
 * Revision 1.7  2005/07/03 17:13:14  ktr
 * All variables are initialized.
 * Bool used instead of int where appropriate
 *
 * Revision 1.6  2005/04/22 10:08:04  ktr
 * Works with Marielyst compiler.
 *
 * Revision 1.5  2004/11/26 17:38:04  mwe
 * corrected includes
 *
 * Revision 1.4  2004/11/22 18:33:19  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.3  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.2  2002/10/09 02:06:25  dkr
 * no changes done
 *
 * Revision 1.1  2002/10/08 22:10:05  dkr
 * Initial revision
 *
 *
 * created from WLUnroll.h, Revision 3.1 on 2002/10/10 by dkr
 *
 */

#ifndef _SAC_SSAWLUNROLL_H_
#define _SAC_SSAWLUNROLL_H_

#include "types.h"

/*****************************************************************************
 *
 * SSAWLUnroll
 *
 * prefix: WLU
 *
 *****************************************************************************/

extern node *WLURap (node *arg_node, info *arg_info);
extern node *WLURassign (node *arg_node, info *arg_info);
extern node *WLURfundef (node *arg_node, info *arg_info);
extern node *WLURwith (node *arg_node, info *arg_info);

extern node *WLURdoWithloopUnroll (node *syntax_tree);

#endif /* _SAC_SSAWLUNROLL_H_ */
