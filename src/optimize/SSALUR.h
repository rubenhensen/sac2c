/*
 * $Log$
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
extern node *SSALURlet (node *arg_node, node *arg_info);
extern node *SSALURid (node *arg_node, node *arg_info);

#endif /* SAC_SSALUR_H */
