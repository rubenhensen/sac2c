/*
 * $Log$
 * Revision 1.2  2001/04/02 11:08:20  nmw
 * handling for multiple used special functions added
 *
 * Revision 1.1  2001/03/26 15:37:45  nmw
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSALIR.h
 *
 * prefix: SSALIR
 *
 * description:
 *   this module does loop invariant removal on a function in ssa form.
 *
 *
 *****************************************************************************/

#ifndef SAC_SSALIR_H

#define SAC_SSALIR_H

extern node *SSALoopInvariantRemoval (node *fundef, node *modul);

extern node *SSALIRfundef (node *arg_node, node *arg_info);
extern node *SSALIRarg (node *arg_node, node *arg_info);
extern node *SSALIRvardec (node *arg_node, node *arg_info);
extern node *SSALIRblock (node *arg_node, node *arg_info);
extern node *SSALIRassign (node *arg_node, node *arg_info);
extern node *SSALIRlet (node *arg_node, node *arg_info);
extern node *SSALIRid (node *arg_node, node *arg_info);
extern node *SSALIRap (node *arg_node, node *arg_info);
extern node *SSALIRcond (node *arg_node, node *arg_info);
extern node *SSALIRreturn (node *arg_node, node *arg_info);
extern node *SSALIRNwith (node *arg_node, node *arg_info);
extern node *SSALIRNwithid (node *arg_node, node *arg_info);
#endif /* SAC_SSALIR_H */
