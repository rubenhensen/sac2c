#ifndef _SAC_PRINT_H_
#define _SAC_PRINT_H_

#include "types.h"

#define INDENT_STR "  "

#define DO_INDENT(cnt)                                                                   \
    {                                                                                    \
        size_t j;                                                                           \
        for (j = 0; j < cnt; j++) {                                                      \
            fprintf (global.outfile, INDENT_STR);                                        \
        }                                                                                \
    }

#define INDENT DO_INDENT (global.indent)

/*
 * Functions for printing (parts of) the AST
 */
extern node *PRTdoPrint (node *syntax_tree);
extern node *PRTdoPrintNode (node *node);

extern node *PRTdoPrintNodeFile (FILE *fd, node *arg_node);
extern node *PRTdoPrintFile (FILE *fd, node *arg_node);

/*
 * Functions for printing non-node parts of the AST
 */
extern void PRTprintArgtab (argtab_t *argtab, bool is_def);

/*
 * Other functions for external use
 */

extern void PRTprintHomsv (FILE *handle, int *vect, int dims);

/*
 * Functions for internal use during AST traversal only!
 */
extern node *PRTmodule (node *arg_node, info *arg_info);
extern node *PRTimplist (node *arg_node, info *arg_info);
extern node *PRTstructdef (node *arg_node, info *arg_info);
extern node *PRTtypedef (node *arg_node, info *arg_info);
extern node *PRTobjdef (node *arg_node, info *arg_info);
extern node *PRTfundef (node *arg_node, info *arg_info);
extern node *PRTannotate (node *arg_node, info *arg_info);
extern node *PRTret (node *arg_node, info *arg_info);
extern node *PRTarg (node *arg_node, info *arg_info);
extern node *PRTstructelem (node *arg_node, info *arg_info);
extern node *PRTvardec (node *arg_node, info *arg_info);
extern node *PRTblock (node *arg_node, info *arg_info);
extern node *PRTreturn (node *arg_node, info *arg_info);
extern node *PRTassign (node *arg_node, info *arg_info);
extern node *PRTdo (node *arg_node, info *arg_info);
extern node *PRTwhile (node *arg_node, info *arg_info);
extern node *PRTcond (node *arg_node, info *arg_info);
extern node *PRTcast (node *arg_node, info *arg_info);
extern node *PRTlet (node *arg_node, info *arg_info);
extern node *PRTprf (node *arg_node, info *arg_info);
extern node *PRTap (node *arg_node, info *arg_info);
extern node *PRTspap (node *arg_node, info *arg_info);
extern node *PRTspmop (node *arg_node, info *arg_info);
extern node *PRTarray (node *arg_node, info *arg_info);
extern node *PRTexprs (node *arg_node, info *arg_info);
extern node *PRTid (node *arg_node, info *arg_info);
extern node *PRTids (node *arg_node, info *arg_info);
extern node *PRTspid (node *arg_node, info *arg_info);
extern node *PRTspids (node *arg_node, info *arg_info);
extern node *PRTtypepattern (node *arg_node, info *arg_info);
extern node *PRTnum (node *arg_node, info *arg_info);
extern node *PRTnumbyte (node *arg_node, info *arg_info);
extern node *PRTnumshort (node *arg_node, info *arg_info);
extern node *PRTnumint (node *arg_node, info *arg_info);
extern node *PRTnumlong (node *arg_node, info *arg_info);
extern node *PRTnumlonglong (node *arg_node, info *arg_info);
extern node *PRTnumubyte (node *arg_node, info *arg_info);
extern node *PRTnumushort (node *arg_node, info *arg_info);
extern node *PRTnumuint (node *arg_node, info *arg_info);
extern node *PRTnumulong (node *arg_node, info *arg_info);
extern node *PRTnumulonglong (node *arg_node, info *arg_info);
extern node *PRTfloat (node *arg_node, info *arg_info);
extern node *PRTfloatvec (node *arg_node, info *arg_info);
extern node *PRTdouble (node *arg_node, info *arg_info);
extern node *PRTbool (node *arg_node, info *arg_info);
extern node *PRTnested_init (node *arg_node, info *arg_info);
extern node *PRTtype (node *arg_node, info *arg_info);
extern node *PRTdot (node *arg_node, info *arg_info);
extern node *PRTsetwl (node *arg_node, info *arg_info);
extern node *PRTstr (node *arg_node, info *arg_info);
extern node *PRTchar (node *arg_node, info *arg_info);
extern node *PRTvectinfo (node *arg_node, info *arg_info);
extern node *PRTicm (node *arg_node, info *arg_info);
extern node *PRTpragma (node *arg_node, info *arg_info);
extern node *PRTex (node *arg_node, info *arg_info);
extern node *PRTmt (node *arg_node, info *arg_info);
extern node *PRTst (node *arg_node, info *arg_info);
extern node *PRTcudast (node *arg_node, info *arg_info);
extern node *PRTssacnt (node *arg_node, info *arg_info);
extern node *PRTavis (node *arg_node, info *arg_info);
extern node *PRTconstraint (node *arg_node, info *arg_info);
extern node *PRTfuncond (node *arg_node, info *arg_info);
extern node *PRTdataflowgraph (node *arg_node, info *arg_info);
extern node *PRTdataflownode (node *arg_node, info *arg_info);
extern node *PRTglobobj (node *arg_node, info *arg_info);
extern node *PRTerror (node *arg_node, info *arg_info);
extern node *PRTlivevars (node *arg_node, info *arg_info);

