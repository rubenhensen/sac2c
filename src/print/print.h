/*
 *
 * $Log$
 * Revision 3.30  2004/11/24 17:18:31  sbs
 * SacDevCamp04
 *
 * Revision 3.29  2004/11/23 11:27:12  sbs
 * next
 *
 * Revision 3.28  2004/11/22 16:10:11  sbs
 * SACDevCamp04
 *
 * Revision 3.27  2004/10/15 15:01:43  sah
 * added support for new module system nodes
 *
 * Revision 3.26  2004/08/12 12:39:28  skt
 * PrintDataflowgraph & PrintDataflownode added
 *
 * Revision 3.25  2004/07/28 17:27:02  skt
 * PrintEX added
 *
 * Revision 3.24  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.23  2004/03/05 19:14:27  mwe
 * support for new node N_funcond added
 *
 * Revision 3.22  2003/10/19 21:39:45  dkrHH
 * prf_string moved from print.[ch] to globals.[ch] (for BEtest)
 *
 * Revision 3.21  2002/09/11 23:15:57  dkr
 * prf_name_string[] removed
 * prf_symbol[] added
 *
 * Revision 3.20  2002/09/09 19:25:42  dkr
 * prf_name_string added
 *
 * Revision 3.19  2002/09/06 10:35:43  sah
 * added PrintSetWL
 *
 * Revision 3.18  2002/08/09 16:36:21  sbs
 * basic support for N_mop written.
 *
 * Revision 3.17  2002/08/09 14:11:31  dkr
 * signature of PrintFunctionHeader modified
 *
 * Revision 3.16  2002/06/25 14:07:03  sbs
 * PrintDot added .
 *
 * Revision 3.15  2002/04/09 15:54:53  dkr
 * some comments added.
 * PrintArgtab() added.
 *
 * Revision 3.14  2002/04/08 19:59:01  dkr
 * PrintInfo() added
 *
 * Revision 3.12  2001/03/27 21:39:23  dkr
 * macro PRINT_VECT moved to internal_lib.h
 *
 * Revision 3.11  2001/03/22 19:26:25  dkr
 * include of tree.h eliminated
 *
 * Revision 3.8  2001/02/12 15:57:34  nmw
 * Print functions for N_cseinfo, N_ssacnt and N_avis added
 *
 * Revision 3.7  2001/02/06 01:21:35  dkr
 * macros INDENT_STR and DO_INDENT added
 *
 * Revision 3.6  2001/01/29 16:07:10  dkr
 * PrintWLstrideVar and PrintWLstride replaced by PrintWLstridex
 * PrintWLgridVar and PrintWLgrid replaced by PrintWLgridx
 *
 * Revision 3.5  2001/01/24 23:41:20  dkr
 * PrintWLseg and PrintWLsegVar replaced by PrintWLsegx
 * PrintWLblock and PrintWLublock replaced by PrintWLxblock
 *
 * Revision 3.4  2001/01/09 17:22:13  dkr
 * PrintWLstriVar renamed into PrintWLstrideVar
 *
 * Revision 3.3  2000/12/12 12:16:54  dkr
 * nodes N_pre, N_post, N_inc, N_dec removed
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
            fprintf (outfile, INDENT_STR);                                               \
        }                                                                                \
    }

#define INDENT DO_INDENT (indent)

/*
 * Functions for printing (parts of) the AST
 */
extern node *PRTdoPrint (node *syntax_tree);
extern node *PRTdoPrintNode (node *node);

/* debug output */
extern void PRTdoPrintAST (node *node);
extern void PRTdoPrintNodeAST (node *node);

/*
 * Functions for printing non-node parts of the AST
 */
extern void PRTprintArgtab (argtab_t *argtab, bool is_def);

/*
 * Other functions for external use
 */
extern void PRTprintFunctionHeader (node *arg_node, info *arg_info, bool in_comment);

/*
 * Functions for internal use during AST traversal only!
 */
extern node *PRTmodul (node *arg_node, info *arg_info);
extern node *PRTimplist (node *arg_node, info *arg_info);
extern node *PRTtypedef (node *arg_node, info *arg_info);
extern node *PRTobjdef (node *arg_node, info *arg_info);
extern node *PRTfundef (node *arg_node, info *arg_info);
extern node *PRTannotate (node *arg_node, info *arg_info);
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
