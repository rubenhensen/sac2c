/*
 * $Log$
 * Revision 1.4  2001/04/12 12:40:14  nmw
 * SSALIRexprs added
 *
 * Revision 1.3  2001/04/09 15:57:08  nmw
 * first implementation of code move up (not tested yet)
 *
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

/* traversal functions to infere loop invariant expressions */
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
extern node *SSALIRexprs (node *arg_node, node *arg_info);

/* traversal functions to move loop invariant expressions */
extern node *LIRMOVid (node *arg_node, node *arg_info);
extern node *LIRMOVNwithid (node *arg_node, node *arg_info);
extern node *LIRMOVblock (node *arg_node, node *arg_info);
extern node *LIRMOVassign (node *arg_node, node *arg_info);
extern node *LIRMOVlet (node *arg_node, node *arg_info);
extern node *LIRMOVreturn (node *arg_node, node *arg_info);
#endif /* SAC_SSALIR_H */