/* with-loop (frontend) */
extern node *PRTwith (node *arg_node, info *arg_info);
extern node *PRTwithid (node *arg_node, info *arg_info);
extern node *PRTcode (node *arg_node, info *arg_info);
extern node *PRTpart (node *arg_node, info *arg_info);
extern node *PRTgenerator (node *arg_node, info *arg_info);
extern node *PRTdefault (node *arg_node, info *arg_info);
extern node *PRTgenarray (node *arg_node, info *arg_info);
extern node *PRTmodarray (node *arg_node, info *arg_info);
extern node *PRTfold (node *arg_node, info *arg_info);
extern node *PRTspfold (node *arg_node, info *arg_info);
extern node *PRTbreak (node *arg_node, info *arg_info);
extern node *PRTpropagate (node *arg_node, info *arg_info);

/* with-loop (backend) */
extern node *PRTwith2 (node *arg_node, info *arg_info);
extern node *PRTwlseg (node *arg_node, info *arg_info);
extern node *PRTwlxblock (node *arg_node, info *arg_info);
extern node *PRTwlstride (node *arg_node, info *arg_info);
extern node *PRTwlgrid (node *arg_node, info *arg_info);
extern node *PRTwlublock (node *arg_node, info *arg_info);
extern node *PRTwlblock (node *arg_node, info *arg_info);
extern node *PRTwiths (node *arg_node, info *arg_info);

/* with-loop (mutc) */
extern node *PRTwith3 (node *arg_node, info *arg_info);
extern node *PRTrange (node *arg_node, info *arg_info);

/* pre- and post-processing during traversal */
extern node *PRTtravPre (node *arg_node, info *arg_info);
extern node *PRTtravPost (node *arg_node, info *arg_info);

/* new module system */
extern node *PRTimport (node *arg_node, info *arg_info);
extern node *PRTexport (node *arg_node, info *arg_info);
extern node *PRTuse (node *arg_node, info *arg_info);
extern node *PRTprovide (node *arg_node, info *arg_info);
extern node *PRTsymbol (node *arg_node, info *arg_info);
extern node *PRTset (node *arg_node, info *arg_info);

/* sac4c */
extern node *PRTfunbundle (node *arg_node, info *arg_info);

//#ifdef NTFSANTANU
extern node *PRTtfspec (node *arg_node, info *arg_info);
extern node *PRTtfdag (node *arg_node, info *arg_info);
extern node *PRTtfcomponent (node *arg_node, info *arg_info);
extern node *PRTtfvertex (node *arg_node, info *arg_info);
extern node *PRTtfrel (node *arg_node, info *arg_info);
extern node *PRTtfabs (node *arg_node, info *arg_info);
extern node *PRTtfusr (node *arg_node, info *arg_info);
extern node *PRTtfbin (node *arg_node, info *arg_info);
extern node *PRTtfedge (node *arg_node, info *arg_info);
extern node *PRTtypecomponentarg (node *arg_node, info *arg_info);
extern node *PRTtfexpr (node *arg_node, info *arg_info);
//#endif

#endif /* _SAC_PRINT_H_ */
