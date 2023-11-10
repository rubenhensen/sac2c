
#ifndef _SAC_VISUAL_H_
#define _SAC_VISUAL_H_
#include "types.h"

extern char *VISUALdoVisual (node *syntax_tree);

/*
 * Functions for internal use during AST traversal only!
 */
extern node *VISUALmodule (node *arg_node, info *arg_info);
extern node *VISUALimplist (node *arg_node, info *arg_info);
extern node *VISUALstructdef (node *arg_node, info *arg_info);
extern node *VISUALtypedef (node *arg_node, info *arg_info);
extern node *VISUALobjdef (node *arg_node, info *arg_info);
extern node *VISUALfundef (node *arg_node, info *arg_info);
extern node *VISUALannotate (node *arg_node, info *arg_info);
extern node *VISUALret (node *arg_node, info *arg_info);
extern node *VISUALarg (node *arg_node, info *arg_info);
extern node *VISUALstructelem (node *arg_node, info *arg_info);
extern node *VISUALvardec (node *arg_node, info *arg_info);
extern node *VISUALblock (node *arg_node, info *arg_info);
extern node *VISUALreturn (node *arg_node, info *arg_info);
extern node *VISUALassign (node *arg_node, info *arg_info);
extern node *VISUALdo (node *arg_node, info *arg_info);
extern node *VISUALwhile (node *arg_node, info *arg_info);
extern node *VISUALcond (node *arg_node, info *arg_info);
extern node *VISUALcast (node *arg_node, info *arg_info);
extern node *VISUALlet (node *arg_node, info *arg_info);
extern node *VISUALprf (node *arg_node, info *arg_info);
extern node *VISUALap (node *arg_node, info *arg_info);
extern node *VISUALspap (node *arg_node, info *arg_info);
extern node *VISUALspmop (node *arg_node, info *arg_info);
extern node *VISUALempty (node *arg_node, info *arg_info);
extern node *VISUALarray (node *arg_node, info *arg_info);
extern node *VISUALexprs (node *arg_node, info *arg_info);
extern node *VISUALid (node *arg_node, info *arg_info);
extern node *VISUALids (node *arg_node, info *arg_info);
extern node *VISUALspid (node *arg_node, info *arg_info);
extern node *VISUALspids (node *arg_node, info *arg_info);
extern node *VISUALtypepattern (node *arg_node, info *arg_info);
extern node *VISUALnum (node *arg_node, info *arg_info);
extern node *VISUALnumbyte (node *arg_node, info *arg_info);
extern node *VISUALnumshort (node *arg_node, info *arg_info);
extern node *VISUALnumint (node *arg_node, info *arg_info);
extern node *VISUALnumlong (node *arg_node, info *arg_info);
extern node *VISUALnumlonglong (node *arg_node, info *arg_info);
extern node *VISUALnumubyte (node *arg_node, info *arg_info);
extern node *VISUALnumushort (node *arg_node, info *arg_info);
extern node *VISUALnumuint (node *arg_node, info *arg_info);
extern node *VISUALnumulong (node *arg_node, info *arg_info);
extern node *VISUALnumulonglong (node *arg_node, info *arg_info);
extern node *VISUALfloat (node *arg_node, info *arg_info);
extern node *VISUALfloatvec (node *arg_node, info *arg_info);
extern node *VISUALdouble (node *arg_node, info *arg_info);
extern node *VISUALbool (node *arg_node, info *arg_info);
extern node *VISUALtype (node *arg_node, info *arg_info);
extern node *VISUALdot (node *arg_node, info *arg_info);
extern node *VISUALsetwl (node *arg_node, info *arg_info);
extern node *VISUALstr (node *arg_node, info *arg_info);
extern node *VISUALchar (node *arg_node, info *arg_info);
extern node *VISUALvectinfo (node *arg_node, info *arg_info);
extern node *VISUALicm (node *arg_node, info *arg_info);
extern node *VISUALpragma (node *arg_node, info *arg_info);
extern node *VISUALex (node *arg_node, info *arg_info);
extern node *VISUALmt (node *arg_node, info *arg_info);
extern node *VISUALst (node *arg_node, info *arg_info);
extern node *VISUALcudast (node *arg_node, info *arg_info);
extern node *VISUALssacnt (node *arg_node, info *arg_info);
extern node *VISUALavis (node *arg_node, info *arg_info);
extern node *VISUALconstraint (node *arg_node, info *arg_info);
extern node *VISUALfuncond (node *arg_node, info *arg_info);
extern node *VISUALdataflowgraph (node *arg_node, info *arg_info);
extern node *VISUALdataflownode (node *arg_node, info *arg_info);
extern node *VISUALglobobj (node *arg_node, info *arg_info);
extern node *VISUALerror (node *arg_node, info *arg_info);
extern node *VISUALlivevars (node *arg_node, info *arg_info);
extern node *VISUALnested_init (node *arg_node, info *arg_info);

