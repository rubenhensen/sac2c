/*
 *
 * $Log$
 * Revision 3.8  2001/05/08 12:30:38  dkr
 * macro IS_REFCOUNTED removed (use new macros RC_IS_ACTIVE, ... instead
 * !)
 *
 * Revision 3.5  2001/02/09 13:33:40  dkr
 * COMPIcm added
 *
 * Revision 3.4  2001/02/02 09:46:15  dkr
 * COMPWLstride and COMPWLstrideVar replaced by COMPWLstridex, ...
 *
 * Revision 3.3  2001/01/09 17:21:46  dkr
 * COMPWLstriVar renamed into COMPWLstrideVar
 *
 * Revision 3.2  2000/12/12 11:50:02  dkr
 * function declarations for Make...RcIcm() added
 *
 * Revision 3.1  2000/11/20 18:01:10  sacbase
 * new release made
 *
 * Revision 2.14  2000/10/17 13:03:30  dkr
 * macro COUNT_ELEMS renamed into EXPRS_LENGTH and moved to tree_compound.h
 * macro FUN_DOES_REFCOUNT moved from compile.c to compile.h
 *
 * Revision 2.13  2000/10/16 16:46:15  dkr
 * names of local vars in macros have a prefix '_' now
 *
 * Revision 2.12  2000/10/16 11:11:13  dkr
 * IS_REFCOUNTED moved from compile.c to compile.h
 *
 * Revision 2.11  2000/10/09 19:20:05  dkr
 * GetUnadjustedFoldCode() renamed into GetFoldCode()
 * GetAdjustedFoldCode() removed
 *
 * Revision 2.10  2000/07/31 10:45:52  cg
 * Eventually, the son ICM_NEXT is removed from the N_icm node.
 * The creation function MakeIcm is adjusted accordingly.
 *
 * Revision 2.9  2000/07/13 11:59:07  jhs
 * Splited ICM_INDENT into ICM_INDENT_BEFORE and ICM_INDENT_AFTER.
 *
 * Revision 2.8  2000/05/26 19:25:13  dkr
 * signature of GetAdjustedFoldCode() modified
 *
 * Revision 2.7  2000/05/25 23:04:20  dkr
 * Prototype for GetAdjustedFoldCode() added
 * GetFoldCode() renamed into GetUnadjustedFoldCode()
 *
 * Revision 2.6  2000/04/20 11:36:13  jhs
 * Added COMPMT(signal|alloc|sync)
 *
 * Revision 2.5  2000/04/18 14:00:48  jhs
 * Added COMPSt and COMPMt.
 *
 * [ eliminated ]
 *
 */

#ifndef _sac_compile_h
#define _sac_compile_h

extern node *MakeAdjustRcIcm (char *name, types *type, int rc, int num);
extern node *MakeIncRcIcm (char *name, types *type, int rc, int num);
extern node *MakeDecRcIcm (char *name, types *type, int rc, int num);

extern node *Compile (node *arg_node);

extern node *COMPModul (node *arg_node, node *arg_info);
extern node *COMPTypedef (node *arg_node, node *arg_info);
extern node *COMPObjdef (node *arg_node, node *arg_info);
extern node *COMPVardec (node *arg_node, node *arg_info);
extern node *COMPPrf (node *arg_node, node *arg_info);
extern node *COMPAssign (node *arg_node, node *arg_info);
extern node *COMPLet (node *arg_node, node *arg_info);
extern node *COMPArray (node *arg_node, node *arg_info);
extern node *COMPId (node *arg_node, node *arg_info);
extern node *COMPAp (node *arg_node, node *arg_info);
extern node *COMPReturn (node *arg_node, node *arg_info);
extern node *COMPArg (node *arg_node, node *arg_info);
extern node *COMPFundef (node *arg_node, node *arg_info);
extern node *COMPLoop (node *arg_node, node *arg_info);
extern node *COMPCond (node *arg_node, node *arg_info);
extern node *COMPBlock (node *arg_node, node *arg_info);
extern node *COMPIcm (node *arg_node, node *arg_info);
extern node *COMPCast (node *arg_node, node *arg_info);
extern node *COMPSpmd (node *arg_node, node *arg_info);
extern node *COMPSync (node *arg_node, node *arg_info);

extern node *COMPNwith2 (node *arg_node, node *arg_info);
extern node *COMPWLsegx (node *arg_node, node *arg_info);
extern node *COMPWLxblock (node *arg_node, node *arg_info);
extern node *COMPWLstridex (node *arg_node, node *arg_info);
extern node *COMPWLgridx (node *arg_node, node *arg_info);
extern node *COMPNcode (node *arg_node, node *arg_info);

extern node *COMPMt (node *arg_node, node *arg_info);
extern node *COMPSt (node *arg_node, node *arg_info);
extern node *COMPMTsignal (node *arg_node, node *arg_info);
extern node *COMPMTalloc (node *arg_node, node *arg_info);
extern node *COMPMTsync (node *arg_node, node *arg_info);

extern char *GenericFun (int which, types *type);
extern node *GetFoldCode (node *fundef);
extern node *GetFoldVardecs (node *fundef);

#endif /* _sac_compile_h */
