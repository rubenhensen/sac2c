/*
 *
 * $Log$
 * Revision 3.19  2004/12/18 15:31:09  sbs
 * COMPpart, COMPwithid, COMPgenerator added.
 *
 * Revision 3.18  2004/11/29 10:27:41  sah
 * renamed some functions
 *
 * Revision 3.17  2004/11/26 18:14:40  ktr
 * added some comments.
 *
 * Revision 3.16  2004/11/26 17:35:58  ktr
 * COMPILES!!!
 *
 * Revision 3.15  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.14  2004/07/17 17:07:16  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.13  2004/03/09 23:52:27  dkrHH
 * file compile.tagged.h renamed into compile.h
 * old backend removed
 *
 * Revision 1.7  2003/09/25 10:53:33  dkr
 * GetFoldCode() added
 *
 * Revision 1.6  2002/08/07 13:49:46  dkr
 * COMPWith added
 *
 * Revision 1.5  2002/06/02 21:39:02  dkr
 * some more stuff for new backend added
 *
 * Revision 1.4  2002/04/03 14:42:56  dkr
 * COMPArg removed
 *
 * Revision 1.3  2001/12/13 11:49:08  dkr
 * some more functions removed
 *
 * Revision 1.2  2001/12/13 11:12:06  dkr
 * some functions removed
 *
 * Revision 1.1  2001/12/10 15:34:16  dkr
 * Initial revision
 *
 */

#ifndef _SAC_COMPILE_H_
#define _SAC_COMPILE_H_

#include "types.h"

extern node *COMPgetFoldCode (node *fundef);

/******************************************************************************
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
extern node *COMPwlsegvar (node *arg_node, info *arg_info);
extern node *COMPwlblock (node *arg_node, info *arg_info);
extern node *COMPwlublock (node *arg_node, info *arg_info);
extern node *COMPwlstride (node *arg_node, info *arg_info);
extern node *COMPwlstridevar (node *arg_node, info *arg_info);
extern node *COMPwlgrid (node *arg_node, info *arg_info);
extern node *COMPwlgridvar (node *arg_node, info *arg_info);
extern node *COMPcode (node *arg_node, info *arg_info);

extern node *COMPspmd (node *arg_node, info *arg_info);
extern node *COMPsync (node *arg_node, info *arg_info);

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
