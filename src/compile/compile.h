/*
 *
 * $Log$
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

#ifndef _sac_compile_h
#define _sac_compile_h

extern node *GetFoldCode (node *fundef);

extern node *Compile (node *arg_node);

extern node *COMPModul (node *arg_node, info *arg_info);
extern node *COMPTypedef (node *arg_node, info *arg_info);
extern node *COMPObjdef (node *arg_node, info *arg_info);
extern node *COMPVardec (node *arg_node, info *arg_info);
extern node *COMPPrf (node *arg_node, info *arg_info);
extern node *COMPAssign (node *arg_node, info *arg_info);
extern node *COMPLet (node *arg_node, info *arg_info);
extern node *COMPScalar (node *arg_node, info *arg_info);
extern node *COMPArray (node *arg_node, info *arg_info);
extern node *COMPId (node *arg_node, info *arg_info);
extern node *COMPAp (node *arg_node, info *arg_info);
extern node *COMPReturn (node *arg_node, info *arg_info);
extern node *COMPFundef (node *arg_node, info *arg_info);
extern node *COMPLoop (node *arg_node, info *arg_info);
extern node *COMPCond (node *arg_node, info *arg_info);
extern node *COMPBlock (node *arg_node, info *arg_info);
extern node *COMPIcm (node *arg_node, info *arg_info);
extern node *COMPCast (node *arg_node, info *arg_info);
extern node *COMPSpmd (node *arg_node, info *arg_info);
extern node *COMPSync (node *arg_node, info *arg_info);

extern node *COMPWith (node *arg_node, info *arg_info);
extern node *COMPWith2 (node *arg_node, info *arg_info);
extern node *COMPWLsegx (node *arg_node, info *arg_info);
extern node *COMPWLxblock (node *arg_node, info *arg_info);
extern node *COMPWLstridex (node *arg_node, info *arg_info);
extern node *COMPWLgridx (node *arg_node, info *arg_info);
extern node *COMPWLcode (node *arg_node, info *arg_info);

extern node *COMPMt (node *arg_node, info *arg_info);
extern node *COMPSt (node *arg_node, info *arg_info);
extern node *COMPMTsignal (node *arg_node, info *arg_info);
extern node *COMPMTalloc (node *arg_node, info *arg_info);
extern node *COMPMTsync (node *arg_node, info *arg_info);

#endif /* _sac_compile_h */
