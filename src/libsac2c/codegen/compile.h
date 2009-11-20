#ifndef _SAC_COMPILE_H_
#define _SAC_COMPILE_H_

#include "types.h"

extern node *COMPgetFoldCode (node *fundef);

/******************************************************************************
 *
 * $Id$
 *
 * Compilation traversal ( comp_tab)
 *
 * Prefix: COMP
 *
 *****************************************************************************/
extern node *COMPdoCompile (node *arg_node);

extern node *COMPap (node *arg_node, info *arg_info);
extern node *COMParray (node *arg_node, info *arg_info);
extern node *COMPassign (node *arg_node, info *arg_info);
extern node *COMPblock (node *arg_node, info *arg_info);
extern node *COMPcond (node *arg_node, info *arg_info);
extern node *COMPfundef (node *arg_node, info *arg_info);
extern node *COMPid (node *arg_node, info *arg_info);
extern node *COMPlet (node *arg_node, info *arg_info);
extern node *COMPdo (node *arg_node, info *arg_info);
extern node *COMPmodule (node *arg_node, info *arg_info);
extern node *COMPobjdef (node *arg_node, info *arg_info);
extern node *COMPprf (node *arg_node, info *arg_info);
extern node *COMPreturn (node *arg_node, info *arg_info);
extern node *COMPtypedef (node *arg_node, info *arg_info);
extern node *COMPvardec (node *arg_node, info *arg_info);

extern node *COMPnum (node *arg_node, info *arg_info);
extern node *COMPchar (node *arg_node, info *arg_info);
extern node *COMPbool (node *arg_node, info *arg_info);
extern node *COMPfloat (node *arg_node, info *arg_info);
extern node *COMPdouble (node *arg_node, info *arg_info);

extern node *COMPwith (node *arg_node, info *arg_info);
extern node *COMPpart (node *arg_node, info *arg_info);
extern node *COMPwithid (node *arg_node, info *arg_info);
extern node *COMPgenerator (node *arg_node, info *arg_info);
extern node *COMPwith2 (node *arg_node, info *arg_info);
extern node *COMPwlseg (node *arg_node, info *arg_info);
extern node *COMPwlblock (node *arg_node, info *arg_info);
extern node *COMPwlublock (node *arg_node, info *arg_info);
extern node *COMPwlstride (node *arg_node, info *arg_info);
extern node *COMPwlgrid (node *arg_node, info *arg_info);
extern node *COMPcode (node *arg_node, info *arg_info);

extern node *COMPwith3 (node *arg_node, info *arg_info);
extern node *COMPrange (node *arg_node, info *arg_info);

/******************************************************************************
 *
 * NODES WHICH MUST NOT BE TRAVERSED
 *
 * N_icm: Using TRAVsons, COMPid would be applied to ICM_ARG1.
 *
 *****************************************************************************/

/******************************************************************************
 *
 * NODES WHICH RESULT IN A TRAVERROR
 *
 * N_cast: Is already eliminated.
 * N_while: Is already eliminated.
 * N_funcond: Does only exist in SSA form.
 *
 *****************************************************************************/

#endif /* _SAC_COMPILE_H_ */