/* with-loop (frontend) */
extern node *VISUALwith (node *arg_node, info *arg_info);
extern node *VISUALwithid (node *arg_node, info *arg_info);
extern node *VISUALcode (node *arg_node, info *arg_info);
extern node *VISUALpart (node *arg_node, info *arg_info);
extern node *VISUALgenerator (node *arg_node, info *arg_info);
extern node *VISUALdefault (node *arg_node, info *arg_info);
extern node *VISUALgenarray (node *arg_node, info *arg_info);
extern node *VISUALmodarray (node *arg_node, info *arg_info);
extern node *VISUALfold (node *arg_node, info *arg_info);
extern node *VISUALspfold (node *arg_node, info *arg_info);
extern node *VISUALbreak (node *arg_node, info *arg_info);
extern node *VISUALpropagate (node *arg_node, info *arg_info);

/* with-loop (backend) */
extern node *VISUALwith2 (node *arg_node, info *arg_info);
extern node *VISUALwlseg (node *arg_node, info *arg_info);
extern node *VISUALwlxblock (node *arg_node, info *arg_info);
extern node *VISUALwlstride (node *arg_node, info *arg_info);
extern node *VISUALwlgrid (node *arg_node, info *arg_info);
extern node *VISUALwlublock (node *arg_node, info *arg_info);
extern node *VISUALwlblock (node *arg_node, info *arg_info);
extern node *VISUALwiths (node *arg_node, info *arg_info);

/* with-loop (mutc) */
extern node *VISUALwith3 (node *arg_node, info *arg_info);
extern node *VISUALrange (node *arg_node, info *arg_info);

/* pre- and post-processing during traversal */
extern node *VISUALtravPre (node *arg_node, info *arg_info);
extern node *VISUALtravPost (node *arg_node, info *arg_info);

/* new module system */
extern node *VISUALimport (node *arg_node, info *arg_info);
extern node *VISUALexport (node *arg_node, info *arg_info);
extern node *VISUALuse (node *arg_node, info *arg_info);
extern node *VISUALprovide (node *arg_node, info *arg_info);
extern node *VISUALsymbol (node *arg_node, info *arg_info);
extern node *VISUALset (node *arg_node, info *arg_info);

/* sac4c */
extern node *VISUALfunbundle (node *arg_node, info *arg_info);

//#ifdef NTFSANTANU
extern node *VISUALtfdag (node *arg_node, info *arg_info);
extern node *VISUALtfspec (node *arg_node, info *arg_info);
extern node *VISUALtfcomponent (node *arg_node, info *arg_info);
extern node *VISUALtfvertex (node *arg_node, info *arg_info);
extern node *VISUALtfrel (node *arg_node, info *arg_info);
extern node *VISUALtfedge (node *arg_node, info *arg_info);
extern node *VISUALtypecomponentarg (node *arg_node, info *arg_info);
extern node *VISUALtfexpr (node *arg_node, info *arg_info);
//#endif

#endif /* _SAC_VISUALPH_H_ */
