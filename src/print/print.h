/*
 *
 * $Log$
 * Revision 3.34  2004/11/27 00:14:38  cg
 * New types are printed whenever available.
 *
 * Revision 3.33  2004/11/26 22:31:33  sbs
 * *** empty log message ***
 *
 * Revision 3.32  2004/11/25 22:17:50  khf
 * added PRTprintHomsv (moved from wltransform.h)
 *
 * Revision 3.31  2004/11/24 17:22:59  jhb
 * changed outfile and indent to global.
 *
 * [ eliminated ]
 *
 */

#ifndef _SAC_PRINT_H_
#define _SAC_PRINT_H_

#include "types.h"

#define INDENT_STR "  "

#define DO_INDENT(cnt)                                                                   \
    {                                                                                    \
        int j;                                                                           \
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
extern node *PRTtypedef (node *arg_node, info *arg_info);
extern node *PRTobjdef (node *arg_node, info *arg_info);
extern node *PRTfundef (node *arg_node, info *arg_info);
extern node *PRTannotate (node *arg_node, info *arg_info);
extern node *PRTret (node *arg_node, info *arg_info);
extern node *PRTarg (node *arg_node, info *arg_info);
extern node *PRTvardec (node *arg_node, info *arg_info);
extern node *PRTblock (node *arg_node, info *arg_info);
extern node *PRTreturn (node *arg_node, info *arg_info);
extern node *PRTassign (node *arg_node, info *arg_info);
extern node *PRTdo (node *arg_node, info *arg_info);
extern node *PRTwhile (node *arg_node, info *arg_info);
extern node *PRTcond (node *arg_node, info *arg_info);
extern node *PRTcast (node *arg_node, info *arg_info);
extern node *PRTlet (node *arg_node, info *arg_info);
extern node *PRTarf (node *arg_node, info *arg_info);
extern node *PRTap (node *arg_node, info *arg_info);
extern node *PRTmop (node *arg_node, info *arg_info);
extern node *PRTempty (node *arg_node, info *arg_info);
extern node *PRTarray (node *arg_node, info *arg_info);
extern node *PRTexprs (node *arg_node, info *arg_info);
extern node *PRTid (node *arg_node, info *arg_info);
extern node *PRTnum (node *arg_node, info *arg_info);
extern node *PRTfloat (node *arg_node, info *arg_info);
extern node *PRTdouble (node *arg_node, info *arg_info);
extern node *PRTbool (node *arg_node, info *arg_info);
extern node *PRTdot (node *arg_node, info *arg_info);
extern node *PRTsetwl (node *arg_node, info *arg_info);
extern node *PRTstr (node *arg_node, info *arg_info);
extern node *PRTchar (node *arg_node, info *arg_info);
extern node *PRTvectinfo (node *arg_node, info *arg_info);
extern node *PRTicm (node *arg_node, info *arg_info);
extern node *PRTpragma (node *arg_node, info *arg_info);
extern node *PRTspmd (node *arg_node, info *arg_info);
extern node *PRTsync (node *arg_node, info *arg_info);
extern node *PRTex (node *arg_node, info *arg_info);
extern node *PRTmt (node *arg_node, info *arg_info);
extern node *PRTst (node *arg_node, info *arg_info);
extern node *PRTssacnt (node *arg_node, info *arg_info);
extern node *PRTcseinfo (node *arg_node, info *arg_info);
extern node *PRTavis (node *arg_node, info *arg_info);
extern node *PRTinfo (node *arg_node, info *arg_info);
extern node *PRTcwrapper (node *arg_node, info *arg_info);
extern node *PRTfuncond (node *arg_node, info *arg_info);
extern node *PRTdataflowgraph (node *arg_node, info *arg_info);
extern node *PRTdataflownode (node *arg_node, info *arg_info);

/* with-loop (frontend) */
extern node *PRTwith (node *arg_node, info *arg_info);
extern node *PRTwithid (node *arg_node, info *arg_info);
extern node *PRTcode (node *arg_node, info *arg_info);
extern node *PRTpart (node *arg_node, info *arg_info);
extern node *PRTgenerator (node *arg_node, info *arg_info);
extern node *PRTgenarray (node *arg_node, info *arg_info);
extern node *PRTmodarray (node *arg_node, info *arg_info);
extern node *PRTfold (node *arg_node, info *arg_info);

/* with-loop (backend) */
extern node *PRTwith2 (node *arg_node, info *arg_info);
extern node *PRTwlsegx (node *arg_node, info *arg_info);
extern node *PRTwlxblock (node *arg_node, info *arg_info);
extern node *PRTwlstridex (node *arg_node, info *arg_info);
extern node *PRTwlgridx (node *arg_node, info *arg_info);

/* pre- and post-processing during traversal */
extern node *PRTtravPre (node *arg_node, info *arg_info);
extern node *PRTtravPost (node *arg_node, info *arg_info);

/* new module system */
extern node *PRTimport (node *arg_node, info *arg_info);
extern node *PRTexport (node *arg_node, info *arg_info);
extern node *PRTuse (node *arg_node, info *arg_info);
extern node *PRTprovide (node *arg_node, info *arg_info);
extern node *PRTsymbol (node *arg_node, info *arg_info);

#endif /* _SAC_PRINT_H_ */
