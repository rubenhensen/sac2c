/*
 * $Log$
 * Revision 1.3  2001/05/25 08:42:18  nmw
 * comments added, code beautyfied
 *
 * Revision 1.2  2001/05/07 09:03:00  nmw
 * withloop unrolling by WLUnroll integrated in traversal
 *
 * Revision 1.1  2001/04/20 11:20:56  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSALUR.h
 *
 * prefix: SSALUR
 *
 * description:
 *
 *   This module implements loop-unrolling for special do-functions in ssa
 *   form. all while loops have been removed and converted to do-loops before
 *   so we have to deal only with the do loops.
 *
 *****************************************************************************/

#ifndef SAC_SSALUR_H

#define SAC_SSALUR_H

extern node *SSALoopUnrolling (node *fundef, node *modul);

extern node *SSALURfundef (node *arg_node, node *arg_info);
extern node *SSALURassign (node *arg_node, node *arg_info);
extern node *SSALURap (node *arg_node, node *arg_info);
extern node *SSALURNwith (node *arg_node, node *arg_info);

#endif /* SAC_SSALUR_H */
