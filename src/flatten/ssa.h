/*
 * $Log$
 * Revision 1.6  2005/06/06 13:21:00  jhb
 * removed SSATransformExplicitAllocs
 *
 * Revision 1.5  2004/11/24 19:16:24  mwe
 * changing function names according to naming conventions
 *
 * Revision 1.4  2004/11/22 12:37:33  ktr
 * Ismop SacDevCamp 04
 * ,.
 *
 * Revision 1.3  2004/09/18 15:58:34  ktr
 * added RestoreSSAExplicitAllocs
 *
 * Revision 1.2  2004/02/25 15:53:06  cg
 * New functions RestoreSSAOneFunction and RestoreSSAOneFundef
 * now provide access to SSA transformations on a per function
 * basis.
 * Only functions from ssa.[ch] should be used to initiate the
 * transformation process in either direction!
 *
 * Revision 1.1  2004/02/02 15:48:32  skt
 * Initial revision
 *
 */

#ifndef _SAC_SSA_H_
#define _SAC_SSA_H_

#include "types.h"

/******************************************************************************
 *
 * Tool package to create resp. to reverse the ssa-form
 *
 * Prefix: SSA
 *
 *****************************************************************************/
extern node *SSArestoreSsaOneFunction (node *fundef);
extern node *SSArestoreSsaOneFundef (node *fundef);

#endif /* _SAC_SSA_H_ */
