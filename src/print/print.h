/*
 *
 * $Log$
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

#ifndef _sac_print_h
#define _sac_print_h

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "globals.h"

#define INDENT_STR "  "

#define DO_INDENT(cnt)                                                                   \
    {                                                                                    \
        int j;                                                                           \
        for (j = 0; j < cnt; j++) {                                                      \
            fprintf (outfile, INDENT_STR);                                               \
        }                                                                                \
    }

#define INDENT DO_INDENT (indent)

extern char *prf_string[];

/*
 * Functions for printing (parts of) the AST
 */
extern node *Print (node *syntax_tree);
extern node *PrintNode (node *node);

/* debug output */
extern void PrintAST (node *node);
extern void PrintNodeAST (node *node);

/*
 * Functions for printing non-node parts of the AST
 */
extern void PrintArgtab (argtab_t *argtab, bool is_def);

/*
 * Other functions for external use
 */
extern void PrintFunctionHeader (node *arg_node, node *arg_info, bool in_comment);

/*
 * Functions for internal use during AST traversal only!
 */
extern node *PrintModul (node *arg_node, node *arg_info);
extern node *PrintImplist (node *arg_node, node *arg_info);
extern node *PrintTypedef (node *arg_node, node *arg_info);
extern node *PrintObjdef (node *arg_node, node *arg_info);
extern node *PrintFundef (node *arg_node, node *arg_info);
extern node *PrintAnnotate (node *arg_node, node *arg_info);
extern node *PrintArg (node *arg_node, node *arg_info);
extern node *PrintVardec (node *arg_node, node *arg_info);
extern node *PrintBlock (node *arg_node, node *arg_info);
extern node *PrintReturn (node *arg_node, node *arg_info);
extern node *PrintAssign (node *arg_node, node *arg_info);
extern node *PrintDo (node *arg_node, node *arg_info);
extern node *PrintWhile (node *arg_node, node *arg_info);
extern node *PrintCond (node *arg_node, node *arg_info);
extern node *PrintCast (node *arg_node, node *arg_info);
extern node *PrintLet (node *arg_node, node *arg_info);
extern node *PrintPrf (node *arg_node, node *arg_info);
extern node *PrintAp (node *arg_node, node *arg_info);
extern node *PrintMop (node *arg_node, node *arg_info);
extern node *PrintEmpty (node *arg_node, node *arg_info);
extern node *PrintArray (node *arg_node, node *arg_info);
extern node *PrintExprs (node *arg_node, node *arg_info);
extern node *PrintId (node *arg_node, node *arg_info);
extern node *PrintNum (node *arg_node, node *arg_info);
extern node *PrintFloat (node *arg_node, node *arg_info);
extern node *PrintDouble (node *arg_node, node *arg_info);
extern node *PrintBool (node *arg_node, node *arg_info);
extern node *PrintDot (node *arg_node, node *arg_info);
extern node *PrintSetWL (node *arg_node, node *arg_info);
extern node *PrintStr (node *arg_node, node *arg_info);
extern node *PrintChar (node *arg_node, node *arg_info);
extern node *PrintVectInfo (node *arg_node, node *arg_info);
extern node *PrintIcm (node *arg_node, node *arg_info);
extern node *PrintPragma (node *arg_node, node *arg_info);
extern node *PrintSpmd (node *arg_node, node *arg_info);
extern node *PrintSync (node *arg_node, node *arg_info);
extern node *PrintMT (node *arg_node, node *arg_info);
extern node *PrintST (node *arg_node, node *arg_info);
extern node *PrintMTsignal (node *arg_node, node *arg_info);
extern node *PrintMTsync (node *arg_node, node *arg_info);
extern node *PrintMTalloc (node *arg_node, node *arg_info);
extern node *PrintSSAcnt (node *arg_node, node *arg_info);
extern node *PrintCSEinfo (node *arg_node, node *arg_info);
extern node *PrintAvis (node *arg_node, node *arg_info);
extern node *PrintInfo (node *arg_node, node *arg_info);
extern node *PrintCWrapper (node *arg_node, node *arg_info);

/* with-loop (frontend) */
extern node *PrintNwith (node *arg_node, node *arg_info);
extern node *PrintNwithid (node *arg_node, node *arg_info);
extern node *PrintNcode (node *arg_node, node *arg_info);
extern node *PrintNpart (node *arg_node, node *arg_info);
extern node *PrintNgenerator (node *arg_node, node *arg_info);
extern node *PrintNwithop (node *arg_node, node *arg_info);

/* with-loop (backend) */
extern node *PrintNwith2 (node *arg_node, node *arg_info);
extern node *PrintWLsegx (node *arg_node, node *arg_info);
extern node *PrintWLxblock (node *arg_node, node *arg_info);
extern node *PrintWLstridex (node *arg_node, node *arg_info);
extern node *PrintWLgridx (node *arg_node, node *arg_info);

/* pre- and post-processing during traversal */
extern node *PrintTravPre (node *arg_node, node *arg_info);
extern node *PrintTravPost (node *arg_node, node *arg_info);

#endif /* _sac_print_h */
